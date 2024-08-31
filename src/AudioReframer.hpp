#ifndef AUDIO_REFRAMER_HPP
#define AUDIO_REFRAMER_HPP

#include <vector>
#include <deque>
#include <cstdint>

class AudioReframer
{
public:
    AudioReframer(unsigned int inputSampleRate, unsigned int inputSamplesPerFrame, unsigned int outputSamplesPerFrame);

    void addFrame(const std::vector<int16_t>& frameData, int64_t timestamp);

    void getReframedFrame(std::vector<int16_t>& outputFrame, int64_t& outputTimestamp);

    bool hasMoreFrames() const;

private:
    unsigned int inputSampleRate;
    unsigned int inputSamplesPerFrame;
    unsigned int outputSamplesPerFrame;
    int64_t currentTimestamp;
    size_t samplesAccumulated;

    using Frame = std::vector<int16_t>;
    std::deque<Frame> frameQueue;
};

#endif // AUDIO_REFRAMER_HPP
