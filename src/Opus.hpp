#ifndef OPUS_ENCODER_HPP
#define OPUS_ENCODER_HPP

#include "IMPAudio.hpp"
#include <opus/opus.h>

class Opus : public IMPAudioEncoder
{
public:
    static Opus* createNew(int sampleRate, int numChn);

    Opus(int sampleRate, int numChn) : sampleRate(sampleRate), numChn(numChn)
    {
    };

    virtual ~Opus();

    int open() override;
    int encode(IMPAudioFrame *data, unsigned char *outbuf, int *outLen) override;
    int close() override;

private:
    int sampleRate;
    int numChn;
    OpusEncoder* encoder;
};

#endif // OPUS_ENCODER_HPP
