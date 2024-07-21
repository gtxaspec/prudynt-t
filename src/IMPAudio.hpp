#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <imp/imp_audio.h>

class IMPAudio
{
public:
    static IMPAudio *createNew(_stream *stream, std::shared_ptr<CFG> cfg, int inChn, int devId, const char *name);

    IMPAudio(_stream *stream, std::shared_ptr<CFG> cfg, int inChn, int devId, const char *name) : stream(stream), cfg(cfg), inChn(inChn), devId(devId), name(name)
    {
        init();
    };

    ~IMPAudio(){
        destroy();
    };

    int init();
    int deinit();
    int destroy();

private:
    
    const char *name{};
    std::shared_ptr<CFG> cfg{};
    _stream *stream{};
    int inChn{};
    int devId{};
};

#endif