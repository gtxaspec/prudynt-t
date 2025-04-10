#ifndef AUDIO_WORKER_HPP
#define AUDIO_WORKER_HPP

#include "AudioReframer.hpp"
#include "IMPAudio.hpp"

#include <memory>

#if defined(AUDIO_SUPPORT)

class AudioWorker
{
public:
    explicit AudioWorker(int encChn);
    ~AudioWorker();

    static void *thread_entry(void *arg);

private:
    void run();
    void process_audio_frame(IMPAudioFrame &frame);
    void process_frame(IMPAudioFrame &frame);

    int encChn;
    std::unique_ptr<AudioReframer> reframer;
};

#endif // AUDIO_SUPPORT
#endif // AUDIO_WORKER_HPP
