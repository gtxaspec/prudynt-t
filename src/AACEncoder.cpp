#include "AACEncoder.hpp"
#include "Config.hpp"
#include "Logger.hpp"

static constexpr unsigned SAMPLE_RATE = 16000;
static constexpr unsigned CHANNELS = 1;
static constexpr unsigned AAC_BUFFER_SIZE = 1280 * 3;

AACEncoder* AACEncoder::createNew(UsageEnvironment& env, FramedSource* inputSource) {
    return new AACEncoder(env, inputSource);
}

AACEncoder::AACEncoder(UsageEnvironment& env, FramedSource* inputSource)
    : FramedFilter(env, inputSource), inputSamples(0), outputBufferSize(AAC_BUFFER_SIZE),
      faacEncoder(faacEncOpen(SAMPLE_RATE, CHANNELS, &inputSamples, &outputBufferSize)) {

    outputBuffer = new unsigned char[outputBufferSize];

    if (!faacEncoder) {
        LOG_ERROR("Failed to open FAAC encoder");
        return;
    }

    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(faacEncoder);
    config->aacObjectType = LOW;
    config->bandWidth = SAMPLE_RATE;
    config->bitRate = cfg->audio.output_bitrate;
    config->inputFormat = FAAC_INPUT_16BIT;
    config->mpegVersion = MPEG4;
    config->outputFormat = ADTS_STREAM;

    // Disable to reduce CPU utilization
    config->allowMidside = 0;
    config->useTns = 0;

    if (!faacEncSetConfiguration(faacEncoder, config)) {
        LOG_ERROR("Failed to configure FAAC encoder");
	return;
    }
}

AACEncoder::~AACEncoder() {
    delete outputBuffer;
    if (faacEncoder) {
        faacEncClose(faacEncoder);
    }
}

void AACEncoder::doGetNextFrame() {
    fInputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this, FramedSource::handleClosure, this);
}

void AACEncoder::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                   struct timeval presentationTime, unsigned durationInMicroseconds) {
    auto* encoder = static_cast<AACEncoder*>(clientData);
    encoder->processFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void AACEncoder::processFrame(unsigned frameSize, unsigned numTruncatedBytes,
                              struct timeval presentationTime, unsigned durationInMicroseconds) {
    if (!faacEncoder) {
        LOG_ERROR("FAAC encoder not available");
	return;
    }

    unsigned numSamples = frameSize / sizeof(int16_t);

    int bytesEncoded = faacEncEncode(
        faacEncoder,
	reinterpret_cast<int32_t*>(fTo),
	numSamples,
	outputBuffer,
	outputBufferSize);
    if (bytesEncoded < 0) {
        LOG_WARN("Encoding failed with error code: " << bytesEncoded);
        return;
    }

    std::memcpy(fTo, outputBuffer, bytesEncoded);

    fFrameSize = bytesEncoded;
    fNumTruncatedBytes = numTruncatedBytes;
    fPresentationTime = presentationTime;
    fDurationInMicroseconds = numSamples * 1000000 / SAMPLE_RATE;

    afterGetting(this);
}
