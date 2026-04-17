#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Config.hpp"
#include <imp/imp_audio.h>
#include "Logger.hpp"
#include <stdexcept>

enum IMPAudioFormat
{
    PCM,
    G711A,
    G711U,
    G726,
    OPUS,
    AAC,
};

class IMPAudioEncoder
{
public:
    virtual int open() = 0;
    virtual int encode(IMPAudioFrame* data, unsigned char* outbuf, int* outLen) = 0;
    virtual int close() = 0;
    virtual ~IMPAudioEncoder() = default;
};

class IMPAudio
{
public:
    static IMPAudio *createNew(int devId, int inChn, int aeChn);

    IMPAudio(int devId, int inChn, int aeChn) : devId(devId), inChn(inChn), aeChn(aeChn)
    {
        if (init() != 0) {
            throw std::runtime_error("Failed to initialize IMPAudio - hardware may not be properly initialized");
        }
    };

    ~IMPAudio()
    {
        deinit();
    };

    int init();
    int deinit();
    int bitrate;    // computed during setup, in Kbps
    int sample_rate;
    IMPAudioFormat format;

    int devId{};
    int inChn{};
    int aeChn{};
    int outChnCnt = 1;

private:
    bool enabledAgc = false;
    bool enabledHpf = false;
    bool enabledNs = false;
    int handle = 0;
    const char *name{};
    _stream *stream{};
};

#endif
