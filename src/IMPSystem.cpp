#include "IMPSystem.hpp"
#include "Config.hpp"

#define MODULE "IMP_SYSTEM"

IMPSensorInfo IMPSystem::create_sensor_info(const char *sensor_name)
{
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));
    LOG_INFO("Sensor: " << cfg->sensor.model);
    strcpy(out.name, cfg->sensor.model);
    out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
    strcpy(out.i2c.type, cfg->sensor.model);
    out.i2c.addr = cfg->sensor.i2c_address;
    return out;
}

IMPSystem *IMPSystem::createNew()
{
    return new IMPSystem();
}

int IMPSystem::init()
{
    LOG_DEBUG("IMPSystem::init()");
    int ret = 0;

    ret = IMP_OSD_SetPoolSize(cfg->general.osd_pool_size * 1024);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_SetPoolSize(" << (cfg->general.osd_pool_size * 1024) << ")");
    
    IMPVersion impVersion;
    ret = IMP_System_GetVersion(&impVersion);
    LOG_INFO("LIBIMP Version " << impVersion.aVersion);

    SUVersion suVersion;
    ret = SU_Base_GetVersion(&suVersion);
    LOG_INFO("SYSUTILS Version: " << suVersion.chr);

    const char *cpuInfo = IMP_System_GetCPUInfo();
    LOG_INFO("CPU Information: " << cpuInfo);

    ret = IMP_ISP_Open();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Open()");

    /* sensor */
    sinfo = create_sensor_info(cfg->sensor.model);
    ret = IMP_ISP_AddSensor(&sinfo);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_AddSensor(&sinfo)");

    ret = IMP_ISP_EnableSensor();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_EnableSensor()");

    /* system */
    ret = IMP_System_Init();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Init()");

    ret = IMP_ISP_EnableTuning();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_EnableTuning()");

    /* Image tuning */
    unsigned char value;

#if !defined(NO_TUNINGS)

    ret = IMP_ISP_Tuning_SetContrast(cfg->image.contrast);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetContrast(" << cfg->image.contrast << ")");

    ret = IMP_ISP_Tuning_SetSharpness(cfg->image.sharpness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSharpness(" << cfg->image.sharpness << ")");

    ret = IMP_ISP_Tuning_SetSaturation(cfg->image.saturation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetBrightness(cfg->image.brightness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBrightness(" << cfg->image.brightness << ")");

    ret = IMP_ISP_Tuning_SetContrast(cfg->image.contrast);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetContrast(" << cfg->image.contrast << ")");

    ret = IMP_ISP_Tuning_SetSharpness(cfg->image.sharpness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSharpness(" << cfg->image.sharpness << ")");

    ret = IMP_ISP_Tuning_SetSaturation(cfg->image.saturation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetBrightness(cfg->image.brightness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBrightness(" << cfg->image.brightness << ")");

    ret = IMP_ISP_Tuning_SetSinterStrength(cfg->image.sinter_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSinterStrength(" << cfg->image.sinter_strength << ")");

    ret = IMP_ISP_Tuning_SetTemperStrength(cfg->image.temper_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetTemperStrength(" << cfg->image.temper_strength << ")");

    ret = IMP_ISP_Tuning_SetISPHflip((IMPISPTuningOpsMode)cfg->image.hflip);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPHflip(" << cfg->image.hflip << ")");

    ret = IMP_ISP_Tuning_SetISPVflip((IMPISPTuningOpsMode)cfg->image.vflip);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPVflip(" << cfg->image.vflip << ")");

    ret = IMP_ISP_Tuning_SetISPRunningMode((IMPISPRunningMode)cfg->image.running_mode);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPRunningMode(" << cfg->image.running_mode << ")");

    ret = IMP_ISP_Tuning_SetISPBypass(IMPISP_TUNING_OPS_MODE_ENABLE);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPBypass(" << IMPISP_TUNING_OPS_MODE_ENABLE << ")");

    IMPISPAntiflickerAttr flickerAttr;
    memset(&flickerAttr, 0, sizeof(IMPISPAntiflickerAttr));
    ret = IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)cfg->image.anti_flicker);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAntiFlickerAttr(" << cfg->image.anti_flicker << ")");

#if !defined(PLATFORM_T21)
    ret = IMP_ISP_Tuning_SetAeComp(cfg->image.ae_compensation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAeComp(" << cfg->image.ae_compensation << ")");
#endif

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetHiLightDepress(cfg->image.highlight_depress);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetHiLightDepress(" << cfg->image.highlight_depress << ")");
#else
    ret = IMP_ISP_Tuning_SetHiLightDepress(cfg->image.highlight_depress);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetHiLightDepress(" << cfg->image.highlight_depress << ")");
#endif

    ret = IMP_ISP_Tuning_SetMaxAgain(cfg->image.max_again);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxAgain(" << cfg->image.max_again << ")");

    ret = IMP_ISP_Tuning_SetMaxDgain(cfg->image.max_dgain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxDgain(" << cfg->image.max_dgain << ")");

    IMPISPWB wb;
    memset(&wb, 0, sizeof(IMPISPWB));
    wb.mode = (isp_core_wb_mode)cfg->image.core_wb_mode;
    wb.rgain = cfg->image.wb_rgain;
    wb.bgain = cfg->image.wb_bgain;
    ret = IMP_ISP_Tuning_SetWB(&wb);
    if (ret != 0)
    {
        LOG_ERROR("Unable to set white balance. Mode: " << cfg->image.core_wb_mode << ", rgain: "
                                                        << cfg->image.wb_rgain << ", bgain: " << cfg->image.wb_bgain);
    }
    else
    {
        LOG_DEBUG("Set white balance. Mode: " << cfg->image.core_wb_mode << ", rgain: "
                                                << cfg->image.wb_rgain << ", bgain: " << cfg->image.wb_bgain);
    }

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBcshHue(cfg->image.hue);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBcshHue(" << cfg->image.hue << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    uint8_t _defog_strength = static_cast<uint8_t>(cfg->image.defog_strength);
    ret = IMP_ISP_Tuning_SetDefog_Strength(reinterpret_cast<uint8_t *>(&_defog_strength));
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDefog_Strength(" << cfg->image.defog_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDPC_Strength(cfg->image.dpc_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDPC_Strength(" << cfg->image.dpc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDRC_Strength(cfg->image.drc_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDRC_Strength(" << cfg->image.drc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
        ret = IMP_ISP_Tuning_SetBacklightComp(cfg->image.backlight_compensation);
        LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBacklightComp(" << cfg->image.backlight_compensation << ")");
#endif

#endif // #if !defined(NO_TUNINGS)

    LOG_DEBUG("ISP Tuning Defaults set");

    ret = IMP_ISP_Tuning_SetSensorFPS(cfg->sensor.fps, 1);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Tuning_SetSensorFPS(" << cfg->sensor.fps << ", 1);");

    // Set the ISP to DAY on launch
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Tuning_SetISPRunningMode(" << IMPISP_RUNNING_MODE_DAY << ")");

    return ret;
}

int IMPSystem::destroy()
{
    int ret;

    ret = IMP_System_Exit();
    LOG_DEBUG_OR_ERROR(ret, "IMP_System_Exit()");

    ret = IMP_ISP_DisableSensor();
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_DisableSensor()");

    ret = IMP_ISP_DelSensor(&sinfo);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_DelSensor()");

    ret = IMP_ISP_DisableTuning();
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_DisableTuning()");

    ret = IMP_ISP_Close();
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Close()");

    return 0;
}
