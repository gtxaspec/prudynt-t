#include "AACSink.hpp"
#include "globals.hpp"
#include "GroupsockHelper.hh"
#include "liveMedia.hh"
#include "IMPAudio.hpp"
#include "IMPDeviceSource.hpp"
#include "IMPAudioServerMediaSubsession.hpp"
#include "SimpleRTPSink.hh"
#include <cstdio>
#include <cstring>

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
    LOG_INFO("IMPAudioServerMediaSubsession init");
}

IMPAudioServerMediaSubsession::~IMPAudioServerMediaSubsession()
{
    if (fAuxSDPLine) {
        delete[] fAuxSDPLine;
        fAuxSDPLine = nullptr;
    }
}

#if defined(USE_AUDIO_STREAM_REPLICATOR)
FramedSource* IMPAudioServerMediaSubsession::createNewStreamSource(
    unsigned clientSessionId,
    unsigned& estBitrate)
{
    estBitrate = global_audio[audioChn]->imp_audio->bitrate;
    FramedSource* audioSourceReplica = global_audio[audioChn]->streamReplicator->createStreamReplica();
    return audioSourceReplica;
}
#else
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
#endif

RTPSink* IMPAudioServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    unsigned rtpPayloadFormat = rtpPayloadTypeIfDynamic;
    unsigned rtpTimestampFrequency = global_audio[audioChn]->imp_audio->sample_rate;
    const char* rtpPayloadFormatName = "L16";
    bool allowMultipleFramesPerPacket = true;
    int outChnCnt = cfg->audio.force_stereo ? 2 : 1;
    switch (global_audio[audioChn]->imp_audio->format)
    {
    case IMPAudioFormat::PCM:
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
        // RFC 7587: Opus over RTP MUST use a 48 kHz RTP timestamp clock, regardless of
        // the encoder's input sample rate. With unified 20 ms packetization, each RTP
        // timestamp MUST advance by 960 ticks (0.020 * 48000) per packet.
        // Application-level frames (IMPAudio/AudioWorker) use wall-clock PTS based on
        // the actual input sample rate for accumulation and pacing; live555 maps these
        // to RTP timestamps using the frequency specified below (48000).
        rtpTimestampFrequency = 48000;
        rtpPayloadFormatName = "OPUS";
        allowMultipleFramesPerPacket = false; // one Opus packet per RTP packet
        outChnCnt = cfg->audio.force_stereo ? 2 : 1;
        break;
    case IMPAudioFormat::AAC:
        return AACSink::createNew(
            envir(), rtpGroupsock, rtpPayloadFormat, rtpTimestampFrequency,
            /* numChannels */ outChnCnt);
    }

    LOG_DEBUG("createNewRTPSink: " << rtpPayloadFormatName << ", " << rtpTimestampFrequency);

    // RFC 7587 (SDP Considerations): a=rtpmap MUST use 48000/2 for Opus
    // Always advertise 2 channels in SDP for Opus, regardless of actual encoded channels.
    int sdpChannels = outChnCnt;
    if (global_audio[audioChn]->imp_audio->format == IMPAudioFormat::OPUS) {
        sdpChannels = 2;
    }

    return SimpleRTPSink::createNew(
        envir(), rtpGroupsock, rtpPayloadFormat, rtpTimestampFrequency,
        /* sdpMediaTypeString*/ "audio",
        rtpPayloadFormatName,
        /* numChannels */ sdpChannels,
        allowMultipleFramesPerPacket);
}

char const* IMPAudioServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    // Provide Opus fmtp parameters explicitly when using SimpleRTPSink
    if (global_audio[audioChn]->imp_audio->format == IMPAudioFormat::OPUS && rtpSink != nullptr) {
        unsigned pt = rtpSink->rtpPayloadType();
        unsigned maxavg = (unsigned)(cfg->audio.input_bitrate * 1000); // bps
        char buf[256];
        int n = snprintf(buf, sizeof(buf),
                         "a=fmtp:%u stereo=0; sprop-stereo=0; maxplaybackrate=48000; maxaveragebitrate=%u\r\n",
                         pt, maxavg);
        if (n > 0) {
            if (fAuxSDPLine) { delete[] fAuxSDPLine; fAuxSDPLine = nullptr; }
            fAuxSDPLine = new char[(size_t)n + 1];
            memcpy(fAuxSDPLine, buf, (size_t)n + 1);
            return fAuxSDPLine;
        }
    }
    return OnDemandServerMediaSubsession::getAuxSDPLine(rtpSink, inputSource);
}
