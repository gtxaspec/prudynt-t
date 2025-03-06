#include "AudioReframer.hpp"
#include <algorithm>
#include <stdexcept>
#include <chrono>

AudioReframer::AudioReframer(unsigned int inputSampleRate, unsigned int inputSamplesPerFrame, unsigned int outputSamplesPerFrame)
    : inputSampleRate(inputSampleRate),
      inputSamplesPerFrame(inputSamplesPerFrame),
      outputSamplesPerFrame(outputSamplesPerFrame),
      currentTimestamp(0),
      samplesAccumulated(0),
      buffer(2 * std::max(inputSamplesPerFrame, outputSamplesPerFrame) * sizeof(uint16_t)),
      base_timestamp(0),
      timestamp_initialized(false),
      resetCounter(0)
{
    if (inputSamplesPerFrame == 0 || outputSamplesPerFrame == 0)
    {
        throw std::invalid_argument("Number of samples per frame must be greater than zero.");
    }
}

void AudioReframer::addFrame(const uint8_t* frameData, int64_t timestamp)
{
    if (frameData == nullptr)
    {
        throw std::invalid_argument("Frame data cannot be null.");
    }

    // Reset the timestamp base periodically to prevent long-term drift
    // This helps maintain synchronization with video over time
    if (resetCounter >= 300) { // Reset roughly every 300 frames
        timestamp_initialized = false;
        resetCounter = 0;
    }

    size_t inputFrameSize = inputSamplesPerFrame * sizeof(uint16_t);
    buffer.push(frameData, inputFrameSize);

    // For the first frame or after a reset, initialize timestamps to start at zero
    if (!timestamp_initialized) {
        currentTimestamp = 0; // Start at zero
        base_timestamp = timestamp;
        timestamp_initialized = true;
    }

    resetCounter++;
    samplesAccumulated += inputSamplesPerFrame;
}

void AudioReframer::getReframedFrame(uint8_t* frameData, int64_t& timestamp)
{
    if (!hasMoreFrames())
    {
        throw std::runtime_error("Insufficient samples to generate a reframed output.");
    }

    if (frameData == nullptr) {
        throw std::invalid_argument("Output frame cannot be null.");
    }

    size_t outputFrameSize = outputSamplesPerFrame * sizeof(uint16_t);
    buffer.fetch(frameData, outputFrameSize);
    samplesAccumulated -= outputSamplesPerFrame;

    // Return a clean timestamp that is a multiple of frame duration
    // This ensures consistent intervals between audio frames
    timestamp = currentTimestamp;

    // Calculate frame duration with high precision in microseconds
    // This is critical for maintaining audio/video sync
    int64_t frameDuration = (int64_t)((double)outputSamplesPerFrame * 1000000.0 / (double)inputSampleRate);
    
    // Update the timestamp for the next frame
    currentTimestamp += frameDuration;
}

bool AudioReframer::hasMoreFrames() const
{
    return samplesAccumulated >= outputSamplesPerFrame;
}

