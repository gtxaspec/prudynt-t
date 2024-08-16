#include "Logger.hpp"
#include "IMPFramesource.hpp"

#define MODULE "IMP_FRAMESOURCE"

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

IMPFramesource *IMPFramesource::createNew(
    _stream *stream,
    _sensor *sensor,
    int chnNr)
{
    return new IMPFramesource(stream, sensor, chnNr);
}

int IMPFramesource::init()
{
    LOG_DEBUG("IMPFramesource::init()");

    int ret = 0, scale = 0;

    IMPFSChnAttr chnAttr;
    memset(&chnAttr, 0, sizeof(IMPFSChnAttr));

    ret = IMP_FrameSource_GetChnAttr(chnNr, &chnAttr);

    if ((sensor->width != stream->width) ||
        (sensor->height != stream->height))
    {
        scale = 1;
    }
    else
    {
        scale = 0;
    }

    chnAttr.pixFmt = PIX_FMT_NV12;
    chnAttr.outFrmRateNum = stream->fps;
    chnAttr.outFrmRateDen = 1;
    chnAttr.nrVBs = stream->buffers;
    chnAttr.type = FS_PHY_CHANNEL;
    chnAttr.crop.enable = 0;
    chnAttr.crop.top = 0;
    chnAttr.crop.left = 0;
    chnAttr.crop.width = sensor->width;
    chnAttr.crop.height = sensor->height;
    chnAttr.scaler.enable = scale;
    chnAttr.scaler.outwidth = stream->width;
    chnAttr.scaler.outheight = stream->height;
    chnAttr.picWidth = stream->width;
    chnAttr.picHeight = stream->height;

#if !defined(KERNEL_VERSION_4)
#if defined(PLATFORM_T31)

    int rotation = stream->rotation;
    int rot_height = stream->height;
    int rot_width = stream->width;

    // Set rotate before FS creation
    // IMP_Encoder_SetFisheyeEnableStatus(0, 1);
    // IMP_Encoder_SetFisheyeEnableStatus(1, 1);

    // ret = IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width);
    //  LOG_ERROR_OR_DEBUG(ret, "IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width)");

#endif
#endif

    ret = IMP_FrameSource_CreateChn(chnNr, &chnAttr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_CreateChn(" << chnNr << ", &chnAttr)");

    ret = IMP_FrameSource_SetChnAttr(chnNr, &chnAttr);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnAttr(" << chnNr << ", &chnAttr)");

#if !defined(NO_FIFO)
    IMPFSChnFifoAttr fifo;
    ret = IMP_FrameSource_GetChnFifoAttr(chnNr, &fifo);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_GetChnFifoAttr(" << chnNr << ", &fifo)");

    fifo.maxdepth = 0;
    ret = IMP_FrameSource_SetChnFifoAttr(chnNr, &fifo);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetChnFifoAttr(" << chnNr << ", &fifo)");

    ret = IMP_FrameSource_SetFrameDepth(chnNr, 1.5);
    LOG_DEBUG_OR_ERROR(ret, "IMP_FrameSource_SetFrameDepth(" << chnNr << ", 0)");
#endif

    //ret = IMP_FrameSource_EnableChn(chnNr);
    //LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_EnableChn(" << chnNr << ")");

    return ret;
}

int IMPFramesource::enable()
{

    int ret;

    ret = IMP_FrameSource_EnableChn(chnNr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_EnableChn(" << chnNr << ")");

    return 0;
}

int IMPFramesource::disable()
{

    int ret;

    ret = IMP_FrameSource_DisableChn(chnNr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_DisableChn(" << chnNr << ")");

    return 0;
}

int IMPFramesource::destroy()
{

    int ret;

    ret = IMP_FrameSource_DestroyChn(chnNr);
    LOG_DEBUG_OR_ERROR_AND_EXIT(ret, "IMP_FrameSource_DestroyChn(" << chnNr << ")");

    return 0;
}
