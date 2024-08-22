#include "Config.hpp"
#include "IMPAudio.hpp"
#include "Opus.hpp"
#include <thread>

#define MODULE "IMPAUDIO"

static thread_local IMPAudioEncoder *encoder = nullptr;

static int openEncoder(void* attr, void* enc)
{
    return encoder ? encoder->open() : -1;
}

static int encodeFrame(void* enc, IMPAudioFrame* data, unsigned char* outbuf, int* outLen)
{
    return encoder ? encoder->encode(data, outbuf, outLen) : -1;
}

static int closeEncoder(void* enc)
{
    return encoder ? encoder->close() : -1;
}

IMPAudio *IMPAudio::createNew(
    int devId,
    int inChn,
    int aeChn)
{
    return new IMPAudio(devId, inChn, aeChn);
}

int IMPAudio::init()
{
    LOG_DEBUG("IMPAudio::init()");
    int ret;

    format = IMPAudioFormat::PCM;
    IMPAudioIOAttr ioattr = {
        .samplerate = AUDIO_SAMPLE_RATE_16000,
        .bitwidth = AUDIO_BIT_WIDTH_16,
        .soundmode = AUDIO_SOUND_MODE_MONO,
        .frmNum = 30,
        .chnCnt = 1,
    };
    IMPAudioEncChnAttr encattr = {
        .type = IMPAudioPalyloadType::PT_PCM,
        .bufSize = 2,
    };

    if (strcmp(cfg->audio.input_format, "OPUS") == 0)
    {
        format = IMPAudioFormat::OPUS;
        encattr.type = IMPAudioPalyloadType::PT_MAX;
        ioattr.samplerate = AUDIO_SAMPLE_RATE_48000;
    }
    else if (strcmp(cfg->audio.input_format, "G711A") == 0)
    {
        format = IMPAudioFormat::G711A;
        encattr.type = IMPAudioPalyloadType::PT_G711A;
        ioattr.samplerate = AUDIO_SAMPLE_RATE_8000;
    }
    else if (strcmp(cfg->audio.input_format, "G711U") == 0)
    {
        format = IMPAudioFormat::G711U;
        encattr.type = IMPAudioPalyloadType::PT_G711U;
        ioattr.samplerate = AUDIO_SAMPLE_RATE_8000;
    }
    else if (strcmp(cfg->audio.input_format, "G726") == 0)
    {
        format = IMPAudioFormat::G726;
        encattr.type = IMPAudioPalyloadType::PT_G726;
        ioattr.samplerate = AUDIO_SAMPLE_RATE_8000;
    }
    else if (strcmp(cfg->audio.input_format, "PCM") != 0)
    {
        LOG_ERROR("unsupported audio->input_format (" << cfg->audio.input_format
            << "). we only support OPUS, G711A, G711U, G726, and PCM.");
    }

    // sample points per frame
    ioattr.numPerFrm = (int)ioattr.samplerate * 0.040;
    // compute bitrate in kbps
    bitrate = (int) ioattr.bitwidth * (int) ioattr.samplerate / 1000;

    if (encattr.type == IMPAudioPalyloadType::PT_MAX)
    {
        if (format == IMPAudioFormat::OPUS)
        {
            encoder = Opus::createNew(ioattr.samplerate, ioattr.chnCnt);
        }

        IMPAudioEncEncoder enc;
        enc.maxFrmLen = 1024; // Maximum code stream length
        std::snprintf(enc.name, sizeof(enc.name), "%s", cfg->audio.input_format);
        enc.openEncoder = openEncoder;
        enc.encoderFrm = encodeFrame;
        enc.closeEncoder = closeEncoder;

        ret = IMP_AENC_RegisterEncoder(&handle, &enc);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AENC_RegisterEncoder(&handle, &enc)");

        encattr.type = static_cast<IMPAudioPalyloadType>(handle);
    }

    if (encattr.type > IMPAudioPalyloadType::PT_PCM)
    {
        ret = IMP_AENC_CreateChn(aeChn, &encattr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AENC_CreateChn(" << aeChn << ", &encattr)");
    }

    ret = IMP_AI_SetPubAttr(devId, &ioattr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetPubAttr(" << devId << ")");

    memset(&ioattr, 0x0, sizeof(ioattr));
    ret = IMP_AI_GetPubAttr(devId, &ioattr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_GetPubAttr(" << devId << ")");

    ret = IMP_AI_Enable(devId);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Enable(" << devId << ")");

    IMPAudioIChnParam chnParam = {
        .usrFrmDepth = 30, // frame buffer depth
    };

    ret = IMP_AI_SetChnParam(devId, inChn, &chnParam);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetChnParam(" << devId << ", " << inChn << ")");

    memset(&chnParam, 0x0, sizeof(chnParam));
    ret = IMP_AI_GetChnParam(devId, inChn, &chnParam);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_GetChnParam(" << devId << ", " << inChn << ")");

    ret = IMP_AI_EnableChn(devId, inChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableChn(" << devId << ", " << inChn << ")");

    ret = IMP_AI_SetVol(devId, inChn, cfg->audio.input_vol);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetVol(" << devId << ", " << inChn << ", " << cfg->audio.input_vol << ")");

    int vol;
    ret = IMP_AI_GetVol(devId, inChn, &vol);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_GetVol(" << devId << ", " << inChn << ", &vol)");

    ret = IMP_AI_SetGain(devId, inChn, cfg->audio.input_gain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetGain(" << devId << ", " << inChn << ", " << cfg->audio.input_gain << ")");

    int gain;
    ret = IMP_AI_GetGain(devId, inChn, &gain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_GetGain(" << devId << ", " << inChn << ", &gain)");

    LOG_INFO("Audio In: format:" << cfg->audio.input_format <<
            ", vol:" << vol <<
            ", gain:" << gain <<
            ", samplerate:" << ioattr.samplerate <<
            ", bitwidth:" << ioattr.bitwidth <<
            ", soundmode:" << ioattr.soundmode <<
            ", frmNum:" << ioattr.frmNum <<
            ", numPerFrm:" << ioattr.numPerFrm <<
            ", chnCnt:" << ioattr.chnCnt <<
            ", usrFrmDepth:" << chnParam.usrFrmDepth);

#if defined(LIB_AUDIO_PROCESSING)
    if (cfg->audio.input_noise_suppression)
    {
        ret = IMP_AI_EnableNs(&ioattr, cfg->audio.input_noise_suppression);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableNs(&ioattr, " << cfg->audio.input_noise_suppression << ")");
    }
    else
    {
        //ret = IMP_AI_DisableNs();
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableNs()");
    }

    if (cfg->audio.input_high_pass_filter)
    {
        ret = IMP_AI_EnableHpf(&ioattr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableHpf(&ioattr)");
    }
    else
    {
        //ret = IMP_AI_DisableHpf();
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableHpf()");
    }

#if defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30) || defined(PLATFORM_T31)
    if(cfg->audio.input_agc_enabled) {
        IMPAudioAgcConfig agcConfig = {
            /**< Gain level, with a range of [0, 31]. This represents the target
            volume level, measured in dB (decibels), and is a negative value. The
            smaller the value, the higher the volume. */
            .TargetLevelDbfs = cfg->audio.input_agc_target_level_dbfs,
            /**< Sets the maximum gain value, with a range of [0, 90]. 0 means no
            gain, and the larger the value, the higher the gain. */
            .CompressionGaindB = cfg->audio.input_agc_compression_gain_db,
        };
        /* Enable automatic gain control on platforms that advertise it. */
        ret = IMP_AI_EnableAgc(&ioattr, agcConfig);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AI_EnableAgc({" << agcConfig.TargetLevelDbfs << ", " << agcConfig.CompressionGaindB << "})");
    }
#endif
#if defined(PLATFORM_T21) || (defined(PLATFORM_T31))
    ret = IMP_AI_SetAlcGain(devId, inChn, cfg->audio.input_alc_gain);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_SetAlcGain(" << devId << ", " << inChn << ", " << cfg->audio.input_alc_gain << ")");
#endif        
#endif //LIB_AUDIO_PROCESSING
    return 0;
}

int IMPAudio::deinit()
{
    LOG_DEBUG("IMPAudio::deinit()");
    int ret;

    if (encoder)
    {
        ret = IMP_AENC_UnRegisterEncoder(&handle);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AENC_UnRegisterEncoder(&handle)");

        delete encoder;
        encoder = nullptr;
        handle = 0;
    }

    if (format != IMPAudioFormat::PCM)
    {
        ret = IMP_AENC_DestroyChn(aeChn);
        LOG_DEBUG_OR_ERROR(ret, "IMP_AENC_DestroyChn(" << aeChn << ")");
    }

    ret = IMP_AI_DisableChn(devId, inChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_DisableChn(" << devId << ", " << inChn << ")");

    ret = IMP_AI_Disable(devId);
    LOG_DEBUG_OR_ERROR(ret, "IMP_AI_Disable(" << devId << ")");

    return 0;
}
