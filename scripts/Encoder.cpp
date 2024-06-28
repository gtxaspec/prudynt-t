#include <iostream>
#include <cstring>
#include <ctime>
#include <fstream>
#include "Encoder.hpp"
#include "Config.hpp"
#include <chrono>

#define MODULE "ENCODER"

#define OSDPoolSize 200000

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

unsigned long long timeDiffInMs(struct timeval *startTime) {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long seconds = currentTime.tv_sec - startTime->tv_sec;
    long microseconds = currentTime.tv_usec - startTime->tv_usec;

    unsigned long long milliseconds = (seconds * 1000) + (microseconds / 1000);

    return milliseconds;
}

EncoderSink *Encoder::stream_sinks[2];
pthread_mutex_t Encoder::stream_locks[2];


IMPEncoderCHNAttr createEncoderProfile(_stream &stream)
{

    IMPEncoderRcAttr *rcAttr;
    IMPEncoderCHNAttr channel_attr;
    memset(&channel_attr, 0, sizeof(IMPEncoderCHNAttr));
    rcAttr = &channel_attr.rcAttr;

#if defined(PLATFORM_T31)

    IMPEncoderRcMode rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    IMPEncoderProfile encoderProfile;
    encoderProfile = (strcmp(stream.format, "H265") == 0) ? IMP_ENC_PROFILE_HEVC_MAIN : IMP_ENC_PROFILE_AVC_HIGH;

    if (strcmp(stream.mode, "FIXQB") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_FIXQP;
    }
    else if (strcmp(stream.mode, "VBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_VBR;
    }
    else if (strcmp(stream.mode, "CBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CBR;
    }
    else if (strcmp(stream.mode, "CAPPED_VBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CAPPED_VBR;
    }
    else if (strcmp(stream.mode, "CAPPED_QUALITY") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    }
    else
    {
        LOG_ERROR("unsupported stream.mode (" << stream.mode << "). we only support FIXQB, CBR, VBR, CAPPED_VBR and CAPPED_QUALITY on T31");
    }

    IMP_Encoder_SetDefaultParam(
        &channel_attr, encoderProfile, rcMode, stream.width, stream.height,
        stream.fps, 1, stream.gop, 2, -1, stream.bitrate);

    switch (rcMode)
    {
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
        // rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
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
        // rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
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
        // rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
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
        // rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = stream.bitrate;
        rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
        break;
    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)

#if defined(PLATFORM_T30)
    channel_attr.encAttr.enType = (strcmp(stream.format, "H264") == 0) ? PT_H264 : PT_H265;
#else
    channel_attr.encAttr.enType = PT_H264;
#endif

    IMPEncoderRcMode rcMode = ENC_RC_MODE_SMART;

    if (strcmp(stream.mode, "FIXQB") == 0)
    {
        rcMode = ENC_RC_MODE_FIXQP;
    }
    else if (strcmp(stream.mode, "VBR") == 0)
    {
        rcMode = ENC_RC_MODE_CBR;
    }
    else if (strcmp(stream.mode, "CBR") == 0)
    {
        rcMode = ENC_RC_MODE_VBR;
    }
    else if (strcmp(stream.mode, "SMART") == 0)
    {
        rcMode = ENC_RC_MODE_SMART;
    }
    else
    {
        LOG_ERROR("unsupported stream.mode (" << stream.mode << "). we only support FIXQB, CBR, VBR and SMART");
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

    if (channel_attr.encAttr.enType = PT_H264)
    {
        switch (rcMode)
        {
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
    else if (channel_attr.encAttr.enType = PT_H265)
    {
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

    LOG_DEBUG("STREAM PROFILE " << stream.rtsp_endpoint << ", " << channel_attr.rcAttr.outFrmRate.frmRateNum << "fps, "
                                << stream.bitrate << "bps, " << stream.gop << "gop, profile:" << stream.profile << ", mode:" << rcMode);

    return channel_attr;
}

int Encoder::channel_init(int chn_nr, int grp_nr, IMPEncoderCHNAttr *chn_attr)
{
    int ret;

    ret = IMP_Encoder_CreateChn(chn_nr, chn_attr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateChn(" << chn_nr << ", chn_attr)");

    ret = IMP_Encoder_RegisterChn(grp_nr, chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_RegisterChn(" << chn_nr << ", chn_attr)");

    return ret;
}

int Encoder::channel_deinit(int chn_nr)
{
    int ret;

    ret = IMP_Encoder_UnRegisterChn(chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_UnRegisterChn(" << chn_nr << ")");

    ret = IMP_Encoder_DestroyChn(chn_nr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_DestroyChn(" << chn_nr << ")");

    return ret;
}

int Encoder::encoder_init()
{
    int ret = 0;

#if defined(PLATFORM_T31)
    /* Encoder preview channel */
    if (cfg->stream2.enabled && cfg->stream0.enabled)
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

        stream0Status |= 2;
    }

    /* Encoder lowres channel */
    if (cfg->stream1.enabled)
    {
        IMPEncoderCHNAttr chn_attr_low = createEncoderProfile(cfg->stream1);
        ret = channel_init(1, 1, &chn_attr_low);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "channel_init(1, 1, &chn_attr_high)")

        stream1Status |= 2;
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

    impsystem = IMPSystem::createNew(&cfg->image, &cfg->sensor);

    if(cfg->stream0.enabled) {
        framesources[0] = IMPFramesource::createNew(&cfg->stream0, &cfg->sensor, 0);
    }

    if(cfg->stream1.enabled) {
        framesources[1] = IMPFramesource::createNew(&cfg->stream1, &cfg->sensor, 1);
    }

    ret = encoder_init();
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "encoder_init()");

    /* Encoder highres channel */
    if (cfg->stream0.enabled)
    {

        ret = IMP_Encoder_CreateGroup(0);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateGroup(0)");
        stream0Status |= 1;

        if (!cfg->stream0.osd.enabled)
        {

            ret = IMP_System_Bind(&high_fs, &high_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&high_fs, &high_enc)");
            stream0Status |= 2;
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

            stream0Status |= 4;
        }

        framesources[0]->enable();
    }

    /* Encoder lowres channel */
    if (cfg->stream1.enabled)
    {
        ret = IMP_Encoder_CreateGroup(1);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateGroup(1)");
        stream1Status |= 1;

        if (!cfg->stream1.osd.enabled)
        {
            ret = IMP_System_Bind(&low_fs, &low_enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&low_fs, &low_enc)");
            stream1Status |= 2;
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

            stream1Status |= 4;
        }

        framesources[1]->enable();
    }

    ret = IMP_OSD_SetPoolSize(OSDPoolSize);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_OSD_SetPoolSize(" << OSDPoolSize << ")");
    
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

    // deinit disable framesources 
    if(framesources[1]) {
        framesources[1]->disable();
    }

    if(framesources[0]) {
        framesources[0]->disable();
    }

    // deinit bindings stream0 
    if (stream0Status & 4)
    {

        stream0_osd->exit();

        ret = IMP_System_UnBind(&high_fs, &high_osd_cell);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_fs, &high_osd_cell)");

        ret = IMP_System_UnBind(&high_osd_cell, &high_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_osd_cell, &high_enc)");
        stream0Status ^= 4;
    }
    else if (stream0Status & 2)
    {

        ret = IMP_System_UnBind(&high_fs, &high_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&high_fs, &high_enc)");
        stream0Status ^= 2;
    }

    // deinit bindings stream1 
    if (stream1Status & 4)
    {

        stream1_osd->exit();

        ret = IMP_System_UnBind(&low_fs, &low_osd_cell);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_fs, &low_osd_cell)");

        ret = IMP_System_UnBind(&low_osd_cell, &low_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_osd_cell, &low_enc)");
        stream1Status ^= 4;
    }
    else if (stream1Status & 2)
    {

        ret = IMP_System_UnBind(&low_fs, &low_enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&low_fs, &low_enc)");
        stream1Status ^= 2;
    }

    /* deinit destroy framesource & destroy encoder channel */
    if (stream1Status & 1)
    {
        channel_deinit(1);
        stream1Status ^= 1;
    }

    if(framesources[1]) {
        delete framesources[1];
        framesources[1] = nullptr;
    }

    if (stream0Status & 1)
    {

        channel_deinit(0);
        stream0Status ^= 1;
    }

    if(framesources[0]) {
        delete framesources[0];
        framesources[0] = nullptr;
    }

    /* deinit jpeg */
    channel_deinit(2);
    ret = IMP_FrameSource_DestroyChn(2);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_DestroyChn(2)");

    /* deinit destroy groups */
    if (stream0Status & 4)
    {

        ret = IMP_Encoder_DestroyGroup(0);
        LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_DestroyGroup(0)");
        stream1Status ^= 4;
    }

    if (stream1Status & 4)
    {

        ret = IMP_Encoder_DestroyGroup(1);
        LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_DestroyGroup(1)");
        stream1Status ^= 4;
    }

    if (motionInitialized)
    {
        if (!motion.exit())
        {
            LOG_ERROR("Exit motion detection failed.");
        }
    }

    delete impsystem;

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
    //nice(-18);

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
                                        cfg->stream0.width, cfg->stream0.height, 1000 / cfg->stream2.jpeg_refresh, 1, 0, 0, cfg->stream2.jpeg_quality, 0);

#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
                IMPEncoderAttr *enc_attr;
                enc_attr = &channel_attr_jpg.encAttr;
                enc_attr->enType = PT_JPEG;
                enc_attr->bufSize = 0;
                enc_attr->profile = 2;
                enc_attr->picWidth = cfg->stream0.width;
                enc_attr->picHeight = cfg->stream0.height;
#endif

            channel_attr_jpg.rcAttr.outFrmRate.frmRateNum = 1000 / cfg->stream2.jpeg_refresh;
            channel_attr_jpg.rcAttr.outFrmRate.frmRateDen = 1;

            ret = channel_init(2, 0, &channel_attr_jpg);
            if (ret < 0)
            {
                LOG_ERROR("channel_init() == " << ret);
            }

            //std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait for ISP to init before saving initial image

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

            LOG_ERROR("IMP_Encoder_PollingStream 3");
            IMP_Encoder_PollingStream(2, 1000); // Wait for frame

            LOG_ERROR("IMP_Encoder_GetStream 3");
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
                        LOG_DEBUG("JPEG snapshot successfully updated");
                    }
                }
                else
                {
                    LOG_ERROR("Failed to open JPEG snapshot for writing: " << tempPath);
                }

                // Delay before we release, otherwise an overflow may occur
                //std::this_thread::sleep_for(std::chrono::milliseconds(cfg->stream2.jpeg_refresh)); // Control the rate
                LOG_ERROR("IMP_Encoder_ReleaseStream 3");
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

void Encoder::stream_grabber(Channel *channel)
{
    LOG_DEBUG("Start stream_grabber thread for stream " << channel->encChn);

    // The encoder thread is very important, but we
    // want sink threads to have higher priority.
    //nice(-19);

    int ret, errorCount, signal;
    uint32_t bps, fps, lps, ms;
    int64_t last_nal_ts, nal_ts;
    struct timeval imp_time_base;
    EncoderSink *sink = stream_sinks[channel->encChn];

    gettimeofday(&imp_time_base, NULL);

    ret = IMP_Encoder_StartRecvPic(channel->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << channel->encChn << ")");
    if (ret != 0)
        return;

    channel->thread_signal.store(true);

    // 256 = exit thread
    while (channel->thread_signal.load())
    {
        //pthread_mutex_lock(&stream_locks[channel->encChn]);
        std::unique_lock<std::mutex> lock(thread_locks[channel->encChn]);
        if (IMP_Encoder_PollingStream(channel->encChn, 5000) == 0)
        {
            IMPEncoderStream stream;
            if (IMP_Encoder_GetStream(channel->encChn, &stream, false) != 0)
            {
                LOG_ERROR("IMP_Encoder_GetStream(" << channel->encChn << ") failed");
                errorCount++;
                continue;
            }

            int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;
            if (nal_ts - last_nal_ts > 1.5 * (1000000 / channel->stream->fps))
            {
                // Silence for now until further tests / THINGINO
                // LOG_WARN("The encoder 0 dropped a frame. " << (nalTs - channel.lastNalTs) << ", " << (1.5 * (1000000 / cfg->stream0.fps)));
            }

            struct timeval encode_time;
            encode_time.tv_sec = nal_ts / 1000000;
            encode_time.tv_usec = nal_ts % 1000000;

            for (unsigned int i = 0; i < stream.packCount; ++i)
            {
                fps++;
                bps += stream.pack[i].length;

                if(stream_sinks[channel->encChn]) {

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
                        nalu.duration = nal_ts - last_nal_ts;
                    }
                    else if (stream.pack[i].nalType.h265NalType == 19 ||
                            stream.pack[i].nalType.h265NalType == 20 ||
                            stream.pack[i].nalType.h265NalType == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                        if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                        {
                            nalu.duration = nal_ts - last_nal_ts;
                        }
#elif defined(PLATFORM_T30)
                    if (stream.pack[i].dataType.h264Type == 5 || stream.pack[i].dataType.h264Type == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
                    else if (stream.pack[i].dataType.h265Type == 19 ||
                            stream.pack[i].dataType.h265Type == 20 ||
                            stream.pack[i].dataType.h265Type == 1)
                    {
                        nalu.duration = nal_ts - last_nal_ts;
                    }
#endif
                    // We use start+4 because the encoder inserts 4-byte MPEG
                    //'startcodes' at the beginning of each NAL. Live555 complains
                    nalu.data.insert(nalu.data.end(), start + 4, end);

                    if (!stream_sinks[channel->encChn]->IDR)
                    {
#if defined(PLATFORM_T31)
                        if (stream.pack[i].nalType.h264NalType == 7 ||
                            stream.pack[i].nalType.h264NalType == 8 ||
                            stream.pack[i].nalType.h264NalType == 5)
                        {
                            stream_sinks[channel->encChn]->IDR = true;
                        }
                        else if (stream.pack[i].nalType.h265NalType == 32)
                        {
                            stream_sinks[channel->encChn]->IDR = true;
                        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23)
                            if (stream.pack[i].dataType.h264Type == 7 ||
                                stream.pack[i].dataType.h264Type == 8 ||
                                stream.pack[i].dataType.h264Type == 5)
                            {
                                stream_sinks[channel->encChn]->IDR = true;
                            }
#elif defined(PLATFORM_T30)
                        if (stream.pack[i].dataType.h264Type == 7 ||
                            stream.pack[i].dataType.h264Type == 8 ||
                            stream.pack[i].dataType.h264Type == 5)
                        {
                            stream_sinks[channel->encChn]->IDR = true;
                        }
                        else if (stream.pack[i].dataType.h265Type == 32)
                        {
                            stream_sinks[channel->encChn]->IDR = true;
                        }
#endif
                    }
                    if (stream_sinks[channel->encChn]->IDR)
                    {
                        if (stream_sinks[channel->encChn]->data_available_callback(nalu))
                        {

                            LOG_ERROR("stream encChn:" << sink->encChn << ", size:" << nalu.data.size()
                                    << ", pC:" << stream.packCount << ", pS:" << nalu.data.size() << ", pN:" 
                                    << i << " clogged!");
                        }
                    }
                }
            }

            IMP_Encoder_ReleaseStream(channel->encChn, &stream);
            last_nal_ts = nal_ts;

            if (channel->updateOsd)
            {
                channel->updateOsd();
            }
            
            ms = timeDiffInMs(&channel->stream->osd.stats.ts);
            if (ms > 1000)
            {
                channel->stream->osd.stats.bps = ((bps * 8) * 1000 / ms) / 1000; bps = 0;
                channel->stream->osd.stats.fps = fps * 1000 / ms; fps = 0;
                gettimeofday(&channel->stream->osd.stats.ts, NULL);
            }
        }
        else
        {
            LOG_DEBUG("IMP_Encoder_PollingStream(" << channel->encChn << ", 5000) timeout !");
        }
        lock.unlock();
    }
    LOG_DEBUG("BYE " << channel->encChn);
    return;
}

void Encoder::run()
{
    LOG_DEBUG("Encoder::run()");

    // The encoder thread is very important, but we
    // want sink threads to have higher priority.
    //nice(-19);

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
            if (ret != 0)
                return;

            IMP_System_RebaseTimeStamp(0);

            if (cfg->stream0.enabled)
            {
                channels[0] = new Channel{0};
                //pthread_mutex_init(&stream_locks[0], NULL);
                gettimeofday(&cfg->stream0.osd.stats.ts, NULL);
                channels[0]->stream = &cfg->stream0;
                if (cfg->stream0.osd.enabled)
                {
                    channels[0]->updateOsd = std::bind(&OSD::update, stream0_osd);
                }
                stream_threads[0] = std::thread(&Encoder::stream_grabber, this, channels[0]);
            }

            if (cfg->stream1.enabled)
            {
                channels[1] = new Channel{1};
                //pthread_mutex_init(&stream_locks[1], NULL);
                gettimeofday(&cfg->stream1.osd.stats.ts, NULL);
                channels[1]->stream = &cfg->stream1;
                if (cfg->stream1.osd.enabled)
                {
                    channels[1]->updateOsd = std::bind(&OSD::update, stream1_osd);
                }
                stream_threads[1] = std::thread(&Encoder::stream_grabber, this, channels[1]);
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

        LOG_DEBUG("PRE cfg->encoder_thread_signal.wait(3);");

        cfg->encoder_thread_signal.wait(3);

        LOG_DEBUG("POST cfg->encoder_thread_signal.wait(3);");

        cfg->jpg_thread_signal.fetch_or(4);

        // 4 = Stop threads
        if (cfg->encoder_thread_signal.load() & 4)
        {
            if (cfg->stream0.enabled)
            {
                LOG_DEBUG("WAIT FOR THREAD EXIT 0");
                channels[0]->thread_signal.store(false);
                if(stream_threads[0].joinable()){
                    stream_threads[0].join();
                }
                LOG_DEBUG("THREAD EXIT 0");
                IMP_Encoder_StopRecvPic(0);
                //pthread_mutex_destroy(&stream_locks[0]);
                delete channels[0];
                channels[0] = nullptr;
            }

            if (cfg->stream1.enabled)
            {
                LOG_DEBUG("WAIT FOR THREAD EXIT 1");
                channels[1]->thread_signal.store(false);
                if(stream_threads[1].joinable()){
                    stream_threads[1].join();
                }
                LOG_DEBUG("THREAD EXIT 1");
                IMP_Encoder_StopRecvPic(1);
                //pthread_mutex_destroy(&stream_locks[1]);
                delete channels[1];
                channels[1] = nullptr;                
            }

            exit();
        }
    }
}
