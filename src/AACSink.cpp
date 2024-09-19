#include "AACSink.hpp"
#include <iomanip>
#include <sstream>

static uint8_t identifySamplingFrequencyIndex(unsigned samplingFrequency)
{
    // https://wiki.multimedia.cx/index.php/MPEG-4_Audio#Sampling_Frequencies
    static constexpr size_t numFrequencies = 13;
    static constexpr unsigned frequencies[numFrequencies] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};

    for (unsigned i = 0; i < numFrequencies; ++i) {
        if (frequencies[i] == samplingFrequency) {
            return i;
        }
    }

    return 15; // Reserved if not found
}

static std::string generateConfig(unsigned samplingFrequency, unsigned numChannels)
{
    // See RFC 3640
    unsigned samplingFrequencyIndex = identifySamplingFrequencyIndex(samplingFrequency);
    constexpr uint8_t objectType = 2; // AAC-LC
    uint8_t channelConfiguration = static_cast<uint8_t>(numChannels);

    uint16_t combined = (objectType << 11) | (samplingFrequencyIndex << 7) | (channelConfiguration << 3);

    std::ostringstream configStream;
    configStream << std::hex << std::setw(4) << std::setfill('0') << combined;
    return configStream.str();
}

AACSink* AACSink::createNew(UsageEnvironment& env, Groupsock* RTPgs,
                            u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
                            unsigned numChannels)
{
    return new AACSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, numChannels);
}

AACSink::AACSink(UsageEnvironment& env, Groupsock* RTPgs,
                 u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
                 unsigned numChannels)
    : MPEG4GenericRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, "audio", "AAC-hbr",
                          config = strdup(generateConfig(rtpTimestampFrequency, numChannels).c_str()),
                          numChannels)
{
}

AACSink::~AACSink()
{
    free(config);
}
