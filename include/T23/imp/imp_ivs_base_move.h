/*
 * IMP IVS Move func header file.
 *
 * Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_IVS_BASE_MOVE_H__
#define __IMP_IVS_BASE_MOVE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#define IMP_IVS_MOVE_MAX_ROI_CNT		52

#include <imp/imp_ivs.h>

/**
 * @file
 * IMP IVS Motion detection module
 */
/**
 * @defgroup MoveDetection
 * @ingroup IMP_IVS
 * @brief motion detection interface
 * @{
*/


/**
 * Base Motion detection algorithm input parameter structure
 */
typedef struct {
	int             skipFrameCnt;                      /*< motion detected interval frame numbers */
	int             referenceNum;                      /*specified the referenceNum frame relative to current frame as reference frame*/
	int             sadMode;                           /*<SAD mode, 0 signify 8**/
	int             sense;                             /*Sensitivity of motion detection:range:0-3, The bigger the value, the more sensitive*/
	IMPFrameInfo    frameInfo;                         /**< frame information, only need to assign width and height */

} IMP_IVS_BaseMoveParam;

/*
 * Base Motion detection algorithm output parameter structure
 */
typedef struct {
	int ret;
	uint8_t* data;
	int datalen;
	int64_t timeStamp;
} IMP_IVS_BaseMoveOutput;

/**
 * Create motion detection interface
 *
 * @fn IMPIVSInterface *IMP_IVS_CreateBaseMoveInterface(IMP_IVS_BaseMoveParam *param);
 *
 * @param[in] param motion detection algorithm input parameter structure
 *
 * @retval not NULL success, returns the motion detection interface handler pointer
 * @retval NULL failed
 *
 * @attention null
 */

IMPIVSInterface *IMP_IVS_CreateBaseMoveInterface(IMP_IVS_BaseMoveParam *param);

/**
 * Destroy move detection interface
 *
 * @fn void IMP_IVS_DestroyBaseMoveInterface(IMPIVSInterface *moveInterface);
 *
 * @param[in] moveInterface motion detection interface handler pointer
 *
 * @retval no return
 */
void IMP_IVS_DestroyBaseMoveInterface(IMPIVSInterface *moveInterface);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IMP_IVS_MOVE_H__ */
