#ifndef IMPAudioServerMediaSubsession_hpp
#define IMPAudioServerMediaSubsession_hpp

#include "OnDemandServerMediaSubsession.hh"
#include "IMPAudioDeviceSource.hpp"

class IMPAudioServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    static IMPAudioServerMediaSubsession* createNew(
        UsageEnvironment& env,
        int audioChn);

protected:
    IMPAudioServerMediaSubsession(
        UsageEnvironment& env,
        int audioChn);
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
};

#endif // IMPAudioServerMediaSubsession_hpp