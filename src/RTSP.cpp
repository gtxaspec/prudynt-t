#include "RTSP.hpp"
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "IMPServerMediaSubsession.hpp"
#include "IMPDeviceSource.hpp"
#include "Config.hpp"

#define MODULE "RTSP"

void RTSP::run() {

    IMPServerMediaSubsession::init(cfg);

    LOG_DEBUG("RTSP :: " << cfg->rtsp_thread_signal);
    
    while((cfg->rtsp_thread_signal & 256) != 256) {
            
        if(cfg->rtsp_thread_signal == 0) {
                
            nice(-20);

            TaskScheduler *scheduler = BasicTaskScheduler::createNew();
            UsageEnvironment *env = BasicUsageEnvironment::createNew(*scheduler);

            RTSPServer *rtspServer;
            if (cfg->rtsp.auth_required) {
                UserAuthenticationDatabase *auth = new UserAuthenticationDatabase;
                auth->addUserRecord(
                    cfg->rtsp.username.c_str(),
                    cfg->rtsp.password.c_str()
                );
                rtspServer = RTSPServer::createNew(*env, cfg->rtsp.port, auth);
            } else {
                rtspServer = RTSPServer::createNew(*env, cfg->rtsp.port);
            }
            if (rtspServer == NULL) {
                LOG_ERROR("Failed to create RTSP server: " << env->getResultMsg() << "\n");
                return;
            }
            OutPacketBuffer::maxSize = cfg->rtsp.out_buffer_size;

            
            if(1) {
                LOG_DEBUG("identify stream 0");
                IMPDeviceSource* deviceSource = IMPDeviceSource::createNew(*env, 0);
                H264NALUnit sps, pps; // Declare outside the loop!
                H264NALUnit* vps = nullptr; // Use a pointer for VPS
                bool have_pps = false, have_sps = false, have_vps = false;
                // Read from the stream until we capture the SPS and PPS. Only capture VPS if needed.
                while (!have_pps || !have_sps || (cfg->stream0.format == "H265" && !have_vps)) {
                    LOG_DEBUG("wait_read");
                    H264NALUnit unit = deviceSource->wait_read();
                    LOG_DEBUG("wait_read");
                    if (cfg->stream0.format == "H265") {
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
                Encoder::remove_sink(deviceSource->sinkId);
                LOG_DEBUG("Got necessary NAL Units.");

                ServerMediaSession *sms = ServerMediaSession::createNew(
                    *env, cfg->stream0.rtsp_endpoint.c_str(), "Main", cfg->rtsp.name.c_str()
                );
                IMPServerMediaSubsession *sub = IMPServerMediaSubsession::createNew(
                    *env, (cfg->stream0.format == "H265" ? vps : nullptr), sps, pps, 0 // Conditional VPS
                );
                sms->addSubsession(sub);
                rtspServer->addServerMediaSession(sms);
                
                char* url = rtspServer->rtspURL(sms);
                LOG_INFO("stream 0 available at: " << url);
            }
            
            if(1) {
                LOG_DEBUG("identify stream 1");
                IMPDeviceSource* deviceSource = IMPDeviceSource::createNew(*env, 1);
                H264NALUnit sps, pps; // Declare outside the loop!
                H264NALUnit* vps = nullptr; // Use a pointer for VPS
                bool have_pps = false, have_sps = false, have_vps = false;
                // Read from the stream until we capture the SPS and PPS. Only capture VPS if needed.
                while (!have_pps || !have_sps || (cfg->stream0.format == "H265" && !have_vps)) {
                    H264NALUnit unit = deviceSource->wait_read();
                    if (cfg->stream0.format == "H265") {
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
                Encoder::remove_sink(deviceSource->sinkId);
                LOG_DEBUG("Got necessary NAL Units.");

                ServerMediaSession *sms = ServerMediaSession::createNew(
                    *env, "ch1", "ch1", "ch1"
                );
                IMPServerMediaSubsession *sub = IMPServerMediaSubsession::createNew(
                    *env, (cfg->stream0.format == "H265" ? vps : nullptr), sps, pps, 1 // Conditional VPS
                );
                
                sms->addSubsession(sub);
                rtspServer->addServerMediaSession(sms);
                
                char* url = rtspServer->rtspURL(sms);
                LOG_INFO("stream 0 available at: " << url);
            }      
            
            env->taskScheduler().doEventLoop(&cfg->rtsp_thread_signal);

            // Clean up VPS if it was allocated
            /*
            if (vps) {
                delete vps;
                vps = nullptr;
            }
            */

            LOG_DEBUG("Stop RTSP Server.");
            cfg->rtsp_thread_signal = 2;

            // Cleanup RTSP server and environment
            Medium::close(rtspServer);
            env->reclaim();
            delete scheduler;            
        }

        usleep(1000);
    }
}
