#ifndef AUDIO_REFRAMER_HPP
#define AUDIO_REFRAMER_HPP

#include <cstdint>
#include <cstddef>
#include "RingBuffer.hpp"

class AudioReframer
{
public:
    AudioReframer(unsigned int inputSampleRate, unsigned int inputSamplesPerFrame, unsigned int outputSamplesPerFrame);

    void addFrame(const uint8_t* frameData, int64_t timestamp);

    void getReframedFrame(uint8_t* frameData, int64_t& timestamp);

    bool hasMoreFrames() const;

private:
    unsigned int inputSampleRate;
    unsigned int inputSamplesPerFrame;
    unsigned int outputSamplesPerFrame;
    int64_t currentTimestamp;
    size_t samplesAccumulated;

    RingBuffer buffer;
};

#endif // AUDIO_REFRAMER_HPP

