#include "Config.hpp"
#include "Logger.hpp"
#include "Opus.hpp"

static constexpr unsigned SAMPLE_RATE = 16000;
static constexpr unsigned CHANNELS = 1;

Opus* Opus::createNew(UsageEnvironment& env, FramedSource* inputSource) {
    return new Opus(env, inputSource);
}

Opus::Opus(UsageEnvironment& env, FramedSource* inputSource)
    : FramedFilter(env, inputSource) {

    int opusError;
    opusEncoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &opusError);
    if (opusError != OPUS_OK) {
        LOG_ERROR("Failed to create Opus encoder: " << opus_strerror(opusError));
	return;
    }

    opusError = opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(cfg->audio.output_bitrate));
    if (opusError != OPUS_OK) {
        LOG_ERROR("Failed to set bitrate for Opus encoder: " << opus_strerror(opusError));
    }

    int bitrate;
    opusError = opus_encoder_ctl(opusEncoder, OPUS_GET_BITRATE(&bitrate));
    if (opusError != OPUS_OK) {
        LOG_ERROR("Failed to get bitrate from Opus encoder: " << opus_strerror(opusError));
    } else {
        LOG_INFO("Opus encoder bitrate set to: " << bitrate);
    }
}

Opus::~Opus() {
    if (opusEncoder) {
        opus_encoder_destroy(opusEncoder);
    }
}

void Opus::doGetNextFrame() {
    fInputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this, FramedSource::handleClosure, this);
}

void Opus::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                             struct timeval presentationTime, unsigned durationInMicroseconds) {
    auto* encoder = static_cast<Opus*>(clientData);
    encoder->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void Opus::processFrame(unsigned frameSize, unsigned numTruncatedBytes,
                        struct timeval presentationTime, unsigned durationInMicroseconds) {

    unsigned numSamples = frameSize / sizeof(int16_t);
    opus_int32 bytesEncoded = opus_encode(opusEncoder, reinterpret_cast<opus_int16*>(fTo),
                                          numSamples, outputBuffer, BUFFER_SIZE);
    if (bytesEncoded < 0) {
        LOG_WARN("Opus encoding failed with error: " << opus_strerror(bytesEncoded));
        return;
    }

    std::memcpy(fTo, outputBuffer, bytesEncoded);

    fFrameSize = bytesEncoded;
    fNumTruncatedBytes = numTruncatedBytes;
    fPresentationTime = presentationTime;
    fDurationInMicroseconds = numSamples * 1000000 / SAMPLE_RATE;

    afterGetting(this);
}
