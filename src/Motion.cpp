#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>

#include "Motion.hpp"
#include "Config.hpp"

extern "C" {
    #include <unistd.h>
}

void Motion::detect_start(Motion *m) { m->detect(); }
using namespace std::chrono;
bool ignoreInitialPeriod = true;

void Motion::detect() {

    LOG_INFO("Start motion detection thread.");
    cfg->motion_thread_signal.fetch_or(2);

    int ret;
    int debounce = 0;
    IMP_IVS_MoveOutput *result;
    bool isInCooldown = false;
    auto cooldownEndTime = steady_clock::now();
    auto motionEndTime = steady_clock::now();
    auto startTime = steady_clock::now();

    while (cfg->motion_thread_signal.load() & 2) {

        // stop thread request
        if(cfg->motion_thread_signal.load() & 4) {

            // !2 = exit while 
            cfg->motion_thread_signal.fetch_xor(2);
            continue;
        }

        ret = IMP_IVS_PollingResult(0, cfg->motion.thread_wait); // IMP_IVS_DEFAULT_TIMEOUTMS ~10sec are really long 
        if (ret < 0) {
            LOG_WARN("IMP_IVS_PollingResult error: " << ret);
            continue;
        }

        ret = IMP_IVS_GetResult(0, (void**)&result);
        if (ret < 0) {
            LOG_WARN("IMP_IVS_GetResult error: " << ret);
            continue;
        }

        auto currentTime = steady_clock::now();
        auto elapsedTime = duration_cast<seconds>(currentTime - startTime);

        if (ignoreInitialPeriod && elapsedTime.count() < cfg->motion.init_time) {
            continue;
        } else {
            ignoreInitialPeriod = false;
        }

        if (isInCooldown && duration_cast<seconds>(currentTime - cooldownEndTime).count() < cfg->motion.cooldown_time) {
            continue;
        } else {
            isInCooldown = false;
        }

        bool motionDetected = false;
        for (int i = 0; i < IMP_IVS_MOVE_MAX_ROI_CNT; i++) {
            if (result->retRoi[i]) {
                motionDetected = true;
                LOG_INFO("Active motion detected in region " << i);
                debounce++;
                if (debounce >= cfg->motion.debounce_time) {
                    if (!moving.load()) {
                        moving = true;
                        LOG_INFO("Motion Start");

                        ret = system(cfg->motion.script_path);
                        if (ret != 0) {
                            LOG_ERROR("Motion script failed:" << cfg->motion.script_path);
                        }                        
                    }
                    indicator = true;
                    motionEndTime = steady_clock::now(); // Update last motion time
                }
            }
        }

        if (!motionDetected) {
            debounce = 0;
            if (moving && duration_cast<seconds>(currentTime - motionEndTime).count() >= cfg->motion.post_time) {
                LOG_INFO("End of Motion");
                moving = false;
                indicator = false;
                cooldownEndTime = steady_clock::now(); // Start cooldown
                isInCooldown = true;
            }
        }

        ret = IMP_IVS_ReleaseResult(0, (void*)result);
        if (ret < 0) {
            LOG_WARN("IMP_IVS_ReleaseResult error: " << ret);
            continue;
        }
    }

    LOG_DEBUG("Exit motion detect thread.");
    cfg->motion_thread_signal.fetch_xor(4);
    cfg->motion_thread_signal.fetch_or(8);
}

int Motion::init(std::shared_ptr<CFG> _cfg) {

    LOG_INFO("Initialize motion detection.");
    
    nice(-20);

    int ret;
    cfg = _cfg;
    cfg->motion_thread_signal.store(0);

    ret = IMP_IVS_CreateGroup(0);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_CreateGroup(0)");

    memset(&move_param, 0, sizeof(IMP_IVS_MoveParam));
    //OSD is affecting motion for some reason.
    //Sensitivity range is 0-4
    move_param.sense[0] = cfg->motion.sensitivity;
    move_param.skipFrameCnt = cfg->motion.skip_frame_count;
    move_param.frameInfo.width = cfg->motion.frame_width;
    move_param.frameInfo.height = cfg->motion.frame_width;
    
    move_param.roiRect[0].p0.x = cfg->motion.roi_0_x;
    move_param.roiRect[0].p0.y = cfg->motion.roi_0_y;
    move_param.roiRect[0].p1.x = cfg->motion.roi_1_x - 1;
    move_param.roiRect[0].p1.y = cfg->motion.roi_1_y - 1;
    move_param.roiRectCnt = cfg->motion.roi_count;
    move_intf = IMP_IVS_CreateMoveInterface(&move_param);

    ret = IMP_IVS_CreateChn(0, move_intf);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_CreateChn(0, move_intf)");

    ret = IMP_IVS_RegisterChn(0, 0);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_RegisterChn(0, 0)");

    ret = IMP_IVS_StartRecvPic(0);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_StartRecvPic(0)")

    //Framesource -> IVS
    ret = IMP_System_Bind(&fs, &ivs_cell);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&fs, &ivs_cell)");

    //initialize and start
    cfg->motion_thread_signal.fetch_or(1);

    // Start the detection thread
    detect_thread = std::thread(Motion::detect_start, this);

    return ret;
}

int Motion::exit() {

    int ret;

    LOG_DEBUG("Exit motion detection.");

    cfg->motion_thread_signal.fetch_or(4);

    ret = IMP_IVS_StopRecvPic(0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_StopRecvPic(0)");

    ret = IMP_IVS_UnRegisterChn(0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_UnRegisterChn(0)");

    ret = IMP_IVS_DestroyChn(0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_DestroyChn(0)");

    ret = IMP_IVS_DestroyGroup(0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_DestroyGroup(0)");

    IMP_IVS_DestroyMoveInterface(move_intf);

    std::chrono::milliseconds duration;
    auto t0 = std::chrono::high_resolution_clock::now();
    while((cfg->motion_thread_signal.load() & 8)!=8) {
        LOG_DEBUG("Wait for motion detect thread exit." << cfg->motion_thread_signal.load());
        duration = duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0);
        if( duration.count() > 1000 * 1000 * 30 ) { //30 seconds
            LOG_ERROR("Motion thread exit timeout.");
            return false;
        }
        usleep(1000 * 1000);
    }
    LOG_DEBUG("Join motion detect thread to cleanup.");
    detect_thread.join();

    cfg->motion_thread_signal.fetch_xor(1);

    return ret;
}