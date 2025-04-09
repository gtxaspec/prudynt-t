#include "IMPBackchannel.hpp"

#include "Config.hpp"
#include "Logger.hpp"

#include <aaccommon.h>
#include <aacdec.h>

#include <imp/imp_audio.h>

#define MODULE "IMPBackchannel"

// Thread-local storage for the AAC decoder instance.
thread_local HAACDecoder tl_aacDecoder = nullptr;
thread_local _AACFrameInfo aacFrameInfo{};

static int aac_openDecoder(void * /*pvoidDecoderAttr*/, void * /*pDecoder*/)
{
    if (tl_aacDecoder != nullptr)
    {
        LOG_WARN("AAC decoder already initialized for this thread.");
        return 0;
    }
    tl_aacDecoder = AACInitDecoder();
    if (!tl_aacDecoder)
    {
        LOG_ERROR("Failed to create AAC decoder for this thread.");
        return -1;
    }

    memset(&aacFrameInfo, 0, sizeof(_AACFrameInfo));
    aacFrameInfo.nChans = 1;
    aacFrameInfo.sampRateCore = cfg->audio.output_sample_rate;
    aacFrameInfo.profile = AAC_PROFILE_LC;

    int raw_ret = AACSetRawBlockParams(tl_aacDecoder, 0, &aacFrameInfo);
    if (raw_ret != ERR_AAC_NONE)
    {
        LOG_ERROR("AACSetRawBlockParams failed: " << raw_ret);
    }

    LOG_DEBUG("Thread-local AAC decoder opened successfully.");
    return 0;
}

static int aac_decodeFrm(void * /*pDecoder*/,
                         unsigned char *inputBuffer,
                         int inputLength,
                         unsigned short *outputBuffer,
                         int *outputLengthPtr,
                         int * /*ps32Chns*/)
{
    if (!tl_aacDecoder)
    {
        LOG_ERROR("AAC decoder instance is not initialized for this thread in decodeFrm");
        *outputLengthPtr = 0;
        return -1;
    }

    unsigned char *inPtr = inputBuffer;
    int bytesLeft = inputLength;
    short *outPtr = (short *) outputBuffer;

    // Parse MPEG4-GENERIC Header (RFC 3640)
    if (bytesLeft < 2)
    {
        LOG_WARN("Input buffer too small for AU-headers-length (" << bytesLeft << " < 2)");
        *outputLengthPtr = 0;
        return 0;
    }

    // Read AU-headers-length (in bits)
    uint16_t auHeadersLengthBits = (inPtr[0] << 8) | inPtr[1];
    int auHeadersSizeBytes = auHeadersLengthBits / 8;
    int totalHeaderSizeBytes = 2 + auHeadersSizeBytes;

    if (bytesLeft < totalHeaderSizeBytes)
    {
        LOG_WARN("Input buffer too small for full MPEG4-GENERIC header ("
                 << bytesLeft << " < " << totalHeaderSizeBytes << ")");
        *outputLengthPtr = 0;
        return 0;
    }

    // Assuming only one AU-header follows the length field (common case)
    if (auHeadersSizeBytes < 2)
    {
        LOG_ERROR("Expected at least one 2-byte AU-header, but AU-headers-length is only "
                  << auHeadersLengthBits << " bits.");
        *outputLengthPtr = 0;
        return -1;
    }

    uint16_t frameLengthBits = (inPtr[2] << 8) | inPtr[3];
    int frameLengthBytes = frameLengthBits / 8;
    unsigned char *aacDataPtr = inPtr + totalHeaderSizeBytes;
    int bytesAvailableAfterHeader = bytesLeft - totalHeaderSizeBytes;
    if (bytesAvailableAfterHeader < frameLengthBytes)
    {
        LOG_WARN("Buffer contains insufficient data after header for declared frame length ("
                 << bytesAvailableAfterHeader << " < " << frameLengthBytes << ")");
        *outputLengthPtr = 0;
        return 0;
    }

    // Set the pointer and length for AACDecode
    inPtr = aacDataPtr;
    bytesLeft = frameLengthBytes;
    if (bytesLeft <= 0)
    {
        LOG_WARN("Calculated AAC frame length is zero or negative.");
        *outputLengthPtr = 0;
        return 0;
    }

    int decodeBytesLeft = bytesLeft;
    int ret = AACDecode(tl_aacDecoder, &inPtr, &decodeBytesLeft, outPtr);

    if (ret < 0)
    {
        if (ret != ERR_AAC_INDATA_UNDERFLOW)
        {
            LOG_ERROR("Thread-local AAC decode failed: " << ret << " (" << decodeBytesLeft
                                                         << " bytes left unprocessed)");
        }
        *outputLengthPtr = 0;
        return (ret == ERR_AAC_INDATA_UNDERFLOW) ? 0 : -1;
    }

    // Get frame info to determine actual output channels
    AACFrameInfo frameInfoOut;
    AACGetLastFrameInfo(tl_aacDecoder, &frameInfoOut);
    *outputLengthPtr = frameInfoOut.outputSamps * sizeof(short);
    return 0;
}

static int aac_closeDecoder(void * /*pDecoder*/)
{
    if (!tl_aacDecoder)
    {
        LOG_WARN("aac_closeDecoder called but thread-local decoder instance is already NULL.");
        return 0;
    }

    AACFreeDecoder(tl_aacDecoder);
    tl_aacDecoder = nullptr;
    memset(&aacFrameInfo, 0, sizeof(_AACFrameInfo));
    LOG_DEBUG("Thread-local AAC decoder closed successfully.");
    return 0;
}

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
    adec_attr.bufSize = 20;

    // Create G711U channel
    adec_attr.type = PT_G711U;
    int adChn_pcmu = (int) IMPBackchannelFormat::PCMU;
    ret = IMP_ADEC_CreateChn(adChn_pcmu, &adec_attr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_CreateChn(PCMU, " << adChn_pcmu << ")");

    // Create G711A channel
    adec_attr.type = PT_G711A;
    int adChn_pcma = (int) IMPBackchannelFormat::PCMA;
    ret = IMP_ADEC_CreateChn(adChn_pcma, &adec_attr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_CreateChn(PCMA, " << adChn_pcma << ")");

    // Register the custom AAC decoder callbacks with the IMP SDK if not already done.
    if (aacDecoderHandle == -1)
    {
        IMPAudioDecDecoder aacDecoderCallbacks;
        aacDecoderCallbacks.type = PT_MAX;
        snprintf(aacDecoderCallbacks.name, sizeof(aacDecoderCallbacks.name), "AAC");
        aacDecoderCallbacks.openDecoder = aac_openDecoder;
        aacDecoderCallbacks.decodeFrm = aac_decodeFrm;
        aacDecoderCallbacks.getFrmInfo = NULL;
        aacDecoderCallbacks.closeDecoder = aac_closeDecoder;

        ret = IMP_ADEC_RegisterDecoder(&aacDecoderHandle, &aacDecoderCallbacks);
        if (ret != 0)
        {
            LOG_ERROR("Failed to register AAC decoder: " << ret);
            aacDecoderHandle = -1;
        }
        else
        {
            LOG_DEBUG("Registered AAC decoder with handle: " << aacDecoderHandle);
        }
    }
    else
    {
        LOG_DEBUG("AAC decoder already registered with handle: " << aacDecoderHandle);
    }

    if (aacDecoderHandle != -1)
    {
        // Use the handle returned by RegisterDecoder as the type for this channel
        adec_attr.type = (IMPAudioPalyloadType) aacDecoderHandle;
        int adChn_aac = (int) IMPBackchannelFormat::AAC;
        ret = IMP_ADEC_CreateChn(adChn_aac, &adec_attr);
        LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_CreateChn(AAC, " << adChn_aac << ")");
    }

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
        LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_DestroyChn(" << adChn << ")"); \
    }
    X_FOREACH_BACKCHANNEL_FORMAT(DESTROY_ADEC)
#undef DESTROY_ADEC

    if (aacDecoderHandle != -1)
    {
        ret = IMP_ADEC_UnRegisterDecoder(&aacDecoderHandle);
        LOG_DEBUG_OR_ERROR(ret, "IMP_ADEC_UnRegisterDecoder(" << aacDecoderHandle << ")");
        aacDecoderHandle = -1;
    }
}
