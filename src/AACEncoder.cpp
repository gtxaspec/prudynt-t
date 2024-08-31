#include "AACEncoder.hpp"
#include "Config.hpp"
#include <cstdint>
#include "Logger.hpp"

AACEncoder* AACEncoder::createNew(int sampleRate, int numChn)
{
    return new AACEncoder(sampleRate, numChn);
}

AACEncoder::AACEncoder(int sampleRate, int numChn) : sampleRate(sampleRate), numChn(numChn)
{
}

AACEncoder::~AACEncoder()
{
    close();
}

int AACEncoder::open()
{
    unsigned long outputBufferSize;
    handle = faacEncOpen(sampleRate, numChn, &inputSamples, &outputBufferSize);
    if (!handle)
    {
        LOG_ERROR("Failed to open FAAC encoder");
        return -1;
    }

    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(handle);
    config->aacObjectType = LOW;
    config->bandWidth = sampleRate;
    config->bitRate = cfg->audio.input_bitrate * 1000;
    config->inputFormat = FAAC_INPUT_16BIT;
    config->mpegVersion = MPEG4;
    config->outputFormat = RAW_STREAM; // no need for ADTS headers

    // Disable to reduce CPU utilization
    config->allowMidside = 0;
    config->useTns = 0;

    if (!faacEncSetConfiguration(handle, config))
    {
        LOG_ERROR("Failed to configure FAAC encoder");
        return -1;
    }

    LOG_INFO("FAAC expects " << inputSamples << " samples per frame");

    return 0;
}

int AACEncoder::close()
{
    if (handle)
    {
        faacEncClose(handle);
    }
    handle = nullptr;
    return 0;
}

int AACEncoder::encode(IMPAudioFrame *data, unsigned char *outbuf, int *outLen)
{
    if (!handle)
    {
        LOG_ERROR("FAAC encoder not available");
        return -1;
    }

    const int frameSamples = data->len / sizeof(int16_t);
    if (frameSamples != inputSamples)
    {
        LOG_WARN("FAAC was provided with " << frameSamples << " samples per frame.");
    }
    const int frameLen = faacEncEncode(
        handle,
        reinterpret_cast<int32_t*>(data->virAddr),
        frameSamples,
        reinterpret_cast<unsigned char*>(outbuf),
        1024);
    *outLen = frameLen;
    if (frameLen == 0)
    {
        LOG_INFO("FAAC buffered frame: " <<  data->seq);
    }
    else if (frameLen < 0)
    {
        LOG_WARN("Encoding failed with error code: " << frameLen);
        return -1;
    }

    return 0;
}
