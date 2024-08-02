#include "Motion.hpp"

using namespace std::chrono;
bool ignoreInitialPeriod = true;

std::string Motion::getConfigPath(const char *itemName)
{
    return "motion." + std::string(itemName);
}

void Motion::detect()
{
    LOG_INFO("Start motion detection thread.");

    int ret;
    int debounce = 0;
    IMP_IVS_MoveOutput *result;
    bool isInCooldown = false;
    auto cooldownEndTime = steady_clock::now();
    auto motionEndTime = steady_clock::now();
    auto startTime = steady_clock::now();

    if(init() != 0) return;

    global_motion_thread_signal = true;
    while (global_motion_thread_signal)
    {

        ret = IMP_IVS_PollingResult(ivsChn, cfg->motion.ivs_polling_timeout);
        if (ret < 0)
        {
            LOG_WARN("IMP_IVS_PollingResult error: " << ret);
            continue;
        }

        ret = IMP_IVS_GetResult(ivsChn, (void **)&result);
        if (ret < 0)
        {
            LOG_WARN("IMP_IVS_GetResult error: " << ret);
            continue;
        }

        auto currentTime = steady_clock::now();
        auto elapsedTime = duration_cast<seconds>(currentTime - startTime);

        if (ignoreInitialPeriod && elapsedTime.count() < cfg->motion.init_time)
        {
            continue;
        }
        else
        {
            ignoreInitialPeriod = false;
        }

        if (isInCooldown && duration_cast<seconds>(currentTime - cooldownEndTime).count() < cfg->motion.cooldown_time)
        {
            continue;
        }
        else
        {
            isInCooldown = false;
        }

        bool motionDetected = false;
        for (int i = 0; i < IMP_IVS_MOVE_MAX_ROI_CNT; i++)
        {
            if (result->retRoi[i])
            {
                motionDetected = true;
                LOG_INFO("Active motion detected in region " << i);
                debounce++;
                if (debounce >= cfg->motion.debounce_time)
                {
                    if (!moving.load())
                    {
                        moving = true;
                        LOG_INFO("Motion Start");

                        ret = system(cfg->motion.script_path);
                        if (ret != 0)
                        {
                            LOG_ERROR("Motion script failed:" << cfg->motion.script_path);
                        }
                    }
                    indicator = true;
                    motionEndTime = steady_clock::now(); // Update last motion time
                }
            }
        }

        if (!motionDetected)
        {
            debounce = 0;
            if (moving && duration_cast<seconds>(currentTime - motionEndTime).count() >= cfg->motion.post_time)
            {
                LOG_INFO("End of Motion");
                moving = false;
                indicator = false;
                cooldownEndTime = steady_clock::now(); // Start cooldown
                isInCooldown = true;
            }
        }

        ret = IMP_IVS_ReleaseResult(ivsChn, (void *)result);
        if (ret < 0)
        {
            LOG_WARN("IMP_IVS_ReleaseResult error: " << ret);
            continue;
        }
    }

    exit();

    LOG_DEBUG("Exit motion detect thread.");
}

int Motion::init()
{
    LOG_INFO("Initialize motion detection.");

    if((cfg->motion.monitor_stream == 0 && !cfg->stream0.enabled) || 
       (cfg->motion.monitor_stream == 1 && !cfg->stream1.enabled)) {

        LOG_ERROR("Monitor stream is disabled, abort.");
        return -1;
    }
    int ret;

    ret = IMP_IVS_CreateGroup(0);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_CreateGroup(0)");

    //automatically set frame size / height 
    ret = IMP_Encoder_GetChnAttr(cfg->motion.monitor_stream, &channelAttributes);
    if (ret == 0)
    {
        if (cfg->motion.frame_width == IVS_AUTO_VALUE)
        {
            cfg->set<int>(getConfigPath("frame_width"), channelAttributes.encAttr.picWidth, true);
        }
        if (cfg->motion.frame_height == IVS_AUTO_VALUE)
        {
            cfg->set<int>(getConfigPath("frame_height"), channelAttributes.encAttr.picHeight, true);
        }
        if (cfg->motion.roi_1_x == IVS_AUTO_VALUE)
        {
            cfg->set<int>(getConfigPath("roi_1_x"), channelAttributes.encAttr.picWidth - 1, true);
        }
        if (cfg->motion.roi_1_y == IVS_AUTO_VALUE)
        {
            cfg->set<int>(getConfigPath("roi_1_y"), channelAttributes.encAttr.picHeight - 1, true);
        }        
    }

    memset(&move_param, 0, sizeof(IMP_IVS_MoveParam));
    // OSD is affecting motion for some reason.
    // Sensitivity range is 0-4
    move_param.sense[0] = cfg->motion.sensitivity;
    move_param.skipFrameCnt = cfg->motion.skip_frame_count;
    move_param.frameInfo.width = cfg->motion.frame_width;
    move_param.frameInfo.height = cfg->motion.frame_height;

    LOG_INFO("Motion detection:" << 
             " sensibility: " << move_param.sense[0] << 
             ", skipCnt:" << move_param.skipFrameCnt << 
             ", width:" << move_param.frameInfo.width << 
             ", height:" << move_param.frameInfo.height);

    move_param.roiRect[0].p0.x = cfg->motion.roi_0_x;
    move_param.roiRect[0].p0.y = cfg->motion.roi_0_y;
    move_param.roiRect[0].p1.x = cfg->motion.roi_1_x - 1;
    move_param.roiRect[0].p1.y = cfg->motion.roi_1_y - 1;
    move_param.roiRectCnt = cfg->motion.roi_count;

    LOG_INFO("Motion detection roi[0]:" << 
             " roi_0_x: " << cfg->motion.roi_0_x << 
             ", roi_0_y:" << cfg->motion.roi_0_y << 
             ", roi_1_x: " << cfg->motion.roi_1_x << 
             ", roi_1_y:" << cfg->motion.roi_1_y);

    move_intf = IMP_IVS_CreateMoveInterface(&move_param);

    ret = IMP_IVS_CreateChn(ivsChn, move_intf);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_CreateChn(" << ivsChn << ", move_intf)");

    ret = IMP_IVS_RegisterChn(ivsGrp, ivsChn);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_RegisterChn(" << ivsGrp << ", " << ivsChn << ")");

    ret = IMP_IVS_StartRecvPic(ivsChn);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_IVS_StartRecvPic(" << ivsChn << ")")

    fs = { 
        /**< Device ID */ DEV_ID_FS, 
        /**< Group ID */  cfg->motion.monitor_stream, 
        /**< output ID */ 1 
    };

    ivs_cell = { 
        /**< Device ID */ DEV_ID_IVS, 
        /**< Group ID */  0, 
        /**< output ID */ 0 
    };

    ret = IMP_System_Bind(&fs, &ivs_cell);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&fs, &ivs_cell)");

    return ret;
}

int Motion::exit()
{
    int ret;

    LOG_DEBUG("Exit motion detection.");

    ret = IMP_IVS_StopRecvPic(ivsChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_StopRecvPic(0)");

    ret = IMP_System_UnBind(&fs, &ivs_cell);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_UnBind(&fs, &ivs_cell)");

    ret = IMP_IVS_UnRegisterChn(ivsChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_UnRegisterChn(0)");

    ret = IMP_IVS_DestroyChn(ivsChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_DestroyChn(0)");

    ret = IMP_IVS_DestroyGroup(ivsGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_IVS_DestroyGroup(0)");

    IMP_IVS_DestroyMoveInterface(move_intf);

    return ret;
}

void *Motion::run(void *arg)
{
    ((Motion *)arg)->detect();
    return nullptr;
}
