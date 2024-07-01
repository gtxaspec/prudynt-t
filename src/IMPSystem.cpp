#include "IMPSystem.hpp"

#define MODULE "IMP_SYSTEM"

IMPSensorInfo IMPSystem::create_sensor_info(const char *sensor_name)
{
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));
    LOG_INFO("Sensor: " << sensor->model);
    strcpy(out.name, sensor->model);
    out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
    strcpy(out.i2c.type, sensor->model);
    out.i2c.addr = sensor->i2c_address;
    return out;
}

IMPSystem *IMPSystem::createNew(
    _image *image,
    _sensor *sensor)
{
    return new IMPSystem(image, sensor);
}

int IMPSystem::init()
{
    LOG_DEBUG("IMPSystem::init()");
    int ret = 0;

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
    sinfo = create_sensor_info(sensor->model);
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
    ret = IMP_ISP_Tuning_GetContrast(&value);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_GetContrast(" << (int)value << ")");

    ret = IMP_ISP_Tuning_SetContrast(image->contrast);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetContrast(" << image->contrast << ")");

    ret = IMP_ISP_Tuning_SetSharpness(image->sharpness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSharpness(" << image->sharpness << ")");

    ret = IMP_ISP_Tuning_SetSaturation(image->saturation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << image->saturation << ")");

    ret = IMP_ISP_Tuning_SetBrightness(image->brightness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBrightness(" << image->brightness << ")");

    ret = IMP_ISP_Tuning_SetSinterStrength(image->sinter_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSinterStrength(" << image->sinter_strength << ")");

    ret = IMP_ISP_Tuning_SetTemperStrength(image->temper_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetTemperStrength(" << image->temper_strength << ")");

    ret = IMP_ISP_Tuning_SetISPHflip((IMPISPTuningOpsMode)image->hflip);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPHflip(" << image->hflip << ")");

    ret = IMP_ISP_Tuning_SetISPVflip((IMPISPTuningOpsMode)image->vflip);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPVflip(" << image->vflip << ")");

    ret = IMP_ISP_Tuning_SetISPRunningMode((IMPISPRunningMode)image->running_mode);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPRunningMode(" << image->running_mode << ")");

    ret = IMP_ISP_Tuning_SetISPBypass(IMPISP_TUNING_OPS_MODE_ENABLE);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetISPBypass(" << IMPISP_TUNING_OPS_MODE_ENABLE << ")");

    ret = IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)image->anti_flicker);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAntiFlickerAttr(" << image->anti_flicker << ")");

#if !defined(PLATFORM_T21)
    ret = IMP_ISP_Tuning_SetAeComp(image->ae_compensation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAeComp(" << image->ae_compensation << ")");
#endif

    ret = IMP_ISP_Tuning_SetHiLightDepress(image->highlight_depress);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetHiLightDepress(" << image->highlight_depress << ")");

    ret = IMP_ISP_Tuning_SetMaxAgain(image->max_again);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxAgain(" << image->max_again << ")");

    ret = IMP_ISP_Tuning_SetMaxDgain(image->max_dgain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxDgain(" << image->max_dgain << ")");

    IMPISPWB wb;
    memset(&wb, 0, sizeof(IMPISPWB));
    ret = IMP_ISP_Tuning_GetWB(&wb);
    if (ret == 0)
    {
        wb.mode = (isp_core_wb_mode)image->core_wb_mode;
        wb.rgain = image->wb_rgain;
        wb.bgain = image->wb_bgain;
        ret = IMP_ISP_Tuning_SetWB(&wb);
        if (ret != 0)
        {
            LOG_ERROR("Unable to set white balance. Mode: " << image->core_wb_mode << ", rgain: "
                                                            << image->wb_rgain << ", bgain: " << image->wb_bgain);
        }
        else
        {
            LOG_DEBUG("Set white balance. Mode: " << image->core_wb_mode << ", rgain: "
                                                  << image->wb_rgain << ", bgain: " << image->wb_bgain);
        }
    }

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBcshHue(image->hue);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBcshHue(" << image->hue << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    uint8_t _defog_strength = static_cast<uint8_t>(image->defog_strength);
    ret = IMP_ISP_Tuning_SetDefog_Strength(reinterpret_cast<uint8_t *>(&_defog_strength));
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDefog_Strength(" << image->defog_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDPC_Strength(image->dpc_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDPC_Strength(" << image->dpc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDRC_Strength(image->dpc_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDRC_Strength(" << image->drc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBacklightComp(image->backlight_compensation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBacklightComp(" << image->backlight_compensation << ")");
#endif

#if defined(AUDIO_SUPPORT)
    /* Audio tuning */
    /*     input    */
    if (cfg->audio.input_enabled)
    {
        ret = IMP_AI_Enable(0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Enable(0)");

        ret = IMP_AI_EnableChn(0, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableChn(0, 0)");

        ret = IMP_AI_SetVol(0, 0, cfg->audio.input_vol);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetVol(0, 0, " << cfg->audio.input_vol << ")");

        ret = IMP_AI_SetGain(0, 0, cfg->audio.input_gain);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetGain(0, 0, " << cfg->audio.input_gain << ")");

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)

        ret = IMP_AI_SetAlcGain(0, 0, cfg->audio.input_alc_gain);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetAlcGain(0, 0, " << cfg->audio.input_alc_gain << ")");

#endif
        if (cfg->audio.input_echo_cancellation)
        {
            ret = IMP_AI_EnableAec(0, 0, 0, 0);
            LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableAec(0)");
        }
        else
        {
            ret = IMP_AI_DisableAec(0, 0);
            LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableAec(0)");
        }

        IMPAudioIOAttr ioattr;
        ret = IMP_AI_GetPubAttr(0, &ioattr);
        if (ret == 0)
        {
            if (cfg->audio.input_noise_suppression)
            {
                ret = IMP_AI_EnableNs(&ioattr, cfg->audio.input_noise_suppression);
                LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableNs(" << cfg->audio.input_noise_suppression << ")");
            }
            else
            {
                ret = IMP_AI_DisableNs();
                LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableNs(0)");
            }
            if (cfg->audio.output_high_pass_filter)
            {
                ret = IMP_AI_EnableHpf(&ioattr);
                LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableHpf(0)");
            }
            else
            {
                ret = IMP_AI_DisableHpf();
                LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableHpf(0)");
            }
        }
        else
        {
            LOG_ERROR("IMP_AI_GetPubAttr failed.");
        }
    }
    else
    {

        ret = IMP_AI_DisableChn(0, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableChn(0)");

        ret = IMP_AI_Disable(0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Disable(0)");
    }

    /*    output    */
    if (cfg->audio.output_enabled)
    {
        ret = IMP_AO_Enable(0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_Enable(0)");

        ret = IMP_AO_EnableChn(0, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_EnableChn(0, 0)");

        ret = IMP_AO_SetVol(0, 0, cfg->audio.output_vol);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_SetVol(0, 0, " << cfg->audio.output_vol << ")");

        ret = IMP_AO_SetGain(0, 0, cfg->audio.output_gain);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_SetGain(0, 0, " << cfg->audio.output_gain << ")");

        IMPAudioIOAttr ioattr;
        ret = IMP_AO_GetPubAttr(0, &ioattr);
        if (ret == 0)
        {
            if (cfg->audio.output_high_pass_filter)
            {
                ret = IMP_AO_EnableHpf(&ioattr);
                LOG_DEBUG_OR_ERROR(ret, "IMP_AO_EnableHpf(0)");
            }
            else
            {
                ret = IMP_AO_DisableHpf();
                LOG_DEBUG_OR_ERROR(ret, "IMP_AO_DisableHpf(0)");
            }
        }
        else
        {
            LOG_ERROR("IMP_AO_GetPubAttr failed.");
        }
    }
    else
    {

        ret = IMP_AO_DisableChn(0, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_DisableChn(0, 0)");

        ret = IMP_AO_Disable(0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AO_Disable(0)");
    }
#endif // #if defined(AUDIO_SUPPORT)

    LOG_DEBUG("ISP Tuning Defaults set");

    ret = IMP_ISP_Tuning_SetSensorFPS(sensor->fps, 1);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Tuning_SetSensorFPS(" << sensor->fps << ", 1);");

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