/*
 * IMP IVS Move func header file.
 *
 * Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_IVS_MOVE_H__
#define __IMP_IVS_MOVE_H__

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
 * Motion detection algorithm input parameter structure
 */
typedef struct {
	int				sense[IMP_IVS_MOVE_MAX_ROI_CNT]; /**< Sensitivity of motion detection: the range to normal camera is 0-4, while to Panoramic camera is 0-8 */
	int				skipFrameCnt;					/*< motion detected interval frame numbers */
	IMPFrameInfo	frameInfo;						/**< frame information, only need to assign width and height */
	IMPRect			roiRect[IMP_IVS_MOVE_MAX_ROI_CNT];	/*< detect region  of information */
	int				roiRectCnt;						/*< detect region information counts */
} IMP_IVS_MoveParam;

/*
 * Motion detection algorithm output parameter structure
 */
typedef struct {
	int retRoi[IMP_IVS_MOVE_MAX_ROI_CNT];				/*< region checkout result, 0:no motion, 1:move, region count equals to roiRectCnt */
} IMP_IVS_MoveOutput;

/**
 * Create motion detection interface
 *
 * @fn IMPIVSInterface *IMP_IVS_CreateMoveInterface(IMP_IVS_MoveParam *param);
 *
 * @param[in] param motion detection algorithm input parameter structure
 *
 * @retval not NULL success, returns the motion detection interface handler pointer
 * @retval NULL failed
 *
 * @attention null
 */
IMPIVSInterface *IMP_IVS_CreateMoveInterface(IMP_IVS_MoveParam *param);

/**
 * Destroy move detection interface
 *
 * @fn void IMP_IVS_DestroyMoveInterface(IMPIVSInterface *moveInterface);
 *
 * @param[in] moveInterface motion detection interface handler pointer
 *
 * @retval no return
 */
void IMP_IVS_DestroyMoveInterface(IMPIVSInterface *moveInterface);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IMP_IVS_MOVE_H__ */
