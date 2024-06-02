#include <iostream>
#include <cstring>
#include <ctime>
#include <fstream>
#include "Encoder.hpp"
#include "Config.hpp"

#define MODULE "ENCODER"

#define OSDPoolSize 131072*4

std::mutex Encoder::sinks_lock;
std::map<uint32_t, EncoderSink> Encoder::sinks;
uint32_t Encoder::sink_id = 0;

//Encoder::Encoder() {}

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

IMPFSChnAttr Encoder::create_fs_attr()
{
    IMPFSChnAttr out;
    memset(&out, 0, sizeof(IMPFSChnAttr));

    // Seems to only support the following (channel enable fails otherwise)
    // PIX_FMT_YUYV422
    // PIX_FMT_UYVY422
    // PIX_FMT_NV12
    // Of those, I have only gotten PIX_FMT_NV12 to produce frames.
    out.pixFmt = PIX_FMT_NV12;
    out.outFrmRateNum = cfg->stream0.fps;
    out.outFrmRateDen = 1;
    out.nrVBs = cfg->stream0.buffers;
    out.type = FS_PHY_CHANNEL;
    out.crop.enable = 0;
    out.crop.top = 0;
    out.crop.left = 0;
    out.crop.width = cfg->stream0.width;
    out.crop.height =cfg->stream0.height;
    out.scaler.enable = cfg->stream0.scale_enabled;
    out.scaler.outwidth = cfg->stream0.scale_width;
    out.scaler.outheight = cfg->stream0.scale_width;
    out.picWidth = cfg->stream0.width; // Testing stream size sync
    out.picHeight = cfg->stream0.height;
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
    LOG_DEBUG("Encoder JPEG Channel " << chn_nr << " created");

    ret = IMP_Encoder_RegisterChn(grp_nr, chn_nr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_RegisterChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    LOG_DEBUG("Encoder Channel " << chn_nr << " registered");

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
    LOG_DEBUG("Encoder Channel " << chn_nr << " unregistered");

    ret = IMP_Encoder_DestroyChn(chn_nr);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_DestroyChn(" << chn_nr << ") == " << ret);
        return 0;
    }
    LOG_DEBUG("Encoder Channel " << chn_nr << " destroyed");

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
    LOG_DEBUG("IMP_OSD_SetPoolSize == " + std::to_string(OSDPoolSize));

    ret = IMP_ISP_Open();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_Open() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("ISP Opened!");

    sinfo = create_sensor_info(cfg->sensor.model.c_str());
    ret = IMP_ISP_AddSensor(&sinfo);
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_AddSensor() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("Sensor Added");

    ret = IMP_ISP_EnableSensor();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_ISP_EnableSensor() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("Sensor Enabled");

    ret = IMP_System_Init();
    if (ret < 0)
    {
        LOG_DEBUG("Error: IMP_System_Init() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP System Initialized");

    // Enable tuning.
    // This is necessary to customize the sensor's image output.
    // Denoising, WDR, Night Mode, and FPS customization require this.
    ret = IMP_ISP_EnableTuning();
    if (ret < 0)
    {
        LOG_DEBUG("ERROR: IMP_ISP_EnableTuning() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_EnableTuning enabled");

    /* Set tuning defaults; on some SoC platforms, if we don't do this, the stream will be dark until manually set */
    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(128);
    IMP_ISP_Tuning_SetISPBypass(IMPISP_TUNING_OPS_MODE_ENABLE);
    IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_60HZ);
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
    int rotation = cfg->stream0.rotation;
    int rot_height = cfg->stream0.height;
    int rot_width = cfg->stream0.width;

    IMPFSChnAttr fs_chn_attr = create_fs_attr();

#if !defined(KERNEL_VERSION_4)
#if defined(PLATFORM_T31)
    // Set rotate before FS creation
    // IMP_Encoder_SetFisheyeEnableStatus(0, 1);
    ret = IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_SetChnRotate() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnRotate == " + std::to_string(rotation));
#endif
#endif

    ret = IMP_FrameSource_CreateChn(0, &fs_chn_attr);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_CreateChn() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_CreateChn created");

    ret = IMP_FrameSource_SetChnAttr(0, &fs_chn_attr);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_SetChnAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnAttr set");

    IMPFSChnFifoAttr fifo;

    ret = IMP_FrameSource_GetChnFifoAttr(0, &fifo);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_GetChnFifoAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_GetChnFifoAttr set");

    fifo.maxdepth = 0;
    ret = IMP_FrameSource_SetChnFifoAttr(0, &fifo);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_SetChnFifoAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnFifoAttr set");

    ret = IMP_FrameSource_SetFrameDepth(0, 0);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_FrameSource_SetFrameDepth() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetFrameDepth set");

    return ret;
}

int Encoder::encoder_init()
{
    int ret = 0;

    IMPEncoderRcAttr *rc_attr;
    IMPEncoderCHNAttr channel_attr;

    memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
    rc_attr = &channel_attr.rcAttr;

#if defined(PLATFORM_T31)
    IMPEncoderProfile encoderProfile;

    // Allow user to specify the profile directly in the future with fallback defaults
    const std::string &format = cfg->stream0.format;
    encoderProfile = (format == "H265") ? IMP_ENC_PROFILE_HEVC_MAIN : IMP_ENC_PROFILE_AVC_HIGH;

    IMP_Encoder_SetDefaultParam(
        &channel_attr, encoderProfile, IMP_ENC_RC_MODE_CAPPED_QUALITY, cfg->stream0.width, cfg->stream0.height,
        cfg->stream0.fps, 1, cfg->stream0.gop, 2,
        -1, cfg->stream0.bitrate);

    switch (rc_attr->attrRcMode.rcMode)
    {
    case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rc_attr->attrRcMode.attrVbr.uTargetBitRate = cfg->stream0.bitrate;
        rc_attr->attrRcMode.attrVbr.uMaxBitRate = cfg->stream0.bitrate;
        rc_attr->attrRcMode.attrVbr.iInitialQP = -1;
        rc_attr->attrRcMode.attrVbr.iMinQP = 20;
        rc_attr->attrRcMode.attrVbr.iMaxQP = 45;
        rc_attr->attrRcMode.attrVbr.iIPDelta = 3;
        rc_attr->attrRcMode.attrVbr.iPBDelta = 3;
        // rc_attr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rc_attr->attrRcMode.attrVbr.uMaxPictureSize = cfg->stream0.width;
        rc_attr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    }

#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
    // channel_attr.encAttr.enType = PT_JPEG;
    const std::string &format = cfg->stream0.format;

#if defined(PLATFORM_T30)
    channel_attr.encAttr.enType = (format == "H264") ? PT_H264 : (format == "H265") ? PT_H265
                                                                                    : PT_H264;
#else
    channel_attr.encAttr.enType = (format == "H264") ? PT_H264 : PT_H264;
#endif

    channel_attr.encAttr.bufSize = 0;

    // 0 = Baseline
    // 1 = Main
    // 2 = High
    // Note: The encoder seems to emit frames at half the
    // requested framerate when the profile is set to Baseline.
    // For this reason, Main or High are recommended.
    channel_attr.encAttr.profile = 2;
    channel_attr.encAttr.picWidth = cfg->stream0.width;
    channel_attr.encAttr.picHeight = cfg->stream0.height;

    channel_attr.rcAttr.outFrmRate.frmRateNum = cfg->stream0.fps;
    channel_attr.rcAttr.outFrmRate.frmRateDen = 1;

    // Setting maxGop to a low value causes the encoder to emit frames at a much
    // slower rate. A sufficiently low value can cause the frame emission rate to
    // drop below the frame rate.
    // I find that 2x the frame rate is a good setting.
    rc_attr->maxGop = cfg->stream0.max_gop;

    if (cfg->stream0.format == "H264")
    {
        rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rc_attr->attrRcMode.attrH264Smart.maxQp = 45;
        rc_attr->attrRcMode.attrH264Smart.minQp = 24;
        rc_attr->attrRcMode.attrH264Smart.staticTime = 2;
        rc_attr->attrRcMode.attrH264Smart.maxBitRate = cfg->stream0.bitrate;
        rc_attr->attrRcMode.attrH264Smart.iBiasLvl = 0;
        rc_attr->attrRcMode.attrH264Smart.changePos = 80;
        rc_attr->attrRcMode.attrH264Smart.qualityLvl = 0;
        rc_attr->attrRcMode.attrH264Smart.frmQPStep = 3;
        rc_attr->attrRcMode.attrH264Smart.gopQPStep = 15;
        rc_attr->attrRcMode.attrH264Smart.gopRelation = false;
#if defined(PLATFORM_T30)
    }
    else if (cfg->stream0.format == "H265")
    {
        rc_attr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rc_attr->attrRcMode.attrH265Smart.maxQp = 45;
        rc_attr->attrRcMode.attrH265Smart.minQp = 15;
        rc_attr->attrRcMode.attrH265Smart.staticTime = 2;
        rc_attr->attrRcMode.attrH265Smart.maxBitRate = cfg->stream0.bitrate;
        rc_attr->attrRcMode.attrH265Smart.iBiasLvl = 0;
        rc_attr->attrRcMode.attrH265Smart.changePos = 80;
        rc_attr->attrRcMode.attrH265Smart.qualityLvl = 2;
        rc_attr->attrRcMode.attrH265Smart.frmQPStep = 3;
        rc_attr->attrRcMode.attrH265Smart.gopQPStep = 15;
        rc_attr->attrRcMode.attrH265Smart.flucLvl = 2;
#endif
    }

    rc_attr->attrHSkip.hSkipAttr.skipType = IMP_Encoder_STYPE_N1X; // IMP_Encoder_STYPE_HN1_TRUE
    rc_attr->attrHSkip.hSkipAttr.m = rc_attr->maxGop - 1;
    rc_attr->attrHSkip.hSkipAttr.n = 1;
    rc_attr->attrHSkip.hSkipAttr.maxSameSceneCnt = 6;
    rc_attr->attrHSkip.hSkipAttr.bEnableScenecut = 0;
    rc_attr->attrHSkip.hSkipAttr.bBlackEnhance = 0;
    rc_attr->attrHSkip.maxHSkipType = IMP_Encoder_STYPE_N1X; // IMP_Encoder_STYPE_N4X

#endif

#if defined(PLATFORM_T31)
    if (cfg->stream1.jpeg_enabled)
    {
        ret = IMP_Encoder_SetbufshareChn(1, 0);
        if (ret < 0)
        {
            LOG_ERROR("IMP_Encoder_SetbufshareChn() == " << ret);
        }
        LOG_DEBUG("IMP_Encoder_SetbufshareChn(1, 0) enabled");
    }
#endif

    ret = channel_init(0, 0, &channel_attr);
    if (ret < 0)
    {
        LOG_ERROR("channel_init() == " << ret);
        return ret;
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
    return ret;
}

bool Encoder::init()
{
    LOG_DEBUG("Encoder::init()");
    int ret = 0;

    ret = system_init();
    if (ret < 0)
    {
        LOG_ERROR("System_init failed ");
    }

    ret = framesource_init();
    if (ret < 0)
    {
        LOG_ERROR("framesource_init failed ");
    }

    ret = encoder_init();
    if (ret < 0)
    {
        LOG_ERROR("Encoder Init Failed");
        return true;
    }

    ret = IMP_Encoder_CreateGroup(0);
    if (ret < 0)
    {
        LOG_ERROR("IMP_Encoder_CreateGroup() == " << ret);
        return true;
    }
    LOG_DEBUG("Encoder Group created");

    if (!cfg->osd.enabled)
    {
        LOG_DEBUG("OSD disabled");

        // Framesource -> ENC
        ret = IMP_System_Bind(&fs, &enc);
        if (ret < 0)
        {
            LOG_ERROR("IMP_System_Bind(FS, ENC) == " << ret);
            return true;
        }
        LOG_DEBUG("IMP_System_Bind(FS, ENC)");
    }
    else
    {
        // If OSD is enabled, initialize OSD and bind FrameSource to OSD, then OSD to Encoder
        LOG_DEBUG("OSD enabled");
        osdInitialized = true;

        ret = osd.init(cfg);
        if (ret)
        {
            LOG_ERROR("OSD Init Failed");
            return true;
        }

        // Framesource -> OSD
        ret = IMP_System_Bind(&fs, &osd_cell);
        if (ret < 0)
        {
            LOG_ERROR("IMP_System_Bind(FS, OSD) == " << ret);
            return true;
        }
        LOG_DEBUG("IMP_System_Bind(FS, OSD)");

        // OSD -> Encoder
        ret = IMP_System_Bind(&osd_cell, &enc);
        if (ret < 0)
        {
            LOG_ERROR("IMP_System_Bind(OSD, ENC) == " << ret);
            return true;
        }
        LOG_DEBUG("IMP_System_Bind(OSD, ENC)");
    }

    ret = IMP_FrameSource_EnableChn(0);
    if (ret < 0)
    {
        LOG_ERROR("IMP_FrameSource_EnableChn() == " << ret);
        return true;
    }
    LOG_DEBUG("Frame Source Channel 0 enabled");

    if(cfg->motion.enabled) {

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
    IMPEncoderChnStat chn_stat;

    if (cfg->stream1.jpeg_enabled)
    {
        cfg->jpg_thread_signal.fetch_or(4);
    }

    IMP_Encoder_StopRecvPic(0);

    ret = ret = IMP_FrameSource_DisableChn(chnNum);
    LOG_DEBUG("IMP_FrameSource_DisableChn(" << chnNum << ") error: " << ret);

    ret = IMP_System_UnBind(&fs, &osd_cell);
    LOG_DEBUG("IMP_System_UnBind 1 error: " << ret);

    ret = IMP_System_UnBind(&osd_cell, &enc);
    LOG_DEBUG("IMP_System_UnBind 2 error: " << ret);

    memset(&chn_stat, 0, sizeof(IMPEncoderChnStat));

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
    LOG_DEBUG("IMP_Encoder_DestroyGroup(" << i << ") error: " << ret);

    if(osdInitialized)
        osd.exit();

    if(motionInitialized) {
        if(!motion.exit()) {
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
    cfg->encoder_thread_signal.fetch_or(8); // set stopped
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

void Encoder::jpeg_snap(std::shared_ptr<CFG>& cfg)
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

            ret = channel_init(1, 0, &channel_attr_jpg);
            if (ret < 0)
            {
                LOG_ERROR("channel_init() == " << ret);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait for ISP to init before saving initial image

            IMP_Encoder_StartRecvPic(1); // Start receiving pictures once

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
            IMP_Encoder_SetJpegeQl(1, &pstJpegeQl);
#endif

            IMP_Encoder_PollingStream(1, 10000); // Wait for frame

            IMPEncoderStream stream_jpeg;
            if (IMP_Encoder_GetStream(1, &stream_jpeg, 1) == 0)
            {                                                                 // Check for success
                std::string tempPath = "/tmp/snapshot.tmp";                   // Temporary path
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
                IMP_Encoder_ReleaseStream(1, &stream_jpeg);                                        // Release stream after saving
            }

            if (cfg->jpg_thread_signal.load() & 4)
            {

                LOG_DEBUG("Stop jpeg thread.");
                cfg->jpg_thread_signal.fetch_xor(7);
                cfg->jpg_thread_signal.fetch_or(8);
            }
        }

        usleep(1000);
    }
}

void Encoder::run()
{
    LOG_DEBUG("Encoder Start. ");

    // The encoder thread is very important, but we
    // want sink threads to have higher priority.
    nice(-19);

    int64_t last_nal_ts;

    // 256 = exit thread
    while ((cfg->encoder_thread_signal.load() & 256) != 256)
    {

        // LOG_DEBUG("thread_signal->load() " << thread_signal->load());
        //  The encoder tracks NAL timestamps with an int64_t.
        //  INT64_MAX = 9,223,372,036,854,775,807
        //  That means the encoder won't overflow its timestamp unless
        //  this program is left running for more than 106,751,991 days,
        //  or nearly 300,000 years. I think it's okay if we don't
        //  handle timestamp overflows. :)
        last_nal_ts = 0;
        
        // 1 = init and start
        if (cfg->encoder_thread_signal.load() & 1)
        {

            init();

            IMP_System_RebaseTimeStamp(0);
            gettimeofday(&imp_time_base, NULL);
            IMP_Encoder_StartRecvPic(0);
            LOG_DEBUG("Encoder StartRecvPic success");

            if (cfg->stream1.jpeg_enabled)
            {

                LOG_DEBUG("JPEG support enabled");
                cfg->jpg_thread_signal.fetch_xor(8); //remove stopped bit
                if (jpeg_thread.joinable())
                {
                    cfg->jpg_thread_signal.fetch_or(1);
                }
                else
                {
                    jpeg_thread = std::thread(&Encoder::jpeg_snap, this, std::ref(cfg));
                }
            }
            cfg->encoder_thread_signal.fetch_or(2);
        }

        while (cfg->encoder_thread_signal.load() & 2)
        {

            IMPEncoderStream stream;

            if (IMP_Encoder_GetStream(0, &stream, true) != 0)
            {
                LOG_ERROR("IMP_Encoder_GetStream() failed");
                break;
            }

            // The I/P NAL is always last, but it doesn't
            // really matter which NAL we select here as they
            // all have identical timestamps.
            int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;
            if (nal_ts - last_nal_ts > 1.5 * (1000000 / cfg->stream0.fps))
            {
                // Silence for now until further tests / THINGINO
                // LOG_WARN("The encoder dropped a frame.");
            }
            struct timeval encode_time;
            encode_time.tv_sec = nal_ts / 1000000;
            encode_time.tv_usec = nal_ts % 1000000;

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
                timeradd(&imp_time_base, &encode_time, &nalu.time);
                nalu.duration = 0;
#if defined(PLATFORM_T31)
                if (stream.pack[i].nalType.h264NalType == 5 || stream.pack[i].nalType.h264NalType == 1)
                {
                    nalu.duration = last_nal_ts - nal_ts;
                }
                else if (stream.pack[i].nalType.h265NalType == 19 ||
                         stream.pack[i].nalType.h265NalType == 20 ||
                         stream.pack[i].nalType.h265NalType == 1)
                {
                    nalu.duration = last_nal_ts - nal_ts;
                }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                {
                    nalu.duration = last_nal_ts - nal_ts;
                }
#elif defined(PLATFORM_T30)
                if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                {
                    nalu.duration = last_nal_ts - nal_ts;
                }
                else if (stream.pack[i].dataType.h265Type == 19 ||
                         stream.pack[i].dataType.h265Type == 20 ||
                         stream.pack[i].dataType.h265Type == 1)
                {
                    nalu.duration = last_nal_ts - nal_ts;
                }
#endif
                // We use start+4 because the encoder inserts 4-byte MPEG
                //'startcodes' at the beginning of each NAL. Live555 complains
                // if those are present.
                nalu.data.insert(nalu.data.end(), start + 4, end);

                std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
                for (std::map<uint32_t, EncoderSink>::iterator it = Encoder::sinks.begin();
                     it != Encoder::sinks.end(); ++it)
                {
#if defined(PLATFORM_T31)
                    if (stream.pack[i].nalType.h264NalType == 7 ||
                        stream.pack[i].nalType.h264NalType == 8 ||
                        stream.pack[i].nalType.h264NalType == 5)
                    {
                        it->second.IDR = true;
                    }
                    else if (stream.pack[i].nalType.h265NalType == 32)
                    {
                        it->second.IDR = true;
                    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                    if (stream.pack[i].dataType.h264Type == 7 ||
                        stream.pack[i].dataType.h264Type == 8 ||
                        stream.pack[i].dataType.h264Type == 5)
                    {
                        it->second.IDR = true;
                    }
#elif defined(PLATFORM_T30)
                    if (stream.pack[i].dataType.h264Type == 7 ||
                        stream.pack[i].dataType.h264Type == 8 ||
                        stream.pack[i].dataType.h264Type == 5)
                    {
                        it->second.IDR = true;
                    }
                    else if (stream.pack[i].dataType.h265Type == 32)
                    {
                        it->second.IDR = true;
                    }
#endif
                    if (it->second.IDR)
                    {
                        if (!it->second.chn->write(nalu))
                        {
                            // Discard old NALUs if our sinks aren't keeping up.
                            // This prevents the MsgChannels from clogging up with
                            // old data.
                            LOG_ERROR("Sink " << it->second.name << " clogged! Discarding NAL.");
                            // H264NALUnit old_nal;
                            // it->second.chn->read(&old_nal);
                        }
                    }
                }
            }

            if (cfg->osd.enabled)
            {
                osd.update();
            }

            IMP_Encoder_ReleaseStream(0, &stream);
            last_nal_ts = nal_ts;
            std::this_thread::yield();
        }

        // 4 = Stop thread
        if(cfg->encoder_thread_signal.load() & 4) {
            exit();
        }

        usleep(1000);
    }
}
