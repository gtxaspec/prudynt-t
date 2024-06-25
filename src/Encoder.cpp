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

IMPEncoderCHNAttr createEncoderProfile(_stream &stream)
{

    IMPEncoderRcAttr *rcAttr;
    IMPEncoderCHNAttr channel_attr;
    memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
    rcAttr = &channel_attr.rcAttr;

#if defined(PLATFORM_T31)

    IMPEncoderRcMode rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    IMPEncoderProfile encoderProfile;
    encoderProfile = (strcmp(stream.format, "H265")==0) ? IMP_ENC_PROFILE_HEVC_MAIN : IMP_ENC_PROFILE_AVC_HIGH;

    if(strcmp(stream.mode, "FIXQB")==0) {
        rcMode = IMP_ENC_RC_MODE_FIXQP;
    } else if(strcmp(stream.mode, "VBR")==0) {
        rcMode = IMP_ENC_RC_MODE_VBR;
    } else if(strcmp(stream.mode, "CBR")==0) {
        rcMode = IMP_ENC_RC_MODE_CBR;
    } else if(strcmp(stream.mode, "CAPPED_VBR")==0) {
        rcMode = IMP_ENC_RC_MODE_CAPPED_VBR;
    } else if(strcmp(stream.mode, "CAPPED_QUALITY")==0) {
        rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    } else {
        LOG_ERROR("unsupported stream.mode ("<< stream.mode <<"). we only support FIXQB, CBR, VBR, CAPPED_VBR and CAPPED_QUALITY on T31");
    }

    IMP_Encoder_SetDefaultParam(
        &channel_attr, encoderProfile, rcMode, stream.width, stream.height,
        stream.fps, 1, stream.gop, 2, -1, stream.bitrate);

    switch (rcMode) {
        case IMP_ENC_RC_MODE_FIXQP:
            rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
            break;
        case IMP_ENC_RC_MODE_CBR:
            rcAttr->attrRcMode.attrCbr.uTargetBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
            rcAttr->attrRcMode.attrCbr.iMinQP = 34;
            rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
            rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
            rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
            //rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
            rcAttr->attrRcMode.attrCbr.uMaxPictureSize = stream.bitrate;
            break;
        case IMP_ENC_RC_MODE_VBR:
            rcAttr->attrRcMode.attrVbr.uTargetBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrVbr.uMaxBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
            rcAttr->attrRcMode.attrVbr.iMinQP = 20;
            rcAttr->attrRcMode.attrVbr.iMaxQP = 45;
            rcAttr->attrRcMode.attrVbr.iIPDelta = 3;
            rcAttr->attrRcMode.attrVbr.iPBDelta = 3;
            //rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
            rcAttr->attrRcMode.attrVbr.uMaxPictureSize = stream.bitrate;
            break;
        case IMP_ENC_RC_MODE_CAPPED_VBR:
            rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
            rcAttr->attrRcMode.attrCappedVbr.iMinQP = 20;
            rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 45;
            rcAttr->attrRcMode.attrCappedVbr.iIPDelta = 3;
            rcAttr->attrRcMode.attrCappedVbr.iPBDelta = 3;
            //rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
            rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = stream.bitrate;
            rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
            break;
        case IMP_ENC_RC_MODE_CAPPED_QUALITY:
            rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = stream.bitrate;
            rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
            rcAttr->attrRcMode.attrCappedQuality.iMinQP = 20;
            rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 45;
            rcAttr->attrRcMode.attrCappedQuality.iIPDelta = 3;
            rcAttr->attrRcMode.attrCappedQuality.iPBDelta = 4;
            //rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
            rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = stream.bitrate;
            rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
            break;
    }
    
    /*
    switch (rc_attr->attrRcMode.rcMode)
    {
    case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rc_attr->attrRcMode.attrVbr.uTargetBitRate = stream.bitrate;
        rc_attr->attrRcMode.attrVbr.uMaxBitRate = stream.bitrate;
        rc_attr->attrRcMode.attrVbr.iInitialQP = -1;
        rc_attr->attrRcMode.attrVbr.iMinQP = 20;
        rc_attr->attrRcMode.attrVbr.iMaxQP = 45;
        rc_attr->attrRcMode.attrVbr.iIPDelta = 3;
        rc_attr->attrRcMode.attrVbr.iPBDelta = 3;
        // rc_high_attr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rc_attr->attrRcMode.attrVbr.uMaxPictureSize = stream.width;
        rc_attr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    }
    */
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)

#if defined(PLATFORM_T30)
    channel_attr.encAttr.enType = (strcmp(stream.format, "H264")==0) ? PT_H264 : PT_H265;
#else
    channel_attr.encAttr.enType = PT_H264;
#endif

    IMPEncoderRcMode rcMode = ENC_RC_MODE_SMART;

    if(strcmp(stream.mode, "FIXQB")==0) {
        rcMode = ENC_RC_MODE_FIXQP;
    } else if(strcmp(stream.mode, "VBR")==0) {
        rcMode = ENC_RC_MODE_CBR;
    } else if(strcmp(stream.mode, "CBR")==0) {
        rcMode = ENC_RC_MODE_VBR;
    } else if(strcmp(stream.mode, "SMART")==0) {
        rcMode = ENC_RC_MODE_SMART;
    } else {
        LOG_ERROR("unsupported stream.mode ("<< stream.mode <<"). we only support FIXQB, CBR, VBR and SMART");
    }

    // 0 = Baseline
    // 1 = Main
    // 2 = High
    // Note: The encoder seems to emit frames at half the
    // requested framerate when the profile is set to Baseline.
    // For this reason, Main or High are recommended.
    channel_attr.encAttr.profile = stream.profile;
    channel_attr.encAttr.bufSize = 0;
    channel_attr.encAttr.picWidth = stream.width;
    channel_attr.encAttr.picHeight = stream.height;
    channel_attr.rcAttr.outFrmRate.frmRateNum = stream.fps;
    channel_attr.rcAttr.outFrmRate.frmRateDen = 1;
    rcAttr->maxGop = stream.max_gop;
    
    if(channel_attr.encAttr.enType = PT_H264) {
        switch (rcMode) {
            case ENC_RC_MODE_FIXQP:
                rcAttr->attrRcMode.rcMode = ENC_RC_MODE_FIXQP;
                rcAttr->attrRcMode.attrH264FixQp.qp = 42;
                rcAttr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
                rcAttr->attrHSkip.hSkipAttr.m = 0;
                rcAttr->attrHSkip.hSkipAttr.n = 0;
                rcAttr->attrHSkip.hSkipAttr.maxSameSceneCnt = 0;
                rcAttr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
                rcAttr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
                rcAttr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
                break;
            case ENC_RC_MODE_CBR:
                rcAttr->attrRcMode.rcMode = ENC_RC_MODE_CBR;
                rcAttr->attrRcMode.attrH264Cbr.outBitRate = stream.bitrate;
                rcAttr->attrRcMode.attrH264Cbr.maxQp = 45;
                rcAttr->attrRcMode.attrH264Cbr.minQp = 15;
                rcAttr->attrRcMode.attrH264Cbr.iBiasLvl = 0;
                rcAttr->attrRcMode.attrH264Cbr.frmQPStep = 3;
                rcAttr->attrRcMode.attrH264Cbr.gopQPStep = 15;
                rcAttr->attrRcMode.attrH264Cbr.adaptiveMode = false;
                rcAttr->attrRcMode.attrH264Cbr.gopRelation = false;
                break;
            case ENC_RC_MODE_VBR:
                rcAttr->attrRcMode.rcMode = ENC_RC_MODE_VBR;
                rcAttr->attrRcMode.attrH264Vbr.maxQp = 45;
                rcAttr->attrRcMode.attrH264Vbr.minQp = 15;
                rcAttr->attrRcMode.attrH264Vbr.staticTime = 2;
                rcAttr->attrRcMode.attrH264Vbr.maxBitRate = stream.bitrate;
                rcAttr->attrRcMode.attrH264Vbr.iBiasLvl = 0;
                rcAttr->attrRcMode.attrH264Vbr.changePos = 80;
                rcAttr->attrRcMode.attrH264Vbr.qualityLvl = 2;
                rcAttr->attrRcMode.attrH264Vbr.frmQPStep = 3;
                rcAttr->attrRcMode.attrH264Vbr.gopQPStep = 15;
                rcAttr->attrRcMode.attrH264Vbr.gopRelation = false;
                break;
            case ENC_RC_MODE_SMART:
                rcAttr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
                rcAttr->attrRcMode.attrH264Smart.maxQp = 45;
                rcAttr->attrRcMode.attrH264Smart.minQp = 15;
                rcAttr->attrRcMode.attrH264Smart.staticTime = 2;
                rcAttr->attrRcMode.attrH264Smart.maxBitRate = stream.bitrate;
                rcAttr->attrRcMode.attrH264Smart.iBiasLvl = 0;
                rcAttr->attrRcMode.attrH264Smart.changePos = 80;
                rcAttr->attrRcMode.attrH264Smart.qualityLvl = 2;
                rcAttr->attrRcMode.attrH264Smart.frmQPStep = 3;
                rcAttr->attrRcMode.attrH264Smart.gopQPStep = 15;
                rcAttr->attrRcMode.attrH264Smart.gopRelation = false;
                break;
        }     
#if defined(PLATFORM_T30)
    }
    else if(channel_attr.encAttr.enType = PT_H265) {
    {
        rcAttr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rcAttr->attrRcMode.attrH265Smart.maxQp = 45;
        rcAttr->attrRcMode.attrH265Smart.minQp = 15;
        rcAttr->attrRcMode.attrH265Smart.staticTime = 2;
        rcAttr->attrRcMode.attrH265Smart.maxBitRate = stream.bitrate;
        rcAttr->attrRcMode.attrH265Smart.iBiasLvl = 0;
        rcAttr->attrRcMode.attrH265Smart.changePos = 80;
        rcAttr->attrRcMode.attrH265Smart.qualityLvl = 2;
        rcAttr->attrRcMode.attrH265Smart.frmQPStep = 3;
        rcAttr->attrRcMode.attrH265Smart.gopQPStep = 15;
        rcAttr->attrRcMode.attrH265Smart.flucLvl = 2;     
#endif
    }

    rcAttr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X;
    rcAttr->attrHSkip.hSkipAttr.m = rcAttr->maxGop - 1;
    rcAttr->attrHSkip.hSkipAttr.n = 1;
    rcAttr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
    rcAttr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
    rcAttr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
    rcAttr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X;
#endif
    
    return channel_attr;
}

IMPSensorInfo Encoder::create_sensor_info(std::string sensor)
{
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));
    LOG_INFO("Sensor: " << cfg->sensor.model);
    std::strcpy(out.name, cfg->sensor.model);
    out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
    std::strcpy(out.i2c.type, cfg->sensor.model);
    out.i2c.addr = cfg->sensor.i2c_address;
    return out;
}

int Encoder::channel_init(int chn_nr, int grp_nr, IMPEncoderCHNAttr *chn_attr)
{
    int ret;

    ret = IMP_Encoder_CreateChn(chn_nr, chn_attr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateChn("<<chn_nr<<", chn_attr)");

    ret = IMP_Encoder_RegisterChn(grp_nr, chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_RegisterChn("<<chn_nr<<", chn_attr)");

    return ret;
}

int Encoder::channel_deinit(int chn_nr)
{
    int ret;

    ret = IMP_Encoder_UnRegisterChn(chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_UnRegisterChn("<<chn_nr<<")");

    ret = IMP_Encoder_DestroyChn(chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_DestroyChn("<<chn_nr<<")");

    return ret;
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
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_OSD_SetPoolSize("<<OSDPoolSize<<")");

    ret = IMP_ISP_Open();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Open()");

    sinfo = create_sensor_info(cfg->sensor.model);
    ret = IMP_ISP_AddSensor(&sinfo);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_AddSensor(&sinfo)");

    ret = IMP_ISP_EnableSensor();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_EnableSensor()");

    ret = IMP_System_Init();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Init()");

    // Enable tuning.
    // This is necessary to customize the sensor's image output.
    // Denoising, WDR, Night Mode, and FPS customization require this.
    ret = IMP_ISP_EnableTuning();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_EnableTuning()");

    /* Image tuning */
    ret = IMP_ISP_Tuning_SetContrast(cfg->image.contrast);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetSharpness(cfg->image.sharpness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSharpness(" << cfg->image.sharpness << ")");

    ret = IMP_ISP_Tuning_SetSaturation(cfg->image.saturation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

    ret = IMP_ISP_Tuning_SetBrightness(cfg->image.brightness);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBrightness(" << cfg->image.brightness << ")");

    ret = IMP_ISP_Tuning_SetSinterStrength(cfg->image.sinter_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetSaturation(" << cfg->image.saturation << ")");

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

    ret = IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)cfg->image.anti_flicker);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAntiFlickerAttr(" << cfg->image.anti_flicker << ")");

#if !defined(PLATFORM_T21)
    ret = IMP_ISP_Tuning_SetAeComp(cfg->image.ae_compensation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetAeComp(" << cfg->image.ae_compensation << ")");
#endif

    ret = IMP_ISP_Tuning_SetHiLightDepress(cfg->image.highlight_depress);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetHiLightDepress(" << cfg->image.highlight_depress << ")");

    ret = IMP_ISP_Tuning_SetMaxAgain(cfg->image.max_again);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxAgain(" << cfg->image.max_again << ")");

    ret = IMP_ISP_Tuning_SetMaxDgain(cfg->image.max_dgain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetMaxDgain(" << cfg->image.max_dgain << ")");

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
    ret = IMP_ISP_Tuning_SetDRC_Strength(cfg->image.dpc_strength);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetDRC_Strength(" << cfg->image.drc_strength << ")");
#endif
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_ISP_Tuning_SetBacklightComp(cfg->image.backlight_compensation);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ISP_Tuning_SetBacklightComp(" << cfg->image.backlight_compensation << ")");
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

    ret = IMP_ISP_Tuning_SetSensorFPS(cfg->sensor.fps, 1);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Tuning_SetSensorFPS("<<cfg->sensor.fps<<", 1);");

    // Set the ISP to DAY on launch
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_ISP_Tuning_SetISPRunningMode("<<IMPISP_RUNNING_MODE_DAY<<")");

    return ret;
}

int Encoder::framesource_init()
{   
    LOG_DEBUG("Encoder::framesource_init()");
    int ret = 0;

    /* FrameSource highres channel */
    IMPFSChnAttr fs_high_chn_attr;
    memset(&fs_high_chn_attr, 0, sizeof(IMPFSChnAttr));

    ret = IMP_FrameSource_GetChnAttr(0, &fs_high_chn_attr);

    fs_high_chn_attr.pixFmt = PIX_FMT_NV12;
    fs_high_chn_attr.outFrmRateNum = cfg->stream0.fps;
    fs_high_chn_attr.outFrmRateDen = 1;
    fs_high_chn_attr.nrVBs = cfg->stream0.buffers;
    fs_high_chn_attr.type = FS_PHY_CHANNEL;
    fs_high_chn_attr.crop.enable = 0;
    fs_high_chn_attr.crop.top = 0;
    fs_high_chn_attr.crop.left = 0;
    fs_high_chn_attr.crop.width = cfg->sensor.width;
    fs_high_chn_attr.crop.height = cfg->sensor.height;
    fs_high_chn_attr.scaler.enable = 0; //cfg->stream0.scale_enabled;
    fs_high_chn_attr.scaler.outwidth = cfg->stream0.width;
    fs_high_chn_attr.scaler.outheight = cfg->stream0.height;
    fs_high_chn_attr.picWidth = cfg->stream0.width; 
    fs_high_chn_attr.picHeight = cfg->stream0.height;

    /* FrameSource lowres channel */
    IMPFSChnAttr fs_low_chn_attr;
    memset(&fs_low_chn_attr, 0, sizeof(IMPFSChnAttr));

    ret = IMP_FrameSource_GetChnAttr(1, &fs_low_chn_attr);

    fs_low_chn_attr.pixFmt = PIX_FMT_NV12;
    fs_low_chn_attr.outFrmRateNum = cfg->stream1.fps;
    fs_low_chn_attr.outFrmRateDen = 1;
    fs_low_chn_attr.nrVBs = 3;
    fs_low_chn_attr.type = FS_PHY_CHANNEL;
    fs_low_chn_attr.crop.enable = 0;
    fs_low_chn_attr.crop.top = 0;
    fs_low_chn_attr.crop.left = 0;
    fs_low_chn_attr.crop.width = cfg->stream0.width;
    fs_low_chn_attr.crop.height = cfg->stream0.height;
    fs_low_chn_attr.scaler.enable = 1; //cfg->stream1.scale_enabled;
    fs_low_chn_attr.scaler.outwidth = cfg->stream1.width;
    fs_low_chn_attr.scaler.outheight = cfg->stream1.height;
    fs_low_chn_attr.picWidth = cfg->stream1.width; 
    fs_low_chn_attr.picHeight = cfg->stream1.height;
#if !defined(KERNEL_VERSION_4)
#if defined(PLATFORM_T31)

    int rotation = cfg->stream0.rotation;
    int rot_height = cfg->stream0.height;
    int rot_width = cfg->stream0.width;

    // Set rotate before FS creation
    //IMP_Encoder_SetFisheyeEnableStatus(0, 1);
    //IMP_Encoder_SetFisheyeEnableStatus(1, 1);


    //ret = IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width);
    // LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width)");
#endif
#endif

    if(cfg->stream0.enabled) {
        /* FrameSource highres channel */
        ret = IMP_FrameSource_CreateChn(0, &fs_high_chn_attr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_CreateChn(0, &fs_high_chn_attr)");

        ret = IMP_FrameSource_SetChnAttr(0, &fs_high_chn_attr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnAttr(0, &fs_high_chn_attr)");

        IMPFSChnFifoAttr fifo;
        ret = IMP_FrameSource_GetChnFifoAttr(0, &fifo);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_GetChnFifoAttr(0, &fifo)");

        fifo.maxdepth = 0;
        ret = IMP_FrameSource_SetChnFifoAttr(0, &fifo);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnFifoAttr(0, &fifo)");

        ret = IMP_FrameSource_SetFrameDepth(0, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetFrameDepth(0, 0)");
    }

    if(cfg->stream1.enabled) {
        /* FrameSource lowres channel */
        ret = IMP_FrameSource_CreateChn(1, &fs_low_chn_attr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_CreateChn(1, &fs_low_chn_attr)");

        ret = IMP_FrameSource_SetChnAttr(1, &fs_low_chn_attr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnAttr(1, &fs_low_chn_attr)");

        IMPFSChnFifoAttr low_fifo;
        ret = IMP_FrameSource_GetChnFifoAttr(1, &low_fifo);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_GetChnFifoAttr(1, &low_fifo)");

        low_fifo.maxdepth = 0;
        ret = IMP_FrameSource_SetChnFifoAttr(1, &low_fifo);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnFifoAttr(1, &low_fifo)");

        ret = IMP_FrameSource_SetFrameDepth(1, 0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetFrameDepth(1, 0)");        
    }
    int x;

    return ret;
}

int Encoder::encoder_init()
{
    int ret = 0;

#if defined(PLATFORM_T31)
    /* Encoder preview channel */
    if (cfg->stream2.enabled)
    {
        ret = IMP_Encoder_SetbufshareChn(2, 0);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_SetbufshareChn(2, 0)")
    }
#endif

    /* Encoder highres channel */
    if (cfg->stream0.enabled)
    {
        IMPEncoderCHNAttr chn_attr_high = createEncoderProfile(cfg->stream0);
        ret = channel_init(0, 0, &chn_attr_high);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "channel_init(0, 0, &chn_attr_high)")
    }

    /* Encoder lowres channel */
    if (cfg->stream1.enabled)
    {
        IMPEncoderCHNAttr chn_attr_low = createEncoderProfile(cfg->stream1);
        ret = channel_init(1, 1, &chn_attr_low);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "channel_init(1, 1, &chn_attr_high)")
    }

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
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "system_init()");

    ret = framesource_init();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "framesource_init()");

    ret = encoder_init();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "encoder_init()");

    /* Encoder highres channel */
    if (cfg->stream0.enabled)
    {

        ret = IMP_Encoder_CreateGroup(0);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateGroup(0)");

        if (!cfg->stream0.osd.enabled)
        {

            ret = IMP_System_Bind(&high_fs, &high_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&high_fs, &high_enc)");
        }
        else
        {
            osdStream0 = true;

            stream0_osd = OSD::createNew(&(cfg->stream0.osd), 0, 0);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "osd.init(cfg, 0)");

            // high framesource -> high OSD
            ret = IMP_System_Bind(&high_fs, &high_osd_cell);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&high_fs, &high_osd_cell)");
                        
            // high OSD -> high Encoder
            ret = IMP_System_Bind(&high_osd_cell, &high_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&high_osd_cell, &high_enc)");
        }     

        ret = IMP_FrameSource_EnableChn(0);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_EnableChn(0)");           
    }

    /* Encoder lowres channel */
    if (cfg->stream1.enabled)
    {
        ret = IMP_Encoder_CreateGroup(1);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateGroup(1)");

        if (!cfg->stream1.osd.enabled)
        {
            ret = IMP_System_Bind(&low_fs, &low_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&low_fs, &low_enc)");
        }
        else
        {
            osdStream1 = true;

            stream1_osd = OSD::createNew(&(cfg->stream1.osd), 1, 1);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "osd.init(cfg, 1)");

            // low framesource -> low OSD
            ret = IMP_System_Bind(&low_fs, &low_osd_cell);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&low_fs, &low_osd_cell)");

            // low OSD -> low Encoder
            ret = IMP_System_Bind(&low_osd_cell, &low_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&low_osd_cell, &low_enc)");            
        }    

        ret = IMP_FrameSource_EnableChn(1);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_EnableChn(1)");            
    }

    if (cfg->motion.enabled)
    {

        LOG_DEBUG("Motion enabled");
        motionInitialized = true;

        ret = motion.init(cfg);
        LOG_DEBUG_OR_ERROR(ret, "motion.init(cfg)");
    }

    return ret;
}

void Encoder::exit()
{

    int ret = 0, i = 0;

    if (cfg->stream2.enabled)
    {
        cfg->jpg_thread_signal.fetch_or(4);
    }

    for (int i = 1; i >= 0; i--)
    {
        ret = ret = IMP_FrameSource_DisableChn(i);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_DisableChn(" << i << ")");
    }


    if(osdStream0) {

        stream0_osd->exit();
        
        ret = IMP_System_UnBind(&high_fs, &high_osd_cell);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_fs, &high_osd_cell)");

        ret = IMP_System_UnBind(&high_osd_cell, &high_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_osd_cell, &high_enc)");
    } else {

        ret = IMP_System_UnBind(&high_fs, &high_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_fs, &high_enc)");
    }

    if(osdStream1) {

        stream1_osd->exit();

        ret = IMP_System_UnBind(&low_fs, &low_osd_cell);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_fs, &low_osd_cell)");

        ret = IMP_System_UnBind(&low_osd_cell, &low_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_osd_cell, &low_enc)");
    } else {

        ret = IMP_System_UnBind(&low_fs, &low_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_fs, &low_enc)");
    }

    for (int i = 2; i >= 0; i--)
    {
        channel_deinit(i);

        ret = IMP_FrameSource_DestroyChn(i);
        LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_DestroyChn(" << i << ")");
    }

    for (int i = 2; i >= 0; i--)
    {
        ret = IMP_Encoder_DestroyGroup(i);
        LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_DestroyGroup(" << i << ")");
    }

    if (motionInitialized)
    {
        if (!motion.exit())
        {
            LOG_ERROR("Exit motion detection failed.");
        }
    }

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
                                        cfg->stream0.width, cfg->stream0.height, 24, 1, 0, 0, cfg->stream2.jpeg_quality, 0);

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
            MakeTables(cfg->stream2.jpeg_quality, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
            pstJpegeQl.user_ql_en = 1;
            IMP_Encoder_SetJpegeQl(2, &pstJpegeQl);
#endif

            IMP_Encoder_PollingStream(2, 1000); // Wait for frame

            IMPEncoderStream stream_jpeg;
            if (IMP_Encoder_GetStream(2, &stream_jpeg, 1) == 0)
            {                                                   // Check for success
                const char *tempPath = "/tmp/snapshot.tmp";     // Temporary path
                const char *finalPath = cfg->stream2.jpeg_path; // Final path for the JPEG snapshot

                // Open and create temporary file with read and write permissions
                int snap_fd = open(tempPath, O_RDWR | O_CREAT | O_TRUNC, 0777);
                if (snap_fd >= 0)
                {
                    // Attempt to lock the temporary file for exclusive access
                    if (flock(snap_fd, LOCK_EX) == -1)
                    {
                        LOG_ERROR("Failed to lock JPEG snapshot for writing: " << tempPath);
                        close(snap_fd);
                        return; // Exit the function if unable to lock the file
                    }

                    // Save the JPEG stream to the file
                    save_jpeg_stream(snap_fd, &stream_jpeg);

                    // Unlock and close the temporary file after writing is done
                    flock(snap_fd, LOCK_UN);
                    close(snap_fd);

                    // Atomically move the temporary file to the final destination
                    if (rename(tempPath, finalPath) != 0)
                    {
                        LOG_ERROR("Failed to move JPEG snapshot from " << tempPath << " to " << finalPath);
                        std::remove(tempPath); // Attempt to remove the temporary file if rename fails
                    }
                    else
                    {
                        // LOG_DEBUG("JPEG snapshot successfully updated");
                    }
                }
                else
                {
                    LOG_ERROR("Failed to open JPEG snapshot for writing: " << tempPath);
                }

                // Delay before we release, otherwise an overflow may occur
                std::this_thread::sleep_for(std::chrono::milliseconds(cfg->stream2.jpeg_refresh)); // Control the rate
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
    nice(-19);

    int ret, xx;
    int64_t last_high_nal_ts;
    int64_t last_low_nal_ts;

    // 256 = exit thread
    while ((cfg->encoder_thread_signal.load() & 256) != 256)
    {

        // 1 = init and start
        if (cfg->encoder_thread_signal.load() & 1)
        {

            ret = init();
            LOG_DEBUG_OR_ERROR(ret, "init()");
            if(ret != 0) return;

            IMP_System_RebaseTimeStamp(0);

            if(cfg->stream0.enabled) {

                channels[0].enabled = true;
                gettimeofday(&channels[0].impTimeBase, NULL);
                if(cfg->stream0.osd.enabled) {
                    channels[0].updateOsd = std::bind(&OSD::update, stream0_osd);
                }

                IMP_Encoder_StartRecvPic(0);
                LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(0)");
                if(ret != 0) return;
            }

            if(cfg->stream1.enabled) {

               channels[1].enabled = true;
                gettimeofday(&channels[1].impTimeBase, NULL);
                if(cfg->stream0.osd.enabled) {
                    channels[1].updateOsd = std::bind(&OSD::update, stream1_osd);
                }
                
                IMP_Encoder_StartRecvPic(1);
                LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(1)");
                if(ret != 0) return;
            }

            if (cfg->stream2.enabled)
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

            for( CHN &channel : channels ) {

                if(channel.enabled) {

                    IMPEncoderStream stream;
                    if (IMP_Encoder_GetStream(channel.encChn, &stream, true) != 0)
                    {
                        LOG_ERROR("IMP_Encoder_GetStream(" << channel.encChn << ") failed");
                        errorCount++;
                        continue;
                    }

                    int64_t nalTs = stream.pack[stream.packCount - 1].timestamp;
                    if (nalTs - channel.lastNalTs > 1.5 * (1000000 / cfg->stream0.fps))
                    {
                        // Silence for now until further tests / THINGINO
                        //LOG_WARN("The encoder 0 dropped a frame. " << (nalTs - channel.lastNalTs) << ", " << (1.5 * (1000000 / cfg->stream0.fps)));
                    }      

                    struct timeval encodeTime;
                    encodeTime.tv_sec = nalTs / 1000000;
                    encodeTime.tv_usec = nalTs % 1000000;

                    for (unsigned int i = 0; i < stream.packCount; ++i)
                    {      
#if defined(PLATFORM_T31)
                        uint8_t *start = (uint8_t *)stream.virAddr + stream.pack[i].offset;
                        uint8_t *end = start + stream.pack[i].length;
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                        uint8_t *start = (uint8_t *)stream.pack[i].virAddr;
                        uint8_t *end = (uint8_t *)stream.pack[i].virAddr + stream.pack[i].length;
#endif
                        H264NALUnit nalu;
                        nalu.imp_ts = stream.pack[i].timestamp;
                        timeradd(&channel.impTimeBase, &encodeTime, &nalu.time);
                        nalu.duration = 0;
#if defined(PLATFORM_T31)
                        if (stream.pack[i].nalType.h264NalType == 5 || stream.pack[i].nalType.h264NalType == 1)
                        {
                            nalu.duration = nalTs - channel.lastNalTs;
                        }
                        else if (stream.pack[i].nalType.h265NalType == 19 ||
                                stream.pack[i].nalType.h265NalType == 20 ||
                                stream.pack[i].nalType.h265NalType == 1)
                        {
                            nalu.duration = nalTs - channel.lastNalTs;
                        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                        if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                        {
                            nalu.duration = nalTs - channel.lastNalTs;
                        }
#elif defined(PLATFORM_T30)
                        if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                        {
                            nalu.duration = nalTs - channel.lastNalTs;
                        }
                        else if (stream.pack[i].dataType.h265Type == 19 ||
                                stream.pack[i].dataType.h265Type == 20 ||
                                stream.pack[i].dataType.h265Type == 1)
                        {
                            nalu.duration = nalTs - channel.lastNalTs;
                        }
#endif
                        // We use start+4 because the encoder inserts 4-byte MPEG
                        //'startcodes' at the beginning of each NAL. Live555 complains
                        nalu.data.insert(nalu.data.end(), start + 4, end);

                        std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
                        for (auto &[sinkId, sink] : Encoder::sinks)
                        {
                            if (sink.encChn == channel.encChn)
                            {
                                if (!sink.IDR)
                                {
#if defined(PLATFORM_T31)
                                    if (stream.pack[i].nalType.h264NalType == 7 ||
                                        stream.pack[i].nalType.h264NalType == 8 ||
                                        stream.pack[i].nalType.h264NalType == 5)
                                    {
                                        sink.IDR = true;
                                    }
                                    else if (stream.pack[i].nalType.h265NalType == 32)
                                    {
                                        sink.IDR = true;
                                    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                                    if (stream.pack[i].dataType.h264Type == 7 ||
                                        stream.pack[i].dataType.h264Type == 8 ||
                                        stream.pack[i].dataType.h264Type == 5)
                                    {
                                        sink.IDR = true;
                                    }
#elif defined(PLATFORM_T30)
                                    if (stream.pack[i].dataType.h264Type == 7 ||
                                        stream.pack[i].dataType.h264Type == 8 ||
                                        stream.pack[i].dataType.h264Type == 5)
                                    {
                                        sink.IDR = true;
                                    }
                                    else if (stream.pack[i].dataType.h265Type == 32)
                                    {
                                        sink.IDR = true;
                                    }
#endif
                                }
                                if (sink.IDR)
                                {
                                    if (sink.data_available_callback(nalu))
                                    {

                                        LOG_ERROR("stream 0, eC:" << sink.encChn << ", id:" << sinkId << ", size:" << nalu.data.size() 
                                            << ", pC:" << stream.packCount << ", pS:" << nalu.data.size() << ", pN:" << i << ", sC:" << Encoder::sinks.size() << " clogged!");
                                    }
                                }
                            }
                        } //for (auto &[sinkId, sink] : Encoder::sinks)
                    }                                         
                
                    IMP_Encoder_ReleaseStream(channel.encChn, &stream);
                    channel.lastNalTs = nalTs;

                    if (channel.updateOsd)
                    {
                        channel.updateOsd();
                    }                       
                }
            }
        }

        // 4 = Stop thread
        if (cfg->encoder_thread_signal.load() & 4)
        {
            if(cfg->stream0.enabled) {
                IMP_Encoder_StopRecvPic(0);
            }

            if(cfg->stream1.enabled) {    
                IMP_Encoder_StopRecvPic(1);
            }

            exit();
        }
    }
}
