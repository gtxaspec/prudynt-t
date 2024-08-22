#ifndef AAC_ENCODER_HPP
#define AAC_ENCODER_HPP

#include <faac.h>
#include <liveMedia.hh>

class AACEncoder : public FramedFilter {
public:
    static AACEncoder* createNew(UsageEnvironment& env, FramedSource* inputSource);

    virtual ~AACEncoder() override;

protected:
    AACEncoder(UsageEnvironment& env, FramedSource* inputSource);

private:
    void doGetNextFrame() override;
    void processFrame(unsigned frameSize, unsigned numTruncatedBytes,
                      struct timeval presentationTime, unsigned durationInMicroseconds);

    static void afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned durationInMicroseconds);

    unsigned char* outputBuffer;
    unsigned long outputBufferSize;
    unsigned long inputSamples;
    faacEncHandle faacEncoder;
};

#endif // AAC_ENCODER_HPP
