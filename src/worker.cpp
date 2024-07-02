
#include <atomic>
#include <thread>
#include <sys/file.h>
#include "Config.hpp"
#include "worker.hpp"
#include "Logger.hpp"

#define MODULE "WORKER"

#define STREAM_POLLING_TIMEOUT 1000
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
    if(!impsystem) {
        impsystem = IMPSystem::createNew(cfg);
    }

    if (cfg->stream0.enabled)
    {
        if(!framesources[0]) {
            framesources[0] = IMPFramesource::createNew(&cfg->stream0, &cfg->sensor, 0);
        }
        encoder[0] = IMPEncoder::createNew(&cfg->stream0, cfg, 0, 0, "stream0");
        framesources[0]->enable();
    }

    if (cfg->stream1.enabled)
    {
        if(!framesources[1]) {
            framesources[1] = IMPFramesource::createNew(&cfg->stream1, &cfg->sensor, 1);
        }
        encoder[1] = IMPEncoder::createNew(&cfg->stream1, cfg, 1, 1, "stream1");
        framesources[1]->enable();
    }

    if (cfg->stream2.enabled)
    {
        encoder[2] = IMPEncoder::createNew(&cfg->stream2, cfg, 2, cfg->stream2.jpeg_channel, "stream2");
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

        //delete framesources[1];
        //framesources[1] = nullptr;

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

        //delete framesources[0];
        //framesources[0] = nullptr;

        delete encoder[2];
        encoder[2] = nullptr;

        delete encoder[0];
        encoder[0] = nullptr;
    }

    //delete impsystem;
    //impsystem = nullptr;

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
    struct timeval ts;
    gettimeofday(&ts, NULL);

    ret = IMP_Encoder_StartRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << channel->encChn << ")");
    if (ret != 0)
        return 0;

    channel->thread_signal.store(true);

    while (channel->thread_signal.load())
    {
        if(tDiffInMs(&ts) > 1000) {

            // LOG_DEBUG("IMP_Encoder_PollingStream 1 " << channel->encChn);
            if (IMP_Encoder_PollingStream(channel->encChn, STREAM_POLLING_TIMEOUT) == 0)
            {
                // LOG_DEBUG("IMP_Encoder_PollingStream 2 " << channel->encChn);

                IMPEncoderStream stream;
                if (IMP_Encoder_GetStream(channel->encChn, &stream, true) == 0)
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
                            //LOG_DEBUG("JPEG snapshot successfully updated");
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
        usleep(1000);
    }

    ret = IMP_Encoder_StopRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << channel->encChn << ")");

    return 0;
}

void *Worker::stream_grabber(void *arg)
{
    Channel *channel = static_cast<Channel *>(arg);

    nice(-19);

    LOG_DEBUG("Start stream_grabber thread for stream " << channel->encChn);

    int ret, errorCount, signal;
    unsigned long long ms;
    uint32_t bps;
    uint32_t fps;
    int64_t last_nal_ts;
    int64_t nal_ts;
    struct timeval imp_time_base;

    gettimeofday(&imp_time_base, NULL);

    ret = IMP_Encoder_StartRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << channel->encChn << ")");
    if (ret != 0)
        return 0;

    channel->thread_signal.store(true);

    while (channel->thread_signal.load())
    {
        if (IMP_Encoder_PollingStream(channel->encChn, STREAM_POLLING_TIMEOUT) == 0)
        {
            IMPEncoderStream stream;
            if (IMP_Encoder_GetStream(channel->encChn, &stream, true) != 0)
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

            for (uint32_t i = 0; i < stream.packCount; ++i)
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
                    if (channel->sink->IDR == false)
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

                    if (channel->sink->IDR == true)
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

                /*if (channel->updateOsd)
                {
                    channel->updateOsd();
                }*/
            }
        }
        else
        {
            LOG_DEBUG("IMP_Encoder_PollingStream(" << channel->encChn << ", 1500) timeout !");
        }
    }

    ret = IMP_Encoder_StopRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << channel->encChn << ")");

    return 0;
}

void Worker::run()
{
    LOG_DEBUG("Worker::run()");

    int ret;

    // 256 = exit thread
    while ((cfg->worker_thread_signal.load() & 256) != 256)
    {
        // 1 = init and start
        if (cfg->worker_thread_signal.load() & 1)
        {
            ret = init();
            LOG_DEBUG_OR_ERROR(ret, "init()");
            if (ret != 0)
                return;

            IMP_System_RebaseTimeStamp(0);

            int policy = SCHED_RR;

            pthread_attr_init(&osd_thread_attr);
            pthread_attr_init(&stream_thread_attr);

            pthread_attr_setschedpolicy(&osd_thread_attr, policy);
            pthread_attr_setschedpolicy(&stream_thread_attr, policy);

            int max_priority = sched_get_priority_max(policy);
            int min_priority = sched_get_priority_min(policy);

            LOG_DEBUG(max_priority);
            LOG_DEBUG(min_priority);

            osd_thread_sheduler.sched_priority = (int)max_priority * 0.1;
            stream_thread_sheduler.sched_priority = (int)max_priority * 0.9;

            pthread_attr_setschedparam(&osd_thread_attr, &osd_thread_sheduler);
            pthread_attr_setschedparam(&stream_thread_attr, &stream_thread_sheduler);

            if (cfg->stream2.enabled)
            {
                channels[2] = new Channel{2, &cfg->stream2};
                pthread_create(&worker_threads[2], nullptr, jpeg_grabber, channels[2]);
            }

            if (cfg->stream1.enabled)
            {
                pthread_mutex_init(&sink_lock1, NULL);
                channels[1] = new Channel{1, &cfg->stream1, stream1_sink, sink_lock1};
                gettimeofday(&cfg->stream1.osd.stats.ts, NULL);
                if (encoder[1]->osd)
                {
                    cfg->stream1.osd.thread_signal.store(true);
                    pthread_create(&stream1_osd_thread, &osd_thread_attr, OSD::updateWrapper, encoder[1]->osd);
                }
                pthread_create(&worker_threads[1], nullptr, stream_grabber, channels[1]);
            }

            if (cfg->stream0.enabled)
            {
                pthread_mutex_init(&sink_lock0, NULL);
                channels[0] = new Channel{0, &cfg->stream0, stream0_sink, sink_lock0};
                gettimeofday(&cfg->stream0.osd.stats.ts, NULL);
                if (encoder[0]->osd)
                {
                    cfg->stream0.osd.thread_signal.store(true);
                    pthread_create(&stream0_osd_thread, &osd_thread_attr, OSD::updateWrapper, encoder[0]->osd);
                }
                pthread_create(&worker_threads[0], nullptr, stream_grabber, channels[0]);
            }

            cfg->rtsp_thread_signal = 0;
            cfg->worker_thread_signal.fetch_or(2);
        }

        // 1 & 2 = the threads are running, we can go to sleep
        if (cfg->worker_thread_signal.load() == 3)
        {
            LOG_DEBUG("The worker control thread goes into sleep mode");
            cfg->worker_thread_signal.wait(3);
            LOG_DEBUG("Worker control thread wakeup");
        }

        // 4 = Stop threads
        if (cfg->worker_thread_signal.load() & 4)
        {
            if (channels[0])
            {
                if(encoder[0]->osd) {
                    cfg->stream0.osd.thread_signal.store(false);
                    LOG_DEBUG("stop signal is sent to stream0 osd thread");
                    if (pthread_join(stream0_osd_thread, NULL) == 0)
                    {
                        LOG_DEBUG("wait for exit stream0 osd thread");
                    }
                    LOG_DEBUG("osd thread for stream0 has been terminated");
                }

                LOG_DEBUG("stop signal is sent to stream_grabber for stream0");
                channels[0]->thread_signal.store(false);
                if (pthread_join(worker_threads[0], NULL) == 0)
                {
                    LOG_DEBUG("wait for stream_grabber exit stream0");
                }
                LOG_DEBUG("stream_grabber for stream0 has been terminated");
                pthread_mutex_destroy(&sink_lock0);
                delete channels[0];
                channels[0] = nullptr;
            }

            if (channels[1])
            {
                if(encoder[1]->osd) {
                    cfg->stream1.osd.thread_signal.store(false);
                    LOG_DEBUG("stop signal is sent to stream1 osd thread");
                    if (pthread_join(stream1_osd_thread, NULL) == 0)
                    {
                        LOG_DEBUG("wait for exit stream1 osd thread");
                    }
                    LOG_DEBUG("osd thread for stream1 has been terminated");
                }

                LOG_DEBUG("stop signal is sent to stream_grabber for stream1");
                channels[1]->thread_signal.store(false);
                if (pthread_join(worker_threads[1], NULL) == 0)
                {
                    LOG_DEBUG("wait for stream_grabber exit stream1");
                }
                LOG_DEBUG("stream_grabber for stream1 has been terminated");
                pthread_mutex_destroy(&sink_lock1);
                delete channels[1];
                channels[1] = nullptr;
            }

            if (channels[2])
            {
                LOG_DEBUG("stop signal is sent to jpeg_grabber");
                channels[2]->thread_signal.store(false);
                if (pthread_join(worker_threads[2], NULL) == 0)
                {
                    LOG_DEBUG("wait for jpeg_grabber exit");
                }
                LOG_DEBUG("jpeg_grabber has been terminated");
                delete channels[2];
                channels[2] = nullptr;
            }

            pthread_attr_destroy(&osd_thread_attr);
            pthread_attr_destroy(&stream_thread_attr);

            deinit();

            // remove stop and set stopped, will be handled in main
            cfg->worker_thread_signal.fetch_xor(4);
            cfg->worker_thread_signal.fetch_or(8);
        }
    }
    usleep(1000);
}