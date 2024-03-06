/*
 * IMP Encoder emulator header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <yakun.li@ingenic.com>
 */

#ifndef __IMP_EMU_FRAMESOURCE_H__
#define __IMP_EMU_FRAMESOURCE_H__

/**
 * @file
 * Encoder emulator func interface.
 */

#include <stdint.h>
#include <linux/videodev2.h>
#include <imp/imp_common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
int IMP_EmuFrameSource_GetChnAttr(uint32_t chn_num, IMPFSChnAttr *chn_attr);
int IMP_EmuFrameSource_EnableChn(uint32_t chn_num);
int IMP_EmuFrameSource_DisableChn(uint32_t chn_num);
int IMP_EmuFrameSource_CreateChn(uint32_t chn_num, IMPFSChnAttr *chn_attr);
int IMP_EmuFrameSource_DestroyChn(uint32_t chn_num);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_EMU_FRAMESOURCE_H__ */
