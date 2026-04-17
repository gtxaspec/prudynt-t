#ifndef AUDIO_WORKER_HPP
#define AUDIO_WORKER_HPP

#include "AudioReframer.hpp"
#include "IMPAudio.hpp"

#include <memory>
#include <vector>
#include <atomic>

#if defined(AUDIO_SUPPORT)

class AudioWorker
{
public:
    explicit AudioWorker(int encChn);
    ~AudioWorker();

    static void *thread_entry(void *arg);

private:
    void run();
    void process_audio_frame_direct(IMPAudioFrame &frame);
    void process_frame(IMPAudioFrame &frame);

    int encChn;
    std::unique_ptr<AudioReframer> reframer;

    // Frame accumulator for Opus
    std::vector<int16_t> frameBuffer;
    int64_t bufferStartTimestamp = 0;
    int targetSamplesPerChannel = 0;

    // Buffer safety controls (computed from targetSamplesPerChannel)
    int maxBufferSamplesPerChannel = 0;     // e.g., 5 * targetSamplesPerChannel
    int warnBufferSamplesPerChannel = 0;    // e.g., 3 * targetSamplesPerChannel

    // Diagnostics / metrics
    std::atomic<uint32_t> bufferDropCount{0};
};

#endif // AUDIO_SUPPORT
#endif // AUDIO_WORKER_HPP
