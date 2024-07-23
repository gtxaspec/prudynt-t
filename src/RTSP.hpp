#ifndef RTSP_hpp
#define RTSP_hpp

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "IMPServerMediaSubsession.hpp"
#include "IMPDeviceSource.hpp"
#include "IMPAudioDeviceSource.hpp"
#include "IMPEncoder.hpp"
#include "Logger.hpp"

class RTSP
{
public:
    RTSP(std::shared_ptr<CFG> _cfg) : cfg(std::move(_cfg)){};
    void addSubsession(int chnNr, _stream &stream);
    void run();

private:
    UsageEnvironment *env{};
    TaskScheduler *scheduler{};
    RTSPServer *rtspServer{};
    std::shared_ptr<CFG> cfg;
};

#endif
