#include "worker.hpp"
#include "Motion.hpp"
#include "AudioReframer.hpp"

#define MODULE "WORKER"

using namespace std::chrono;

unsigned long long tDiffInMs(struct timeval *startTime)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long seconds = currentTime.tv_sec - startTime->tv_sec;
    long microseconds = currentTime.tv_usec - startTime->tv_usec;

    unsigned long long milliseconds = (seconds * 1000) + (microseconds / 1000);

    return milliseconds;
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

    StartHelper *sh = static_cast<StartHelper *>(arg);
    int jpgChn = sh->encChn - 2;
    int ret;
    uint32_t bps{0};
    int delay{0};
    uint32_t fps{(uint32_t)global_jpeg[jpgChn]->stream->fps};
    int64_t auto_fps{(int64_t)((1000000/global_jpeg[jpgChn]->stream->fps) * 0.8)};

    unsigned long long ms{0};
    gettimeofday(&global_jpeg[jpgChn]->stream->stats.ts, NULL);
    global_jpeg[jpgChn]->stream->stats.ts.tv_sec -= 10;

    global_jpeg[jpgChn]->imp_encoder = IMPEncoder::createNew(
        global_jpeg[jpgChn]->stream, sh->encChn, global_jpeg[jpgChn]->stream->jpeg_channel, "stream2");

    // inform main that initialization is complete
    sh->has_started.release();

    ret = IMP_Encoder_StartRecvPic(global_jpeg[jpgChn]->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << global_jpeg[jpgChn]->encChn << ")");
    if (ret != 0)
        return 0;

    global_jpeg[jpgChn]->active = true;
    global_jpeg[jpgChn]->running = true;
    while (global_jpeg[jpgChn]->running)
    {
        /* 'delay'
         * if the last request is handled, the counter is used to serve some additional images
         * By this we can prevent unwanted suspends if less images requested than created
         */ 
        if (global_jpeg[jpgChn]->subscribers || delay)
        {
            /* check if current jpeg channal is running if not start it
             */

            if(!global_video[global_jpeg[jpgChn]->stream->jpeg_channel]->active) {
                
                /* required video channel was not running, we need to start it  
                 * and set run_for_jpeg as a reason.
                */
                std::unique_lock<std::mutex> lock_stream{mutex_main};
                global_video[global_jpeg[jpgChn]->stream->jpeg_channel]->run_for_jpeg = true;
                global_video[global_jpeg[jpgChn]->stream->jpeg_channel]->should_grab_frames.notify_one();
                lock_stream.unlock();
                global_video[global_jpeg[jpgChn]->stream->jpeg_channel]->is_activated.acquire();
            }

            if(!global_jpeg[jpgChn]->subscribers && delay)
                delay--;
            else
                delay = 25;

            auto start_grab = steady_clock::now();

            if (IMP_Encoder_PollingStream(global_jpeg[jpgChn]->encChn, cfg->general.imp_polling_timeout) == 0)
            {
                IMPEncoderStream stream;
                if (IMP_Encoder_GetStream(global_jpeg[jpgChn]->encChn, &stream, GET_STREAM_BLOCKING) == 0)
                {

                    usleep(auto_fps);

                    fps++;
                    bps += stream.pack->length;

                    //  Check for success
                    const char *tempPath = "/tmp/snapshot.tmp";             // Temporary path
                    const char *finalPath = global_jpeg[jpgChn]->stream->jpeg_path; // Final path for the JPEG snapshot

                    // Open and create temporary file with read and write permissions
                    int snap_fd = open(tempPath, O_RDWR | O_CREAT | O_TRUNC, 0666);
                    if (snap_fd >= 0)
                    {
                        // Save the JPEG stream to the file
                        save_jpeg_stream(snap_fd, &stream);

                        // Close the temporary file after writing is done
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

                    IMP_Encoder_ReleaseStream(global_jpeg[jpgChn]->encChn, &stream); // Release stream after saving
                }

                ms = tDiffInMs(&global_jpeg[jpgChn]->stream->stats.ts);
                if (ms > 1000)
                {
                    if(fps > global_jpeg[jpgChn]->stream->fps && auto_fps < 1000000) {
                        auto_fps += (fps - global_jpeg[jpgChn]->stream->fps) * (1000 / global_jpeg[jpgChn]->stream->fps);
                    } else if(fps < global_jpeg[jpgChn]->stream->fps && auto_fps > 0) {
                        auto_fps -= (global_jpeg[jpgChn]->stream->fps - fps) * (1000 / global_jpeg[jpgChn]->stream->fps);
                    }
                    if(auto_fps<0) auto_fps = 0;

                    global_jpeg[jpgChn]->stream->stats.fps = fps;
                    global_jpeg[jpgChn]->stream->stats.bps = bps;
                    fps = 0; bps = 0;
                    gettimeofday(&global_jpeg[jpgChn]->stream->stats.ts, NULL);

                    LOG_DDEBUG("JPG " << jpgChn << 
                              " fps: " << global_jpeg[jpgChn]->stream->stats.fps << 
                              " bps: " << global_jpeg[jpgChn]->stream->stats.bps <<
                              " subscribers: " << global_jpeg[jpgChn]->subscribers <<
                              " auto_fps: " << auto_fps <<
                              " ms: " << ms);
                }                
            }
        }
        else
        {
            LOG_DDEBUG("JPEG LOCK" << 
                       " channel:" << jpgChn);

            global_jpeg[jpgChn]->stream->stats.bps = 0;
            global_jpeg[jpgChn]->stream->stats.fps = 0;

            std::unique_lock<std::mutex> lock_stream{mutex_main};
            global_jpeg[jpgChn]->active = false;
            global_video[global_jpeg[jpgChn]->stream->jpeg_channel]->run_for_jpeg = false;
            while (!global_jpeg[jpgChn]->subscribers && !global_restart_video)
                global_jpeg[jpgChn]->should_grab_frames.wait(lock_stream);

            global_jpeg[jpgChn]->is_activated.release();
            global_jpeg[jpgChn]->active = true;

            LOG_DDEBUG("JPEG UNLOCK" << 
                       " channel:" << jpgChn);            
        }
    }

    if (global_jpeg[jpgChn]->imp_encoder)
    {
        global_jpeg[jpgChn]->imp_encoder->deinit();

        delete global_jpeg[jpgChn]->imp_encoder;
        global_jpeg[jpgChn]->imp_encoder = nullptr;
    }

    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << global_jpeg[jpgChn]->encChn << ")");

    return 0;
}

void *Worker::stream_grabber(void *arg)
{
    StartHelper *sh = static_cast<StartHelper *>(arg);
    int encChn = sh->encChn;

    LOG_DEBUG("Start stream_grabber thread for stream " << encChn);

    int ret;
    int flags{0};
    uint32_t bps;
    uint32_t fps;
    int64_t nal_ts;
    uint32_t error_count;
    unsigned long long ms;
    struct timeval imp_time_base;

    global_video[encChn]->imp_framesource = IMPFramesource::createNew(global_video[encChn]->stream, &cfg->sensor, encChn);
    global_video[encChn]->imp_encoder = IMPEncoder::createNew(global_video[encChn]->stream, encChn, encChn, global_video[encChn]->name);
    global_video[encChn]->imp_framesource->enable();

    gettimeofday(&imp_time_base, NULL);
    IMP_System_RebaseTimeStamp(imp_time_base.tv_sec * (uint64_t)1000000);

    // inform main that initialization is complete
    sh->has_started.release();

    ret = IMP_Encoder_StartRecvPic(encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << encChn << ")");
    if (ret != 0)
        return 0;

    /* 'active' indicates, the thread is activly polling and grabbing images
     * 'running' describes the runlevel of the thread, if this value is set to false
     *           the thread exits and cleanup all ressources 
     */
    global_video[encChn]->active = true;
    global_video[encChn]->running = true;
    while (global_video[encChn]->running)
    {
        /* bool helper to check if this is the active jpeg channel and a jpeg is requested while 
         * the channel is inactive
         */
        bool run_for_jpeg = (encChn == global_jpeg[0]->stream->jpeg_channel && global_video[encChn]->run_for_jpeg);

        /* now we need to verify that 
         * 1. a client is connected (hasDataCallback)
         * 2. a jpeg is requested 
         */
        if (global_video[encChn]->hasDataCallback || run_for_jpeg)
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

                    if (global_video[encChn]->hasDataCallback)
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
                        if (global_video[encChn]->idr == false)
                        {
#if defined(PLATFORM_T31)
                            if (stream.pack[i].nalType.h264NalType == 7 ||
                                stream.pack[i].nalType.h264NalType == 8 ||
                                stream.pack[i].nalType.h264NalType == 5)
                            {
                                global_video[encChn]->idr = true;
                            }
                            else if (stream.pack[i].nalType.h265NalType == 32)
                            {
                                global_video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                            if (stream.pack[i].dataType.h264Type == 7 ||
                                stream.pack[i].dataType.h264Type == 8 ||
                                stream.pack[i].dataType.h264Type == 5)
                            {
                                global_video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T30)
                            if (stream.pack[i].dataType.h264Type == 7 ||
                                stream.pack[i].dataType.h264Type == 8 ||
                                stream.pack[i].dataType.h264Type == 5)
                            {
                                global_video[encChn]->idr = true;
                            }
                            else if (stream.pack[i].dataType.h265Type == 32)
                            {
                                global_video[encChn]->idr = true;
                            }
#endif
                        }

                        if (global_video[encChn]->idr == true)
                        {
                            if (!global_video[encChn]->msgChannel->write(nalu))
                            {
                                LOG_ERROR("video " << 
                                    "channel:" << encChn << ", " <<
                                    "package:" << i << " of " << stream.packCount << ", " <<
                                    "packageSize:" << nalu.data.size() <<
                                    ".  !sink clogged!");
                            }
                            else
                            {
                                std::unique_lock<std::mutex> lock_stream{global_video[encChn]->onDataCallbackLock};
                                if (global_video[encChn]->onDataCallback)
                                    global_video[encChn]->onDataCallback();
                            }
                        }
                    }
                }

                IMP_Encoder_ReleaseStream(encChn, &stream);

                ms = tDiffInMs(&global_video[encChn]->stream->stats.ts);
                if (ms > 1000)
                {

                    /* currently we write into osd and stream stats,
                     * osd will be removed and redesigned in future
                    */
                    global_video[encChn]->stream->stats.bps = bps;
                    global_video[encChn]->stream->osd.stats.bps = bps;
                    global_video[encChn]->stream->stats.fps = fps;
                    global_video[encChn]->stream->osd.stats.fps = fps;

                    fps = 0; bps = 0;
                    gettimeofday(&global_video[encChn]->stream->stats.ts, NULL);
                    global_video[encChn]->stream->osd.stats.ts = global_video[encChn]->stream->stats.ts;
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
                    if (global_video[encChn]->idr_fix)
                    {
                        IMP_Encoder_RequestIDR(encChn);
                        global_video[encChn]->idr_fix--;
                    }
                }
            }
            else
            {
                error_count++;
                LOG_DDEBUG("IMP_Encoder_PollingStream(" << encChn << ", " << cfg->general.imp_polling_timeout << ") timeout !");
            }
        }
        else if (global_video[encChn]->onDataCallback == nullptr && !global_restart_video && !global_video[encChn]->run_for_jpeg)
        {
            LOG_DDEBUG("VIDEO LOCK" << 
                       " channel:" << encChn << 
                       " hasCallbackIsNull:" << (global_video[encChn]->onDataCallback == nullptr) << 
                       " restartVideo:" << global_restart_video << 
                       " runForJpeg:" << global_video[encChn]->run_for_jpeg);

            global_video[encChn]->stream->stats.bps = 0;
            global_video[encChn]->stream->stats.fps = 0;
            global_video[encChn]->stream->osd.stats.bps = 0;
            global_video[encChn]->stream->osd.stats.fps = 0;

            std::unique_lock<std::mutex> lock_stream{mutex_main};           
            global_video[encChn]->active = false;
            while (global_video[encChn]->onDataCallback == nullptr && !global_restart_video && !global_video[encChn]->run_for_jpeg)
                global_video[encChn]->should_grab_frames.wait(lock_stream);

            global_video[encChn]->active = true;
            global_video[encChn]->is_activated.release();

            LOG_DDEBUG("VIDEO UNLOCK" << 
                       " channel:" << encChn);           
        }
    }

    ret = IMP_Encoder_StopRecvPic(encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StopRecvPic(" << encChn << ")");

    if (global_video[encChn]->imp_framesource)
    {
        global_video[encChn]->imp_framesource->disable();

        if (global_video[encChn]->imp_encoder)
        {
            global_video[encChn]->imp_encoder->deinit();
            delete global_video[encChn]->imp_encoder;
            global_video[encChn]->imp_encoder = nullptr;
        }
    }

    return 0;
}
#if defined(AUDIO_SUPPORT)
static void process_frame(int encChn, IMPAudioFrame &frame)
{
    int64_t audio_ts = frame.timeStamp;
    struct timeval encoder_time;
    encoder_time.tv_sec = audio_ts / 1000000;
    encoder_time.tv_usec = audio_ts % 1000000;

    AudioFrame af;
    af.time = encoder_time;

    uint8_t *start = (uint8_t *)frame.virAddr;
    uint8_t *end = start + frame.len;

    IMPAudioStream stream;
    if (global_audio[encChn]->imp_audio->format != IMPAudioFormat::PCM)
    {
        if (IMP_AENC_SendFrame(global_audio[encChn]->aeChn, &frame) != 0)
        {
            LOG_ERROR("IMP_AENC_SendFrame(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aeChn << ") failed");
        }
        else if (IMP_AENC_PollingStream(global_audio[encChn]->aeChn, cfg->general.imp_polling_timeout) != 0)
        {
            LOG_ERROR("IMP_AENC_PollingStream(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aeChn << ") failed");
        }
        else if (IMP_AENC_GetStream(global_audio[encChn]->aeChn, &stream, IMPBlock::BLOCK) != 0)
        {
            LOG_ERROR("IMP_AENC_GetStream(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aeChn << ") failed");
        }
        else
        {
            start = (uint8_t *)stream.stream;
            end = start + stream.len;
        }
    }

    af.data.insert(af.data.end(), start, end);
    if (global_audio[encChn]->hasDataCallback)
    {
        if (!global_audio[encChn]->msgChannel->write(af))
        {
            LOG_ERROR("audio encChn:" << encChn << ", size:" << af.data.size() << " clogged!");
        }
        else
        {
            std::unique_lock<std::mutex> lock_stream{global_audio[encChn]->onDataCallbackLock};
            if (global_audio[encChn]->onDataCallback)
                global_audio[encChn]->onDataCallback();
        }
        // LOG_DEBUG("audio:" <<  global_audio[encChn]->aiChn << " " << af.time.tv_sec << "." << af.time.tv_usec << " " << af.data.size());
    }

    if (global_audio[encChn]->imp_audio->format != IMPAudioFormat::PCM && IMP_AENC_ReleaseStream(global_audio[encChn]->aeChn, &stream) < 0)
    {
        LOG_ERROR("IMP_AENC_ReleaseStream(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aeChn << ", &stream) failed");
    }
}

void *Worker::audio_grabber(void *arg)
{
    StartHelper *sh = static_cast<StartHelper *>(arg);
    int encChn = sh->encChn;

    LOG_DEBUG("Start audio_grabber thread for device " << global_audio[encChn]->devId
        << " and channel " << global_audio[encChn]->aiChn
        << " and encoder " << global_audio[encChn]->aeChn);

    global_audio[encChn]->imp_audio = IMPAudio::createNew(global_audio[encChn]->devId, global_audio[encChn]->aiChn, global_audio[encChn]->aeChn);

    // Initialize AudioReframer only if needed
    std::unique_ptr<AudioReframer> reframer;
    if (global_audio[encChn]->imp_audio->format == IMPAudioFormat::AAC)
    {
        reframer = std::make_unique<AudioReframer>(
            global_audio[encChn]->imp_audio->sample_rate,
            /* inputSamplesPerFrame */ global_audio[encChn]->imp_audio->sample_rate * 0.040,
            /* outputSamplesPerFrame */ 1024
        );
    }

    // inform main that initialization is complete
    sh->has_started.release();

    /* 'active' indicates, the thread is activly polling and grabbing images
     * 'running' describes the runlevel of the thread, if this value is set to false
     *           the thread exits and cleanup all ressources 
     */
    global_audio[encChn]->active = true;
    global_audio[encChn]->running = true;
    while (global_audio[encChn]->running)
    {

        if (global_audio[encChn]->hasDataCallback && cfg->audio.input_enabled)
        {
            if (IMP_AI_PollingFrame(global_audio[encChn]->devId, global_audio[encChn]->aiChn, cfg->general.imp_polling_timeout) == 0)
            {
                IMPAudioFrame frame;
                if (IMP_AI_GetFrame(global_audio[encChn]->devId, global_audio[encChn]->aiChn, &frame, IMPBlock::BLOCK) != 0)
                {
                    LOG_ERROR("IMP_AI_GetFrame(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aiChn << ") failed");
                }

                if (reframer)
                {
                    std::vector<int16_t> frameData(frame.len / sizeof(int16_t));
                    std::memcpy(frameData.data(), frame.virAddr, frame.len);
                    reframer->addFrame(frameData, frame.timeStamp);
                    while (reframer->hasMoreFrames())
                    {
                        int64_t audio_ts;
                        reframer->getReframedFrame(frameData, audio_ts);
                        IMPAudioFrame reframed = {
                            .bitwidth = frame.bitwidth,
                            .soundmode = frame.soundmode,
                            .virAddr = reinterpret_cast<uint32_t*>(frameData.data()),
                            .phyAddr = frame.phyAddr,
                            .timeStamp = audio_ts,
                            .seq = frame.seq,
                            .len = static_cast<int>(frameData.size() * sizeof(int16_t))
                        };
                        process_frame(encChn, reframed);
                    }
                }
                else
                {
                    process_frame(encChn, frame);
                }

                if (IMP_AI_ReleaseFrame(global_audio[encChn]->devId, global_audio[encChn]->aiChn, &frame) < 0)
                {
                    LOG_ERROR("IMP_AI_ReleaseFrame(" << global_audio[encChn]->devId << ", " << global_audio[encChn]->aiChn << ", &frame) failed");
                }
            }
            else
            {
                LOG_DEBUG(global_audio[encChn]->devId << ", " << global_audio[encChn]->aiChn << " POLLING TIMEOUT");
            }
        }
        else
        {
            std::unique_lock<std::mutex> lock_stream{mutex_main};
            global_audio[encChn]->active = false;
            while (global_audio[encChn]->onDataCallback == nullptr && !global_restart_audio)
                global_audio[encChn]->should_grab_frames.wait(lock_stream);

            global_audio[encChn]->active = true;
        }
    } // while (global_audio[encChn]->running)

    if (global_audio[encChn]->imp_audio)
    {
        delete global_audio[encChn]->imp_audio;
    }

    return 0;
}
#endif
void *Worker::update_osd(void *arg)
{

    LOG_DEBUG("start osd update thread.");

    global_osd_thread_signal = true;

    while (global_osd_thread_signal)
    {
        for (auto v : global_video)
        {
            if (v != nullptr)
            {
                if (v->active)
                {
                    if ((v->imp_encoder->osd != nullptr))
                    {
                        if (v->imp_encoder->osd->is_started)
                        {
                            v->imp_encoder->osd->updateDisplayEverySecond();
                        }
                        else
                        {
                            if (v->imp_encoder->osd->startup_delay)
                            {
                                v->imp_encoder->osd->startup_delay--;
                            }
                            else
                            {
                                v->imp_encoder->osd->start();
                            }
                        }
                    }
                }
            }
        }
        usleep(25000);
    }

    LOG_DEBUG("exit osd update thread.");
    return 0;
}
