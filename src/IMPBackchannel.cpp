#include "IMPBackchannel.hpp"

#include "Config.hpp"
#include "Logger.hpp"

#include <imp/imp_audio.h>

#define MODULE "IMPBackchannel"

IMPBackchannel *IMPBackchannel::createNew()
{
    return new IMPBackchannel();
}

int IMPBackchannel::init()
{
    LOG_DEBUG("IMPBackchannel::init()");
    int ret = 0;

    IMPAudioDecChnAttr adec_attr;
    adec_attr.mode = ADEC_MODE_PACK;

    // Create G711U channel
    adec_attr.bufSize = 20; // Default buffer size for G711
    adec_attr.type = PT_G711U;
    int adChn = (int) IMPBackchannelFormat::PCMU;
    ret = IMP_ADEC_CreateChn(adChn, &adec_attr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_CreateChn(PCMU, " << adChn << ")");

    // Create G711A channel
    adec_attr.bufSize = 20; // Default buffer size for G711
    adec_attr.type = PT_G711A;
    adChn = (int) IMPBackchannelFormat::PCMA;
    ret = IMP_ADEC_CreateChn(adChn, &adec_attr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_CreateChn(PCMA, " << adChn << ")");

    return 0;
}

void IMPBackchannel::deinit()
{
    LOG_DEBUG("IMPBackchannel::deinit()");
    int ret;

#define DESTROY_ADEC(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        int adChn = (int) IMPBackchannelFormat::EnumName; \
        ret = IMP_ADEC_DestroyChn(adChn); \
        LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_DestroyChn(" #EnumName ", " << adChn << ")"); \
    }
    X_FOREACH_BACKCHANNEL_FORMAT(DESTROY_ADEC)
#undef DESTROY_ADEC
}
