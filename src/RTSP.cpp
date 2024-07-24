#include "RTSP.hpp"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "IMPServerMediaSubsession.hpp"
#include "IMPDeviceSource.hpp"
#include "Config.hpp"
#include "globals.hpp"

#define MODULE "RTSP"

void RTSP::run() {
    nice(-20);
    TaskScheduler *scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

    RTSPServer *rtspServer;
    if (Config::singleton()->rtspAuthRequired) {
        UserAuthenticationDatabase *auth = new UserAuthenticationDatabase;
        auth->addUserRecord(
            Config::singleton()->rtspUsername.c_str(),
            Config::singleton()->rtspPassword.c_str()
        );
        rtspServer = RTSPServer::createNew(*env, Config::singleton()->rtspPort, auth);
    } else {
        rtspServer = RTSPServer::createNew(*env, Config::singleton()->rtspPort);
    }
    if (rtspServer == NULL) {
        LOG_ERROR("Failed to create RTSP server: " << env->getResultMsg() << "\n");
        return;
    }
    OutPacketBuffer::maxSize = Config::singleton()->rtspOutBufferSize;

    H264NALUnit sps, pps; // Declare outside the loop!
    H264NALUnit* vps = nullptr; // Use a pointer for VPS
    bool have_pps = false, have_sps = false, have_vps = false;
    // Read from the stream until we capture the SPS and PPS. Only capture VPS if needed.
    while (!have_pps || !have_sps || (Config::singleton()->stream0format == "H265" && !have_vps)) {
        H264NALUnit unit = input_chn->wait_read();
        if (Config::singleton()->stream0format == "H265") {
            uint8_t nalType = (unit.data[0] & 0x7E) >> 1; // H265 NAL unit type extraction
            if (nalType == 33) { // SPS for H265
                LOG_DEBUG("Got SPS (H265)");
                sps = unit;
                have_sps = true;
            } else if (nalType == 34) { // PPS for H265
                LOG_DEBUG("Got PPS (H265)");
                pps = unit;
                have_pps = true;
            } else if (nalType == 32) { // VPS, only for H265
                LOG_DEBUG("Got VPS");
                if (!vps) vps = new H264NALUnit(unit); // Allocate and store VPS
                have_vps = true;
            }
        } else { // Assuming H264 if not H265
            uint8_t nalType = (unit.data[0] & 0x1F); // H264 NAL unit type extraction
            if (nalType == 7) { // SPS for H264
                LOG_DEBUG("Got SPS (H264)");
                sps = unit;
                have_sps = true;
            } else if (nalType == 8) { // PPS for H264
                LOG_DEBUG("Got PPS (H264)");
                pps = unit;
                have_pps = true;
            }
            // No VPS in H264, so no need to check for it
        }
    }
    enc.set_output_channel(nullptr);
    LOG_DEBUG("Got necessary NAL Units.");

    ServerMediaSession *sms = ServerMediaSession::createNew(
        *env, Config::singleton()->stream0endpoint.c_str(), "Main", Config::singleton()->rtspName.c_str()
    );
    IMPServerMediaSubsession *sub = IMPServerMediaSubsession::createNew(
        *env, (Config::singleton()->stream0format == "H265" ? vps : nullptr), sps, pps // Conditional VPS
    );
    sms->addSubsession(sub);
    rtspServer->addServerMediaSession(sms);

    char* url = rtspServer->rtspURL(sms);
    LOG_INFO("stream 0 available at: " << url);

    env->taskScheduler().doEventLoop();

    // Clean up VPS if it was allocated
    if (vps) {
        delete vps;
        vps = nullptr;
    }
}
