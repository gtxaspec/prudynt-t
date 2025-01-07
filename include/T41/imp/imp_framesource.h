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
 *
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
* Define return value
*/
enum
{
	IMP_OK_FS_ALL 					= 0x0 , 		/* Normal operation */
	/* FrameSource */
	IMP_ERR_FS_CHNID 				= 0x80010001,	/* The channel ID exceeds the legal range */
	IMP_ERR_FS_PARAM 				= 0x80010002,	/* Parameter out of legal range */
	IMP_ERR_FS_EXIST 				= 0x80010004,	/* Attempt to apply for or create an existing device, channel or resource */
	IMP_ERR_FS_UNEXIST 				= 0x80010008,	/* Attempt to use or destroy non-existent equipment, channels or resources */
	IMP_ERR_FS_NULL_PTR 			= 0x80010010,	/* Null pointer in function parameter */
	IMP_ERR_FS_NOT_CONFIG 			= 0x80010020,	/* Not configured before use */
	IMP_ERR_FS_NOT_SUPPORT 			= 0x80010040,	/* Unsupported parameters or functions */
	IMP_ERR_FS_PERM 				= 0x80010080,	/* operation not permitted */
	IMP_ERR_FS_NOMEM 				= 0x80010100,	/* Failed to allocate memory */
	IMP_ERR_FS_NOBUF 				= 0x80010200,	/* Failed to allocate buffer */
	IMP_ERR_FS_BUF_EMPTY 			= 0x80010400,	/* No data in buffer */
	IMP_ERR_FS_BUF_FULL 			= 0x80010800,	/* The buffer is full */
	IMP_ERR_FS_SYS_NOTREADY			= 0x80011000,	/* The system is not initialized or the corresponding module is not loaded */
	IMP_ERR_FS_OVERTIME				= 0x80012000,	/* Wait timeout */
	IMP_ERR_FS_RESOURCE_REQUEST		= 0x80014000,	/* Resource request failed */
};

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
	FS_INJ_CHANNEL,			/**< external inject channel */
} IMPFSChnType;

/**
* Channel FIFO Type
*/
typedef enum {
	FIFO_CACHE_PRIORITY = 0,	/**< FIFO caches first，then output data */
	FIFO_DATA_PRIORITY,			/**< FIFO output data first, then caches */
} IMPFSChnFifoType;

/**
* The structure of FIFO Channel's attributes
*/
typedef struct {
	int maxdepth;				/**< FIFO maximum depth */
	IMPFSChnFifoType type;		/**< Channel FIFO Type */
} IMPFSChnFifoAttr;

/**
  * I2DAttr
 */
 typedef struct i2dattr{
     int i2d_enable;            /**< output image rotate enable */
     int flip_enable;           /**< output image flip enable */
     int mirr_enable;           /**< output image mirror enable */
     int rotate_enable;         /**< output image mirror enable */
     int rotate_angle;          /**< output image rotate */
 }IMPFSI2DAttr;


/**
 * The structure of frame channel's attributes
 */
typedef struct {
    IMPFSI2DAttr i2dattr;       /**< I2d Attr */
	int picWidth;				/**< output image width */
	int picHeight;				/**< output image height */
	IMPPixelFormat pixFmt;		/**< output image format */
	IMPFSChnCrop crop;			/**< the attribute of image cropping */
	IMPFSChnScaler scaler;			/**< the attribute of image scaling */
	int outFrmRateNum;			/**< the molecular of output fps */
	int outFrmRateDen;			/**< the denominator of output fps */
	int nrVBs;				/**< the number of Video buffers */
	IMPFSChnType type;			/**< the type of the frame channel */
	IMPFSChnCrop fcrop;			/**< picture fcrop Attr*/
	int mirr_enable;			/**< picture mirror enable*/
} IMPFSChnAttr;

typedef enum {
	FRAME_ALIGN_8BYTE,
	FRAME_ALIGN_16BYTE,
	FRAME_ALIGN_32BYTE,
}FSChannelYuvAlign;

struct yuvaliparm{
	FSChannelYuvAlign w;
	FSChannelYuvAlign h;
};

typedef struct {
	int32_t enable;
	struct yuvaliparm param;
}IMPFrameAlign;

/**
* @fn int IMP_FrameSource_GetI2dAttr(int chnNum,IMPFSI2DAttr *pI2dAttr)
*
* get I2d attr
*
* @param[in] chnNum
* @param[in] pI2dAttr I2d struct
*
* @retval 0
* @retval Other values means failure, its value is an error code.
*
* @remark
*
* @attention
*/
int IMP_FrameSource_GetI2dAttr(int chnNum,IMPFSI2DAttr *pI2dAttr);

/**
* @fn int IMP_FrameSource_SetI2dAttr(int chnNum,IMPFSI2DAttr *pI2dAttr)
*
* set I2d attr
*
* @param[in] chnNum
* @param[in] pI2dAttr channnel I2D struct
*
* @retval 0
* @retval Other values means failure, its value is an error code.
*
* @remark set I2d attr ,can use flip ,mirror,rotate
*
* @attention
*/
int IMP_FrameSource_SetI2dAttr(int chnNum,IMPFSI2DAttr *pI2dAttr);

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
 * @param[in] chnAttr Pointer of the channel's attribute.
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
 * @param[out] depth  the maximum space(depth) for the images of the Frame channel.
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
 * @attention The IMP_FrameSource_CreateChn function is used after IMP_FrameSource_CreateChn.
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
 * @retval 0     success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none.
 *
 * @attention Use it after calling IMP_FrameSource_CreateChn.
 */
int IMP_FrameSource_GetChnFifoAttr(int chnNum, IMPFSChnFifoAttr *attr);

/**
 * @fn IMP_FrameSource_GetFrameEx(int chnNum,IMPFrameInfo **frame);
 *
 * Get channel frame
 *
 * @param[in] chnNum 	  Channel ID
 * @param[out] frame	  frame info
 *
 * @retval 0     success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none.
 *
 * @attention Use it after calling IMP_FrameSource_CreateChn.
 */
int IMP_FrameSource_GetFrameEx(int chnNum,IMPFrameInfo **frame);

/**
 * @fn IMP_FrameSource_ReleaseFrameEx(int chnNum,IMPFrameInfo *pframe);
 *
 * release frame info
 *
 * @param[in] chnNum 	  Channel ID
 * @param[out] pframe	  frame info
 *
 * @retval 0     success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none.
 *
 * @attention Use it after calling IMP_FrameSource_GetFrameEx.
 */
int IMP_FrameSource_ReleaseFrameEx(int chnNum,IMPFrameInfo *pframe);

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
 * @fn int IMP_FrameSource_SetYuvAlign(int chnNum,IMPFrameAlign *param);
 *
 * set yuv align param
 *
 * @param[in] chnNum 	  Channel ID
 * @param[out] param	  frame align
 *
 * @retval 0     success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none.
 *
 * @attention Use it before IMP_FrameSource_EnableChn.
 */
int IMP_FrameSource_SetYuvAlign(int chnNum,IMPFrameAlign *param);


/**
 * @fn int IMP_FrameSource_ExternInject_CreateChn(int chnNum, IMPFSChnAttr *chnAttr)
 *
 * Create channel importing external video
 *
 * @param[in] chnNum Channel ID
 * @param[in] chnAttr frame info
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @remark Create an analog channel to import external NV12 data, provide a data source for the backend module. \n
 * Can set the channel correlation property, including: image width and height images, image format, the output of the channel frame rate, cache buf number, do not support the tailoring and scaling properties.\n
 *
 * @attention none.
 */
int IMP_FrameSource_ExternInject_CreateChn(int chnNum, IMPFSChnAttr *chnAttr);

/**
 * @fn IMP_FrameSource_ExternInject_DestroyChn(int chnNum)
 *
 * Destroy channel importing external video
 *
 * @param[in] chnNum ID
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @remark destroy channel
 *
 * @attention it be called when channel is disabled by use IMP_FrameSource_ExternInject_DisableChn.
 */
int IMP_FrameSource_ExternInject_DestroyChn(int chnNum);

/**
 * @fn int IMP_FrameSource_ExternInject_EnableChn(int chnNum)
 *
 * Enable channel
 *
 * @param[in] chnNum channel ID
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none
 *
 * @attention Use the api after the channel have been created .
 */
int IMP_FrameSource_ExternInject_EnableChn(int chnNum);

/**
 * @fn int IMP_FrameSource_ExternInject_DisableChn(int chnNum)
 *
 * Disable channel
 *
 * @param[in] chnNum channel ID
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none
 *
 * @attention none
 */
int IMP_FrameSource_ExternInject_DisableChn(int chnNum);

/**
 * @fn int IMP_FrameSource_DequeueBuffer(int chnNum, IMPFrameInfo **frame)
 *
 * get free buffer frame
 *
 * @param[in] chnNum channel ID
 *
 * @param[out] frame frame info
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @attention none
 */
int IMP_FrameSource_DequeueBuffer(int chnNum, IMPFrameInfo **frame);

/**
 * @fn int IMP_FrameSource_QueueBuffer(int chnNum, const IMPFrameInfo *frame)
 *
 * put use frame buffer to FramSource
 *
 * @param[in] chnNum channel ID
 *
 * @param[in] frame frame info
 *
 * @retval 0 success
 * @retval non-0 failure，its value is an error code
 *
 * @remark none
 *
 * @attention DequeueBuffer and QueueBuffer come in pairs
 */
int IMP_FrameSource_QueueBuffer(int chnNum, const IMPFrameInfo *frame);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_FRAMESOURCE_H__ */
