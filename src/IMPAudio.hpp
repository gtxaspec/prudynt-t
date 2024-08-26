#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <imp/imp_audio.h>

enum IMPAudioFormat
{
    PCM,
    G711A,
    G711U,
    G726
};

class IMPAudio
{
public:
    static IMPAudio *createNew(int devId, int inChn, int aeChn);

    IMPAudio(int devId, int inChn, int aeChn) : devId(devId), inChn(inChn), aeChn(aeChn)
    {
        init();
    };

    ~IMPAudio(){
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
    
    const char *name{};
    _stream *stream{};
};

#endif
