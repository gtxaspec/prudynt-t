
#include <atomic>
#include <thread>
#include <sys/file.h>
#include "Config.hpp"
#include "worker.hpp"
#include "Logger.hpp"

#define MODULE "WORKER"

#define OSDPoolSize 200000

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

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
EncoderSink *Worker::stream0_sink = new EncoderSink{"Stream0Sink", 0, false, nullptr};
EncoderSink *Worker::stream1_sink = new EncoderSink{"Stream1Sink", 1, false, nullptr};

Worker *Worker::createNew(
    std::shared_ptr<CFG> cfg)
{
    return new Worker(cfg);
}

int Worker::init()
{

    LOG_DEBUG("Worker::init()");

    int ret;

    impsystem = IMPSystem::createNew(&cfg->image, &cfg->sensor);

    if (cfg->stream0.enabled)
    {
        framesources[0] = IMPFramesource::createNew(&cfg->stream0, &cfg->sensor, 0);
        encoder[0] = IMPEncoder::createNew(&cfg->stream0, 0, 0);
        framesources[0]->enable();
    }

    if (cfg->stream1.enabled)
    {
        framesources[1] = IMPFramesource::createNew(&cfg->stream1, &cfg->sensor, 1);
        encoder[1] = IMPEncoder::createNew(&cfg->stream1, 1, 1);
        framesources[1]->enable();
    }

    if (cfg->stream2.enabled)
    {
        encoder[2] = IMPEncoder::createNew(&cfg->stream2, 2, 0);
    }

    ret = IMP_OSD_SetPoolSize(OSDPoolSize);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_OSD_SetPoolSize(" << OSDPoolSize << ")");

    if (cfg->motion.enabled)
    {

        LOG_DEBUG("Motion enabled");

        ret = motion.init(cfg);
        LOG_DEBUG_OR_ERROR(ret, "motion.init(cfg)");
    }

    return ret;

    return 0;
}

int Worker::deinit()
{

    int ret;
    for (int i = 2; i >= 0; i--)
    {
        LOG_DEBUG(i);
        if (framesources[i])
        {

            framesources[i]->disable();
        }
    }

    for (int i = 1; i >= 0; i--)
    {

        if (encoder[i])
        {

            encoder[i]->deinit();
        }
    }

    for (int i = 2; i >= 0; i--)
    {

        if (framesources[i])
        {

            delete framesources[i];
            framesources[i] = nullptr;
        }
    }

    for (int i = 1; i >= 0; i--)
    {

        if (encoder[i])
        {

            delete encoder[i];
            encoder[i] = nullptr;
        }
    }

    delete impsystem;
    impsystem = nullptr;

    return 0;
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
    Channel *channel = static_cast<Channel *>(arg);

    LOG_DEBUG("Start jpeg_grabber thread for stream " << channel->encChn);

    int ret;

    ret = IMP_Encoder_StartRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << channel->encChn << ")");
    if (ret != 0)
        return 0;

    channel->thread_signal.store(true);

    while (channel->thread_signal.load())
    {
        // LOG_DEBUG("IMP_Encoder_PollingStream 1 " << channel->encChn);
        if (IMP_Encoder_PollingStream(channel->encChn, 1000) == 0)
        {
            // LOG_DEBUG("IMP_Encoder_PollingStream 2 " << channel->encChn);

            IMPEncoderStream stream;
            if (IMP_Encoder_GetStream(channel->encChn, &stream, 0) == 0)
            {
                // LOG_DEBUG("IMP_Encoder_GetStream" << channel->encChn);
                //  Check for success
                const char *tempPath = "/tmp/snapshot.tmp";         // Temporary path
                const char *finalPath = channel->stream->jpeg_path; // Final path for the JPEG snapshot

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
    }

    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic 1 (" << channel->encChn << ")");

    ret = IMP_Encoder_StopRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << channel->encChn << ")");

    return 0;
}

void *Worker::stream_grabber(void *arg)
{
    Channel *channel = static_cast<Channel *>(arg);

    LOG_DEBUG("Start stream_grabber thread for stream " << channel->encChn);

    int ret, errorCount, signal;
    uint32_t bps, fps, lps, ms;
    int64_t last_nal_ts, nal_ts;
    struct timeval imp_time_base;

    gettimeofday(&imp_time_base, NULL);

    ret = IMP_Encoder_StartRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << channel->encChn << ")");
    if (ret != 0)
        return 0;

    channel->thread_signal.store(true);

    while (channel->thread_signal.load())
    {
        if (IMP_Encoder_PollingStream(channel->encChn, 1000) == 0)
        {
            IMPEncoderStream stream;
            if (IMP_Encoder_GetStream(channel->encChn, &stream, false) != 0)
            {
                LOG_ERROR("IMP_Encoder_GetStream(" << channel->encChn << ") failed");
                errorCount++;
                continue;
            }

            int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;
            if (nal_ts - last_nal_ts > 1.5 * (1000000 / channel->stream->fps))
            {
                // Silence for now until further tests / THINGINO
                // LOG_WARN("The encoder 0 dropped a frame. " << (nalTs - channel.lastNalTs) << ", " << (1.5 * (1000000 / cfg->stream0.fps)));
            }

            struct timeval encode_time;
            encode_time.tv_sec = nal_ts / 1000000;
            encode_time.tv_usec = nal_ts % 1000000;

            for (unsigned int i = 0; i < stream.packCount; ++i)
            {
                fps++;
                bps += stream.pack[i].length;

                pthread_mutex_lock(&channel->lock);
                if (channel->sink->data_available_callback != nullptr)
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
                    timeradd(&imp_time_base, &encode_time, &nalu.time);
                    nalu.duration = 0;
#if defined(PLATFORM_T31)
                    if (stream.pack[i].nalType.h264NalType == 5 || stream.pack[i].nalType.h264NalType == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
                    else if (stream.pack[i].nalType.h265NalType == 19 ||
                             stream.pack[i].nalType.h265NalType == 20 ||
                             stream.pack[i].nalType.h265NalType == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                    if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
#elif defined(PLATFORM_T30)
                    if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
                    else if (stream.pack[i].dataType.h265Type == 19 ||
                             stream.pack[i].dataType.h265Type == 20 ||
                             stream.pack[i].dataType.h265Type == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
#endif
                    // We use start+4 because the encoder inserts 4-byte MPEG
                    //'startcodes' at the beginning of each NAL. Live555 complains
                    nalu.data.insert(nalu.data.end(), start + 4, end);
                    if (!channel->sink->IDR)
                    {
#if defined(PLATFORM_T31)
                        if (stream.pack[i].nalType.h264NalType == 7 ||
                            stream.pack[i].nalType.h264NalType == 8 ||
                            stream.pack[i].nalType.h264NalType == 5)
                        {
                            channel->sink->IDR = true;
                        }
                        else if (stream.pack[i].nalType.h265NalType == 32)
                        {
                            channel->sink->IDR = true;
                        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                        if (stream.pack[i].dataType.h264Type == 7 ||
                            stream.pack[i].dataType.h264Type == 8 ||
                            stream.pack[i].dataType.h264Type == 5)
                        {
                            channel->sink->IDR = true;
                        }
#elif defined(PLATFORM_T30)
                        if (stream.pack[i].dataType.h264Type == 7 ||
                            stream.pack[i].dataType.h264Type == 8 ||
                            stream.pack[i].dataType.h264Type == 5)
                        {
                            channel->sink->IDR = true;
                        }
                        else if (stream.pack[i].dataType.h265Type == 32)
                        {
                            channel->sink->IDR = true;
                        }
#endif
                    }

                    if (channel->sink->IDR)
                    {
                        if (channel->sink->data_available_callback(nalu))
                        {
                            LOG_ERROR("stream encChn:" << channel->encChn << ", size:" << nalu.data.size()
                                                       << ", pC:" << stream.packCount << ", pS:" << nalu.data.size() << ", pN:"
                                                       << i << " clogged!");
                        }
                    }
                }
                pthread_mutex_unlock(&channel->lock);
            }

            IMP_Encoder_ReleaseStream(channel->encChn, &stream);
            last_nal_ts = nal_ts;

            ms = tDiffInMs(&channel->stream->osd.stats.ts);
            if (ms > 1000)
            {
                channel->stream->osd.stats.bps = ((bps * 8) * 1000 / ms) / 1000;
                bps = 0;
                channel->stream->osd.stats.fps = fps * 1000 / ms;
                fps = 0;
                gettimeofday(&channel->stream->osd.stats.ts, NULL);

                if (channel->updateOsd)
                {
                    channel->updateOsd();
                }
            }
        }
        else
        {
            LOG_DEBUG("IMP_Encoder_PollingStream(" << channel->encChn << ", 5000) timeout !");
        }
    }

    ret = IMP_Encoder_StopRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << channel->encChn << ")");

    return 0;
}

void Worker::run()
{
    LOG_DEBUG("Worker::run()");

    int ret, xx;
    int64_t last_high_nal_ts;
    int64_t last_low_nal_ts;

    // 256 = exit thread
    while ((cfg->worker_thread_signal.load() & 256) != 256)
    {

        LOG_DEBUG(cfg->worker_thread_signal.load());

        // 1 = init and start
        if (cfg->worker_thread_signal.load() & 1)
        {

            ret = init();
            LOG_DEBUG_OR_ERROR(ret, "init()");
            if (ret != 0)
                return;

            IMP_System_RebaseTimeStamp(0);

            if (cfg->stream2.enabled)
            {
                channels[2] = new Channel{2, &cfg->stream2};
                pthread_create(&worker_threads[2], nullptr, jpeg_grabber, channels[2]);
                // worker_threads[2] = std::thread(&Worker::jpeg_grabber, this, channels[2]);
            }

            if (cfg->stream1.enabled)
            {
                pthread_mutex_init(&sink_lock1, NULL);
                channels[1] = new Channel{1, &cfg->stream1, stream1_sink, sink_lock1};
                gettimeofday(&cfg->stream1.osd.stats.ts, NULL);
                if (encoder[1]->osd)
                {
                    channels[1]->updateOsd = std::bind(&OSD::update, encoder[1]->osd);
                }
                pthread_create(&worker_threads[1], nullptr, stream_grabber, channels[1]);
                // worker_threads[1] = std::thread(&Worker::stream_grabber, this, channels[1]);
            }

            if (cfg->stream0.enabled)
            {
                pthread_mutex_init(&sink_lock0, NULL);
                channels[0] = new Channel{0, &cfg->stream0, stream0_sink, sink_lock0};
                gettimeofday(&cfg->stream0.osd.stats.ts, NULL);
                if (encoder[0]->osd)
                {
                    channels[0]->updateOsd = std::bind(&OSD::update, encoder[0]->osd);
                }
                pthread_create(&worker_threads[0], nullptr, stream_grabber, channels[0]);
                // worker_threads[0] = std::thread(&Worker::stream_grabber, this, channels[0]);
            }

            cfg->rtsp_thread_signal = 0;
            cfg->worker_thread_signal.fetch_or(2);
        }

        LOG_DEBUG("PRE cfg->worker_thread_signal.wait(3);");

        cfg->worker_thread_signal.wait(3);

        LOG_DEBUG("POST cfg->worker_thread_signal.wait(3);");

        // 4 = Stop threads
        if (cfg->worker_thread_signal.load() & 4)
        {

            if (channels[0])
            {
                LOG_DEBUG("WAIT FOR THREAD EXIT " << channels[0]->encChn);
                channels[0]->thread_signal.store(false);
                /*
                if (worker_threads[0].joinable())
                {
                    LOG_DEBUG("THREAD JOIN 0");
                    worker_threads[0].join();
                }
                */
                LOG_DEBUG("THREAD EXIT 0");
                delete channels[0];
                channels[0] = nullptr;
            }

            if (channels[1])
            {
                LOG_DEBUG("WAIT FOR THREAD EXIT 1");
                channels[1]->thread_signal.store(false);
                /*
                if (worker_threads[1].joinable())
                {
                    LOG_DEBUG("THREAD JOIN 1");
                    worker_threads[1].join();
                }
                */
                LOG_DEBUG("THREAD EXIT 1");
                delete channels[1];
                channels[1] = nullptr;
            }

            if (channels[2])
            {
                LOG_DEBUG("WAIT FOR THREAD EXIT 2");
                channels[2]->thread_signal.store(false);
                /*
                if (worker_threads[2].joinable())
                {
                    LOG_DEBUG("THREAD JOIN 0");
                    worker_threads[2].join();
                }
                */
                LOG_DEBUG("THREAD EXIT 2");
                delete channels[2];
                channels[2] = nullptr;
            }

            deinit();
        }

        cfg->worker_thread_signal.fetch_xor(4);
        cfg->worker_thread_signal.fetch_or(8);

        usleep(1000);
        LOG_DEBUG("LOOP");
    }
}