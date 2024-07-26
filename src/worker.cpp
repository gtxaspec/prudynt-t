#include <atomic>
#include <thread>
#include <sys/file.h>
#include "Config.hpp"
#include "worker.hpp"
#include "Logger.hpp"
#include "globals.hpp"

#define MODULE "WORKER"

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

extern std::shared_ptr<CFG> cfg;

unsigned long long tDiffInMs(struct timeval *startTime)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long seconds = currentTime.tv_sec - startTime->tv_sec;
    long microseconds = currentTime.tv_usec - startTime->tv_usec;

    unsigned long long milliseconds = (seconds * 1000) + (microseconds / 1000);

    return milliseconds;
}

pthread_mutex_t Worker::sink_lock0;
pthread_mutex_t Worker::sink_lock1;
pthread_mutex_t Worker::sink_lock2;
EncoderSink *Worker::stream0_sink = new EncoderSink{"Stream0Sink", 0, false, nullptr};
EncoderSink *Worker::stream1_sink = new EncoderSink{"Stream1Sink", 1, false, nullptr};
AudioSink *Worker::audio_sink = new AudioSink{"AudioSink", nullptr};

Worker *Worker::createNew()
{
    return new Worker();
}

int Worker::init()
{
    LOG_DEBUG("Worker::init()");
    int ret = 0;

    if (cfg->stream0.enabled)
    {
        if (!framesources[0])
        {
            framesources[0] = IMPFramesource::createNew(&cfg->stream0, &cfg->sensor, 0);
        }
        encoder[0] = IMPEncoder::createNew(&cfg->stream0, 0, 0, "stream0");
        framesources[0]->enable();
    }

    if (cfg->stream1.enabled)
    {
        if (!framesources[1])
        {
            framesources[1] = IMPFramesource::createNew(&cfg->stream1, &cfg->sensor, 1);
        }
        encoder[1] = IMPEncoder::createNew(&cfg->stream1, 1, 1, "stream1");
        framesources[1]->enable();
    }

    if (cfg->stream2.enabled)
    {
        encoder[2] = IMPEncoder::createNew(&cfg->stream2, 2, cfg->stream2.jpeg_channel, "stream2");
    }

    if (cfg->motion.enabled)
    {
        LOG_DEBUG("Motion enabled");
        ret = motion.init(cfg);
        LOG_DEBUG_OR_ERROR(ret, "motion.init(cfg)");
    }

    return ret;
}

int Worker::deinit()
{
    int ret;

    if (framesources[1])
    {
        framesources[1]->disable();

        if (encoder[1])
        {
            encoder[1]->deinit();
        }

        // delete framesources[1];
        // framesources[1] = nullptr;

        delete encoder[1];
        encoder[1] = nullptr;
    }

    if (framesources[0])
    {
        framesources[0]->disable();

        if (encoder[2])
        {
            encoder[2]->deinit();
        }

        if (encoder[0])
        {
            encoder[0]->deinit();
        }

        // delete framesources[0];
        // framesources[0] = nullptr;

        delete encoder[2];
        encoder[2] = nullptr;

        delete encoder[0];
        encoder[0] = nullptr;
    }

    // delete impsystem;
    // impsystem = nullptr;

    return 0;
}

std::vector<uint8_t> Worker::capture_jpeg_image(int encChn)
{
    std::vector<uint8_t> jpeg_data;
    int ret = 0;

    ret = IMP_Encoder_StartRecvPic(encChn);
    if (ret != 0)
    {
        std::cerr << "IMP_Encoder_StartRecvPic(" << encChn << ") failed: " << strerror(errno) << std::endl;
        return jpeg_data;
    }

    if (IMP_Encoder_PollingStream(encChn, 1000) == 0)
    {

        IMPEncoderStream stream;
        if (IMP_Encoder_GetStream(encChn, &stream, GET_STREAM_BLOCKING) == 0)
        {
            int nr_pack = stream.packCount;

            for (int i = 0; i < nr_pack; i++)
            {
                void *data_ptr;
                size_t data_len;

#if defined(PLATFORM_T31)
                IMPEncoderPack *pack = &stream.pack[i];
                uint32_t remSize = 0; // Declare remSize here
                if (pack->length)
                {
                    remSize = stream.streamSize - pack->offset;
                    data_ptr = (void *)((char *)stream.virAddr + ((remSize < pack->length) ? 0 : pack->offset));
                    data_len = (remSize < pack->length) ? remSize : pack->length;
                }
                else
                {
                    continue; // Skip empty packs
                }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                data_ptr = reinterpret_cast<void *>(stream.pack[i].virAddr);
                data_len = stream.pack[i].length;
#endif

                // Write data to vector
                jpeg_data.insert(jpeg_data.end(), (uint8_t *)data_ptr, (uint8_t *)data_ptr + data_len);

#if defined(PLATFORM_T31)
                // Check the condition only under T31 platform, as remSize is used here
                if (remSize && pack->length > remSize)
                {
                    data_ptr = (void *)((char *)stream.virAddr);
                    data_len = pack->length - remSize;
                    jpeg_data.insert(jpeg_data.end(), (uint8_t *)data_ptr, (uint8_t *)data_ptr + data_len);
                }
#endif
            }

            IMP_Encoder_ReleaseStream(encChn, &stream); // Release stream after saving
        }
    }

    // ret = IMP_Encoder_StopRecvPic(encChn);
    if (ret != 0)
    {
        std::cerr << "IMP_Encoder_StopRecvPic(" << encChn << ") failed: " << strerror(errno) << std::endl;
    }

    return jpeg_data;
}

static int save_jpeg_stream(int fd, IMPEncoderStream *stream)
{
    int ret, i, nr_pack = stream->packCount;

    for (i = 0; i < nr_pack; i++)
    {
        void *data_ptr;
        size_t data_len;

#if defined(PLATFORM_T31)
        IMPEncoderPack *pack = &stream->pack[i];
        uint32_t remSize = 0; // Declare remSize here
        if (pack->length)
        {
            remSize = stream->streamSize - pack->offset;
            data_ptr = (void *)((char *)stream->virAddr + ((remSize < pack->length) ? 0 : pack->offset));
            data_len = (remSize < pack->length) ? remSize : pack->length;
        }
        else
        {
            continue; // Skip empty packs
        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
        data_ptr = reinterpret_cast<void *>(stream->pack[i].virAddr);
        data_len = stream->pack[i].length;
#endif

        // Write data to file
        ret = write(fd, data_ptr, data_len);
        if (ret != static_cast<int>(data_len))
        {
            printf("Stream write error: %s\n", strerror(errno));
            return -1; // Return error on write failure
        }

#if defined(PLATFORM_T31)
        // Check the condition only under T31 platform, as remSize is used here
        if (remSize && pack->length > remSize)
        {
            ret = write(fd, (void *)((char *)stream->virAddr), pack->length - remSize);
            if (ret != static_cast<int>(pack->length - remSize))
            {
                printf("Stream write error (remaining part): %s\n", strerror(errno));
                return -1;
            }
        }
#endif
    }

    return 0;
}

void *Worker::jpeg_grabber(void *arg)
{
    LOG_DEBUG("Start jpeg_grabber thread.");

    int ret;
    struct timeval ts;
    gettimeofday(&ts, NULL);

    global_jpeg->imp_encoder = IMPEncoder::createNew(global_jpeg->stream, 2, global_jpeg->stream->jpeg_channel, "stream2");

    //inform main that initialization is complete 
    global_cv_lock_main.notify_one();

    ret = IMP_Encoder_StartRecvPic(global_jpeg->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << global_jpeg->encChn << ")");
    if (ret != 0)
        return 0;

    global_jpeg->running = true;    
    while (global_jpeg->running)
    {
        if (tDiffInMs(&ts) > 1000)
        {

            if (IMP_Encoder_PollingStream(global_jpeg->encChn, cfg->general.imp_polling_timeout) == 0)
            {

                IMPEncoderStream stream;
                if (IMP_Encoder_GetStream(global_jpeg->encChn, &stream, GET_STREAM_BLOCKING) == 0)
                {

                    //  Check for success
                    const char *tempPath = "/tmp/snapshot.tmp"; // Temporary path
                    const char *finalPath = global_jpeg->stream->jpeg_path; // Final path for the JPEG snapshot

                    // Open and create temporary file with read and write permissions
                    int snap_fd = open(tempPath, O_RDWR | O_CREAT | O_TRUNC, 0777);
                    if (snap_fd >= 0)
                    {
                        // Attempt to lock the temporary file for exclusive access
                        if (flock(snap_fd, LOCK_EX) == -1)
                        {
                            LOG_ERROR("Failed to lock JPEG snapshot for writing: " << tempPath);
                            close(snap_fd);
                            return 0; // Exit the function if unable to lock the file
                        }

                        // Save the JPEG stream to the file
                        save_jpeg_stream(snap_fd, &stream);

                        // Unlock and close the temporary file after writing is done
                        flock(snap_fd, LOCK_UN);
                        close(snap_fd);

                        // Atomically move the temporary file to the final destination
                        if (rename(tempPath, finalPath) != 0)
                        {
                            LOG_ERROR("Failed to move JPEG snapshot from " << tempPath << " to " << finalPath);
                            std::remove(tempPath); // Attempt to remove the temporary file if rename fails
                        }
                        else
                        {
                            // LOG_DEBUG("JPEG snapshot successfully updated");
                        }
                    }
                    else
                    {
                        LOG_ERROR("Failed to open JPEG snapshot for writing: " << tempPath);
                    }

                    IMP_Encoder_ReleaseStream(2, &stream); // Release stream after saving
                }
            }

            gettimeofday(&ts, NULL);
        }
        usleep(THREAD_SLEEP);
    }

    if (global_jpeg->imp_encoder)
    {
        global_jpeg->imp_encoder->deinit();

        delete global_jpeg->imp_encoder;
        global_jpeg->imp_encoder = nullptr;         
    }

    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << global_jpeg->encChn << ")");

    return 0;
}

void *Worker::stream_grabber(void *arg)
{
    //Channel *channel = static_cast<Channel *>(arg);
    int encChn = *static_cast<int *>(arg);

    LOG_DEBUG("Start stream_grabber thread for stream " << encChn);

    int ret;
    int flags{0};
    uint32_t bps;
    uint32_t fps;
    int64_t nal_ts;
    uint32_t error_count;
    unsigned long long ms;
    struct timeval imp_time_base;

    video[encChn]->imp_framesource = IMPFramesource::createNew(video[encChn]->stream, &cfg->sensor, encChn);
    video[encChn]->imp_encoder = IMPEncoder::createNew(video[encChn]->stream, encChn, encChn, video[encChn]->name);
    video[encChn]->imp_framesource->enable();   

    gettimeofday(&imp_time_base, NULL);
    IMP_System_RebaseTimeStamp(imp_time_base.tv_sec * (uint64_t)1000000);

    //inform main that initialization is complete 
    global_cv_lock_main.notify_one();

    ret = IMP_Encoder_StartRecvPic(encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << encChn << ")");
    if (ret != 0)
        return 0;

    video[encChn]->running = true;   
    while (video[encChn]->running)
    {
        if (video[encChn]->onDataCallback != nullptr)
        {
            if (IMP_Encoder_PollingStream(encChn, cfg->general.imp_polling_timeout) == 0)
            {
                IMPEncoderStream stream;
                if (IMP_Encoder_GetStream(encChn, &stream, GET_STREAM_BLOCKING) != 0)
                {
                    LOG_ERROR("IMP_Encoder_GetStream(" << encChn << ") failed");
                    error_count++;
                    continue;
                }

                int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;

                struct timeval encoder_time;
                encoder_time.tv_sec = nal_ts / 1000000;
                encoder_time.tv_usec = nal_ts % 1000000;

                for (uint32_t i = 0; i < stream.packCount; ++i)
                {
                    fps++;
                    bps += stream.pack[i].length;

                    if (video[encChn]->onDataCallback != nullptr)
                    {
#if defined(PLATFORM_T31)
                        uint8_t *start = (uint8_t *)stream.virAddr + stream.pack[i].offset;
                        uint8_t *end = start + stream.pack[i].length;
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                        uint8_t *start = (uint8_t *)stream.pack[i].virAddr;
                        uint8_t *end = (uint8_t *)stream.pack[i].virAddr + stream.pack[i].length;
#endif
                        H264NALUnit nalu;

                        nalu.imp_ts = stream.pack[i].timestamp;
                        nalu.time = encoder_time;

                        // We use start+4 because the encoder inserts 4-byte MPEG
                        //'startcodes' at the beginning of each NAL. Live555 complains
                        nalu.data.insert(nalu.data.end(), start + 4, end);
                        if (video[encChn]->idr == false)
                        {
#if defined(PLATFORM_T31)
                            if (stream.pack[i].nalType.h264NalType == 7 ||
                                stream.pack[i].nalType.h264NalType == 8 ||
                                stream.pack[i].nalType.h264NalType == 5)
                            {
                                video[encChn]->idr = true;
                            }
                            else if (stream.pack[i].nalType.h265NalType == 32)
                            {
                                video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                            if (stream.pack[i].dataType.h264Type == 7 ||
                                stream.pack[i].dataType.h264Type == 8 ||
                                stream.pack[i].dataType.h264Type == 5)
                            {
                                video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T30)
                            if (stream.pack[i].dataType.h264Type == 7 ||
                                stream.pack[i].dataType.h264Type == 8 ||
                                stream.pack[i].dataType.h264Type == 5)
                            {
                                video[encChn]->idr = true;
                            }
                            else if (stream.pack[i].dataType.h265Type == 32)
                            {
                                video[encChn]->IDR = true;
                            }
#endif
                        }

                        if (video[encChn]->idr == true)
                        {
                            if (!video[encChn]->msgChannel->write(nalu)) {
                                LOG_ERROR("stream encChn:" << encChn << ", size:" << nalu.data.size()
                                                           << ", pC:" << stream.packCount << ", pS:" << nalu.data.size() << ", pN:"
                                                           << i << " clogged!");                                
                            } else {
                                if(video[encChn]->onDataCallback)
                                    video[encChn]->onDataCallback();
                            }
                        }
                    }
                }

                IMP_Encoder_ReleaseStream(encChn, &stream);

                ms = tDiffInMs(&video[encChn]->stream->osd.stats.ts);
                if (ms > 1000)
                {
                    video[encChn]->stream->osd.stats.bps = ((bps * 8) * 1000 / ms) / 1000;
                    bps = 0;
                    video[encChn]->stream->osd.stats.fps = fps * 1000 / ms;
                    fps = 0;
                    gettimeofday(&video[encChn]->stream->osd.stats.ts, NULL);

                    /*
                    IMPEncoderCHNStat encChnStats;
                    IMP_Encoder_Query(channel->encChn, &encChnStats);
                    LOG_DEBUG("ChannelStats::" << channel->encChn <<
                                ", registered:" << encChnStats.registered <<
                                ", leftPics:" << encChnStats.leftPics <<
                                ", leftStreamBytes:" << encChnStats.leftStreamBytes <<
                                ", leftStreamFrames:" << encChnStats.leftStreamFrames <<
                                ", curPacks:" << encChnStats.curPacks <<
                                ", work_done:" << encChnStats.work_done);
                    */
                }
            }
            else
            {
                error_count++;
                LOG_DDEBUG("IMP_Encoder_PollingStream(" << encChn << ", " << cfg->general.imp_polling_timeout << ") timeout !");
                usleep(THREAD_SLEEP);
            }
        }
        else
        {          
            video[encChn]->stream->osd.stats.bps = 0;
            video[encChn]->stream->osd.stats.fps = 1;
            usleep(THREAD_SLEEP);
        }
    }

    ret = IMP_Encoder_StopRecvPic(encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << encChn << ")");

    if (video[encChn]->imp_framesource)
    {
        video[encChn]->imp_framesource->disable();

        if (video[encChn]->imp_encoder)
        {
            video[encChn]->imp_encoder->deinit();
            delete video[encChn]->imp_encoder;
            video[encChn]->imp_encoder = nullptr;            
        }
    }

    return 0;
}

void *Worker::audio_grabber(void *arg)
{
    AudioChannel *channel = static_cast<AudioChannel *>(arg);

    LOG_DEBUG("Start audio_grabber thread for device " << channel->devId << " and channel " << channel->inChn);

    while(true) {
            
        if (IMP_AI_PollingFrame(channel->devId, channel->inChn, 1000) == 0)
        {
            IMPAudioFrame frame;
            if (IMP_AI_GetFrame(channel->devId, channel->inChn, &frame, IMPBlock::BLOCK) != 0)
            {
                LOG_ERROR("IMP_AI_GetFrame(" << channel->devId << ", " << channel->inChn << ") failed");
            }

            if (channel->sink->data_available_callback != nullptr)
            {

                int64_t audio_ts = frame.timeStamp;
                struct timeval encoder_time;
                encoder_time.tv_sec = audio_ts / 1000000;
                encoder_time.tv_usec = audio_ts % 1000000;

                AudioFrame af;
                af.time = encoder_time;

                uint8_t *start = (uint8_t *)frame.virAddr;
                uint8_t *end = start + frame.len;
                LOG_DEBUG("insert");
                af.data.insert(af.data.end(), start, end);
                LOG_DEBUG("callback");
                if (channel->sink->data_available_callback(af))
                {
                    LOG_ERROR("stream audio:" << channel->devId << ", size:" << frame.len << " clogged!");
                }
                LOG_DEBUG("release");
                if(IMP_AI_ReleaseFrame(channel->devId, channel->inChn, &frame) < 0) {
                    LOG_ERROR("IMP_AI_ReleaseFrame(" << channel->devId << ", " << channel->inChn << ", &frame) failed");
                }

                LOG_DEBUG(channel->devId << ", " << channel->inChn << " == " << frame.len);
            }
        } else {

            LOG_DEBUG(channel->devId << ", " << channel->inChn << " POLLING TIMEOUT");
        }

        usleep(1000 * 1000);
    }

    return 0;
}

void *Worker::update_osd(void *arg) {

    LOG_DEBUG("start osd update thread.");
    
    global_osd_thread_signal = true;

    while(global_osd_thread_signal) {
        for(auto v : video) {
            if(v != nullptr) {
                if(v->running) {
                    if((v->imp_encoder->osd != nullptr)) {
                        v->imp_encoder->osd->updateDisplayEverySecond();
                    }
                }
            }
        }
        usleep(THREAD_SLEEP*2);
    }

    LOG_DEBUG("exit osd update thread.");
    return 0;
}