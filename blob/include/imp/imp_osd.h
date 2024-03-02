/*
 * IMP OSD header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_OSD_H__
#define __IMP_OSD_H__

#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */
/**
 * @file
 * IMP OSD module header file
 */


/**
 * @defgroup IMP_OSD
 * @ingroup imp
 * @brief OSD module，On the video stream we can superimpose pictures, bitmap,line,rectangular box
 *
 * @section osd_summary 1 module introduction
 * The full name of OSD is On-Screen Display. The module function is to superimpose lines, pictures and other information on each source.
 *
 * @section osd_concept 2 The related concepts
 * @subsection osd_region 2.1 Region
 * Region is a superimposed area, in the API Rgn. each Region has a certain image information, after superimposing each OSD module, and the background image into a picture.
 * For the image superimposition, you can also achieve the Alpha effect. For a detailed description of the various types please refer to @ref osd_region_type.
 *
 * @subsection osd_region_type 2.1 Region type
 * There are several types of Region, respectively as:\n
 * OSD_REG_LINE：line\n
 * OSD_REG_RECT：rectangle\n
 * OSD_REG_BITMAP：bitmap\n
 * OSD_REG_COVER：cover\n
 * OSD_REG_PIC：picture\n
 * Among them, the difference between the bitmap and the picture is that the bitmap image is only a single color coverage, while the picture is a RGBA image Alpha superimposition.
 * @section osd_fun 3 Module function
 * OSD module support line, rectangular box, bitmap superimposition, rectangle cover and image superimposition.
 * WE use software to achieve Line, rectangular box and bitmap; We use Hardware to achieve rectangle cover and image superimposition.
 * @section osd_use 4 Module use
 * The use of OSD generally has the following steps
 * 1. create the OSD group
 * 2. bind OSD groups to the system
 * 3. create the OSD region
 * 4. register the OSD region to the OSD group
 * 5. Set the attributes of OSD group region and OSD region.
 * 6. set OSD function switch
 * @{
 */

/**
 * Error return value
 */
#define INVHANDLE		(-1)

/**
 * OSD region handle
 */
typedef int IMPRgnHandle;

/**
 * OSD color type, color format is bgra
 */
typedef enum {
	OSD_BLACK	= 0xff000000, /**< black*/
	OSD_WHITE	= 0xffffffff, /**< white*/
	OSD_RED		= 0xffff0000, /**< red*/
	OSD_GREEN	= 0xff00ff00, /**< green*/
	OSD_BLUE	= 0xff0000ff, /**< blue*/
}IMPOsdColour;

/**
 * OSD region type
 */
typedef enum {
	OSD_REG_INV			= 0, /**< undefined region type */
	OSD_REG_LINE		= 1, /**< line*/
	OSD_REG_RECT		= 2, /**< rectangle*/
	OSD_REG_BITMAP		= 3, /**< dot matrix image*/
	OSD_REG_COVER		= 4, /**< rectangle cover*/
	OSD_REG_PIC			= 5, /**< picture, suitable for Logo or time stamp*/
	OSD_REG_PIC_RMEM	= 6, /**< picture, suitable for Logo or time stamp,use RMEM memory*/
} IMPOsdRgnType;

/**
 * OSD region line and rectangle data
 */
typedef struct {
	uint32_t		color;			/**< color,only supports OSD_WHITE，OSD_BLACK，OSD_RED three formats*/
	uint32_t		linewidth;		/**< line width*/
} lineRectData;

/**
 * OSD region cover data
 */
typedef struct {
	uint32_t		color;			/**< color, only supports bgra color format*/
} coverData;


/**
 * OSD region picture data
 */
typedef struct {
	void				*pData;			/**< picture data pointer*/
} picData;

/**
 * OSD region attribute data
 */
typedef union {
	void				*bitmapData;		/**< bitmapData data*/
	lineRectData			lineRectData;		/**< line, rectangle data*/
	coverData			coverData;		/**< cover data*/
	picData				picData;		/**< picture data */
} IMPOSDRgnAttrData;

/**
 * OSD region attribute data
 */
typedef struct {
	IMPOsdRgnType		type;			/**< OSD region type*/
	IMPRect				rect;			/**< rectangle data*/
	IMPPixelFormat		fmt;			/**< point format*/
	IMPOSDRgnAttrData	data;			/**< OSD region attribute data*/
} IMPOSDRgnAttr;

/**
 * OSD Rgn vaild timestamp
 */
typedef struct {
	uint64_t ts;						/**< timestamp */
	uint64_t minus;						/**< mix to timestamp  */
	uint64_t plus;						/**< max to timestamp */
} IMPOSDRgnTimestamp;

/**
 * OSD group region attribute
 */
typedef struct {
	int				show;			/**< display variable*/
	IMPPoint			offPos;			/**< display start coordinates*/
	float				scalex;			/**< zoom x parameters*/
	float				scaley;			/**< zoom y parameters*/
	int				gAlphaEn;		/**< Alpha switch*/
	int				fgAlhpa;		/**< prospect Alpha*/
	int				bgAlhpa;		/**< background Alpha*/
	int				layer;			/**< display layer*/
} IMPOSDGrpRgnAttr;


/**
 * @fn int IMP_OSD_CreateGroup(int grpNum)
 *
 * create OSD group
 *
 * @param[in] grpNum OSD group number, range: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks no。
 *
 * @attention no.
 */
int IMP_OSD_CreateGroup(int grpNum);

/**
 * @fn int IMP_OSD_DestroyGroup(int grpNum)
 *
 * destory OSD group
 *
 * @param[in] grpNum OSD group number, range: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks When this is called the corresponding group should be created.
 *
 * @attention no.
 */
int IMP_OSD_DestroyGroup(int grpNum);

/**
 * @fn int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to)
 *
 * Adding OSD group to the system.
 *
 * @param[in] from OSD unit
 * @param[in] to Other unit in the system
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The new version of SDK is recommended to use Bind in order to string OSD into the system's data stream, Attach is no longer recommended to use, but API reservations, easy to be compatible with the previous version of the software.
 * reference :  @ref bind
 *
 * @attention no.
 */
int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to);

/**
 * @fn IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr)
 *
 * Create OSD region
 *
 * @param[in] prAttr OSD region attribute
 *
 * @retval greater than or equal to 0 success
 * @retval less than 0 failure
 *
 * @remarks no.
 *
 * @attention no.
 */
IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr);

/**
 * @fn void IMP_OSD_DestroyRgn(IMPRgnHandle handle)
 *
 * Destory OSD region
 *
 * @param[in] prAttr region handle, IMP_OSD_CreateRgn return value
 *
 * @retval no
 *
 * @remarks no.
 *
 * @attention no.
 */
void IMP_OSD_DestroyRgn(IMPRgnHandle handle);

/**
 * @fn int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Register OSD region
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[in] grpNum OSD group number
 * @param[in] pgrAttr OSD group display attribute
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The corresponding OSD group should be already created before calling this function.
 *
 * @attention no.
 */
int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum)
 *
 * Register OSD region
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[in] grpNum OSD group number
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The corresponding OSD group should be already created and registered before calling this function.
 *
 * @attention no.
 */
int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum);

/**
 * @fn int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * Set region properties
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[in] prAttr OSD region attribute
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The corresponding OSD group should be already created before calling this function.
 *
 * @attention no.
 */
int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs)
 *
 * Set region properties and effective time
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[in] prAttr OSD region attribute
 * @param[in] prTs effective time
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The corresponding OSD group should be already created before calling this function.
 *
 * @attention none.
 */
int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs);

/**
 * @fn int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * Get region attribute
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[out] prAttr OSD region attribute
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks The corresponding OSD group should be already created before calling this function.
 *
 * @attention no.
 */
int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData)
 *
 * Update the regional data attribute, only for OSD_REG_BITMAP and OSD_REG_PIC regional types
 *
 * @param[in] handle       region handle, IMP_OSD_CreateRgn return value
 * @param[in] prAttrData   OSD regional data attributes
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created and region attributes already set to OSD_REG_BITMAP or OSD_REG_PIC.
 *
 * @attention no.
 */
int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData);

/**
 * @fn int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Set OSD group region attributes
 *
 * @param[in] handle 	region handle, IMP_OSD_CreateRgn return value
 * @param[in] grpNum 	OSD group number
 * @param[in] pgrAttr 	OSD group region attributes
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created and the region should be created and registered.
 *
 * @attention no.
 */
int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Get OSD group region attributes
 *
 * @param[in] handle region handle, IMP_OSD_CreateRgn return value
 * @param[in] grpNum OSD group number
 * @param[out] pgrAttr OSD group region attributes
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created and the region should be created and registered.
 *
 * @attention no.
 */
int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag)
 *
 * Set whether the group region is displayed or not.
 *
 * @param[in] handle 	region handle, IMP_OSD_CreateRgn return value
 * @param[in] grpNum 	OSD group number
 * @param[in] showFlag 	OSD group region display switch
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created and the region should be created and registered.
 *
 * @attention no.
 */
int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag);

/**
 * @fn int IMP_OSD_Start(int grpNum)
 *
 * Set start OSD group display
 *
 * @param[in] grpNum OSD group number
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created.
 *
 * @attention no.
 */
int IMP_OSD_Start(int grpNum);

/**
 * @fn int IMP_OSD_Stop(int grpNum)
 *
 * Set the display to stop the OSD group
 *
 * @param[in] grpNum OSD group number
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Before calling this function, the corresponding OSD group should be already created.
 *
 * @attention no.
 */
int IMP_OSD_Stop(int grpNum);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_OSD_H__ */
