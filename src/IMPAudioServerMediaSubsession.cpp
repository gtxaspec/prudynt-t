#include "globals.hpp"
#include "liveMedia.hh"
#include "IMPDeviceSource.hpp"
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
    estBitrate = global_audio[audioChn]->imp_audio->bitrate;
    IMPDeviceSource<AudioFrame, audio_stream> * audioSource = IMPDeviceSource<AudioFrame, audio_stream> ::createNew(envir(), audioChn, global_audio[audioChn], "audio");

    //return audioSource;

    FramedSource* endianSwapSource = EndianSwap16::createNew(envir(), audioSource);
    return endianSwapSource;
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
