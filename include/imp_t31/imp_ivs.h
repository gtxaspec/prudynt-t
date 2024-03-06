/*
 * IMP IVS header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_IVS_H__
#define __IMP_IVS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include <imp/imp_common.h>

/**
 * @file
 * IMP IVS module
 */

#define IMP_IVS_DEFAULT_TIMEOUTMS		(-1)
/**
 * @defgroup IMP_IVS
 * @ingroup imp
 * @brief IVS intelligent analysis common API
 *
 * @section concept 1 Related concepts
 * IMP IVS's main goal is to provide users with a number of embedded intelligent analysis algorithm to SDK interface.
 *
 * @subsection IMPIVSInterface 1.1 IMPIVSInterface
 * IMPIVSInterface is the common algorithm interface, the Specific algorithm through the implementation of this interface and pass it to the IVS IMP to achieve the purpose of running the specific algorithm in SDK.
 *
 * For a channel to be an algorithm that can run the carrier, the Channel has to be into the SDK. To do so, we need to transmit the specific implementation of the general algorithm to the specific interface of this Channel.
 *
 * IMPIVSInterface member parameter is the init member parameter, it is a must to include int (*free_data)(void *data) member, and assigned to IMP_IVS_ReleaseDa in order to avoid a lock (out).
 *
 *
 * @section ivs_usage 2 Using method
 * Motion detection, as an example, reference: sample-move_c.c document.

 * Step.1 Init system,
	call sample_system_init() realized in examples. \n
 * All applications should be initialized only one time.

 * step.2 Init framesource,
	if it is already created, you can just use it( continue with the process)
	if it is not created, call sample_framesource_init(IVS_FS_CHN, &fs_chn_attr)

 * step.3 Create IVS group \n
 * Multiple algorithm can be used to share a channel group, also can partly use the channel group     	reference:  sample_ivs_move_init() function.
 * @code
 * int sample_ivs_move_init(int grp_num)
 * {
 *  	int ret = 0;
 *
 *		ret = IMP_IVS_CreateGroup(grp_num);
 *		if (ret < 0) {
 *			IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
 *			return -1;
 *		}
 *		return 0;
 * }
 * @endcode

 * step.4 Bind IVS group to Framesource group
 * @code
 *	IMPCell framesource_cell = {DEV_ID_FS, IVS_FS_CHN, 0};
 *	IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};
 *	ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
 *	if (ret < 0) {
 *		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", IVS_FS_CHN);
 *		return -1;
 *	}
 * @endcode

 * step.5 Enable Framesource and channel algorithm.
     Advice : ivs index is equal to ivs channel num in order to make it more convenient to use them.
 * @code
 *	IMP_FrameSource_SetFrameDepth(0, 0);
 *	ret = sample_framesource_streamon(IVS_FS_CHN);
 *	if (ret < 0) {
 *		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
 *		return -1;
 *	}
 *	ret = sample_ivs_move_start(0, 0, &inteface);
 *	if (ret < 0) {
 *		IMP_LOG_ERR(TAG, "sample_ivs_move_start(0, 0) failed\n");
 *		return -1;
 *	}
 * @endcode
 *
 * step.6 Get algorithm result\n
 * Obtain the results and release the results must be strictly corresponding, can not be interrupted in the middle.
 * @code
 *	for (i = 0; i < NR_FRAMES_TO_IVS; i++) {
 *		ret = IMP_IVS_PollingResult(0, IMP_IVS_DEFAULT_TIMEOUTMS);
 *		if (ret < 0) {
 *			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
 *			return -1;
 *		}
 *		ret = IMP_IVS_GetResult(0, (void **)&result);
 *		if (ret < 0) {
 *			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
 *			return -1;
 *		}
 *		IMP_LOG_INFO(TAG, "frame[%d], result->ret=%d\n", i, result->ret);
 *
 *		ret = IMP_IVS_ReleaseResult(0, (void *)result);
 *		if (ret < 0) {
 *			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
 *			return -1;
 *		}
 *	}
 * @endcode
 *
 * step.7	Release resources.
 * @code
 * sample_ivs_move_stop(0, inteface);
 *	sample_framesource_streamoff(1);
 *	IMP_System_UnBind(&framesource_cell, &ivs_cell);
 *	sample_ivs_move_exit(0);
 *	sample_framesource_exit(IVS_FS_CHN);
 *	sample_system_exit();
 * @endcode
 * @{
 */

/**
 * IVS common interface
 */

typedef struct IMPIVSInterface IMPIVSInterface;

struct IMPIVSInterface {
	void  *param;												/**< algorithm input parameter */
	int   paramSize;											/**< parameter size */
	IMPPixelFormat pixfmt;										/**< input pixel format */
	int  (*init)(IMPIVSInterface *inf);							/**< init algorithm func */
	void (*exit)(IMPIVSInterface *inf);							/**< exit algorithm func */
	int  (*preProcessSync)(IMPIVSInterface *inf, IMPFrameInfo *frame);/**< algorithm preprocess func, it's parameter frame hasn't been special locked by SDK IVS module, so no need to unlock by using free date, return >=0-> ok，-1->error */
	int  (*processAsync)(IMPIVSInterface *inf, IMPFrameInfo *frame);/**< algorithm process func, should be sure unused frames must be unlocked by using free data asap, must be realized for it's a key func to generate algorithm result, return 0->nomal，1->skip check，-1->error */
	int  (*getResult)(IMPIVSInterface *inf, void **result);		/**< get algorithm result */
	int  (*releaseResult)(IMPIVSInterface *inf, void *result);	/**< release algorithm result */
	int	 (*getParam)(IMPIVSInterface *inf, void *param);		/**< get algorithm parameter */
	int	 (*setParam)(IMPIVSInterface *inf, void *param);		/**< set algorithm parameter */
	int	 (*flushFrame)(IMPIVSInterface *inf);					/**< Released all frame which got and cached by processAsync */
	void *priv;													/**< private info */
};

/**
 * Create IVS group
 *
 * @fn int IMP_IVS_CreateGroup(int GrpNum);
 *
 * @param[in] GrpNum IVS group number
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_CreateGroup(int GrpNum);

/**
 * Destroy IVS group
 *
 * @fn int IMP_IVS_DestroyGroup(int GrpNum);
 *
 * @param[in] GrpNum IVS group number
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_DestroyGroup(int GrpNum);
/**
 * Create IVS channel to one algorithm entity.
 *
 * @fn int IMP_IVS_CreateChn(int ChnNum, IMPIVSInterface *handler);
 *
 * @param[in] ChnNum 	IVS channel number, advice being equal to algorithm index
 *
 * @param[in] handler 	IVS algorithm handler, an entity of one algorithm interface.
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_CreateChn(int ChnNum, IMPIVSInterface *handler);

/**
 * Destroy IVS channel.
 *
 * @fn int IMP_IVS_DestroyChn(int ChnNum);
 *
 * @param[in] ChnNum ivs channel number
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_DestroyChn(int ChnNum);

/**
 * Register one ivs channel to an ivs group
 *
 * @fn int IMP_IVS_RegisterChn(int GrpNum, int ChnNum);
 *
 * @param[in] GrpNum IVS group number
 *
 * @param[in] ChnNum IVS channel number
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks ChnNum is the registered Channel to the Group(GrpNum)
 */
int IMP_IVS_RegisterChn(int GrpNum, int ChnNum);

/**
 * Unregister one ivs channel from its registered ivs group
 *
 * @fn int IMP_IVS_UnRegisterChn(int ChnNum);
 *
 * @param[in] ChnNum IVS channel number
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_UnRegisterChn(int ChnNum);

/**
 * Channel starts receiving picture(s)
 *
 * @fn int IMP_IVS_StartRecvPic(int ChnNum);
 *
 * @param[in] ChnNum ivs channel number
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks ChnNum is the Channel which starts receiving pictures
 */
int IMP_IVS_StartRecvPic(int ChnNum);

/**
 * Channel stops receiving picture(s)
 *
 * @fn int IMP_IVS_StopRecvPic(int ChnNum);
 *
 * @param[in] ChnNum ivs channel number
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks ChnNum is the Channel which stops receiving pictures
 */
int IMP_IVS_StopRecvPic(int ChnNum);

/**
 * Whether the blocking judge can or can not get the result of IVS function
 *
 * @fn int IMP_IVS_PollingResult(int ChnNum, int timeoutMs);
 *
 * @param[in] ChnNum 	ivs channel number
 * @param[in] timeout 	max wait time，unit:ms;IMP_IVS_DEFAULT_TIMEOUTMS:internel wait time,0:no wait, >0: customer set wait time
 *
 * @retval 0 success
 * @retval 1 failed
 *
 * @remark Only when the channel is created then the parameter IMPIVSInterface structure (in the ProcessAsync member) returns 0, that can help to say that the return function is working properly, so the PollingResult return function can be a success.
 */
int IMP_IVS_PollingResult(int ChnNum, int timeoutMs);

/**
 * Get IVS algorithm check result
 *
 * @fn int IMP_IVS_GetResult(int ChnNum, void **result);
 *
 * @param[in] ChnNum 	IVS channel number
 * @param[in] result 	the result to one check process of the algorithm registerd to the ivs channel(ChnNum)
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks According to different functions of IVS binding channel, the output might be different (corresponding results).
 */
int IMP_IVS_GetResult(int ChnNum, void **result);

/**
 * Release IVS algorithm check result
 *
 * @fn int IMP_IVS_ReleaseResult(int ChnNum, void *result);
 *
 * @param[in] GrpNum IVS group number
 * @param[in] ChnNum IVS channel number
 *
 * @param[in] result the result to one check process of the algorithm registerd to the ivs channel which num is ChnNum
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks According to different functions of IVS binding channel, it might Release different result resource for its output.
 */
int IMP_IVS_ReleaseResult(int ChnNum, void *result);

/**
 * Release IVS function to calculate the resources result
 *
 * @fn int IMP_IVS_ReleaseData(void *vaddr);
 *
 * @param[in] vaddr the released frame's virtual address
 *
 * @retval 0 success
 * @retval 1 failed
 * @remarks if libimp is used, it is a must to use the free_data of IVS function.
 */
int IMP_IVS_ReleaseData(void *vaddr);

/**
 * Get the algorithm parameter held by the channel indexed by ChnNum
 *
 * @fn int IMP_IVS_GetParam(int chnNum, void *param);
 *
 * @param[in] ChnNum 	IVS channel number
 * @param[in] param 	algorithm parameter (virtual address pointer).
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_GetParam(int chnNum, void *param);

/**
 * Set the algorithm parameter held by the channel indexed by ChnNum
 *
 * @fn int IMP_IVS_SetParam(int chnNum, void *param);
 *
 * @param[in] ChnNum 	IVS channel number
 * @param[in] param 	algorithm parameter pointer.
 *
 * @retval 0 success
 * @retval 1 failed
 */
int IMP_IVS_SetParam(int chnNum, void *param);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_IVS_H__ */
