#include "IMPAudio.hpp"

#define MODULE "IMPAUDIO"

IMPAudio *IMPAudio::createNew(
    std::shared_ptr<CFG> cfg,
    int devId,
    int inChn)
{
    return new IMPAudio(cfg, devId, inChn);
}

int IMPAudio::init()
{
    LOG_DEBUG("IMPAudio::init()");
    int ret;

    IMPAudioIOAttr attr;
    attr.samplerate = AUDIO_SAMPLE_RATE_8000;
    attr.bitwidth = AUDIO_BIT_WIDTH_16;
    attr.soundmode = AUDIO_SOUND_MODE_MONO;
    attr.frmNum = 20;
    attr.numPerFrm = 400;
    attr.chnCnt = 1;

    ret = IMP_AI_SetPubAttr(devId, &attr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetPubAttr(" << devId << ")");

    ret = IMP_AI_Enable(devId);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Enable(" << devId << ")");

    IMPAudioIChnParam chnParam {};
    chnParam.usrFrmDepth = 20;
    ret = IMP_AI_SetChnParam(devId, inChn, &chnParam);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetChnParam(" << devId << ", " << inChn << ")");

    ret = IMP_AI_EnableChn(devId, inChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableChn(" << devId << ", " << inChn << ")");

    ret = IMP_AI_SetVol(devId, inChn, cfg->audio.input_vol);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetVol(" << devId << ", " << inChn << ", " << cfg->audio.input_vol << ")");

    ret = IMP_AI_SetGain(devId, inChn, cfg->audio.input_gain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetGain(" << devId << ", " << inChn << ", " << cfg->audio.input_gain << ")");

#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
    ret = IMP_AI_SetAlcGain(devId, inChn, cfg->audio.input_alc_gain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetAlcGain(" << devId << ", " << inChn << ", " << cfg->audio.input_alc_gain << ")");
#endif

    IMPAudioIOAttr ioattr;
    ret = IMP_AI_GetPubAttr(devId, &ioattr);
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
        if (cfg->audio.input_high_pass_filter)
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

    return 0;
}

int IMPAudio::deinit()
{
    LOG_DEBUG("IMPAudio::deinit()");
    int ret;

    ret = IMP_AI_DisableChn(devId, inChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableChn(0)");

    ret = IMP_AI_Disable(devId);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Disable(0)");

    return 0;
}

int IMPAudio::destroy()
{

    return 0;
}
