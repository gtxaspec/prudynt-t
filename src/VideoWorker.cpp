#include "VideoWorker.hpp"

#include "Config.hpp"
#include "IMPEncoder.hpp"
#include "IMPFramesource.hpp"
#include "Logger.hpp"
#include "WorkerUtils.hpp"
#include "globals.hpp"

#define MODULE "VideoWorker"

VideoWorker::VideoWorker(int chn)
    : encChn(chn)
{
    LOG_DEBUG("VideoWorker created for channel " << encChn);
}

VideoWorker::~VideoWorker()
{
    LOG_DEBUG("VideoWorker destroyed for channel " << encChn);
}

void VideoWorker::run()
{
    LOG_DEBUG("Start video processing run loop for stream " << encChn);

    uint32_t bps = 0;
    uint32_t fps = 0;
    uint32_t error_count = 0; // Keep track of polling errors
    unsigned long long ms = 0;
    bool run_for_jpeg = false;

    while (global_video[encChn]->running)
    {
        /* bool helper to check if this is the active jpeg channel and a jpeg is requested while 
         * the channel is inactive
         */
        run_for_jpeg = (encChn == global_jpeg[0]->streamChn && global_video[encChn]->run_for_jpeg);

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

                /* timestamp fix, can be removed if solved
                int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;
                struct timeval encoder_time;
                encoder_time.tv_sec = nal_ts / 1000000;
                encoder_time.tv_usec = nal_ts % 1000000;
                */

                for (uint32_t i = 0; i < stream.packCount; ++i)
                {
                    fps++;
                    bps += stream.pack[i].length;

                    if (global_video[encChn]->hasDataCallback)
                    {
#if defined(PLATFORM_T31) || defined(PLATFORM_T40) || defined(PLATFORM_T41)
                        uint8_t *start = (uint8_t *) stream.virAddr + stream.pack[i].offset;
                        uint8_t *end = start + stream.pack[i].length;
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) \
    || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                        uint8_t *start = (uint8_t *) stream.pack[i].virAddr;
                        uint8_t *end = (uint8_t *) stream.pack[i].virAddr + stream.pack[i].length;
#endif
                        H264NALUnit nalu;

                        /* timestamp fix, can be removed if solved
                        nalu.imp_ts = stream.pack[i].timestamp;
                        nalu.time = encoder_time;
                        */

                        // We use start+4 because the encoder inserts 4-byte MPEG
                        //'startcodes' at the beginning of each NAL. Live555 complains
                        nalu.data.insert(nalu.data.end(), start + 4, end);
                        if (global_video[encChn]->idr == false)
                        {
#if defined(PLATFORM_T31) || defined(PLATFORM_T40) || defined(PLATFORM_T41)
                            if (stream.pack[i].nalType.h264NalType == 7
                                || stream.pack[i].nalType.h264NalType == 8
                                || stream.pack[i].nalType.h264NalType == 5)
                            {
                                global_video[encChn]->idr = true;
                            }
                            else if (stream.pack[i].nalType.h265NalType == 32)
                            {
                                global_video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) \
    || defined(PLATFORM_T23)
                            if (stream.pack[i].dataType.h264Type == 7
                                || stream.pack[i].dataType.h264Type == 8
                                || stream.pack[i].dataType.h264Type == 5)
                            {
                                global_video[encChn]->idr = true;
                            }
#elif defined(PLATFORM_T30)
                            if (stream.pack[i].dataType.h264Type == 7
                                || stream.pack[i].dataType.h264Type == 8
                                || stream.pack[i].dataType.h264Type == 5)
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
                                LOG_ERROR("video " << "channel:" << encChn << ", "
                                                   << "package:" << i << " of " << stream.packCount
                                                   << ", " << "packageSize:" << nalu.data.size()
                                                   << ".  !sink clogged!");
                            }
                            else
                            {
                                std::unique_lock<std::mutex> lock_stream{
                                    global_video[encChn]->onDataCallbackLock};
                                if (global_video[encChn]->onDataCallback)
                                    global_video[encChn]->onDataCallback();
                            }
                        }
#if defined(USE_AUDIO_STREAM_REPLICATOR)
                        /* Since the audio stream is permanently in use by the stream replicator, 
                         * and the audio grabber and encoder standby is also controlled by the video threads
                         * we need to wakeup the audio thread 
                        */
                        if (cfg->audio.input_enabled && !global_audio[0]->active && !global_restart)
                        {
                            LOG_DDEBUG("NOTIFY AUDIO " << !global_audio[0]->active << " "
                                                       << cfg->audio.input_enabled);
                            global_audio[0]->should_grab_frames.notify_one();
                        }
#endif
                    }
                }

                IMP_Encoder_ReleaseStream(encChn, &stream);

                ms = WorkerUtils::tDiffInMs(&global_video[encChn]->stream->stats.ts);
                if (ms > 1000)
                {
                    /* currently we write into osd and stream stats,
                     * osd will be removed and redesigned in future
                    */
                    global_video[encChn]->stream->stats.bps = bps;
                    global_video[encChn]->stream->osd.stats.bps = bps;
                    global_video[encChn]->stream->stats.fps = fps;
                    global_video[encChn]->stream->osd.stats.fps = fps;

                    fps = 0;
                    bps = 0;
                    gettimeofday(&global_video[encChn]->stream->stats.ts, NULL);
                    global_video[encChn]->stream->osd.stats.ts = global_video[encChn]
                                                                     ->stream->stats.ts;
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
                LOG_DDEBUG("IMP_Encoder_PollingStream("
                           << encChn << ", " << cfg->general.imp_polling_timeout << ") timeout !");
            }
        }
        else if (global_video[encChn]->onDataCallback == nullptr && !global_restart_video
                 && !global_video[encChn]->run_for_jpeg)
        {
            LOG_DDEBUG("VIDEO LOCK" << " channel:" << encChn << " hasCallbackIsNull:"
                                    << (global_video[encChn]->onDataCallback == nullptr)
                                    << " restartVideo:" << global_restart_video
                                    << " runForJpeg:" << global_video[encChn]->run_for_jpeg);

            global_video[encChn]->stream->stats.bps = 0;
            global_video[encChn]->stream->stats.fps = 0;
            global_video[encChn]->stream->osd.stats.bps = 0;
            global_video[encChn]->stream->osd.stats.fps = 0;

            std::unique_lock<std::mutex> lock_stream{mutex_main};
            global_video[encChn]->active = false;
            while (global_video[encChn]->onDataCallback == nullptr && !global_restart_video
                   && !global_video[encChn]->run_for_jpeg)
                global_video[encChn]->should_grab_frames.wait(lock_stream);

            global_video[encChn]->active = true;
            global_video[encChn]->is_activated.release();

            // unlock audio
            global_audio[0]->should_grab_frames.notify_one();

            LOG_DDEBUG("VIDEO UNLOCK" << " channel:" << encChn);
        }
    }
}

void *VideoWorker::thread_entry(void *arg)
{
    StartHelper *sh = static_cast<StartHelper *>(arg);
    int encChn = sh->encChn;

    LOG_DEBUG("Start stream_grabber thread for stream " << encChn);

    int ret;

    global_video[encChn]->imp_framesource = IMPFramesource::createNew(global_video[encChn]->stream,
                                                                      &cfg->sensor,
                                                                      encChn);
    global_video[encChn]->imp_encoder = IMPEncoder::createNew(global_video[encChn]->stream,
                                                              encChn,
                                                              encChn,
                                                              global_video[encChn]->name);
    global_video[encChn]->imp_framesource->enable();
    global_video[encChn]->run_for_jpeg = false;

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
    VideoWorker worker(encChn);
    worker.run();

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
