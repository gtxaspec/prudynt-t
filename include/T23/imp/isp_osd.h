/*
 * Ingenic IMP ISPOSD solution.
 *
 * Copyright (C) 2021 Ingenic Semiconductor Co.,Ltd
 * Author: Jim <jim.wang@ingenic.com>
 */

#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <imp/imp_log.h>
#include <imp/imp_osd.h>
//#include <constraints.h>
#include <imp/imp_utils.h>

#ifndef __ISP_OSD_H__
#define __ISP_OSD_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */


/**
 * @fn int IMP_OSD_Init_ISP(void);
 *
 * ISPOSD资源初始化，集成在IMP_system_init接口中，用户无需再调用
 *
 * @param[in]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_Init_ISP(void);

/**
 * @fn int IMP_OSD_SetPoolSize_ISP(int size)
 *
 * 创建ISPOSD使用的rmem内存大小
 *
 * @param[in]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_SetPoolSize_ISP(int size);

/**
 * @fn int IMP_OSD_CreateRgn_ISP(int chn,IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 创建ISPOSD区域
 *
 * @param[in] chn通道号，IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_CreateRgn_ISP(int chn,IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_OSD_SetRgnAttr_PicISP(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 设置ISPOSD 通道区域的属性
 *
 * @param[in] chn通道号，handle号 IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_SetRgnAttr_PicISP(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_OSD_GetRgnAttr_ISPPic(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 获取ISPOSD 通道号中的区域属性
 *
 * @param[in] chn 通道号，handle号，IMPOSDRgnCreateStat 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */

int IMP_OSD_GetRgnAttr_ISPPic(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_OSD_ShowRgn_ISP( int chn,int handle, int showFlag)
 *
 * 设置ISPOSD通道号中的handle对应的显示状态
 *
 * @param[in] chn通道号，handle号，showFlag显示状态(0:关闭显示，1:开启显示)
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_ShowRgn_ISP( int chn,int handle, int showFlag);

/**
 * @fn int IMP_OSD_DestroyRgn_ISP(int chn,int handle)
 *
 * 销毁通道中对应的handle节点
 *
 * @param[in] chn通道号，handle号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_DestroyRgn_ISP(int chn,int handle);

/**
 * @fn void IMP_OSD_Exit_ISP(void)
 *
 * 销毁ISPOSD相关资源，集成在IMP_System_Exit接口中，用户无需再调用
 *
 * @param[in]
 *
 * @retval 无
 *
 * @remarks 无。
 *
 * @attention 无。
 */
void IMP_OSD_Exit_ISP(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

