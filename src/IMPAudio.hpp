#ifndef IMPAudio_hpp
#define IMPAudio_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <imp/imp_audio.h>

class IMPAudio
{
public:
    static IMPAudio *createNew(std::shared_ptr<CFG> cfg,  int devId, int inChn);

    IMPAudio(std::shared_ptr<CFG> cfg, int devId, int inChn) : cfg(cfg), devId(devId), inChn(inChn)
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
    std::shared_ptr<CFG> cfg{};
    _stream *stream{};
};

#endif