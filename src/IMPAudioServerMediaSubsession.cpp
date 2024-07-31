#include "IMPAudioServerMediaSubsession.hpp"
#include "SimpleRTPSink.hh"
#include "GroupsockHelper.hh"

IMPAudioServerMediaSubsession* IMPAudioServerMediaSubsession::createNew(
    UsageEnvironment& env,
    int audioChn)
{
    return new IMPAudioServerMediaSubsession(env, audioChn);
}

IMPAudioServerMediaSubsession::IMPAudioServerMediaSubsession(
    UsageEnvironment& env,
    int audioChn)
    : OnDemandServerMediaSubsession(env, true),
      audioChn(audioChn)
{
}

IMPAudioServerMediaSubsession::~IMPAudioServerMediaSubsession()
{
}

FramedSource* IMPAudioServerMediaSubsession::createNewStreamSource(
    unsigned clientSessionId,
    unsigned& estBitrate)
{
    estBitrate = cfg->audio.input_bitrate; 
    IMPAudioDeviceSource* audioSource = IMPAudioDeviceSource::createNew(envir(), audioChn);
    return audioSource;
}

RTPSink* IMPAudioServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    return SimpleRTPSink::createNew(
        envir(), rtpGroupsock,
        /* rtpPayloadFormat */ rtpPayloadTypeIfDynamic,
        /* rtpTimestampFrequency */ 16000,
        /* sdpMediaTypeString*/ "audio",
        /* rtpPayloadFormatName */ "L16",
        /* numChannels */ 1);    
}
