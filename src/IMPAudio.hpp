#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Config.hpp"
#include <imp/imp_audio.h>
#include "Logger.hpp"

enum IMPAudioFormat
{
    PCM,
    G711A,
    G711U,
    G726,
    OPUS,
};

class IMPAudioEncoder
{
public:
    virtual int open() = 0;
    virtual int encode(IMPAudioFrame* data, unsigned char* outbuf, int* outLen) = 0;
    virtual int close() = 0;
};

class IMPAudio
{
public:
    static IMPAudio *createNew(int devId, int inChn, int aeChn);

    IMPAudio(int devId, int inChn, int aeChn) : devId(devId), inChn(inChn), aeChn(aeChn)
    {
        init();
    };

    ~IMPAudio()
    {
        deinit();
    };

    int init();
    int deinit();
    int bitrate;    // computed during setup, in Kbps
    IMPAudioFormat format;

    int inChn{};
    int aeChn{};
    int devId{};

private:
    int handle = 0;
    const char *name{};
    _stream *stream{};
};

#endif
