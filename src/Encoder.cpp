#include <iostream>
#include <cstring>
#include <ctime>
#include <fstream>
#include "Encoder.hpp"
#include "Config.hpp"

#define MODULE "ENCODER"

#define OSDPoolSize 200000

#if defined(PLATFORM_T31)
	#define IMPEncoderCHNAttr IMPEncoderChnAttr
	#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

std::mutex Encoder::sinks_lock;
std::map<uint32_t, EncoderSink> Encoder::sinks;
uint32_t Encoder::sink_id = 0;

IMPSensorInfo Encoder::create_sensor_info(std::string sensor)
{
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));
    LOG_INFO("Sensor: " << cfg->sensor.model.c_str());
    std::strcpy(out.name, cfg->sensor.model.c_str());
    out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
    std::strcpy(out.i2c.type, cfg->sensor.model.c_str());
    out.i2c.addr = cfg->sensor.i2c_address;
    return out;
}

int Encoder::channel_init(int chn_nr, int grp_nr, IMPEncoderCHNAttr *chn_attr)
{

    int ret;

    ret = IMP_Encoder_CreateChn(chn_nr, chn_attr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_CreateChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    else
    {
        LOG_DEBUG("IMP_Encoder_CreateChn(" << chn_nr << ") created");
    }

    ret = IMP_Encoder_RegisterChn(grp_nr, chn_nr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_RegisterChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    else
    {
        LOG_DEBUG("IMP_Encoder_RegisterChn(" << chn_nr << ") registered");
    }
    return 1;
}

int Encoder::channel_deinit(int chn_nr)
{

    int ret;

    ret = IMP_Encoder_UnRegisterChn(chn_nr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_UnRegisterChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    else
    {
        LOG_DEBUG("Encoder Channel " << chn_nr << " unregistered");
    }

    ret = IMP_Encoder_DestroyChn(chn_nr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_DestroyChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    else
    {
        LOG_DEBUG("Encoder Channel " << chn_nr << " destroyed");
    }

    return 1;
}

int Encoder::system_init()
{
    LOG_DEBUG("Encoder::system_init()");
    int ret = 0;

    // If you are having problems with video quality, please make
    // sure you are linking against the latest version of libimp
    // available for the target soc
    IMPVersion impVersion;
    ret = IMP_System_GetVersion(&impVersion);
    LOG_INFO("LIBIMP Version " << impVersion.aVersion);

    SUVersion suVersion;
    ret = SU_Base_GetVersion(&suVersion);
    LOG_INFO("SYSUTILS Version: " << suVersion.chr);

    const char *cpuInfo = IMP_System_GetCPUInfo();
    LOG_INFO("CPU Information: " << cpuInfo);

    ret = IMP_OSD_SetPoolSize(OSDPoolSize);
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_OSD_SetPoolSize == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("IMP_OSD_SetPoolSize == " + std::to_string(OSDPoolSize));
    }

    ret = IMP_ISP_Open();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_Open() == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("ISP Opened!");
    }

    sinfo = create_sensor_info(cfg->sensor.model.c_str());
    ret = IMP_ISP_AddSensor(&sinfo);
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_AddSensor() == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("Sensor Added");
    }

    ret = IMP_ISP_EnableSensor();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_EnableSensor() == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("Sensor Enabled");
    }

    ret = IMP_System_Init();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_System_Init() == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("IMP System Initialized");
    }

    // Enable tuning.
    // This is necessary to customize the sensor's image output.
    // Denoising, WDR, Night Mode, and FPS customization require this.
    ret = IMP_ISP_EnableTuning();
    if (ret < 0)
    {
        LOG_DEBUG("ERROR: IMP_ISP_EnableTuning() == " + std::to_string(ret));
        return ret;
    }
    else
    {
        LOG_DEBUG("IMP_ISP_EnableTuning enabled");
    }

    /* Image tuning */
    ret = IMP_ISP_Tuning_SetContrast(cfg->image.contrast);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetSharpness(cfg->image.sharpness);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetSharpness(" << cfg->image.sharpness << ")");

    ret = IMP_ISP_Tuning_SetSaturation(cfg->image.saturation);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetBrightness(cfg->image.brightness);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetBrightness(" << cfg->image.brightness << ")");

    ret = IMP_ISP_Tuning_SetSinterStrength(cfg->image.sinter_strength);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetTemperStrength(cfg->image.temper_strength);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetTemperStrength(" << cfg->image.temper_strength << ")");

    ret = IMP_ISP_Tuning_SetISPHflip((IMPISPTuningOpsMode)cfg->image.hflip);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetISPHflip(" << cfg->image.hflip << ")");

    ret = IMP_ISP_Tuning_SetISPVflip((IMPISPTuningOpsMode)cfg->image.vflip);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetISPVflip(" << cfg->image.vflip << ")");

    ret = IMP_ISP_Tuning_SetISPRunningMode((IMPISPRunningMode)cfg->image.running_mode);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetISPRunningMode(" << cfg->image.running_mode << ")");

    ret = IMP_ISP_Tuning_SetISPBypass(IMPISP_TUNING_OPS_MODE_ENABLE);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetISPBypass(" << IMPISP_TUNING_OPS_MODE_ENABLE << ")");

    ret = IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)cfg->image.anti_flicker);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetAntiFlickerAttr(" << cfg->image.anti_flicker << ")");

#if !defined(PLATFORM_T21)
    ret = IMP_ISP_Tuning_SetAeComp(cfg->image.ae_compensation);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetAeComp(" << cfg->image.ae_compensation << ")");
#endif

    ret = IMP_ISP_Tuning_SetHiLightDepress(cfg->image.highlight_depress);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetHiLightDepress(" << cfg->image.highlight_depress << ")");

    ret = IMP_ISP_Tuning_SetMaxAgain(cfg->image.max_again);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetMaxAgain(" << cfg->image.max_again << ")");

    ret = IMP_ISP_Tuning_SetMaxDgain(cfg->image.max_dgain);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetMaxDgain(" << cfg->image.max_dgain << ")");

    IMPISPWB wb;
    memset(&wb, 0, sizeof(IMPISPWB));
    ret = IMP_ISP_Tuning_GetWB(&wb);
    if (ret == 0)
    {
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
    }

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBcshHue(cfg->image.hue);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetBcshHue(" << cfg->image.hue << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    uint8_t _defog_strength = static_cast<uint8_t>(cfg->image.defog_strength);
    ret = IMP_ISP_Tuning_SetDefog_Strength(reinterpret_cast<uint8_t *>(&_defog_strength));
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetDefog_Strength(" << cfg->image.defog_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDPC_Strength(cfg->image.dpc_strength);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetDPC_Strength(" << cfg->image.dpc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetDRC_Strength(cfg->image.dpc_strength);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetDRC_Strength(" << cfg->image.drc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBacklightComp(cfg->image.backlight_compensation);
    LOG_ERROR_OR_DEBUG(ret, "IMP_ISP_Tuning_SetBacklightComp(" << cfg->image.backlight_compensation << ")");
#endif

#if defined(AUDIO_SUPPORT)   
    /* Audio tuning */
    /*     input    */
    if (cfg->audio.input_enabled)
    {
        ret = IMP_AI_Enable(0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_Enable(0)");

        ret = IMP_AI_EnableChn(0, 0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_EnableChn(0, 0)");

        ret = IMP_AI_SetVol(0, 0, cfg->audio.input_vol);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_SetVol(0, 0, " << cfg->audio.input_vol << ")");

        ret = IMP_AI_SetGain(0, 0, cfg->audio.input_gain);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_SetGain(0, 0, " << cfg->audio.input_gain << ")");

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)

        ret = IMP_AI_SetAlcGain(0, 0, cfg->audio.input_alc_gain);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_SetAlcGain(0, 0, " << cfg->audio.input_alc_gain << ")");

#endif
        if (cfg->audio.input_echo_cancellation)
        {
            ret = IMP_AI_EnableAec(0, 0, 0, 0);
            LOG_ERROR_OR_DEBUG(ret, "IMP_AI_EnableAec(0)");
        }
        else
        {
            ret = IMP_AI_DisableAec(0, 0);
            LOG_ERROR_OR_DEBUG(ret, "IMP_AI_DisableAec(0)");
        }

        IMPAudioIOAttr ioattr;
        ret = IMP_AI_GetPubAttr(0, &ioattr);
        if (ret == 0)
        {
            if (cfg->audio.input_noise_suppression)
            {
                ret = IMP_AI_EnableNs(&ioattr, cfg->audio.input_noise_suppression);
                LOG_ERROR_OR_DEBUG(ret, "IMP_AI_EnableNs(" << cfg->audio.input_noise_suppression << ")");
            }
            else
            {
                ret = IMP_AI_DisableNs();
                LOG_ERROR_OR_DEBUG(ret, "IMP_AI_DisableNs(0)");
            }
            if (cfg->audio.output_high_pass_filter)
            {
                ret = IMP_AI_EnableHpf(&ioattr);
                LOG_ERROR_OR_DEBUG(ret, "IMP_AI_EnableHpf(0)");
            }
            else
            {
                ret = IMP_AI_DisableHpf();
                LOG_ERROR_OR_DEBUG(ret, "IMP_AI_DisableHpf(0)");
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
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_DisableChn(0)"); 

        ret = IMP_AI_Disable(0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AI_Disable(0)");
    }

    /*    output    */
    if (cfg->audio.output_enabled)
    {
        ret = IMP_AO_Enable(0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_Enable(0)");

        ret = IMP_AO_EnableChn(0, 0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_EnableChn(0, 0)"); 

        ret = IMP_AO_SetVol(0, 0, cfg->audio.output_vol);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_SetVol(0, 0, " << cfg->audio.output_vol << ")");

        ret = IMP_AO_SetGain(0, 0, cfg->audio.output_gain);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_SetGain(0, 0, " << cfg->audio.output_gain << ")");

        IMPAudioIOAttr ioattr;
        ret = IMP_AO_GetPubAttr(0, &ioattr);
        if (ret == 0)
        {     
            if (cfg->audio.output_high_pass_filter)
            {
                ret = IMP_AO_EnableHpf(&ioattr);
                LOG_ERROR_OR_DEBUG(ret, "IMP_AO_EnableHpf(0)");
            }
            else
            {
                ret = IMP_AO_DisableHpf();
                LOG_ERROR_OR_DEBUG(ret, "IMP_AO_DisableHpf(0)");
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
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_DisableChn(0, 0)"); 

        ret = IMP_AO_Disable(0);
        LOG_ERROR_OR_DEBUG(ret, "IMP_AO_Disable(0)");
    }
#endif //#if defined(AUDIO_SUPPORT)   

    LOG_DEBUG("ISP Tuning Defaults set");

    ret = IMP_ISP_Tuning_SetSensorFPS(cfg->sensor.fps, 1);
    if (ret < 0)
    {
        LOG_DEBUG("ERROR: IMP_ISP_Tuning_SetSensorFPS() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_Tuning_SetSensorFPS == " + std::to_string(cfg->sensor.fps));

    // Set the ISP to DAY on launch
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0)
    {
        LOG_DEBUG("ERROR: IMP_ISP_Tuning_SetISPRunningMode() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_Tuning_SetISPRunningMode == " + std::to_string(IMPISP_RUNNING_MODE_DAY));

    return ret;
}

int Encoder::framesource_init()
{
    LOG_DEBUG("Encoder::framesource_init()");
    int ret = 0;

    /* FrameSource highres channel */
    IMPFSChnAttr fs_high_chn_attr;
    memset(&fs_high_chn_attr, 0, sizeof(IMPFSChnAttr));

    fs_high_chn_attr.pixFmt = PIX_FMT_NV12;
    fs_high_chn_attr.outFrmRateNum = cfg->stream0.fps;
    fs_high_chn_attr.outFrmRateDen = 1;
    fs_high_chn_attr.nrVBs = cfg->stream0.buffers;
    fs_high_chn_attr.type = FS_PHY_CHANNEL;
    fs_high_chn_attr.crop.enable = 0;
    fs_high_chn_attr.crop.top = 0;
    fs_high_chn_attr.crop.left = 0;
    fs_high_chn_attr.crop.width = cfg->stream0.width;
    fs_high_chn_attr.crop.height = cfg->stream0.height;
    fs_high_chn_attr.scaler.enable = cfg->stream0.scale_enabled;
    fs_high_chn_attr.scaler.outwidth = cfg->stream0.scale_width;
    fs_high_chn_attr.scaler.outheight = cfg->stream0.scale_width;
    fs_high_chn_attr.picWidth = cfg->stream0.width; // Testing stream size sync
    fs_high_chn_attr.picHeight = cfg->stream0.height;

#if defined(LOW_STREAM)
    /* FrameSource lowres channel */
    IMPFSChnAttr fs_low_chn_attr;
    memset(&fs_low_chn_attr, 0, sizeof(IMPFSChnAttr));

    fs_low_chn_attr.pixFmt = PIX_FMT_NV12;
    fs_low_chn_attr.outFrmRateNum = cfg->stream0.fps;
    fs_low_chn_attr.outFrmRateDen = 1;
    fs_low_chn_attr.nrVBs = 1;
    fs_low_chn_attr.type = FS_PHY_CHANNEL;
    fs_low_chn_attr.picWidth = 640;
    fs_low_chn_attr.picHeight = 340;
    fs_low_chn_attr.scaler.enable = 1;
    fs_low_chn_attr.scaler.outwidth = 640;
    fs_low_chn_attr.scaler.outheight = 340;
#endif    
#if !defined(KERNEL_VERSION_4)
#if defined(PLATFORM_T31)

    int rotation = cfg->stream0.rotation;
    int rot_height = cfg->stream0.height;
    int rot_width = cfg->stream0.width;

    // Set rotate before FS creation
    // IMP_Encoder_SetFisheyeEnableStatus(0, 1);
    //ret = IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width);
    //LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width)");
#endif
#endif

    /* FrameSource lowres channel */
    ret = IMP_FrameSource_CreateChn(0, &fs_high_chn_attr);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_CreateChn(0, &fs_high_chn_attr)");

    ret = IMP_FrameSource_SetChnAttr(0, &fs_high_chn_attr);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnAttr(0, &fs_high_chn_attr)");

    IMPFSChnFifoAttr fifo;
    ret = IMP_FrameSource_GetChnFifoAttr(0, &fifo);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_GetChnFifoAttr(0, &fifo)");

    fifo.maxdepth = 0;
    ret = IMP_FrameSource_SetChnFifoAttr(0, &fifo);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnFifoAttr(0, &fifo)");

    ret = IMP_FrameSource_SetFrameDepth(0, 0);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetFrameDepth(0, 0)");
    
#if defined(LOW_STREAM)
    /* FrameSource lowres channel */
    ret = IMP_FrameSource_CreateChn(1, &fs_low_chn_attr);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_CreateChn(1, &fs_low_chn_attr)");

    ret = IMP_FrameSource_SetChnAttr(1, &fs_low_chn_attr);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnAttr(1, &fs_low_chn_attr)");

    IMPFSChnFifoAttr low_fifo;
    ret = IMP_FrameSource_GetChnFifoAttr(1, &low_fifo);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_GetChnFifoAttr(1, &low_fifo)");

    fifo.maxdepth = 0;
    ret = IMP_FrameSource_SetChnFifoAttr(1, &low_fifo);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnFifoAttr(1, &low_fifo)");

    ret = IMP_FrameSource_SetFrameDepth(1, 0);
    LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetFrameDepth(1, 0)");
#endif

    return ret;
}

int Encoder::encoder_init()
{
    int ret = 0;

    /* Encoder highres channel */
    IMPEncoderRcAttr *rc_high_attr;
    IMPEncoderCHNAttr high_channel_attr;
    memset(&high_channel_attr, 0, sizeof(IMPEncoderCHNAttr));
    rc_high_attr = &high_channel_attr.rcAttr;

#if defined(LOW_STREAM)
    /* Encoder lowres channel */
    IMPEncoderRcAttr *rc_low_attr;
    IMPEncoderCHNAttr low_channel_attr;
    memset(&low_channel_attr, 0, sizeof(IMPEncoderCHNAttr));
    rc_low_attr = &low_channel_attr.rcAttr;
#endif

#if defined(PLATFORM_T31)

    IMPEncoderProfile encoderProfile;

    // Allow user to specify the profile directly in the future with fallback defaults
    const std::string &format = cfg->stream0.format;
    encoderProfile = (format == "H265") ? IMP_ENC_PROFILE_HEVC_MAIN : IMP_ENC_PROFILE_AVC_HIGH;

     /* Encoder highres channel */
    IMP_Encoder_SetDefaultParam(
        &high_channel_attr, encoderProfile, IMP_ENC_RC_MODE_CAPPED_QUALITY, cfg->stream0.width, cfg->stream0.height,
        cfg->stream0.fps, 1, cfg->stream0.gop, 2,
        -1, cfg->stream0.bitrate);

    switch (rc_high_attr->attrRcMode.rcMode)
    {
    case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rc_high_attr->attrRcMode.attrVbr.uTargetBitRate = cfg->stream0.bitrate;
        rc_high_attr->attrRcMode.attrVbr.uMaxBitRate = cfg->stream0.bitrate;
        rc_high_attr->attrRcMode.attrVbr.iInitialQP = -1;
        rc_high_attr->attrRcMode.attrVbr.iMinQP = 20;
        rc_high_attr->attrRcMode.attrVbr.iMaxQP = 45;
        rc_high_attr->attrRcMode.attrVbr.iIPDelta = 3;
        rc_high_attr->attrRcMode.attrVbr.iPBDelta = 3;
        // rc_high_attr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rc_high_attr->attrRcMode.attrVbr.uMaxPictureSize = cfg->stream0.width;
        rc_high_attr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    }

#if defined(LOW_STREAM)
    /* Encoder lowres channel */
    // Allow user to specify the profile directly in the future with fallback defaults
    IMP_Encoder_SetDefaultParam(
        &low_channel_attr, encoderProfile, IMP_ENC_RC_MODE_CAPPED_QUALITY, 640, 340,
        cfg->stream0.fps, 1, cfg->stream0.gop, 2,
        -1, 500);

    switch (rc_low_attr->attrRcMode.rcMode)
    {
    case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rc_low_attr->attrRcMode.attrVbr.uTargetBitRate = 500;
        rc_low_attr->attrRcMode.attrVbr.uMaxBitRate = 500;
        rc_low_attr->attrRcMode.attrVbr.iInitialQP = -1;
        rc_low_attr->attrRcMode.attrVbr.iMinQP = 20;
        rc_low_attr->attrRcMode.attrVbr.iMaxQP = 45;
        rc_low_attr->attrRcMode.attrVbr.iIPDelta = 3;
        rc_low_attr->attrRcMode.attrVbr.iPBDelta = 3;
        // rc_low_attr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rc_low_attr->attrRcMode.attrVbr.uMaxPictureSize = 640;
        rc_low_attr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    }
#endif //LOW_STREAM

#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
    // high_channel_attr.encAttr.enType = PT_JPEG;
    const std::string &format = cfg->stream0.format;

#if defined(PLATFORM_T30)
    high_channel_attr.encAttr.enType = (format == "H264") ? PT_H264 : (format == "H265") ? PT_H265
                                                                                    : PT_H264;
#else
    high_channel_attr.encAttr.enType = (format == "H264") ? PT_H264 : PT_H264;
#endif

    high_channel_attr.encAttr.bufSize = 0;

    // 0 = Baseline
    // 1 = Main
    // 2 = High
    // Note: The encoder seems to emit frames at half the
    // requested framerate when the profile is set to Baseline.
    // For this reason, Main or High are recommended.
    high_channel_attr.encAttr.profile = 2;
    high_channel_attr.encAttr.picWidth = cfg->stream0.width;
    high_channel_attr.encAttr.picHeight = cfg->stream0.height;

    high_channel_attr.rcAttr.outFrmRate.frmRateNum = cfg->stream0.fps;
    high_channel_attr.rcAttr.outFrmRate.frmRateDen = 1;

    // Setting maxGop to a low value causes the encoder to emit frames at a much
    // slower rate. A sufficiently low value can cause the frame emission rate to
    // drop below the frame rate.
    // I find that 2x the frame rate is a good setting.
    rc_high_attr->maxGop = cfg->stream0.max_gop;

    if (cfg->stream0.format == "H264")
    {
        rc_high_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rc_high_attr->attrRcMode.attrH264Smart.maxQp = 45;
        rc_high_attr->attrRcMode.attrH264Smart.minQp = 24;
        rc_high_attr->attrRcMode.attrH264Smart.staticTime = 2;
        rc_high_attr->attrRcMode.attrH264Smart.maxBitRate = cfg->stream0.bitrate;
        rc_high_attr->attrRcMode.attrH264Smart.iBiasLvl = 0;
        rc_high_attr->attrRcMode.attrH264Smart.changePos = 80;
        rc_high_attr->attrRcMode.attrH264Smart.qualityLvl = 0;
        rc_high_attr->attrRcMode.attrH264Smart.frmQPStep = 3;
        rc_high_attr->attrRcMode.attrH264Smart.gopQPStep = 15;
        rc_high_attr->attrRcMode.attrH264Smart.gopRelation = false;
#if defined(PLATFORM_T30)
    }
    else if (cfg->stream0.format == "H265")
    {
        rc_high_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rc_high_attr->attrRcMode.attrH265Smart.maxQp = 45;
        rc_high_attr->attrRcMode.attrH265Smart.minQp = 15;
        rc_high_attr->attrRcMode.attrH265Smart.staticTime = 2;
        rc_high_attr->attrRcMode.attrH265Smart.maxBitRate = cfg->stream0.bitrate;
        rc_high_attr->attrRcMode.attrH265Smart.iBiasLvl = 0;
        rc_high_attr->attrRcMode.attrH265Smart.changePos = 80;
        rc_high_attr->attrRcMode.attrH265Smart.qualityLvl = 2;
        rc_high_attr->attrRcMode.attrH265Smart.frmQPStep = 3;
        rc_high_attr->attrRcMode.attrH265Smart.gopQPStep = 15;
        rc_high_attr->attrRcMode.attrH265Smart.flucLvl = 2;
#endif
    }

    rc_high_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X; // IMP_Encoder_STYPE_HN1_TRUE
    rc_high_attr->attrHSkip.hSkipAttr.m = rc_high_attr->maxGop - 1;
    rc_high_attr->attrHSkip.hSkipAttr.n = 1;
    rc_high_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
    rc_high_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
    rc_high_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
    rc_high_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X; // IMP_Encoder_STYPE_N4X

#endif

#if defined(PLATFORM_T31)
    if (cfg->stream1.jpeg_enabled)
    {
        ret = IMP_Encoder_SetbufshareChn(2, 0);
        if (ret < 0)
        {
            LOG_ERROR("IMP_Encoder_SetbufshareChn(2, 0) == " << ret);
        }
        LOG_DEBUG("IMP_Encoder_SetbufshareChn(2, 0) enabled");
    }
#endif

    /* Encoder highres channel */
    ret = channel_init(0, 0, &high_channel_attr);
    if (ret < 0)
    {
        LOG_ERROR("channel_init(0, 0, &high_channel_attr) == " << ret);
        return ret;
    }

#if defined(LOW_STREAM)
    /* Encoder lowres channel */
    ret = channel_init(1, 1, &low_channel_attr);
    if (ret < 0)
    {
        LOG_ERROR("channel_init(1, 1, &low_channel_attr) == " << ret);
        return ret;
    }
#endif
#if defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
/*
    //The SuperFrame configuration basically puts
    //a hard upper limit on NAL sizes. The defaults are
    //insane, you would never hit them normally.
    IMPEncoderSuperFrmCfg sfcfg;
    IMP_Encoder_GetSuperFrameCfg(0, &sfcfg);
    sfcfg.superIFrmBitsThr = 250000*8;
    sfcfg.superPFrmBitsThr = 60000*8;
    sfcfg.rcPriority = IMP_RC_PRIORITY_FRAMEBITS_FIRST;
    sfcfg.superFrmMode = IMP_RC_SUPERFRM_REENCODE;
    ret = IMP_Encoder_SetSuperFrameCfg(0, &sfcfg);*/
#endif
    return 0;
}

bool Encoder::init()
{
    LOG_DEBUG("Encoder::init()");
    int ret = 0;

    ret = system_init();
    LOG_ERROR_OR_DEBUG_EXIT(ret, "system_init(0)");

    ret = framesource_init();
    LOG_ERROR_OR_DEBUG_EXIT(ret, "framesource_init(0)");

    ret = encoder_init();
    LOG_ERROR_OR_DEBUG_EXIT(ret, "encoder_init(0)");

    /* Encoder highres channel */
    ret = IMP_Encoder_CreateGroup(0);
    LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_Encoder_CreateGroup(0)");

#if defined(LOW_STREAM)
    /* Encoder lowres channel */
    ret = IMP_Encoder_CreateGroup(1);
    LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_Encoder_CreateGroup(1)");
#endif

    if (!cfg->osd.enabled)
    {
        LOG_DEBUG("OSD disabled");

        // high framesource -> high ENC
        ret = IMP_System_Bind(&high_fs, &high_enc);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&high_fs, &high_enc)");

#if defined(LOW_STREAM)
        // low framesource -> low ENC
        ret = IMP_System_Bind(&low_fs, &low_enc);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&low_fs, &low_enc)");        
#endif
    }
    else
    {
        // If OSD is enabled, initialize OSD and bind FrameSource to OSD, then OSD to Encoder
        LOG_DEBUG("OSD enabled");
        osdInitialized = true;

        //ret = osd.init(cfg);
        stream0_osd = OSD::createNew(cfg, 0, 0);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "osd.init(cfg, 0)");

#if defined(LOW_STREAM)
        stream1_osd = OSD::createNew(cfg, 1, 1);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "osd.init(cfg, 1)");
#endif

        // high framesource -> high OSD
        ret = IMP_System_Bind(&high_fs, &high_osd_cell);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&high_fs, &high_osd_cell)");

        // high OSD -> high Encoder
        ret = IMP_System_Bind(&high_osd_cell, &high_enc);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&high_osd_cell, &high_enc)");

#if defined(LOW_STREAM)
        // low framesource -> low OSD
        ret = IMP_System_Bind(&low_fs, &low_osd_cell);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&low_fs, &low_osd_cell)");

        // low OSD -> low Encoder
        ret = IMP_System_Bind(&low_osd_cell, &low_enc);
        LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_System_Bind(&low_osd_cell, &low_enc)"); 
#endif       
    }

    ret = IMP_FrameSource_EnableChn(0);
    LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_FrameSource_EnableChn(0)");

#if defined(LOW_STREAM)
    ret = IMP_FrameSource_EnableChn(1);
    LOG_ERROR_OR_DEBUG_EXIT(ret, "IMP_FrameSource_EnableChn(1)");
#endif

    if (cfg->motion.enabled)
    {

        LOG_DEBUG("Motion enabled");
        motionInitialized = true;

        ret = motion.init(cfg);
        if (!ret)
        {
            LOG_ERROR("Motion Init Failed");
            return true;
        }
    }

    return false;
}

void Encoder::exit()
{

    int ret = 0, i = 0, chnNum = 0;
    IMPEncoderCHNStat chn_stat;

#if !defined(LOW_STREAM)
    if (cfg->stream1.jpeg_enabled)
    {
        cfg->jpg_thread_signal.fetch_or(4);
    }
#endif

    ret = ret = IMP_FrameSource_DisableChn(chnNum);
    LOG_DEBUG("IMP_FrameSource_DisableChn(" << chnNum << ") error: " << ret);

#if defined(LOW_STREAM)
    ret = ret = IMP_FrameSource_DisableChn(1);
    LOG_DEBUG("IMP_FrameSource_DisableChn(1) error: " << ret);

    ret = IMP_System_UnBind(&low_fs, &low_enc);
    LOG_DEBUG("IMP_System_UnBind 1 error: " << ret);
#endif

    ret = IMP_System_UnBind(&high_fs, &high_osd_cell);
    LOG_DEBUG("IMP_System_UnBind 1 error: " << ret);

    ret = IMP_System_UnBind(&high_osd_cell, &high_enc);
    LOG_DEBUG("IMP_System_UnBind 2 error: " << ret);

    memset(&chn_stat, 0, sizeof(IMPEncoderCHNStat));

    for (int i = 1; i >= 0; i--)
    {

        ret = IMP_Encoder_Query(i, &chn_stat);
        LOG_DEBUG("IMP_Encoder_Query(" << i << ") registred: " << chn_stat.registered << ", ret: " << ret);

        if (ret == 0 && chn_stat.registered)
        {

            channel_deinit(i);

            ret = IMP_FrameSource_DestroyChn(i);
            LOG_DEBUG("IMP_FrameSource_DestroyChn(" << i << ") error: " << ret);
        }
    }

    ret = IMP_Encoder_DestroyGroup(0);
    LOG_DEBUG("IMP_Encoder_DestroyGroup(0) error: " << ret);

#if defined(LOW_STREAM)
    ret = IMP_Encoder_DestroyGroup(1);
    LOG_DEBUG("IMP_Encoder_DestroyGroup(1) error: " << ret);
#endif

    if (osdInitialized) {
        stream0_osd->exit();
        stream1_osd->exit();
    }

    if (motionInitialized)
    {
        if (!motion.exit())
        {
            LOG_ERROR("Exit motion detection failed.");
        }
    }

    IMP_System_Exit();
    LOG_DEBUG("IMP_System_Exit");

    ret = IMP_ISP_DisableSensor();
    if (ret < 0)
    {
        LOG_DEBUG("failed to EnableSensor\n");
    }

    ret = IMP_ISP_DelSensor(&sinfo);
    if (ret < 0)
    {
        LOG_DEBUG("failed to AddSensor");
    }

    ret = IMP_ISP_DisableTuning();
    if (ret < 0)
    {
        LOG_DEBUG("failed to disable tuning");
    }

    IMP_ISP_Close();
    LOG_DEBUG("IMP_ISP_Close");

    cfg->encoder_thread_signal.fetch_xor(4); // remove stopping
    cfg->encoder_thread_signal.fetch_or(8);  // set stopped
}

static int save_jpeg_stream(int fd, IMPEncoderStream *stream)
{
    int ret, i, nr_pack = stream->packCount;

    for (i = 0; i < nr_pack; i++)
    {
        void *data_ptr;
        size_t data_len;

#if defined(PLATFORM_T31)
        IMPEncoderPack *pack = &stream->pack[i];
        uint32_t remSize = 0; // Declare remSize here
        if (pack->length)
        {
            remSize = stream->streamSize - pack->offset;
            data_ptr = (void *)((char *)stream->virAddr + ((remSize < pack->length) ? 0 : pack->offset));
            data_len = (remSize < pack->length) ? remSize : pack->length;
        }
        else
        {
            continue; // Skip empty packs
        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
        data_ptr = reinterpret_cast<void *>(stream->pack[i].virAddr);
        data_len = stream->pack[i].length;
#endif

        // Write data to file
        ret = write(fd, data_ptr, data_len);
        if (ret != static_cast<int>(data_len))
        {
            printf("Stream write error: %s\n", strerror(errno));
            return -1; // Return error on write failure
        }

#if defined(PLATFORM_T31)
        // Check the condition only under T31 platform, as remSize is used here
        if (remSize && pack->length > remSize)
        {
            ret = write(fd, (void *)((char *)stream->virAddr), pack->length - remSize);
            if (ret != static_cast<int>(pack->length - remSize))
            {
                printf("Stream write error (remaining part): %s\n", strerror(errno));
                return -1;
            }
        }
#endif
    }

    return 0;
}

void MakeTables(int q, uint8_t *lqt, uint8_t *cqt)
{
    // Ensure q is within the expected range
    q = std::max(1, std::min(q, 99));

    // Adjust q based on factor
    if (q < 50)
    {
        q = 5000 / q;
    }
    else
    {
        q = 200 - 2 * q;
    }

    // Fill the quantization tables
    for (int i = 0; i < 64; ++i)
    {
        int lq = (jpeg_luma_quantizer[i] * q + 50) / 100;
        int cq = (jpeg_chroma_quantizer[i] * q + 50) / 100;

        // Ensure the quantization values are within [1, 255]
        lqt[i] = static_cast<uint8_t>(std::max(1, std::min(lq, 255)));
        cqt[i] = static_cast<uint8_t>(std::max(1, std::min(cq, 255)));
    }
}

void Encoder::jpeg_snap(std::shared_ptr<CFG> &cfg)
{
    nice(-18);

    int ret;
    // IMPEncoderRcAttr *rc_attr;
    IMPEncoderCHNAttr channel_attr_jpg;

    // memset(&channel_attr_jpg, 0, sizeof(IMPEncoderCHNAttr));
    // rc_attr = &channel_attr_jpg.rcAttr;

    while ((cfg->jpg_thread_signal.load() & 256) != 256)
    {
        // init
        if (cfg->jpg_thread_signal.load() & 1)
        {
            LOG_DEBUG("Init jpeg thread.");
#if defined(PLATFORM_T31)
            IMP_Encoder_SetDefaultParam(&channel_attr_jpg, IMP_ENC_PROFILE_JPEG, IMP_ENC_RC_MODE_FIXQP,
                                        cfg->stream0.width, cfg->stream0.height, 24, 1, 0, 0, cfg->stream1.jpeg_quality, 0);

#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
            IMPEncoderAttr *enc_attr;
            enc_attr = &channel_attr_jpg.encAttr;
            enc_attr->enType = PT_JPEG;
            enc_attr->bufSize = 0;
            enc_attr->profile = 2;
            enc_attr->picWidth = cfg->stream0.width;
            enc_attr->picHeight = cfg->stream0.height;
#endif

            ret = channel_init(2, 0, &channel_attr_jpg);
            if (ret < 0)
            {
                LOG_ERROR("channel_init() == " << ret);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait for ISP to init before saving initial image

            IMP_Encoder_StartRecvPic(2); // Start receiving pictures once

            cfg->jpg_thread_signal.fetch_or(2);
            LOG_DEBUG("Start jpeg thread.");
        }

        // running
        while (cfg->jpg_thread_signal.load() & 2)
        {

#if defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
            IMPEncoderJpegeQl pstJpegeQl;
            MakeTables(cfg->stream1.jpeg_quality, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
            pstJpegeQl.user_ql_en = 1;
            IMP_Encoder_SetJpegeQl(2, &pstJpegeQl);
#endif

            IMP_Encoder_PollingStream(2, 10000); // Wait for frame

            IMPEncoderStream stream_jpeg;
            if (IMP_Encoder_GetStream(2, &stream_jpeg, 1) == 0)
            {                                                   // Check for success
                std::string tempPath = "/tmp/snapshot.tmp";     // Temporary path
                std::string finalPath = cfg->stream1.jpeg_path; // Final path for the JPEG snapshot

                // Open and create temporary file with read and write permissions
                int snap_fd = open(tempPath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777);
                if (snap_fd >= 0)
                {
                    // Attempt to lock the temporary file for exclusive access
                    if (flock(snap_fd, LOCK_EX) == -1)
                    {
                        LOG_ERROR("Failed to lock JPEG snapshot for writing: " + tempPath);
                        close(snap_fd);
                        return; // Exit the function if unable to lock the file
                    }

                    // Save the JPEG stream to the file
                    save_jpeg_stream(snap_fd, &stream_jpeg);

                    // Unlock and close the temporary file after writing is done
                    flock(snap_fd, LOCK_UN);
                    close(snap_fd);

                    // Atomically move the temporary file to the final destination
                    if (rename(tempPath.c_str(), finalPath.c_str()) != 0)
                    {
                        LOG_ERROR("Failed to move JPEG snapshot from " + tempPath + " to " + finalPath);
                        std::remove(tempPath.c_str()); // Attempt to remove the temporary file if rename fails
                    }
                    else
                    {
                        // LOG_DEBUG("JPEG snapshot successfully updated");
                    }
                }
                else
                {
                    LOG_ERROR("Failed to open JPEG snapshot for writing: " + tempPath);
                }

                // Delay before we release, otherwise an overflow may occur
                std::this_thread::sleep_for(std::chrono::milliseconds(cfg->stream1.jpeg_refresh)); // Control the rate
                IMP_Encoder_ReleaseStream(2, &stream_jpeg);                                        // Release stream after saving
            }

            if (cfg->jpg_thread_signal.load() & 4)
            {

                LOG_DEBUG("Stop jpeg thread.");

                IMP_Encoder_StopRecvPic(2);

                cfg->jpg_thread_signal.fetch_xor(7);
                cfg->jpg_thread_signal.fetch_or(8);
            }
        }

        usleep(1000);
    }
}

void Encoder::run()
{
    LOG_DEBUG("Encoder::run()");

    // The encoder thread is very important, but we
    // want sink threads to have higher priority.
    //nice(-19);

    int64_t last_high_nal_ts;
    int64_t last_low_nal_ts;

    // 256 = exit thread
    while ((cfg->encoder_thread_signal.load() & 256) != 256)
    {

        last_high_nal_ts = 0;
        last_low_nal_ts = 0;

        // 1 = init and start
        if (cfg->encoder_thread_signal.load() & 1)
        {

            init();

            IMP_System_RebaseTimeStamp(0);

            gettimeofday(&high_imp_time_base, NULL);
            IMP_Encoder_StartRecvPic(0);
            
            gettimeofday(&low_imp_time_base, NULL);
            IMP_Encoder_StartRecvPic(1);

            LOG_DEBUG("Encoder StartRecvPic(0) success");

            if (cfg->stream1.jpeg_enabled)
            {

                LOG_DEBUG("JPEG support enabled");
                cfg->jpg_thread_signal.fetch_xor(8); // remove stopped bit
                if (jpeg_thread.joinable())
                {
                    cfg->jpg_thread_signal.fetch_or(1);
                }
                else
                {
                    jpeg_thread = std::thread(&Encoder::jpeg_snap, this, std::ref(cfg));
                }
            }

            cfg->rtsp_thread_signal = 0;
            cfg->encoder_thread_signal.fetch_or(2);
        }

        while (cfg->encoder_thread_signal.load() & 2)
        {

            IMPEncoderStream stream0;
            if (IMP_Encoder_GetStream(0, &stream0, false) != 0)
            {
                LOG_ERROR("IMP_Encoder_GetStream(0) failed");
                break;
            }
            // The I/P NAL is always last, but it doesn't
            // really matter which NAL we select here as they
            // all have identical timestamps.
            int64_t high_nal_ts = stream0.pack[stream0.packCount - 1].timestamp;
            if (high_nal_ts - last_high_nal_ts > 1.5 * (1000000 / cfg->stream0.fps))
            {
                // Silence for now until further tests / THINGINO
                //LOG_WARN("The encoder 0 dropped a frame. " << (high_nal_ts - last_high_nal_ts) << ", " << (1.5 * (1000000 / cfg->stream0.fps)));
            }
            struct timeval high_encode_time;
            high_encode_time.tv_sec = high_nal_ts / 1000000;
            high_encode_time.tv_usec = high_nal_ts % 1000000;

            for (unsigned int i = 0; i < stream0.packCount; ++i)
            {
                //std::cout << last_high_nal_ts << " " << stream0.pack[i].timestamp << " " << (high_nal_ts - last_high_nal_ts) <<std::endl;

#if defined(PLATFORM_T31)
                uint8_t *start0 = (uint8_t *)stream0.virAddr + stream0.pack[i].offset;
                uint8_t *end0 = start0 + stream0.pack[i].length;
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                uint8_t *start0 = (uint8_t *)stream0.pack[i].virAddr;
                uint8_t *end0 = (uint8_t *)stream0.pack[i].virAddr + stream0.pack[i].length;
#endif

                H264NALUnit nalu0;
                nalu0.imp_ts = stream0.pack[i].timestamp;
                timeradd(&high_imp_time_base, &high_encode_time, &nalu0.time);
                nalu0.duration = 0;
#if defined(PLATFORM_T31)
                if (stream0.pack[i].nalType.h264NalType == 5 || stream0.pack[i].nalType.h264NalType == 1)
                {
                    nalu0.duration = high_nal_ts - last_high_nal_ts;
                }
                else if (stream0.pack[i].nalType.h265NalType == 19 ||
                         stream0.pack[i].nalType.h265NalType == 20 ||
                         stream0.pack[i].nalType.h265NalType == 1)
                {
                    nalu0.duration = high_nal_ts - last_high_nal_ts;
                }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                if (stream0.pack[i].dataType.h264Type == 5 || stream0.pack[i].dataType.h264Type == 1)
                {
                    nalu0.duration = high_nal_ts - last_high_nal_ts;
                }
#elif defined(PLATFORM_T30)
                if (stream0.pack[i].dataType.h264Type == 5 || stream0.pack[i].dataType.h264Type == 1)
                {
                    nalu0.duration = high_nal_ts - last_high_nal_ts;
                }
                else if (stream0.pack[i].dataType.h265Type == 19 ||
                         stream0.pack[i].dataType.h265Type == 20 ||
                         stream0.pack[i].dataType.h265Type == 1)
                {
                    nalu0.duration = high_nal_ts - last_high_nal_ts;
                }
#endif
                // We use start+4 because the encoder inserts 4-byte MPEG
                //'startcodes' at the beginning of each NAL. Live555 complains
                // if those are present.
                nalu0.data.insert(nalu0.data.end(), start0 + 4, end0);

                std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
                //for (std::map<uint32_t, EncoderSink>::iterator it = Encoder::sinks.begin();
                //     it != Encoder::sinks.end(); ++it)
                //{
                for (auto& [sinkId, sink] : Encoder::sinks) 
                {                    
                    if(sink.encChn == 0) {      
                        if(!sink.IDR) {          
#if defined(PLATFORM_T31)
                            if (stream0.pack[i].nalType.h264NalType == 7 ||
                                stream0.pack[i].nalType.h264NalType == 8 ||
                                stream0.pack[i].nalType.h264NalType == 5)
                            {
                                sink.IDR = true;
                            }
                            else if (stream0.pack[i].nalType.h265NalType == 32)
                            {
                                sink.IDR = true;
                            }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                            if (stream0.pack[i].dataType.h264Type == 7 ||
                                stream0.pack[i].dataType.h264Type == 8 ||
                                stream0.pack[i].dataType.h264Type == 5)
                            {
                                sink.IDR = true;
                            }
#elif defined(PLATFORM_T30)
                            if (stream0.pack[i].dataType.h264Type == 7 ||
                                stream0.pack[i].dataType.h264Type == 8 ||
                                stream0.pack[i].dataType.h264Type == 5)
                            {
                                sink.IDR = true;
                            }
                            else if (stream0.pack[i].dataType.h265Type == 32)
                            {
                                sink.IDR = true;
                            }
#endif
                        }
                        if (sink.IDR)
                        {
 
                            if(sink.data_available_callback(nalu0)) {

                                LOG_ERROR("stream 0, eC:" << sink.encChn << ", id:" << sinkId << ", size:" << nalu0.data.size() <<
                                        ", pC:" << stream0.packCount << ", pS:" << nalu0.data.size() << ", pN:" << i << 
                                        ", sC:" << Encoder::sinks.size() << 
                                        " clogged!");                              
                            }
                        }
                    }
                }
            }

            IMP_Encoder_ReleaseStream(0, &stream0);

            IMPEncoderStream stream1;
            if (IMP_Encoder_GetStream(1, &stream1, false) != 0)
            {
                LOG_ERROR("IMP_Encoder_GetStream(1) failed");
                break;
            }

            // The I/P NAL is always last, but it doesn't
            // really matter which NAL we select here as they
            // all have identical timestamps.
            int64_t low_nal_ts = stream1.pack[stream1.packCount - 1].timestamp;
            if (low_nal_ts - last_low_nal_ts > 1.5 * (1000000 / cfg->stream0.fps))
            {
                // Silence for now until further tests / THINGINO
                //LOG_WARN("The encoder 1 dropped a frame. " << (low_nal_ts - last_low_nal_ts) << ", " << (1.5 * (1000000 / cfg->stream0.fps)));
            }
            struct timeval low_encode_time;
            low_encode_time.tv_sec = low_nal_ts / 1000000;
            low_encode_time.tv_usec = low_nal_ts % 1000000;
       
            for (unsigned int i = 0; i < stream1.packCount; ++i)
            {
#if defined(PLATFORM_T31)
                uint8_t *start1 = (uint8_t *)stream1.virAddr + stream1.pack[i].offset;
                uint8_t *end1 = start1 + stream1.pack[i].length;
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                uint8_t *start1 = (uint8_t *)stream1.pack[i].virAddr;
                uint8_t *end1 = (uint8_t *)stream1.pack[i].virAddr + stream1.pack[i].length;
#endif

                H264NALUnit nalu1;
                nalu1.imp_ts = stream1.pack[i].timestamp;
                timeradd(&low_imp_time_base, &low_encode_time, &nalu1.time);
                nalu1.duration = 0;
#if defined(PLATFORM_T31)
                if (stream1.pack[i].nalType.h264NalType == 5 || stream1.pack[i].nalType.h264NalType == 1)
                {
                    nalu1.duration = low_nal_ts - last_low_nal_ts;
                }
                else if (stream1.pack[i].nalType.h265NalType == 19 ||
                         stream1.pack[i].nalType.h265NalType == 20 ||
                         stream1.pack[i].nalType.h265NalType == 1)
                {
                    nalu1.duration = low_nal_ts - last_low_nal_ts;
                }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                if (stream1.pack[i].dataType.h264Type == 5 || stream1.pack[i].dataType.h264Type == 1)
                {
                    nalu1.duration = low_nal_ts - last_low_nal_ts;
                }
#elif defined(PLATFORM_T30)
                if (stream1.pack[i].dataType.h264Type == 5 || stream1.pack[i].dataType.h264Type == 1)
                {
                    nalu1.duration = low_nal_ts - last_low_nal_ts;
                }
                else if (stream1.pack[i].dataType.h265Type == 19 ||
                         stream1.pack[i].dataType.h265Type == 20 ||
                         stream1.pack[i].dataType.h265Type == 1)
                {
                    nalu1.duration = low_nal_ts - last_low_nal_ts;
                }
#endif
                // We use start+4 because the encoder inserts 4-byte MPEG
                //'startcodes' at the beginning of each NAL. Live555 complains
                // if those are present.
                nalu1.data.insert(nalu1.data.end(), start1 + 4, end1);

                std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
                //for (std::map<uint32_t, EncoderSink>::iterator it = Encoder::sinks.begin();
                //     it != Encoder::sinks.end(); ++it)
                //{
                for (auto& [sinkId, sink] : Encoder::sinks)                     
                {
                    if(sink.encChn == 1) { 
                        if(!sink.IDR) {             
#if defined(PLATFORM_T31)
                            if (stream1.pack[i].nalType.h264NalType == 7 ||
                                stream1.pack[i].nalType.h264NalType == 8 ||
                                stream1.pack[i].nalType.h264NalType == 5)
                            {
                                sink.IDR = true;
                            }
                            else if (stream1.pack[i].nalType.h265NalType == 32)
                            {
                                sink.IDR = true;
                            }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                            if (stream1.pack[i].dataType.h264Type == 7 ||
                                stream1.pack[i].dataType.h264Type == 8 ||
                                stream1.pack[i].dataType.h264Type == 5)
                            {
                                sink.IDR = true;
                            }
#elif defined(PLATFORM_T30)
                            if (stream1.pack[i].dataType.h264Type == 7 ||
                                stream1.pack[i].dataType.h264Type == 8 ||
                                stream1.pack[i].dataType.h264Type == 5)
                            {
                                sink.IDR = true;
                            }
                            else if (stream1.pack[i].dataType.h265Type == 32)
                            {
                                sink.IDR = true;
                            }
#endif
                        }
                        if (sink.IDR)
                        {                                                               
                            if(sink.data_available_callback(nalu1)) {

                                LOG_ERROR("stream 0, eC:" << sink.encChn << ", id:" << sinkId << ", size:" << nalu1.data.size() <<
                                        ", pC:" << stream1.packCount << ", pS:" << nalu1.data.size() << ", pN:" << i << 
                                        ", sC:" << Encoder::sinks.size() << 
                                        " clogged!");                            
                            }
                        }
                    }
                }
            }       

            IMP_Encoder_ReleaseStream(1, &stream1);

            if (cfg->osd.enabled)
            {
                stream0_osd->update();               
                stream1_osd->update();
            }

            last_high_nal_ts = high_nal_ts;        
            last_low_nal_ts = low_nal_ts;
        }



        // 4 = Stop thread
        if (cfg->encoder_thread_signal.load() & 4)
        {
            IMP_Encoder_StopRecvPic(0);          
            IMP_Encoder_StopRecvPic(1);                    
            exit();
        }

        usleep(1000);
    }
}
