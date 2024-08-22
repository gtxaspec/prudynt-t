#include "Config.hpp"
#include "Logger.hpp"
#include "Opus.hpp"

Opus* Opus::createNew(int sampleRate, int numChn)
{
    return new Opus(sampleRate, numChn);
}

Opus::~Opus()
{
    close();
}

int Opus::open()
{
    int opusError;

    encoder = opus_encoder_create(sampleRate, numChn, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &opusError);
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to create Opus encoder: " << opus_strerror(opusError));
        return -1;
    }

    int bitrate = cfg->audio.input_bitrate * 1000;
    opusError = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to set bitrate (" << bitrate << ") for Opus encoder: " << opus_strerror(opusError));
    }

    opusError = opus_encoder_ctl(encoder, OPUS_GET_BITRATE(&bitrate));
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to get bitrate from Opus encoder: " << opus_strerror(opusError));
        return -1;
    }

    LOG_INFO("Encoder bitrate: " << bitrate);

    return 0;
}

int Opus::close()
{
    if (encoder)
    {
        opus_encoder_destroy(encoder);
    }
    encoder = nullptr;
    return 0;
}

int Opus::encode(IMPAudioFrame *data, unsigned char *outbuf, int *outLen)
{
    opus_int32 bytesEncoded = opus_encode(
        encoder,
        reinterpret_cast<const opus_int16*>(data->virAddr),
        data->len / sizeof(int16_t),
        reinterpret_cast<unsigned char*>(outbuf),
        1024);

    if (bytesEncoded < 0)
    {
        LOG_WARN("Encoding failed with error code: " << *outLen);
        return -1;
    }

    *outLen = bytesEncoded;

    return 0;
}
