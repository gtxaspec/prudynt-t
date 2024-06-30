#include "IMPEncoder.hpp"

#define MODULE "IMPENCODER"

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

IMPEncoder *IMPEncoder::createNew(
    _stream *stream,
    std::shared_ptr<CFG> cfg,
    int encChn,
    int encGrp,
    const char *name)
{
    return new IMPEncoder(stream, cfg, encChn, encGrp, name);
}

void MakeTables(int q, uint8_t* lqt, uint8_t* cqt) {
    // Ensure q is within the expected range
    q = std::max(1, std::min(q, 99));

    // Adjust q based on factor
    if (q < 50) {
        q = 5000 / q;
    } else {
        q = 200 - 2 * q;
    }

    // Fill the quantization tables
    for (int i = 0; i < 64; ++i) {
        int lq = (jpeg_luma_quantizer[i] * q + 50) / 100;
        int cq = (jpeg_chroma_quantizer[i] * q + 50) / 100;

        // Ensure the quantization values are within [1, 255]
        lqt[i] = static_cast<uint8_t>(std::max(1, std::min(lq, 255)));
        cqt[i] = static_cast<uint8_t>(std::max(1, std::min(cq, 255)));
    }
}

void IMPEncoder::initProfile()
{
    IMPEncoderRcAttr *rcAttr;
    memset(&chnAttr, 0, sizeof(IMPEncoderCHNAttr));
    rcAttr = &chnAttr.rcAttr;

#if defined(PLATFORM_T31)

    IMPEncoderRcMode rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    IMPEncoderProfile encoderProfile = IMP_ENC_PROFILE_AVC_HIGH;

    if (strcmp(stream->format, "H265") == 0)
    {
        encoderProfile = IMP_ENC_PROFILE_HEVC_MAIN;
    }
    else if (strcmp(stream->format, "JPEG") == 0)
    {
        encoderProfile = IMP_ENC_PROFILE_JPEG;
        IMP_Encoder_SetDefaultParam(&chnAttr, encoderProfile, IMP_ENC_RC_MODE_FIXQP,
                                    stream->width, stream->height, 24, 1, 0, 0, stream->jpeg_quality, 0);
        //1000 / stream->jpeg_refresh
        LOG_DEBUG("STREAM PROFILE " << encChn << ", " << encGrp << ", " << stream->format << ", "
                                    << chnAttr.rcAttr.outFrmRate.frmRateNum << "fps, profile:" << stream->profile << ", " << 
                                    stream->width << "x" << stream->height);                                    
        return;
    }

    if (strcmp(stream->mode, "FIXQP") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_FIXQP;
    }
    else if (strcmp(stream->mode, "VBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_VBR;
    }
    else if (strcmp(stream->mode, "CBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CBR;
    }
    else if (strcmp(stream->mode, "CAPPED_VBR") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CAPPED_VBR;
    }
    else if (strcmp(stream->mode, "CAPPED_QUALITY") == 0)
    {
        rcMode = IMP_ENC_RC_MODE_CAPPED_QUALITY;
    }
    else
    {
        LOG_ERROR("unsupported stream->mode (" << stream->mode << "). we only support FIXQB, CBR, VBR, CAPPED_VBR and CAPPED_QUALITY on T31");
    }

    IMP_Encoder_SetDefaultParam(
        &chnAttr, encoderProfile, rcMode, stream->width, stream->height,
        stream->fps, 1, stream->gop, 2, -1, stream->bitrate);

    LOG_DEBUG("PT_H264");

    switch (rcMode)
    {
    case IMP_ENC_RC_MODE_FIXQP:
        rcAttr->attrRcMode.attrFixQp.iInitialQP = 38;
        break;
    case IMP_ENC_RC_MODE_CBR:
        rcAttr->attrRcMode.attrCbr.uTargetBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrCbr.iInitialQP = -1;
        rcAttr->attrRcMode.attrCbr.iMinQP = 34;
        rcAttr->attrRcMode.attrCbr.iMaxQP = 51;
        rcAttr->attrRcMode.attrCbr.iIPDelta = -1;
        rcAttr->attrRcMode.attrCbr.iPBDelta = -1;
        // rcAttr->attrRcMode.attrCbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rcAttr->attrRcMode.attrCbr.uMaxPictureSize = stream->bitrate;
        break;
    case IMP_ENC_RC_MODE_VBR:
        rcAttr->attrRcMode.attrVbr.uTargetBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrVbr.uMaxBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
        rcAttr->attrRcMode.attrVbr.iMinQP = 20;
        rcAttr->attrRcMode.attrVbr.iMaxQP = 45;
        rcAttr->attrRcMode.attrVbr.iIPDelta = 3;
        rcAttr->attrRcMode.attrVbr.iPBDelta = 3;
        // rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rcAttr->attrRcMode.attrVbr.uMaxPictureSize = stream->bitrate;
        break;
    case IMP_ENC_RC_MODE_CAPPED_VBR:
        rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
        rcAttr->attrRcMode.attrCappedVbr.iMinQP = 20;
        rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 45;
        rcAttr->attrRcMode.attrCappedVbr.iIPDelta = 3;
        rcAttr->attrRcMode.attrCappedVbr.iPBDelta = 3;
        // rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = stream->bitrate;
        rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = stream->bitrate;
        rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
        rcAttr->attrRcMode.attrCappedQuality.iMinQP = 20;
        rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 45;
        rcAttr->attrRcMode.attrCappedQuality.iIPDelta = 3;
        rcAttr->attrRcMode.attrCappedQuality.iPBDelta = 4;
        // rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = stream->bitrate;
        rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
        break;
    }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30)
    if (strcmp(stream->format, "JPEG") == 0)
    {
        IMPEncoderAttr *encAttr;
        encAttr = &chnAttr.encAttr;
        encAttr->enType = PT_JPEG;
        encAttr->bufSize = 0;
        encAttr->profile = 2;
        encAttr->picWidth = stream->width;
        encAttr->picHeight = stream->height;
        return;
    }
    else if (strcmp(stream->format, "H264") == 0)
    {
        chnAttr.encAttr.enType = PT_H264;
    }
#if defined(PLATFORM_T30)
    else if (strcmp(stream->format, "H265") == 0)
    {
        chnAttr.encAttr.enType = PT_H265;
    }
#endif

    IMPEncoderRcMode rcMode = ENC_RC_MODE_SMART;

    if (strcmp(stream->mode, "FIXQP") == 0)
    {
        rcMode = ENC_RC_MODE_FIXQP;
    }
    else if (strcmp(stream->mode, "VBR") == 0)
    {
        rcMode = ENC_RC_MODE_VBR;
    }
    else if (strcmp(stream->mode, "CBR") == 0)
    {
        rcMode = ENC_RC_MODE_CBR;
    }
    else if (strcmp(stream->mode, "SMART") == 0)
    {
        rcMode = ENC_RC_MODE_SMART;
    }
    else
    {
        LOG_ERROR("unsupported stream->mode (" << stream->mode << "). we only support FIXQB, CBR, VBR and SMART");
    }

    // 0 = Baseline
    // 1 = Main
    // 2 = High
    // Note: The encoder seems to emit frames at half the
    // requested framerate when the profile is set to Baseline.
    // For this reason, Main or High are recommended.
    chnAttr.encAttr.profile = stream->profile;
    chnAttr.encAttr.bufSize = 0;
    chnAttr.encAttr.picWidth = stream->width;
    chnAttr.encAttr.picHeight = stream->height;
    chnAttr.rcAttr.outFrmRate.frmRateNum = stream->fps;
    chnAttr.rcAttr.outFrmRate.frmRateDen = 1;
    rcAttr->maxGop = stream->max_gop;

    if (chnAttr.encAttr.enType = PT_H264)
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
            rcAttr->attrRcMode.attrH264Cbr.outBitRate = stream->bitrate;
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
            rcAttr->attrRcMode.attrH264Vbr.maxBitRate = stream->bitrate;
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
            rcAttr->attrRcMode.attrH264Smart.minQp = 24;
            rcAttr->attrRcMode.attrH264Smart.staticTime = 2;
            rcAttr->attrRcMode.attrH264Smart.maxBitRate = stream->bitrate;
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
        rcAttr->attrRcMode.rcMode = ENC_RC_MODE_SMART;
        rcAttr->attrRcMode.attrH265Smart.maxQp = 45;
        rcAttr->attrRcMode.attrH265Smart.minQp = 15;
        rcAttr->attrRcMode.attrH265Smart.staticTime = 2;
        rcAttr->attrRcMode.attrH265Smart.maxBitRate = stream->bitrate;
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
    LOG_DEBUG("STREAM PROFILE " << stream->rtsp_endpoint << ", "
                                << chnAttr.rcAttr.outFrmRate.frmRateNum << "fps, " << stream->bitrate << "bps, "
                                << stream->gop << "gop, profile:" << stream->profile << ", mode:" << rcMode << ", " 
                                << stream->width << "x" << stream->height);

}

int getChannelInfo(int encChn)
{
    int ret = 0;

    IMPEncoderCHNAttr iChnAttr{};
    memset(&iChnAttr, 0, sizeof(IMPEncoderCHNAttr));
    ret = IMP_Encoder_GetChnAttr(encChn, &iChnAttr);
    std::cout << "IMP_Encoder_GetChnAttr(" << encChn << ")" << std::endl;;

    std::cout <<
              "iChnAttr {\n" <<
              "  .encAttr {\n" <<
              "    .enType = " << iChnAttr.encAttr.enType << "\n" <<
              "    .bufSize = " << iChnAttr.encAttr.bufSize << "\n" <<
              "    .profile = " << iChnAttr.encAttr.profile << "\n" <<
              "    .picWidth = " << iChnAttr.encAttr.picWidth << "\n" <<
              "    .picHeight = " << iChnAttr.encAttr.picHeight << "\n" <<
              "    .crop {\n" <<
              "      .x = " << iChnAttr.encAttr.crop.x << "\n" <<
              "      .y = " << iChnAttr.encAttr.crop.y << "\n" <<
              "      .w = " << iChnAttr.encAttr.crop.w << "\n" <<
              "      .h = " << iChnAttr.encAttr.crop.h << "\n" <<
              "    }\n" <<
              "    .userData {\n" <<
              "      .maxUserDataCnt = " << iChnAttr.encAttr.userData.maxUserDataCnt << "\n" <<
              "      .maxUserDataSize = " << iChnAttr.encAttr.userData.maxUserDataSize << "\n" <<
              "    }\n" <<
              "  }\n" <<
              "  .rcAttr {\n" <<
              "    .outFrmRate {\n" <<
              "      .frmRateNum = " << iChnAttr.rcAttr.outFrmRate.frmRateNum << "\n" <<
              "      .frmRateDen = " << iChnAttr.rcAttr.outFrmRate.frmRateDen << "\n" <<
              "    }\n" <<
              "    .maxGop = " << iChnAttr.rcAttr.maxGop << "\n" <<
              "    .attrRcMode {\n" <<
              "      .rcMode = " << iChnAttr.rcAttr.attrRcMode.rcMode << "\n";

    switch (iChnAttr.rcAttr.attrRcMode.rcMode)
    {
    case ENC_RC_MODE_FIXQP:
        std::cout <<
                  "      .attrH264FixQp {\n" <<
                  "        .qp = " << iChnAttr.rcAttr.attrRcMode.attrH264FixQp.qp << "\n" <<
                  "      }\n";
        break;
    case ENC_RC_MODE_CBR:
        std::cout <<
                  "      .attrH264Cbr {\n" <<
                  "        .maxQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.maxQp << "\n" <<
                  "        .minQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.minQp << "\n" <<
                  "        .outBitRate = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.outBitRate << "\n" <<
                  "        .iBiasLvl = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.iBiasLvl << "\n" <<
                  "        .frmQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.frmQPStep << "\n" <<
                  "        .gopQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.gopQPStep << "\n" <<
                  "        .adaptiveMode = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.adaptiveMode << "\n" <<
                  "        .gopRelation = " << iChnAttr.rcAttr.attrRcMode.attrH264Cbr.gopRelation << "\n" <<
                  "      }\n";
        break;
    case ENC_RC_MODE_VBR:
        std::cout <<
                  "      .attrH264Vbr {\n" <<
                  "        .maxQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.maxQp << "\n" <<
                  "        .minQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.minQp << "\n" <<
                  "        .staticTime = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.staticTime << "\n" <<
                  "        .maxBitRate = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.maxBitRate << "\n" <<
                  "        .iBiasLvl = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.iBiasLvl << "\n" <<
                  "        .changePos = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.changePos << "\n" <<
                  "        .qualityLvl = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.qualityLvl << "\n" <<
                  "        .frmQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.frmQPStep << "\n" <<
                  "        .gopQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.gopQPStep << "\n" <<
                  "        .gopRelation = " << iChnAttr.rcAttr.attrRcMode.attrH264Vbr.gopRelation << "\n" <<
                  "      }\n";
        break;
    case ENC_RC_MODE_SMART:
        std::cout <<
                  "      .attrH264Smart {\n" <<
                  "        .maxQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.maxQp << "\n" <<
                  "        .minQp = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.minQp << "\n" <<
                  "        .staticTime = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.staticTime << "\n" <<
                  "        .maxBitRate = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.maxBitRate << "\n" <<
                  "        .iBiasLvl = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.iBiasLvl << "\n" <<
                  "        .changePos = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.changePos << "\n" <<
                  "        .qualityLvl = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.qualityLvl << "\n" <<
                  "        .frmQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.frmQPStep << "\n" <<
                  "        .gopQPStep = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.gopQPStep << "\n" <<
                  "        .gopRelation = " << iChnAttr.rcAttr.attrRcMode.attrH264Smart.gopRelation << "\n" <<
                  "      }\n";
        break;
    }

    std::cout <<
              "    }\n" <<
              "    .attrFrmUsed {\n" <<
              "      .enable = " << iChnAttr.rcAttr.attrFrmUsed.enable << "\n" <<
              "      .frmUsedMode = " << iChnAttr.rcAttr.attrFrmUsed.frmUsedMode << "\n" <<
              "      .frmUsedTimes = " << iChnAttr.rcAttr.attrFrmUsed.frmUsedTimes << "\n" <<
              "    }\n" <<
              "    .attrDemask {\n" <<
              "      .enable = " << iChnAttr.rcAttr.attrDemask.enable << "\n" <<
              "      .isAutoMode = " << iChnAttr.rcAttr.attrDemask.isAutoMode << "\n" <<
              "      .demaskCnt = " << iChnAttr.rcAttr.attrDemask.demaskCnt << "\n" <<
              "      .demaskThresd = " << iChnAttr.rcAttr.attrDemask.demaskThresd << "\n" <<
              "    }\n" <<
              "    .attrDenoise {\n" <<
              "      .enable = " << iChnAttr.rcAttr.attrDenoise.enable << "\n" <<
              "      .dnType = " << iChnAttr.rcAttr.attrDenoise.dnType << "\n" <<
              "      .dnIQp = " << iChnAttr.rcAttr.attrDenoise.dnIQp << "\n" <<
              "      .dnPQp = " << iChnAttr.rcAttr.attrDenoise.dnPQp << "\n" <<
              "    }\n" <<
              "    .attrHSkip {\n" <<
              "      .hSkipAttr {\n" <<
              "        .skipType = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.skipType << "\n" <<
              "        .m = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.skipType << "\n" <<
              "        .n = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.skipType << "\n" <<
              "        .maxSameSceneCnt = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.maxSameSceneCnt << "\n" <<
              "        .bEnableScenecut = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.bEnableScenecut << "\n" <<
              "        .bBlackEnhance = " << iChnAttr.rcAttr.attrHSkip.hSkipAttr.bBlackEnhance << "\n" <<
              "      }\n" <<
              "    }\n" <<
              "  }\n" <<
              "}\n";

    return 0;
}

int IMPEncoder::init()
{
    LOG_DEBUG("IMPEncoder::init()");

    int ret = 0;

    initProfile();

    ret = IMP_Encoder_CreateChn(encChn, &chnAttr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateChn(" << encChn << ", chnAttr)");

    ret = IMP_Encoder_RegisterChn(encGrp, encChn);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_RegisterChn(" << encGrp << ", " << encChn << ")");

    if (strcmp(stream->format, "JPEG") != 0)
    {

        ret = IMP_Encoder_CreateGroup(encGrp);
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_CreateGroup(" << encGrp << ")");

        fs = {DEV_ID_FS, encGrp, 0};
        enc = {DEV_ID_ENC, encGrp, 0};
        osd_cell = {DEV_ID_OSD, encGrp, 0};

        if (stream->osd.enabled)
        {
            osd = OSD::createNew(&(stream->osd), cfg, encGrp, encChn, name);

            ret = IMP_System_Bind(&fs, &osd_cell);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&fs, &osd_cell)");

            ret = IMP_System_Bind(&osd_cell, &enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&osd_cell, &enc)");
        }
        else
        {
            ret = IMP_System_Bind(&fs, &enc);
            LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_System_Bind(&fs, &enc)");
        }
    }
    else
    {
#if defined(PLATFORM_T31)
        ret = IMP_Encoder_SetbufshareChn(2, stream->jpeg_channel );
        LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_SetbufshareChn(2, " << stream->jpeg_channel << ")");
#else
        IMPEncoderJpegeQl pstJpegeQl;
        MakeTables(stream->jpeg_quality, &(pstJpegeQl.qmem_table[0]), &(pstJpegeQl.qmem_table[64]));
        pstJpegeQl.user_ql_en = 1;
        IMP_Encoder_SetJpegeQl(2, &pstJpegeQl);
#endif        
    }

    //Debugging function !!
    //getChannelInfo(encChn);

    return ret;
}

int IMPEncoder::deinit()
{
    int ret;

    if (strcmp(stream->format, "JPEG") != 0)
    {
        if (osd)
        {
            osd->exit();
            delete osd;
            osd = nullptr;

            ret = IMP_System_UnBind(&fs, &osd_cell);
            LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&fs, &osd_cell)");

            ret = IMP_System_UnBind(&osd_cell, &enc);
            LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&osd_cell, &enc)");
        }
        else
        {
            ret = IMP_System_UnBind(&fs, &enc);
            LOG_DEBUG_OR_ERROR(ret, "IMP_System_UnBind(&fs, &enc)");
        }
    }

    ret = IMP_Encoder_UnRegisterChn(encChn);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_UnRegisterChn(" << encChn << ")");

    ret = IMP_Encoder_DestroyChn(encChn);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_Encoder_DestroyChn(" << encChn << ")");

    return ret;
}

int IMPEncoder::destroy() {

    int ret;

    ret = IMP_Encoder_DestroyGroup(encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_DestroyGroup(" << encChn << ")");
    
    return ret;
}