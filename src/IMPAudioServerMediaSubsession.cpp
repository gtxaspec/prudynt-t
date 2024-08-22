#include "AACEncoder.hpp"
#include "Config.hpp"
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

    if (strcmp(cfg->audio.output_format, "AAC") == 0) {
        return AACEncoder::createNew(envir(), audioSource);
    }
    else if (strcmp(cfg->audio.output_format, "PCM") != 0) {
        LOG_ERROR("unsupported audio->output_format (" << cfg->audio.output_format << "). we only support AAC and PCM");
    }

    return EndianSwap16::createNew(envir(), audioSource);
}

RTPSink* IMPAudioServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    if (strcmp(cfg->audio.output_format, "AAC") == 0) {
        return MPEG4GenericRTPSink::createNew(
            envir(), rtpGroupsock,
            /* rtpPayloadFormat */ 96,
            /* rtpTimestampFrequency */ 16000,
            /* sdpMediaTypeString*/ "audio",
            /* rtpPayloadFormatName */ "aac-hbr",
            /* configurationString */ "",
            /* numChannels */ 1);
    }

    return SimpleRTPSink::createNew(
        envir(), rtpGroupsock,
        /* rtpPayloadFormat */ rtpPayloadTypeIfDynamic,
        /* rtpTimestampFrequency */ 16000,
        /* sdpMediaTypeString*/ "audio",
        /* rtpPayloadFormatName */ "L16",
        /* numChannels */ 1);
}
