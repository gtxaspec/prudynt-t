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
    static void init(){};

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

    virtual FramedSource *createNewStreamSource(
        unsigned clientSessionId,
        unsigned &estBitrate);
    virtual RTPSink *createNewRTPSink(
        Groupsock *rtpGroupsock,
        unsigned char rtpPayloadTypeIfDynamic,
        FramedSource *inputSource);

    virtual void startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler,
                             void* rtcpRRHandlerClientData, unsigned short& rtpSeqNum, unsigned& rtpTimestamp,
                             ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
                             void* serverRequestAlternativeByteHandlerClientData) override {

        OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken, rtcpRRHandler, rtcpRRHandlerClientData,
                                                   rtpSeqNum, rtpTimestamp, serverRequestAlternativeByteHandler,
                                                   serverRequestAlternativeByteHandlerClientData);
        
        //request idr frame every second for the next x seconds
        global_video[encChn]->idr_fix = 5; 
        IMPEncoder::flush(encChn);
    }
private:
    StreamReplicator *replicator;
    H264NALUnit *vps; // Change to pointer for optional VPS
    H264NALUnit sps;
    H264NALUnit pps;
    int encChn;
};

#endif
