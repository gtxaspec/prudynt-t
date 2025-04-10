#include "AudioWorker.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "WorkerUtils.hpp"
#include "globals.hpp"

#define MODULE "AudioWorker"

#if defined(AUDIO_SUPPORT)

AudioWorker::AudioWorker(int chn)
    : encChn(chn)
{
    LOG_DEBUG("AudioWorker created for channel " << encChn);
}

AudioWorker::~AudioWorker()
{
    LOG_DEBUG("AudioWorker destroyed for channel " << encChn);
}

void AudioWorker::process_audio_frame(IMPAudioFrame &frame)
{
    int64_t audio_ts = frame.timeStamp;
    struct timeval encoder_time;
    encoder_time.tv_sec = audio_ts / 1000000;
    encoder_time.tv_usec = audio_ts % 1000000;

    AudioFrame af;
    af.time = encoder_time;

    uint8_t *start = (uint8_t *) frame.virAddr;
    uint8_t *end = start + frame.len;

    IMPAudioStream stream;
    if (global_audio[encChn]->imp_audio->format != IMPAudioFormat::PCM)
    {
        if (IMP_AENC_SendFrame(global_audio[encChn]->aeChn, &frame) != 0)
        {
            LOG_ERROR("IMP_AENC_SendFrame(" << global_audio[encChn]->devId << ", "
                                            << global_audio[encChn]->aeChn << ") failed");
        }
        else if (IMP_AENC_PollingStream(global_audio[encChn]->aeChn,
                                        cfg->general.imp_polling_timeout)
                 != 0)
        {
            LOG_ERROR("IMP_AENC_PollingStream(" << global_audio[encChn]->devId << ", "
                                                << global_audio[encChn]->aeChn << ") failed");
        }
        else if (IMP_AENC_GetStream(global_audio[encChn]->aeChn, &stream, IMPBlock::BLOCK) != 0)
        {
            LOG_ERROR("IMP_AENC_GetStream(" << global_audio[encChn]->devId << ", "
                                            << global_audio[encChn]->aeChn << ") failed");
        }
        else
        {
            start = (uint8_t *) stream.stream;
            end = start + stream.len;
        }
    }

    if (end > start)
    {
        af.data.insert(af.data.end(), start, end);
    }

    if (!af.data.empty() && global_audio[encChn]->hasDataCallback
        && (global_video[0]->hasDataCallback || global_video[1]->hasDataCallback))
    {
        if (!global_audio[encChn]->msgChannel->write(af))
        {
#if defined(USE_AUDIO_STREAM_REPLICATOR)
            LOG_DDEBUG("audio encChn:" << encChn << ", size:" << af.data.size() << " clogged!");
#else
            LOG_ERROR("audio encChn:" << encChn << ", size:" << af.data.size() << " clogged!");
#endif
        }
        else
        {
            std::unique_lock<std::mutex> lock_stream{global_audio[encChn]->onDataCallbackLock};
            if (global_audio[encChn]->onDataCallback)
                global_audio[encChn]->onDataCallback();
        }
    }

    if (global_audio[encChn]->imp_audio->format != IMPAudioFormat::PCM
        && IMP_AENC_ReleaseStream(global_audio[encChn]->aeChn, &stream) < 0)
    {
        LOG_ERROR("IMP_AENC_ReleaseStream(" << global_audio[encChn]->devId << ", "
                                            << global_audio[encChn]->aeChn << ", &stream) failed");
    }
}

void AudioWorker::process_frame(IMPAudioFrame &frame)
{
    if (global_audio[encChn]->imp_audio->outChnCnt == 2 && frame.soundmode == AUDIO_SOUND_MODE_MONO)
    {
        size_t sample_size = frame.bitwidth / 8;
        size_t num_samples = frame.len / sample_size;
        size_t stereo_size = frame.len * 2;
        uint8_t *stereo_buffer = new uint8_t[stereo_size];

        for (size_t i = 0; i < num_samples; i++)
        {
            uint8_t *mono_sample = ((uint8_t *) frame.virAddr) + (i * sample_size);
            uint8_t *stereo_left = stereo_buffer + (i * sample_size * 2);
            uint8_t *stereo_right = stereo_left + sample_size;
            memcpy(stereo_left, mono_sample, sample_size);
            memcpy(stereo_right, mono_sample, sample_size);
        }

        IMPAudioFrame stereo_frame = frame;
        stereo_frame.virAddr = (uint32_t *) stereo_buffer;
        stereo_frame.len = stereo_size;
        stereo_frame.soundmode = AUDIO_SOUND_MODE_STEREO;

        process_audio_frame(stereo_frame);
        delete[] stereo_buffer;
    }
    else
    {
        process_audio_frame(frame);
    }
}

void AudioWorker::run()
{
    LOG_DEBUG("Start audio processing run loop for channel " << encChn);

    // Initialize AudioReframer only if needed, store in member variable
    if (global_audio[encChn]->imp_audio->format == IMPAudioFormat::AAC)
    {
        reframer = std::make_unique<AudioReframer>(
            global_audio[encChn]->imp_audio->sample_rate,
            /* inputSamplesPerFrame */ global_audio[encChn]->imp_audio->sample_rate * 0.040,
            /* outputSamplesPerFrame */ 1024);
        LOG_DEBUG("AudioReframer created for channel " << encChn);
    }
    else
    {
        LOG_DEBUG("AudioReframer not needed or imp_audio not ready for channel " << encChn);
    }

    while (global_audio[encChn]->running)
    {
        if (global_audio[encChn]->hasDataCallback && cfg->audio.input_enabled
            && (global_video[0]->hasDataCallback || global_video[1]->hasDataCallback))
        {
            if (IMP_AI_PollingFrame(global_audio[encChn]->devId,
                                    global_audio[encChn]->aiChn,
                                    cfg->general.imp_polling_timeout)
                == 0)
            {
                IMPAudioFrame frame;
                if (IMP_AI_GetFrame(global_audio[encChn]->devId,
                                    global_audio[encChn]->aiChn,
                                    &frame,
                                    IMPBlock::BLOCK)
                    != 0)
                {
                    LOG_ERROR("IMP_AI_GetFrame(" << global_audio[encChn]->devId << ", "
                                                 << global_audio[encChn]->aiChn << ") failed");
                }

                if (reframer)
                {
                    reframer->addFrame(reinterpret_cast<uint8_t *>(frame.virAddr), frame.timeStamp);
                    while (reframer->hasMoreFrames())
                    {
                        size_t frameLen = 1024 * sizeof(uint16_t)
                                          * global_audio[encChn]->imp_audio->outChnCnt;
                        std::vector<uint8_t> frameData(frameLen, 0);
                        int64_t audio_ts;
                        reframer->getReframedFrame(frameData.data(), audio_ts);
                        IMPAudioFrame reframed = {.bitwidth = frame.bitwidth,
                                                  .soundmode = frame.soundmode,
                                                  .virAddr = reinterpret_cast<uint32_t *>(
                                                      frameData.data()),
                                                  .phyAddr = frame.phyAddr,
                                                  .timeStamp = audio_ts,
                                                  .seq = frame.seq,
                                                  .len = static_cast<int>(frameLen)};
                        process_frame(reframed);
                    }
                }
                else
                {
                    process_frame(frame);
                }

                if (IMP_AI_ReleaseFrame(global_audio[encChn]->devId,
                                        global_audio[encChn]->aiChn,
                                        &frame)
                    < 0)
                {
                    LOG_ERROR("IMP_AI_ReleaseFrame(" << global_audio[encChn]->devId << ", "
                                                     << global_audio[encChn]->aiChn
                                                     << ", &frame) failed");
                }
            }
            else
            {
                LOG_DEBUG(global_audio[encChn]->devId << ", " << global_audio[encChn]->aiChn
                                                      << " POLLING TIMEOUT");
            }
        }
        else if (cfg->audio.input_enabled && !global_restart)
        {
            std::unique_lock<std::mutex> lock_stream{mutex_main};
            global_audio[encChn]->active = false;
            LOG_DDEBUG("AUDIO LOCK");

            /* Since the audio stream is permanently in use by the stream replicator, 
             * we send the audio grabber and encoder to standby when no video is requested.
            */
            while ((global_audio[encChn]->onDataCallback == nullptr
                    || (!global_video[0]->hasDataCallback && !global_video[1]->hasDataCallback))
                   && !global_restart_audio)
            {
                global_audio[encChn]->should_grab_frames.wait(lock_stream);
            }
            global_audio[encChn]->active = true;
            LOG_DDEBUG("AUDIO UNLOCK");
        }
        else
        {
            /* to prevent clogging on startup or while restarting the threads
             * we wait for 250ms
            */
            usleep(250 * 1000);
        }
    }
}

void *AudioWorker::thread_entry(void *arg)
{
    StartHelper *sh = static_cast<StartHelper *>(arg);
    int encChn = sh->encChn;

    LOG_DEBUG("Start audio_grabber thread for device "
              << global_audio[encChn]->devId << " and channel " << global_audio[encChn]->aiChn
              << " and encoder " << global_audio[encChn]->aeChn);

    global_audio[encChn]->imp_audio = IMPAudio::createNew(global_audio[encChn]->devId,
                                                          global_audio[encChn]->aiChn,
                                                          global_audio[encChn]->aeChn);

    // inform main that initialization is complete
    sh->has_started.release();

    /* 'active' indicates, the thread is activly polling and grabbing images
     * 'running' describes the runlevel of the thread, if this value is set to false
     *           the thread exits and cleanup all ressources 
     */
    global_audio[encChn]->active = true;
    global_audio[encChn]->running = true;

    AudioWorker worker(encChn);
    worker.run();

    if (global_audio[encChn]->imp_audio)
    {
        delete global_audio[encChn]->imp_audio;
    }

    return 0;
}

#endif // AUDIO_SUPPORT
