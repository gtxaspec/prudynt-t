#include "RTSP.hpp"
#include "IMPBackchannel.hpp"
#include "BackchannelServerMediaSubsession.hpp"
#include "GroupsockHelper.hh"
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>

#undef MODULE
#define MODULE "RTSP"

void RTSP::addSubsession(int chnNr, _stream &stream)
{

    LOG_DEBUG("identify stream " << chnNr);
    auto deviceSource = IMPDeviceSource<H264NALUnit,video_stream>::createNew(*env, chnNr, global_video[chnNr], "video/pps/sps/vps");
    H264NALUnit sps;
    H264NALUnit pps;
    H264NALUnit *vps = nullptr;
    bool have_pps = false;
    bool have_sps = false;
    bool have_vps = false;
    bool is_h265 = strcmp(stream.format, "H265") == 0 ? true : false;
    // Read from the stream until we capture the SPS and PPS. Only capture VPS if needed.
    while (!have_pps || !have_sps || (is_h265 && !have_vps))
    {
        H264NALUnit unit = global_video[chnNr]->msgChannel->wait_read();
        if (is_h265)
        {
            uint8_t nalType = (unit.data[0] & 0x7E) >> 1; // H265 NAL unit type extraction
            if (nalType == 33)
            { // SPS for H265
                LOG_DEBUG("Got SPS (H265)");
                sps = unit;
                have_sps = true;
            }
            else if (nalType == 34)
            { // PPS for H265
                LOG_DEBUG("Got PPS (H265)");
                pps = unit;
                have_pps = true;
            }
            else if (nalType == 32)
            { // VPS, only for H265
                LOG_DEBUG("Got VPS");
                if (!vps)
                    vps = new H264NALUnit(unit); // Allocate and store VPS
                have_vps = true;
            }
        }
        else
        {                                            // Assuming H264 if not H265
            uint8_t nalType = (unit.data[0] & 0x1F); // H264 NAL unit type extraction
            if (nalType == 7)
            { // SPS for H264
                LOG_DEBUG("Got SPS (H264)");
                sps = unit;
                have_sps = true;
            }
            else if (nalType == 8)
            { // PPS for H264
                LOG_DEBUG("Got PPS (H264)");
                pps = unit;
                have_pps = true;
            }
            // No VPS in H264, so no need to check for it
        }
    }
    //deviceSource->deinit();
    delete deviceSource;
    LOG_DEBUG("Got necessary NAL Units.");

    ServerMediaSession *sms = ServerMediaSession::createNew(
        *env, stream.rtsp_endpoint, stream.rtsp_info, cfg->rtsp.name);
    IMPServerMediaSubsession *sub = IMPServerMediaSubsession::createNew(
        *env, (is_h265 ? vps : nullptr), sps, pps, chnNr // Conditional VPS
    );

    sms->addSubsession(sub);

#if defined(AUDIO_SUPPORT)
    if (cfg->audio.input_enabled && stream.audio_enabled) {
        IMPAudioServerMediaSubsession *audioSub = IMPAudioServerMediaSubsession::createNew(*env, 0);
        sms->addSubsession(audioSub);
        LOG_INFO("Audio stream " << chnNr << " added to session");
    }

    if (cfg->audio.output_enabled && stream.audio_enabled)
    {
        #define ADD_BACKCHANNEL_SUBSESSION(EnumName, NameString, PayloadType, Frequency, MimeType) \
            { \
                BackchannelServerMediaSubsession* backchannelSub = \
                    BackchannelServerMediaSubsession::createNew(*env, IMPBackchannelFormat::EnumName); \
                sms->addSubsession(backchannelSub); \
                LOG_INFO("Backchannel stream " << NameString << " added to session"); \
            }

        X_FOREACH_BACKCHANNEL_FORMAT(ADD_BACKCHANNEL_SUBSESSION)
        #undef ADD_BACKCHANNEL_SUBSESSION
    }
#endif

    rtspServer->addServerMediaSession(sms);

    char *url = rtspServer->rtspURL(sms);
    LOG_INFO("stream " << chnNr << " available at: " << url);

    // Update RTSP status interface
    RTSPStatus::StreamInfo streamInfo;
    streamInfo.format = stream.format;
    streamInfo.fps = stream.fps;
    streamInfo.width = stream.width;
    streamInfo.height = stream.height;
    streamInfo.endpoint = stream.rtsp_endpoint;
    streamInfo.url = url;
    streamInfo.bitrate = stream.bitrate;
    streamInfo.mode = stream.mode;
    streamInfo.enabled = stream.enabled;

    std::string streamName = "stream" + std::to_string(chnNr);
    RTSPStatus::updateStreamStatus(streamName, streamInfo);

    delete[] url; // Free the URL string allocated by rtspURL()
}

void RTSP::start()
{
    // Initialize RTSP status interface
    if (!RTSPStatus::initialize()) {
        LOG_WARN("Failed to initialize RTSP status interface");
    }

    scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    if (cfg->rtsp.auth_required)
    {
        UserAuthenticationDatabase *auth = new UserAuthenticationDatabase;
        auth->addUserRecord(
            cfg->rtsp.username,
            cfg->rtsp.password);
        rtspServer = RTSPServer::createNew(*env, cfg->rtsp.port, auth, cfg->rtsp.session_reclaim);
    }
    else
    {
        rtspServer = RTSPServer::createNew(*env, cfg->rtsp.port, nullptr, cfg->rtsp.session_reclaim);
    }
    if (rtspServer == NULL)
    {
        LOG_ERROR("Failed to create RTSP server: " << env->getResultMsg() << "\n");
        return;
    }
    OutPacketBuffer::maxSize = cfg->rtsp.out_buffer_size;

    // PRUDYNT-T: Set shared timestamp base for A-V sync
    // Generate a single timestamp base that both audio and video RTPSinks will use
    // This eliminates the massive A-V sync differences caused by random timestamp bases
    uint32_t sharedTimestampBase = our_random32();
    char timestampBaseStr[16];
    snprintf(timestampBaseStr, sizeof(timestampBaseStr), "%u", sharedTimestampBase);
    setenv("PRUDYNT_SHARED_TIMESTAMP_BASE", timestampBaseStr, 1);

    char logMsg[64];
    snprintf(logMsg, sizeof(logMsg), "Set shared RTP timestamp base: %u (0x%08x)", sharedTimestampBase, sharedTimestampBase);
    LOG_INFO(logMsg);

#if defined(USE_AUDIO_STREAM_REPLICATOR)
    if (cfg->audio.input_enabled)
    {
        audioSource = IMPDeviceSource<AudioFrame, audio_stream>::createNew(*env, 0, global_audio[audioChn], "audio");

        if (global_audio[audioChn]->imp_audio->format == IMPAudioFormat::PCM)
            audioSource = (IMPDeviceSource<AudioFrame, audio_stream> *)EndianSwap16::createNew(*env, audioSource);

        global_audio[audioChn]->streamReplicator = StreamReplicator::createNew(*env, audioSource, false);
    }
#endif

    if (cfg->stream0.enabled)
    {
        addSubsession(0, cfg->stream0);
    }

    if (cfg->stream1.enabled)
    {
        addSubsession(1, cfg->stream1);
    }

    global_rtsp_thread_signal = 0;
    env->taskScheduler().doEventLoop(&global_rtsp_thread_signal);

#if defined(USE_AUDIO_STREAM_REPLICATOR)
    if (cfg->audio.input_enabled)
    {
        if(global_audio[audioChn]->streamReplicator != nullptr )
        {
            global_audio[audioChn]->streamReplicator->detachInputSource();
        }
        if(audioSource != nullptr)
        {
            delete audioSource;
            audioSource = nullptr;
        }
    }
#endif

    // Clean up VPS if it was allocated
    /*
    if (vps) {
        delete vps;
        vps = nullptr;
    }
    */

    LOG_DEBUG("Stop RTSP Server.");

    // Cleanup RTSP status interface
    RTSPStatus::cleanup();

    // Cleanup RTSP server and environment
    Medium::close(rtspServer);
    env->reclaim();
    delete scheduler;
}

void* RTSP::run(void* arg) {
    ((RTSP*)arg)->start();
    return nullptr;
}
