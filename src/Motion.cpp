#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>

#include "Motion.hpp"
#include "Config.hpp"
#include "Scripts.hpp"

extern "C" {
    #include <unistd.h>
}

std::atomic<bool> Motion::moving;
std::atomic<bool> Motion::indicator;
std::thread Motion::detect_thread;
void Motion::detect_start(Motion *m) { m->detect(); }
using namespace std::chrono;
bool ignoreInitialPeriod = true;

void Motion::detect() {
    LOG_INFO("Detection thread started");
    int ret;
    int debounce = 0;
    IMP_IVS_MoveOutput *result;
    bool isInCooldown = false;
    auto cooldownEndTime = steady_clock::now();
    auto motionEndTime = steady_clock::now();
    auto startTime = steady_clock::now();

    while (true) {
        ret = IMP_IVS_PollingResult(0, IMP_IVS_DEFAULT_TIMEOUTMS);
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

        if (ignoreInitialPeriod && elapsedTime.count() < Config::singleton()->motionInitTime) {
            continue;
        } else {
            ignoreInitialPeriod = false;
        }

        if (isInCooldown && duration_cast<seconds>(currentTime - cooldownEndTime).count() < Config::singleton()->motionCooldownTime) {
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
                if (debounce >= Config::singleton()->motionDebounce) {
                    if (!Motion::moving.load()) {
                        Motion::moving = true;
                        LOG_INFO("Motion Start");
                        Scripts::motionScript();
                    }
                    Motion::indicator = true;
                    motionEndTime = steady_clock::now(); // Update last motion time
                }
            }
        }

        if (!motionDetected) {
            debounce = 0;
            if (Motion::moving && duration_cast<seconds>(currentTime - motionEndTime).count() >= Config::singleton()->motionPostTime) {
                LOG_INFO("End of Motion");
                Motion::moving = false;
                Motion::indicator = false;
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
}

bool Motion::init() {
    int ret;
    IMP_IVS_MoveParam move_param;
    IMPIVSInterface *move_intf;
    ret = IMP_IVS_CreateGroup(0);
    if (ret < 0) {
        LOG_ERROR("IMP_IVS_CreateGroup() == " << ret);
        return true;
    }

    memset(&move_param, 0, sizeof(IMP_IVS_MoveParam));
    //OSD is affecting motion for some reason.
    //Sensitivity range is 0-4
    move_param.sense[0] = Config::singleton()->motionSensitivity;
    move_param.skipFrameCnt = Config::singleton()->motionSkipFrameCnt;
    move_param.frameInfo.width = Config::singleton()->motionFrameWidth;
    move_param.frameInfo.height = Config::singleton()->motionFrameHeight;
    move_param.roiRect[0].p0.x = Config::singleton()->motionRoi0X;
    move_param.roiRect[0].p0.y = Config::singleton()->motionRoi0Y;
    move_param.roiRect[0].p1.x = Config::singleton()->motionRoi1X - 1;
    move_param.roiRect[0].p1.y = Config::singleton()->motionRoi1Y - 1;
    move_param.roiRectCnt = Config::singleton()->roiCnt;
    move_intf = IMP_IVS_CreateMoveInterface(&move_param);

    ret = IMP_IVS_CreateChn(0, move_intf);
    if (ret < 0) {
        LOG_ERROR("IMP_IVS_CreateChn() == " << ret);
        return true;
    }

    ret = IMP_IVS_RegisterChn(0, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_IVS_RegisterChn() == " << ret);
        return true;
    }

    ret = IMP_IVS_StartRecvPic(0);
    if (ret < 0) {
        LOG_ERROR("IMP_IVS_StartRecvPic() == " << ret);
        return true;
    }

    IMPCell fs = { DEV_ID_FS, 0, 1 };
    IMPCell ivs_cell = { DEV_ID_IVS, 0, 0 };
    //Framesource -> IVS
    ret = IMP_System_Bind(&fs, &ivs_cell);
    if (ret < 0) {
        LOG_ERROR("IMP_System_Bind(FS, IVS) == " << ret);
        return true;
    }
    return false;
}


void Motion::run() {
    LOG_INFO("Starting Motion Detection");
    nice(-20);

    // Start the detection thread
    detect_thread = std::thread(Motion::detect_start, this);

}