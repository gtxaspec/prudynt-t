#ifndef IMPAudioServerMediaSubsession_hpp
#define IMPAudioServerMediaSubsession_hpp

#include "OnDemandServerMediaSubsession.hh"

class IMPAudioServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static IMPAudioServerMediaSubsession* createNew(
        UsageEnvironment& env,
        int audioChn,
        StreamReplicator* streamReplicator);

protected:
    IMPAudioServerMediaSubsession(
        UsageEnvironment& env,
        int audioChn,
        StreamReplicator* streamReplicator);
    virtual ~IMPAudioServerMediaSubsession();

    virtual FramedSource* createNewStreamSource(
        unsigned clientSessionId,
        unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(
        Groupsock* rtpGroupsock,
        unsigned char rtpPayloadTypeIfDynamic,
        FramedSource* inputSource);

private:
    int audioChn;
    StreamReplicator* streamReplicator;
};

#endif // IMPAudioServerMediaSubsession_hpp
