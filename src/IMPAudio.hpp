#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <imp/imp_audio.h>

class IMPAudio
{
public:
    static IMPAudio *createNew(int devId, int inChn);

    IMPAudio(int devId, int inChn) : devId(devId), inChn(inChn)
    {
        init();
    };

    ~IMPAudio(){
        destroy();
    };

    int init();
    int deinit();
    int destroy();

    int inChn{};
    int devId{};
private:
    
    const char *name{};
    _stream *stream{};
};

#endif