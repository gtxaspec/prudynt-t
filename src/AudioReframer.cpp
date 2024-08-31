#include "AudioReframer.hpp"
#include <stdexcept>

AudioReframer::AudioReframer(unsigned int inputSampleRate, unsigned int inputSamplesPerFrame, unsigned int outputSamplesPerFrame)
    : inputSampleRate(inputSampleRate),
      inputSamplesPerFrame(inputSamplesPerFrame),
      outputSamplesPerFrame(outputSamplesPerFrame),
      currentTimestamp(0),
      samplesAccumulated(0)
{
    if (inputSamplesPerFrame == 0 || outputSamplesPerFrame == 0)
    {
        throw std::invalid_argument("Number of samples per frame must be greater than zero.");
    }
}

void AudioReframer::addFrame(const std::vector<int16_t>& frameData, int64_t timestamp)
{
    if (frameData.size() != inputSamplesPerFrame)
    {
        throw std::invalid_argument("Frame data size must match input samples per frame.");
    }

    if (frameQueue.empty())
    {
        currentTimestamp = timestamp; // Initialize timestamp with the first frame
    }

    frameQueue.emplace_back(frameData);
    samplesAccumulated += frameData.size();
}

void AudioReframer::getReframedFrame(std::vector<int16_t>& outputFrame, int64_t& outputTimestamp)
{
    if (!hasMoreFrames())
    {
        throw std::runtime_error("Insufficient samples to generate a reframed output.");
    }

    outputFrame.clear();
    outputFrame.reserve(outputSamplesPerFrame);

    size_t totalSamples = 0;
    while (totalSamples < outputSamplesPerFrame && !frameQueue.empty())
    {
        auto& frameData = frameQueue.front();
        size_t remainingSamples = outputSamplesPerFrame - totalSamples;
        size_t samplesToCopy = std::min(remainingSamples, frameData.size());
        outputFrame.insert(outputFrame.end(), frameData.begin(), frameData.begin() + samplesToCopy);
        totalSamples += samplesToCopy;

        // Remove the samples we used from the current frame
        frameData.erase(frameData.begin(), frameData.begin() + samplesToCopy);

        // If the current frame is fully consumed, remove it from the queue
        if (frameData.empty())
        {
            frameQueue.pop_front();
        }
    }

    // Set the output timestamp based on the first sample used for the output frame
    outputTimestamp = currentTimestamp;

    // Update the timestamp for the next frame to be retrieved
    currentTimestamp += (outputSamplesPerFrame * 1000) / inputSampleRate;
    samplesAccumulated -= outputSamplesPerFrame;
}

bool AudioReframer::hasMoreFrames() const
{
    return samplesAccumulated >= outputSamplesPerFrame;
}
