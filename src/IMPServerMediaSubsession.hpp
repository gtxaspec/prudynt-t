#ifndef IMPServerMediaSubsession_hpp
#define IMPServerMediaSubsession_hpp

#include "Config.hpp"
#include "worker.hpp"
#include "StreamReplicator.hh"
#include "ServerMediaSession.hh"
#include "OnDemandServerMediaSubsession.hh"

class IMPServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static void init(const std::shared_ptr<CFG> &_cfg)
    {
        cfg = _cfg;
    };

    static IMPServerMediaSubsession *createNew(
        UsageEnvironment &env,
        H264NALUnit *vps, // Change to pointer for optional VPS
        H264NALUnit sps,
        H264NALUnit pps,
        int encChn);

protected:
    // Constructor with VPS as a pointer for optional usage
    IMPServerMediaSubsession(
        UsageEnvironment &env,
        H264NALUnit *vps, // Change to pointer for optional VPS
        H264NALUnit sps,
        H264NALUnit pps,
        int encChn);
    virtual ~IMPServerMediaSubsession();

protected:
    virtual FramedSource *createNewStreamSource(
        unsigned clientSessionId,
        unsigned &estBitrate);
    virtual RTPSink *createNewRTPSink(
        Groupsock *rtpGroupsock,
        unsigned char rtpPayloadTypeIfDynamic,
        FramedSource *inputSource);

private:
    StreamReplicator *replicator;
    H264NALUnit *vps; // Change to pointer for optional VPS
    H264NALUnit sps;
    H264NALUnit pps;
    static std::shared_ptr<CFG> cfg;
    int encChn;
};

#endif
