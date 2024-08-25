#ifndef OPUS_ENCODER_HPP
#define OPUS_ENCODER_HPP

#include <liveMedia.hh>
#include <opus/opus.h>

class Opus : public FramedFilter {
public:
    static Opus* createNew(UsageEnvironment& env, FramedSource* inputSource);
    
protected:
    Opus(UsageEnvironment& env, FramedSource* inputSource);
    virtual ~Opus();
    
private:
    virtual void doGetNextFrame() override;
    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes, struct timeval presentationTime,
                                  unsigned durationInMicroseconds);

    void processFrame(unsigned frameSize, unsigned numTruncatedBytes,
                      struct timeval presentationTime, unsigned durationInMicroseconds);

    static constexpr unsigned BUFFER_SIZE = 960;
    OpusEncoder* opusEncoder;
    unsigned char outputBuffer[BUFFER_SIZE];
};

#endif // OPUS_ENCODER_HPP
