/*
 * IMP FrameSource header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_FRAMESOURCE_H__
#define __IMP_FRAMESOURCE_H__

#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * The header file of FrameSource
 */

/**
 * @defgroup IMP_FrameSource
 * @ingroup imp
 * @brief Video Source is the image source of IMP system, it can set some functions such as setting the image resolution, cropping, scaling and other properties as well as the back-end noise reduction function.
 *
 * FrameSource is a data flow related to the concept, you can set the image resolution, format, etc., and provide the original image to the back-end.
 *
 * The scheme of FrameSource is as follow：
 * @image html framesource.png
 * According to this image above, FrameSource has 3 outputs, all of them can be used for encoding
 * *Channel 0 : High clear video stream
 * *Channel 1 : Standard clear video stream, or IVS can only do the algorithm of data source
 * *Channel 2 : Expands the channel, is is used in special applications, it is not recommended to use it.
 * Note: The video that frame channel-1 output maybe used to encode and analyse. \n
 * The example of FrameSource's operating instruction is below,
 * @code
 * IMPFSChnAttr fs_chn_attr;
 * fs_chn_attr.pixFmt = PIX_FMT_NV12;
 * fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
 * fs_chn_attr.outFrmRateDen = 1;
 * fs_chn_attr.nrVBs = 3;
 * fs_chn_attr.type = FS_PHY_CHANNEL;
 *
 * fs_chn_attr.crop.enable = 1;
 * fs_chn_attr.crop.top = 0;
 * fs_chn_attr.crop.left = 0;
 * fs_chn_attr.crop.width = 960;
 * fs_chn_attr.crop.height = 640;
 *
 * fs_chn_attr.scaler.enable = 1;
 * fs_chn_attr.scaler.outwidth = 320;
 * fs_chn_attr.scaler.outheight = 240;
 *
 * ret = IMP_FrameSource_CreateChn(0, &fs_chn_attr); //step.1 Create Channel0
 * if (ret < 0) {
 *     printf("FrameSource_CreateChn(0) error\n");
 *     goto createchn_err;
 * }
 *
 * ret = IMP_FrameSource_EnableChn(0); //step.2 Enable Channel 0, Channel 0 starts to output images.
 * if (ret < 0) {
 *     printf("EnableChn(0) error\n";
 *     return enablechn_err;
 * }
 *
 *  / / so far, FrameSource began to pass data to the back-end Group
 *
 *
 * ret = IMP_FrameSource_DisableChn(0); //step.3 Disable channel 0, Channel 0 stops to output images
 * if (ret < 0) {
 *     printf("FrameSource_DisableChn(0) error\n");
 *     return disablechn_err;
 * }
 *
 * ret = IMP_FrameSource_DestroyChn(0); //step.4 destroy channel 0
 * if (ret < 0) {
 *     printf("FrameSource_DestroyChn error\n");
 *     return destorychn_err;
 * }
 * @endcode
 * There are more examples in the content of Samples.
 * @{
 */

/**
* The structure of cropping operation
*/
typedef struct {
	int enable;		/**< Set the parameter to enable/disable cropping */
	int left;		/**< the X offset of starting pixel */
	int top;		/**< the Y offset of starting pixel */
	int width;		/**< the cropped horizontal window size (width) */
	int height;		/**< the cropped vertical window size (height) */
} IMPFSChnCrop;

/**
* The structure of scaling operation
*/
typedef struct {
	int enable;		/**< Set to the parameter to enable/disable scaling */
	int outwidth;		/**< the horizontal window size after scaling */
	int outheight;		/**< the vertical window size after scaling */
} IMPFSChnScaler;

typedef enum {
	FS_PHY_CHANNEL,			/**< physics frame channel */
	FS_EXT_CHANNEL,			/**< virtual frame channel */
} IMPFSChnType;

/**
* Channel FIFO Type
*/
typedef enum {
	FIFO_CACHE_PRIORITY = 0,	/**< FIFO caches first，then output data */
	FIFO_DATA_PRIORITY,			/**< FIFO output data first, then caches */
} IMPFSChnFifoType;

typedef enum {
	IMP_FSCHANNEL_STATE_CLOSE, /**< fs channel not created or destroied*/
	IMP_FSCHANNEL_STATE_OPEN,  /**< fs channel create but not enable*/
	IMP_FSCHANNEL_STATE_RUN,   /**<fs channel created and enabled*/
} IMPFSChannelState;

/**
* The structure of FIFO Channel's attributes
*/
typedef struct {
	int maxdepth;				/**< FIFO maximum depth */
	IMPFSChnFifoType type;		/**< Channel FIFO Type */
} IMPFSChnFifoAttr;

/**
 * The structure of frame channel's attributes
 */
typedef struct {
	int picWidth;				/**< output image width */
	int picHeight;				/**< output image height */
	IMPPixelFormat pixFmt;			/**< output image format */
	IMPFSChnCrop crop;			/**< the attribute of image cropping */
	IMPFSChnScaler scaler;			/**< the attribute of image scaling */
	int outFrmRateNum;			/**< the molecular of output fps */
	int outFrmRateDen;			/**< the denominator of output fps */
	int nrVBs;				/**< the number of Video buffers */
	IMPFSChnType type;			/**< the type of the frame channel */
	IMPFSChnCrop fcrop;			/**< the attribute of the frame channel */
} IMPFSChnAttr;

/**
 * @fn int IMP_FrameSource_CreateChn(int chnNum, IMPFSChnAttr *chnAttr)
 *
 * Create a frame channel
 *
 * @param[in] chnNum Id of frame channel
 * @param[in] chnAttr  the pointer of the frame channel's attribute
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark Create a channel to provide a data source to the back-end module; \n
 * you can set the channel related attributes, including: the width of the picture, the height of the picture, the picture format, channel output frame rate, number of cache buf, cropping and scaling properties.
 *  On the chip of T10, frame channel 0 and 1 are physics channel, channel 2 and 3 are vritual.
 * @attention none.
 */

int IMP_FrameSource_CreateChn(int chnNum, IMPFSChnAttr *chn_attr);

/**
 * @fn IMP_FrameSource_DestroyChn(int chnNum)
 *
 * Destroy the selected frame channel.
 *
 * @param[in] chnNum Id of frame channel
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark the function will free some resources of the frame channel.
 *
 * @attention If the channel has been enabled (IMP_FrameSource_EnableChn), please disable it (IMP_FrameSource_DisableChn) before calling the current function.
 */
int IMP_FrameSource_DestroyChn(int chnNum);

/**
 * @fn int IMP_FrameSource_EnableChn(int chnNum)
 *
 * Enable the selected frame channel.
 *
 * @param[in] chnNum Id of frame channel
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none
 *
 * @attention Before calling the function, the frame channel has to been created.
 */
int IMP_FrameSource_EnableChn(int chnNum);

/**
 * @fn int IMP_FrameSource_DisableChn(int chnNum)
 *
 * Disable the selected frame channel.
 *
 * @param[in] chnNum Id of frame channel
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none
 *
 * @attention none
 */
int IMP_FrameSource_DisableChn(int chnNum);

/**
 * @fn int IMP_FrameSource_SetSource(int extchnNum, int sourcechnNum)
 *
 * Set sourcechnNum of extchnNum
 *
 * @param[in] extchnNum extchnNum
 *
 * @param[in] sourcechnNum sourcechnNum
 *
 * @retval 0 means success
 * @retval Other values means failure, its value is an error code
 *
 * @remark NONE
 *
 * @attention After IMP_FrameSource_CreateChn and Before IMP_FrameSource_EnableChn to use
 */
int IMP_FrameSource_SetSource(int extchnNum, int sourcechnNum);

/**
 * @fn int IMP_FrameSource_GetChnAttr(int chnNum, IMPFSChnAttr *chnAttr)
 *
 * Obtains the frame channel attribute.
 *
 * @param[in] chnNum Id of frame channel
 *
 * @param[out] chnAttr Pointer of the channel's attribute.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark The attribute contains elements such as: the width ,height and format of the image; the output video FPS, the number of buffers, cropping and scaling properties.
 *
 * @attention none
 */
int IMP_FrameSource_GetChnAttr(int chnNum, IMPFSChnAttr *chnAttr);

/**
 * @fn int IMP_FrameSource_SetChnAttr(int chnNum, const IMPFSChnAttr *chnAttr)
 *
 * Sets the frame channel attribute.
 *
 * @param[in] chnNum Id of frame channel
 *
 * @param[out] chnAttr Pointer of the channel's attribute.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark The attribute contains elements such as: the width ,height and format of the image; the output video FPS, the number of buffers, cropping and scaling properties.
 *
 * @attention none
 */
int IMP_FrameSource_SetChnAttr(int chnNum, const IMPFSChnAttr *chnAttr);

/**
 * @fn IMP_FrameSource_SetFrameDepth(int chnNum, int depth)
 *
 * Sets the maximum space(depth) for the images
 *
 * @param[in] chnNum  Id of frame channel
 * @param[in] depth  the maximum space(depth) for the images of the Frame channel.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark
 *
 * 1.This interface is used to set the video frames of a channel cache. When the user sets caches for multi frame video image, he can access a certain number of consecutive image datas.
 *
 * 2.If the specified depth is 0, that does not require the system to cache the image of the channel, so users can not get the channel image data. The system default is not the channel cache image, so, depth is 0.
 *
 * 3.System will automatically update the most old image data, to ensure that once the user begins to get new image data, you can get the latest image.
 *
 * 4.If the system can not get the image then it automatically stops the cache of the new image, so the user can not get a new image. Therefore it is recommended that the user makes sure to access and release the interface for use.
 *
 * 5.System will automatically update the user that it has not yet acquired the old image data, to ensure that the cache image queue is for the most recent image. If the user can not guarantee acquisition speed, the whole process might end up receiving non-consecutive images.
 *
 * 6 this function can call the location, there is no requirement, but it can be done only once.
 *
 * @attention none.
 */
int IMP_FrameSource_SetFrameDepth(int chnNum, int depth);

/**
 * @fn IMP_FrameSource_GetFrameDepth(int chnNum, int *depth);
 *
 * Obtains the depth of Frame FIFO.
 *
 * @param[in] chnNum  Id of frame channel
 * @param[out] depth  Pointer of the depth value.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention none.
 */
int IMP_FrameSource_GetFrameDepth(int chnNum, int *depth);

/**
 * @fn IMP_FrameSource_GetFrame(int chnNum, IMPFrameInfo **frame);
 *
 * Obtained image.
 *
 * @param[in] chnNum  Id of frame channel
 * @param[out] frame  Pointer of the frame information.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark
 *
 * 1.This interface can obtain the video image information of the specified channel. The image information mainly includes: the width of the image, the height, the pixel format and the image data starting address.
 *
 * 2.This interface will be valid only after the channel has been enabled.
 *
 * 3.It supports multiple access after release, but it is recommended to access and release the right interface for use.
 *
 * 4.The default timeout for this interface is 2S, that means after 2S without receiving any images the will be a timeout.
 *
 * @attention none.
 */
int IMP_FrameSource_GetFrame(int chnNum, IMPFrameInfo **frame);

/**
 * @fn IMP_FrameSource_GetTimedFrame(int chnNum, IMPFrameTimestamp *framets, int block, void *framedata, IMPFrameInfo *frame);
 *
 * Obtained image with specified time.
 *
 * @param[in] chnNum  Id of frame channel
 * @param[in] framets Time info
 * @param[in] block block info
 * @param[in] framedata Mem porinter used to store image
 * @param[in] frame Image info
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark
 *
 * 1.This interface can obtain the video image with certain time information of the specified channel. The image information mainly includes: the width of the image, the height, the pixel format and the image data starting address.
 *
 * 2.This interface will be valid only after the channel has been enabled.
 *
 * 3.Before this interface, IMP_FrameSource_SetMaxDelay and IMP_FrameSource_SetDelay should be used。
 *
 * @attention none.
 */
int IMP_FrameSource_GetTimedFrame(int chnNum, IMPFrameTimestamp *framets, int block, void *framedata, IMPFrameInfo *frame);

/**
 * @fn IMP_FrameSource_ReleaseFrame(int chnNum, IMPFrameInfo *frame);
 *
 * Release the frame
 *
 * @param[in] chnNum  ID of frame channel
 * @param[in] frame   Pointer of frame information.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention none.
 */
int IMP_FrameSource_ReleaseFrame(int chnNum, IMPFrameInfo *frame);

/**
 * @fn IMP_FrameSource_SnapFrame(int chnNum, IMPPixelFormat fmt, int width, int height, void *framedata, IMPFrameInfo *frame);
 *
 * Get frames
 *
 * @param[in] chnNum  ID of frame channel
 * @param[in] fmt     format of image
 * @param[in] width   width of image
 * @param[in] height  height of image
 * @param[in] framedata memory of image, provided by user
 * @param[in] frame     pointer of frame information.
 *
 * @retval 0          means success
 * @retval others     means failure, its value is an error code
 *
 * @remark
 *
 * 1.This interface can snap a video image, format now only support NV12 and YUYV422, resolution support the same size of framesource resolution.
 *
 * 2.This interface does not need to be used with IMP_FrameSource_SetFrameDepth.
 *
 * 3.This interface will be valid only after the channel has been enabled.
 *
 *
 * @attention none.
 */
int IMP_FrameSource_SnapFrame(int chnNum, IMPPixelFormat fmt, int width, int height, void *framedata, IMPFrameInfo *frame);

/**
 * @fn IMP_FrameSource_SetMaxDelay(int chnNum, int maxcnt);
 *
 * Set max frame delay
 *
 * @param[in] chnNum ID of frame channel
 * @param[in] maxcnt max frame delay
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention If need, this function should be called between IMP_FrameSource_CreateChn and IMP_FrameSource_EnableChn.
 */
int IMP_FrameSource_SetMaxDelay(int chnNum, int maxcnt);

/**
 * @fn IMP_FrameSource_GetMaxDelay(int chnNum, int *maxcnt);
 *
 * Get max frame delay
 *
 * @param[in] chnNum ID of frame channel
 * @param[out] maxcnt max frame delay
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention Before calling the function, the frame channel has to been created.
 */
int IMP_FrameSource_GetMaxDelay(int chnNum, int *maxcnt);

/**
 * @fn IMP_FrameSource_SetDelay(int chnNum, int cnt);
 *
 * Set frame delay
 *
 * @param[in] chnNum ID of frame channel
 * @param[in] cnt frame delay
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention If need, this function should be called after IMP_FrameSource_SetMaxDelay.
 */
int IMP_FrameSource_SetDelay(int chnNum, int cnt);

/**
 * @fn IMP_FrameSource_GetDelay(int chnNum, int *cnt);
 *
 * Get frame delay
 *
 * @param[in] chnNum ID of frame channel
 * @param[out] cnt frame delay
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @remark none.
 *
 * @attention Before calling the function, the frame channel has to been created.
 */
int IMP_FrameSource_GetDelay(int chnNum, int *cnt);

/**
 * @fn IMP_FrameSource_SetChnFifoAttr(int chnNum, IMPFSChnFifoAttr *attr);
 *
 * set channel's largest cache FIFO attribute
 *
 * @param[in] chnNum  	  ID of the frame channel
 * @param[in] attr	  	  FIFO attribute，includes FIFO maximum depth，unit(frame)；FIFO type.
 *
 * @retval 0              means success
 * @retval other values   means failure，its value is an error code
 *
 * @remark none.
 *
 * @attention use it in between of these two interfaces IMP_FrameSource_CreateChn, IMP_FrameSource_EnableChn.
 */
int IMP_FrameSource_SetChnFifoAttr(int chnNum, IMPFSChnFifoAttr *attr);

/**
 * @fn IMP_FrameSource_GetChnFifoAttr(int chnNum, IMPFSChnFifoAttr *attr);
 *
 * Get channel maximum caches FIFO attribute
 *
 * @param[in] chnNum 	  Channel ID
 * @param[out] attr	      FIFO attribute.
 *
 * @retval 0 			  means success
 * @retval other values   means failure，its value is an error code
 *
 * @remark none.
 *
 * @attention Use it after calling IMP_FrameSource_CreateChn.
 */
int IMP_FrameSource_GetChnFifoAttr(int chnNum, IMPFSChnFifoAttr *attr);

/**
 * @brief IMP_FrameSource_SetPool(int chnNum, int poolID);
 *
 * bind channel to mempool, let chnNum malloc from pool.
 *
 * @param[in] chnNum		Channnel ID.
 * @param[in] poolID		Pool ID.
 *
 * @retval 0				means success.
 * @retval other values		mean failure.
 *
 * @remarks In order to solve the fragmentation of rmem, the channel FrameSource is bound to
 * the corresponding MemPool. The FrameSource applies for MEM in the MemPool. If it is not
 * called, the FrameSource will apply in rmem. At this time, there is the possibility of
 * fragmentation for rmem
 *
 * @attention: chnNum is greater than or equal to 0 and less than 32.
 */
int IMP_FrameSource_SetPool(int chnNum, int poolID);

/**
 * @brief IMP_FrameSource_GetPool(int chnNum);
 *
 * Get Pool ID by chnannel ID.
 *
 * @param[in] chnNum		Channel ID.
 *
 * @retval  >=0 && < 32     means success.
 * @retval other values		mean failure.
 *
 * @remarks obtains poolid through channelid, which cannot be used by customers temporarily
 *
 * @attention	none.
 */
int IMP_FrameSource_GetPool(int chnNum);

/**
 * @brief IMP_FrameSource_ChnStatQuery(int chnNum, FSChannelState *pstate);
 *
 * Get framesource by channel ID；0：close，1：open，2：run.
 *
 * @param[in] chnNum       Channel ID.
 *
 * @retval  >=0            means success.
 * @retval  <0			   mean failure.
 *
 * @remarks
 *
 * @attention none.
 */
int IMP_FrameSource_ChnStatQuery(int chnNum, IMPFSChannelState *pstate);

/**
 * @fn int IMP_FrameSource_SetChnRotate(int chnNum, uint8_t rotTo90, int width, int height);
 *
 * Enable FS channel rotation and configure rotation attributes
 *
 * @param[in] encChn 	Encoder channel number, value range: [0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] rotTo90 	0: disable, 1: rotate 90 ° counterclockwise, 2: rotate 90 ° clockwise
 * @param[in] width  	width Image width before rotation
 * @param[in] height 	height Image height before rotation
 *
 * @retval  >=0            means success.
 * @retval  <0			   mean failure.
 *
 * @remarks This API needs to be called before channel creation
 * @remarks To use this interface, the resolution must be 64 bit aligned
 * @remarks After using the modified interface, the width and height of the code should be set to the rotated width and height during the initialization of the coding channel
 * @remarks The FS channel rotation function cannot be used together with the encoding soft zoom function
 * @remarks Use this function to request an rmem memory to save the rotated frame data
 * @remarks The image rotation function is implemented by software, which will occupy computing resources. It is recommended that the resolution be set below 1280 * 704, and the frame rate should not exceed 15 frames
 */
int IMP_FrameSource_SetChnRotate(int chnNum, uint8_t rotTo90, int width, int height);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_FRAMESOURCE_H__ */
