#include "AudioWorker.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "WorkerUtils.hpp"
#include "TimestampManager.hpp"
#include "globals.hpp"
#include "RTSPStatus.hpp"
#include <chrono>

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

void AudioWorker::process_audio_frame_direct(IMPAudioFrame &frame)
{
    // SINGLE SOURCE OF TRUTH: Use TimestampManager (same as video)
    struct timeval encoder_time;
    TimestampManager::getInstance().getTimestamp(&encoder_time);

    // TIMESTAMP DEBUG: Log audio frame processing
    LOG_DEBUG("AUDIO_TIMESTAMP_2_PROCESS: frame.timeStamp=" << frame.timeStamp << " encoder_time.tv_sec=" << encoder_time.tv_sec << " encoder_time.tv_usec=" << encoder_time.tv_usec);

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
    // Handle Opus frame accumulation (320 -> 960 samples) to fix timing drift
    if (global_audio[encChn]->imp_audio->format == IMPAudioFormat::OPUS && targetSamplesPerChannel > 0) {
        int samplesPerChannel = (frame.len / sizeof(int16_t)) / global_audio[encChn]->imp_audio->outChnCnt;

        // If this is the first frame in the buffer, save the timestamp
        if (frameBuffer.empty()) {
            // SINGLE SOURCE OF TRUTH: Use TimestampManager timestamp
            bufferStartTimestamp = TimestampManager::getInstance().getTimestampUs();
            // Only log if verbose audio debugging is enabled
            if (cfg->general.audio_debug_verbose) {
                static int accumulationLogCount = 0;
                if (accumulationLogCount < 3) {
                    LOG_DEBUG("Starting new Opus frame accumulation: " << samplesPerChannel << " samples per channel");
                    accumulationLogCount++;
                }
            }
        }

        // Buffer safety: bound growth and drop oldest on overflow
        int outCh = global_audio[encChn]->imp_audio->outChnCnt;
        int16_t *samples = (int16_t*)frame.virAddr;
        int totalSamples = frame.len / sizeof(int16_t);
        int currentSamplesPerChannel = frameBuffer.size() / outCh;
        int incomingSamplesPerChannel = totalSamples / outCh;
        int predictedSamplesPerChannel = currentSamplesPerChannel + incomingSamplesPerChannel;

        if (warnBufferSamplesPerChannel > 0 && predictedSamplesPerChannel >= warnBufferSamplesPerChannel) {
            // Rate-limit buffer capacity warnings to avoid log spam
            static auto lastWarnTime = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastWarn = std::chrono::duration_cast<std::chrono::seconds>(now - lastWarnTime);

            if (timeSinceLastWarn.count() >= 5) { // Warn at most every 5 seconds
                LOG_WARN("AudioWorker buffer nearing capacity: " << predictedSamplesPerChannel
                         << "/" << maxBufferSamplesPerChannel << " samples/ch (" << bufferDropCount.load() << " drops so far)");
                lastWarnTime = now;
            }
        }

        // Drop oldest samples to keep within capacity
        while (maxBufferSamplesPerChannel > 0 && predictedSamplesPerChannel > maxBufferSamplesPerChannel && !frameBuffer.empty()) {
            int dropSamplesPerChannel = std::max(targetSamplesPerChannel, predictedSamplesPerChannel - maxBufferSamplesPerChannel);
            int dropTotalSamples = dropSamplesPerChannel * outCh;
            dropTotalSamples = std::min(dropTotalSamples, (int)frameBuffer.size());
            frameBuffer.erase(frameBuffer.begin(), frameBuffer.begin() + dropTotalSamples);
            predictedSamplesPerChannel -= dropSamplesPerChannel;
            bufferDropCount.fetch_add(1);
            // Advance buffer start PTS accordingly
            bufferStartTimestamp += (int64_t) ( (dropSamplesPerChannel * 1000000LL) / global_audio[encChn]->imp_audio->sample_rate );
            // Expose metrics via RTSPStatus
            {
                std::string streamName = std::string("audio") + std::to_string(encChn);
                RTSPStatus::writeCustomParameter(streamName, "buffer_drop_count", std::to_string(bufferDropCount.load()));
                RTSPStatus::writeCustomParameter(streamName, "buffer_level_samples_per_channel", std::to_string(predictedSamplesPerChannel));
            }
            LOG_WARN("AudioWorker dropped " << dropSamplesPerChannel << " samples/ch to bound buffer");
        }

        // Add samples to buffer
        frameBuffer.insert(frameBuffer.end(), samples, samples + totalSamples);

        currentSamplesPerChannel = frameBuffer.size() / outCh;

        // Check if we have enough samples for a complete Opus frame
        // CRITICAL FIX: Use while loop to process multiple accumulated frames
        while (currentSamplesPerChannel >= targetSamplesPerChannel) {
            // Create a new frame with exactly the right number of samples
            int targetTotalSamples = targetSamplesPerChannel * global_audio[encChn]->imp_audio->outChnCnt;
            int targetBytes = targetTotalSamples * sizeof(int16_t);

            IMPAudioFrame opusFrame = frame;
            opusFrame.virAddr = (uint32_t*)frameBuffer.data();
            opusFrame.len = targetBytes;
            // Keep original timestamp - the monotonic PTS will be applied in process_audio_frame_direct
            opusFrame.timeStamp = bufferStartTimestamp;

            // Only log if verbose audio debugging is enabled
            if (cfg->general.audio_debug_verbose) {
                static int readyLogCount = 0;
                if (readyLogCount < 3) {
                    LOG_DEBUG("Opus frame ready: accumulated " << currentSamplesPerChannel
                             << " samples per channel, sending " << targetSamplesPerChannel);
                    readyLogCount++;
                }
            }

            // Analyze raw PCM data for corruption patterns
            static int analysis_count = 0;
            if (analysis_count < 5) {
                int16_t *samples = (int16_t*)frameBuffer.data();
                int16_t min_val = samples[0], max_val = samples[0];
                int zero_count = 0, clip_count = 0;

                for (int i = 0; i < targetTotalSamples; i++) {
                    if (samples[i] == 0) zero_count++;
                    if (samples[i] >= 32767 || samples[i] <= -32768) clip_count++;
                    if (samples[i] < min_val) min_val = samples[i];
                    if (samples[i] > max_val) max_val = samples[i];
                }

                LOG_DEBUG("Raw PCM analysis " << analysis_count << ": min=" << min_val
                         << ", max=" << max_val << ", zeros=" << zero_count
                         << ", clipped=" << clip_count << "/" << targetTotalSamples);
                analysis_count++;
            }

            // Process the accumulated frame
            process_audio_frame_direct(opusFrame);

            // Remove processed samples from buffer
            frameBuffer.erase(frameBuffer.begin(), frameBuffer.begin() + targetTotalSamples);

            // Update timestamp for next frame
            bufferStartTimestamp += (targetSamplesPerChannel * 1000000LL) / global_audio[encChn]->imp_audio->sample_rate;

            // Recalculate remaining samples for next iteration
            currentSamplesPerChannel = frameBuffer.size() / global_audio[encChn]->imp_audio->outChnCnt;
        }

        return; // Don't process the original frame
    }

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

        process_audio_frame_direct(stereo_frame);
        delete[] stereo_buffer;
    }
    else
    {
        process_audio_frame_direct(frame);
    }
}

void AudioWorker::run()
{
    LOG_DEBUG("Start audio processing run loop for channel " << encChn);

    // Using global TimestampManager for unified audio/video timeline
    LOG_DEBUG("AudioWorker using TimestampManager for unified timeline");

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

    // Initialize frame accumulator for Opus
    // RFC 7587 COMPLIANCE NOTE: OPUS RTP timestamps MUST always use 48kHz clock rate
    // for signaling purposes, but the actual input sampling rate can be different
    // (8kHz, 16kHz, 24kHz, 48kHz, etc.). The frame accumulator must collect samples
    // based on the ACTUAL input sample rate, not the RTP clock rate.
    // For 20ms frames: required_samples = actual_input_rate * 0.020
    if (global_audio[encChn]->imp_audio->format == IMPAudioFormat::OPUS) {
        // Calculate samples for 20ms at the actual input sample rate
        targetSamplesPerChannel = global_audio[encChn]->imp_audio->sample_rate * 0.020;
        frameBuffer.clear();
        // Compute buffer bounds (configurable via cfg->audio.* if provided)
        int warnFrames = 3;
        int capFrames  = 5;
        // Optional config overrides
        if (cfg && cfg->config_loaded) {
            warnFrames = std::max(1, cfg->audio.buffer_warn_frames);
            capFrames  = std::max(warnFrames + 1, cfg->audio.buffer_cap_frames);
        }
        warnBufferSamplesPerChannel = targetSamplesPerChannel * warnFrames;
        maxBufferSamplesPerChannel  = targetSamplesPerChannel * capFrames;
        LOG_DEBUG("Opus frame accumulator initialized: target=" << targetSamplesPerChannel
                 << " samples per channel (20ms at " << global_audio[encChn]->imp_audio->sample_rate << "Hz), "
                 << "warn@" << warnBufferSamplesPerChannel << ", cap@" << maxBufferSamplesPerChannel);
        // Expose initial metrics and thresholds
        {
            std::string streamName = std::string("audio") + std::to_string(encChn);
            RTSPStatus::writeCustomParameter(streamName, "buffer_warn_samples_per_channel", std::to_string(warnBufferSamplesPerChannel));
            RTSPStatus::writeCustomParameter(streamName, "buffer_cap_samples_per_channel", std::to_string(maxBufferSamplesPerChannel));
            RTSPStatus::writeCustomParameter(streamName, "buffer_drop_count", std::to_string(bufferDropCount.load()));
        }
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

                // Hardware timestamps are already monotonic (from IMP_System_RebaseTimeStamp)
                // Use them directly - no conversion needed with 64-bit approach
                // DeepSeek's recommendation: use hardware timestamps as-is since they're already monotonic

                if (reframer)
                {
                    // SINGLE SOURCE OF TRUTH: Use TimestampManager timestamp
                    uint64_t unified_timestamp = TimestampManager::getInstance().getTimestampUs();
                    reframer->addFrame(reinterpret_cast<uint8_t *>(frame.virAddr), unified_timestamp);
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

    try {
        global_audio[encChn]->imp_audio = IMPAudio::createNew(global_audio[encChn]->devId,
                                                              global_audio[encChn]->aiChn,
                                                              global_audio[encChn]->aeChn);
        // Keep global devId in sync with actual initialized device (IMPAudio may adjust devId on some platforms)
        if (global_audio[encChn]->imp_audio && global_audio[encChn]->devId != global_audio[encChn]->imp_audio->devId) {
            LOG_INFO("AudioWorker: remapping devId from " << global_audio[encChn]->devId
                     << " to " << global_audio[encChn]->imp_audio->devId << " based on hardware init");
            global_audio[encChn]->devId = global_audio[encChn]->imp_audio->devId;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to initialize audio: " << e.what());
        global_audio[encChn]->imp_audio = nullptr;
        global_audio[encChn]->running = false;
        // inform main that initialization is complete (even though it failed)
        sh->has_started.release();
        return nullptr;
    }

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
