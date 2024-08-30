#include "AACSink.hpp"
#include "globals.hpp"
#include "GroupsockHelper.hh"
#include "liveMedia.hh"
#include "IMPAudio.hpp"
#include "IMPDeviceSource.hpp"
#include "IMPAudioServerMediaSubsession.hpp"
#include "SimpleRTPSink.hh"

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

    if (global_audio[audioChn]->imp_audio->format == IMPAudioFormat::PCM)
        return EndianSwap16::createNew(envir(), audioSource);

    return audioSource;
}

RTPSink* IMPAudioServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    unsigned rtpPayloadFormat = rtpPayloadTypeIfDynamic;
    unsigned rtpTimestampFrequency = global_audio[audioChn]->imp_audio->sample_rate;
    const char *rtpPayloadFormatName;
    bool allowMultipleFramesPerPacket = true;
    switch (global_audio[audioChn]->imp_audio->format)
    {
    case IMPAudioFormat::PCM:
        rtpPayloadFormatName = "L16";
        break;
    case IMPAudioFormat::G711A:
        rtpPayloadFormat = 8;
        rtpPayloadFormatName = "PCMA";
        break;
    case IMPAudioFormat::G711U:
        rtpPayloadFormat = 0;
        rtpPayloadFormatName = "PCMU";
        break;
    case IMPAudioFormat::G726:
        rtpPayloadFormatName = "G726-16";
        break;
    case IMPAudioFormat::OPUS:
        rtpTimestampFrequency = 48000;
        rtpPayloadFormatName = "OPUS";
        allowMultipleFramesPerPacket = false;
        break;
    case IMPAudioFormat::AAC:
        return AACSink::createNew(
            envir(), rtpGroupsock, rtpPayloadFormat, rtpTimestampFrequency,
            rtpTimestampFrequency,
            /* numChannels */ 1);
    }

    return SimpleRTPSink::createNew(
        envir(), rtpGroupsock, rtpPayloadFormat, rtpTimestampFrequency,
        /* sdpMediaTypeString*/ "audio",
        rtpPayloadFormatName,
        /* numChannels */ 1,
        allowMultipleFramesPerPacket);
}
