/*
 * IMP OSD header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_OSD_H__
#define __IMP_OSD_H__

#include "imp_common.h"
#include "imp_isp.h"

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
 * @brief OSD module, which can overlay pictures, bitmaps, lines, and rectangular boxes on video streams.
 *
 * @section osd_summary 1 Module is introduced.
 * OSD stands for on-screen Display.The function of the module is to superimpose lines, pictures and other information on each source.
 *
 * @section osd_concept 2 Relevant concepts
 * @subsection osd_region 2.1 Region
 *
 * Region is a superimposed Region, which is abbreviated as Rgn in API. Each Region has certain image information and can pass through the OSD module
block superimposed, and the background image into a picture.
 *
 *
 *For image overlay, you can also achieve Alpha effect. See @ref osd_region_type for details on the various overlay types.
 *
 * @subsection osd_region_type 2.1 Region type
 * Region has the following types:\n
 * OSD_REG_LINE：Linear \ n
 * OSD_REG_RECT：Rectangular box \ n
 * OSD_REG_BITMAP：Bitmap \ n
 * OSD_REG_COVER：Keep out \ n
 * OSD_REG_PIC：Picture\n
 *
 * Among them, the difference between bitmaps and images is that bitmaps are only monochrome overlay of pixels, while images are Alpha overlay of RGBA images.
 *
 * @section osd_fun 3 Module function.
 * The OSD module supports line, rectangle frame, bitmap overlay, rectangle occlusion, and image overlay.
 * Lines, rectangular boxes and bitmaps are realized by software. Rectangular occlusion and picture overlay are implemented by hardware.
 *
 * @section osd_use 4
 * The following steps are used to use OSD:
 * 1. Create OSD group
 * 2. Bind an OSD group to the system
 * 3. Creating an OSD Region
 * 4. Register an OSD area with an OSD group
 * 5. Set the area properties and area properties of the OSD group
 * 6. Set the OSD function switch
 * @{
 */

/**
 * Error return value
 */
#define INVHANDLE		(-1)

enum
{
	IMP_OK_OSD_ALL 					= 0x0 , 		/* Normal operation */
	/* OSD */
	IMP_ERR_OSD_CHNID 				= 0x80020001,	/* The channel ID exceeds the legal range */
	IMP_ERR_OSD_PARAM 				= 0x80020002,	/* Parameter out of legal range */
	IMP_ERR_OSD_EXIST 				= 0x80020004,	/* Attempt to apply for or create an existing device, channel or resource */
	IMP_ERR_OSD_UNEXIST 			= 0x80020008,	/* Attempt to use or destroy non-existent equipment, channels or resources */
	IMP_ERR_OSD_NULL_PTR 			= 0x80020010,	/* Null pointer in function parameter */
	IMP_ERR_OSD_NOT_CONFIG 			= 0x80020020,	/* Not configured before use */
	IMP_ERR_OSD_NOT_SUPPORT 		= 0x80020040,	/* Unsupported parameters or functions */
	IMP_ERR_OSD_PERM 				= 0x80020080,	/* operation not permitted */
	IMP_ERR_OSD_NOMEM 				= 0x80020100,	/* Failed to allocate memory */
	IMP_ERR_OSD_NOBUF 				= 0x80020200,	/* Failed to allocate buffer */
	IMP_ERR_OSD_BUF_EMPTY 			= 0x80020400,	/* No data in buffer */
	IMP_ERR_OSD_BUF_FULL 			= 0x80020800,	/* The buffer is full */
	IMP_ERR_OSD_BUF_SIZE 			= 0x80021000,	/* Insufficient buffer space */
	IMP_ERR_OSD_SYS_NOTREADY 		= 0x80022000,	/* The system is not initialized or the corresponding module is not loaded */
	IMP_ERR_OSD_OVERTIME 			= 0x80024000,	/* Wait timeout */
	IMP_ERR_OSD_RESOURCE_REQUEST 	= 0x80028000,	/* Resource request failed */
};

/**
 * OSD Regional handle
 */
typedef int IMPRgnHandle;

/**
 * OSD color type. The color format is BGRA
 */
typedef enum {
	OSD_RED,	/* red */
	OSD_BLACK, 	/* black */
	OSD_GREEN, 	/* green */
	OSD_YELLOW, /* yellow */
}IMPOsdColour;

typedef enum {
     OSD_IPU_BLACK   = 0xff000000, /**< black */
     OSD_IPU_WHITE   = 0xffffffff, /**< white*/
     OSD_IPU_RED     = 0xffff0000, /**< red */
     OSD_IPU_GREEN   = 0xff00ff00, /**< green */
     OSD_IPU_BLUE    = 0xff0000ff, /**< blue */
}IMPIpuColour;

/**
 * OSD Area Type
 */
typedef enum {
	OSD_REG_INV					= 0, /**< undefined */
	OSD_REG_HORIZONTAL_LINE		= 1, /**< Horizontal lineHorizontal line */
	OSD_REG_VERTICAL_LINE		= 2, /**< vertical line */
	OSD_REG_RECT		= 3, /**< rectangular */
	OSD_REG_FOUR_CORNER_RECT  = 4, /**< The corners of the rectangle */
	OSD_REG_BITMAP		= 5, /**< Bitmap images */
	OSD_REG_COVER		= 6, /**< Rectangular block */
	OSD_REG_PIC			= 7, /**< Picture, good for Logo or time stamp */
	OSD_REG_PIC_RMEM	= 8, /**< Pictures, suitable for use as logos or timestamps, using RMEM memory */
	OSD_REG_SLASH       = 9, /*slash*/
	OSD_REG_ISP_PIC 	  = 10, /*ISP drawing picture*/
	OSD_REG_ISP_LINE_RECT = 11, /*ISP draws lines or boxes*/
	OSD_REG_ISP_COVER	  = 12, /*ISP draws rectangle occlusion*/
	OSD_REG_MOSAIC = 13, /* MOSAIC */
} IMPOsdRgnType;

/**
 * OSD area lines and rectangular data
 */
typedef struct {
	uint32_t		color;			/**< Color, support IMPOsdColour enumeration type color*/
	uint32_t		linewidth;		/**< Line width*/
	uint32_t		linelength;		/**< Line length*/
	uint32_t		rectlinelength; /**< The line length of the half border*/
} lineRectData;

/**
 * The OSD area blocks data
 */
typedef struct {
	uint32_t		color;			/**< Color: supports only BGRA color format */
} coverData;


/**
 * OSD area image data
 */
typedef struct {
	void				*pData;			/**< Picture data pointer */
} picData;

/**
 * OSD area attribute data
 */
typedef union {
	void				*bitmapData;		/**< Lattice data */
	lineRectData			lineRectData;		/**< Line and rectangle data */
	coverData			coverData;		/**< Keep out of data */
	picData				picData;		/**< Image data */
} IMPOSDRgnAttrData;

/**
 * OSD font size data
 */
typedef struct {
	unsigned int fontWidth;
	unsigned int fontHeight;
} IMPOSDFontSizeAttrData;

/**
 * OSD font attribute data
 */
typedef struct {
	unsigned int            invertColorSwitch;      /**< Timestamp inversion enable switch */
	unsigned int            luminance;              /**< Brightness reference default 190*/
	unsigned int            length;                 /**< Font length*/
	IMPOSDFontSizeAttrData  data;                   /**< Timestamp character size attribute data */
	unsigned int            istimestamp;            /**< Whether the image data is a timestamp */
	unsigned int            colType[20];            /**< Each character of the main stream timestamp is marked in reverse color */
} IMPOSDFontAttrData;

typedef struct {
	IMPISPDrawBlockAttr stDrawAttr;			/*ISP draws line and box properties */
	IMPISPOSDBlockAttr  stpicAttr;		/*ISP draws picture properties */
	IMPISPMaskBlockAttr stCoverAttr;			/*ISP draws rectangle occlusion */
}IMPOSDIspDraw;

/**
 * OSD Mosaic data
 */
typedef struct mosaicPointAttr{
	int x;					/* Mosaic starting x coordinate (2 aligned) */
	int y;					/* Mosaic starting y coordinate (2 aligned) */
	int mosaic_width;		/* Desired mosaic width (2 aligned and plus x-coordinate not to exceed total channel width) */
	int mosaic_height;		/* Desired mosaic height (2 aligned and plus x-coordinate not to exceed total channel height) */
	int frame_width;		/* Channel Resolution Width */
	int frame_height;		/* Channel Resolution Height */
	int mosaic_min_size;	/* Minimum mosaic unit (2 aligned) */
} IMPOSDMosaicAttr;

/**
 * OSD Area Attributes
 */
typedef struct {
	IMPOsdRgnType		type;			/**< OSD Area Type */
	IMPRect				rect;			/**< Rectangular data */
	IMPLine				line;			/**< Linear data */
	IMPPixelFormat		fmt;			/**< Some format */
	IMPOSDRgnAttrData	data;			/**< OSD area attribute data */
	IMPOSDIspDraw		osdispdraw;		/**< ISP Draws OSD attributes */
	IMPOSDFontAttrData  fontData;       /**< OSD font attribute data */
	IMPOSDMosaicAttr    mosaicAttr;     /**< Mosaic data */
} IMPOSDRgnAttr;

/**
 * Specifies the validity time stamp of the OSD area
 */
typedef struct {
	uint64_t ts;						/**< The time stamp */
	uint64_t minus;						/**< The lower limit */
	uint64_t plus;						/**< ceiling */
} IMPOSDRgnTimestamp;

/**
 * Area attributes of the OSD group
 */
typedef struct {
	int					show;			/**< Whether or not shown */
	IMPPoint			offPos;			/**< Display starting coordinates */
	float				scalex;			/**< Scaling x parameter */
	float				scaley;			/**< Scaling y parameter */
	int					gAlphaEn;		/**< Alpha switch */
	int					fgAlhpa;		/**< prospects Alpha */
	int					bgAlhpa;		/**< background Alpha */
	int					layer;			/**< According to layer */
} IMPOSDGrpRgnAttr;

/*
 * Specifies the status of the OSD region
 * */
typedef struct stRgnCreateStat{
    int status;                         /**< The osd region creation status,
                                        0:not creat; 1:region is create */
}IMPOSDRgnCreateStat;

/*
 * Osd area registration status
 * */
typedef struct stRgnRigsterStat{
    int status;                         /**< The osd region creation status
										0:not creat; 1:region is create */
}IMPOSDRgnRegisterStat;

typedef int (*OSD_Frame_CALLBACK)(int group_index, IMPFrameInfo *frame);

/**
 * @fn int IMP_OSD_SetPoolSize(int size);
 *
 * set OSD rmem pool size
 *
 *
 * @retval  0 success
 * @retval -1 failed
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_SetPoolSize(int size);


/**
 * @fn int IMP_OSD_SetMosaic(unsigned char *frame_virAddr, IMPOSDMosaicAttr *mosaicAttr);
 *
 * OSD Mosaic
 *
 *
 * @retval  0 success
 * @retval -1 failed
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_SetMosaic(unsigned char *frame_virAddr, IMPOSDMosaicAttr *mosaicAttr);

/**
 * @fn int IMP_OSD_GetRegionLuma(IMPRgnHandle handle,IMPOSDRgnAttr *prAttr)
 *
 * OSD time color reverse
 *
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_GetRegionLuma(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_RgnCreate_Query(IMPRgnHandle handle,IMPOSDRgnCreateStat *pstStatus)
 *
 * Example Query the status of an OSD region
 *
 * @param[in] handle number，IMPOSDRgnCreateStat Structure pointer
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_RgnCreate_Query(IMPRgnHandle handle,IMPOSDRgnCreateStat *pstStatus);

/**
 * @fn int IMP_OSD_RgnRegister_Query(IMPRgnHandle handle,int grpNum,IMPOSDRgnRegisterStat *pstStatus)
 *
 * Query the osd region registration status
 *
 * @param[in] handle number，grpNum OSD group number,Value range: [0, @ref NR_MAX_OSD_GROUPS - 1]，IMPOSDRgnRegisterStat Structure pointer
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_RgnRegister_Query(IMPRgnHandle handle,int grpNum,IMPOSDRgnRegisterStat *pstStatus);


/**
 * @fn int IMP_OSD_CreateGroup(int grpNum)
 *
 * Create OSD group
 *
 * @param[in] grpNum OSD group number,Value range: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks none
 *
 * @attention none
 */
int IMP_OSD_CreateGroup(int grpNum);

/**
 * @fn int IMP_OSD_DestroyGroup(int grpNum)
 *
 * Destruction of OSD group
 *
 * @param[in] grpNum OSD group number,Value range: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that the corresponding group has been created.
 *
 * @attention none
 */
int IMP_OSD_DestroyGroup(int grpNum);

/**
 * @fn int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to)
 *
 * Add the OSD group to the system
 *
 * @param[in] from OSD unit
 * @param[in] to Other units in the system
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks In the latest SDK version, it is recommended to Attach the OSD node to the system data flow. However, attaching the OSD node is not recommended is API reserved, convenient compatibility with previous versions of the software.
 * See the @ref bind example for details
 *
 * @attention none
 */
int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to);

/**
 * @fn IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr)
 *
 * Creating an OSD Region
 *
 * @param[in] prAttr OSD Regional attribute
 *
 * @retval Greater than or equal to 0 is successful
 * @retval Less than 0 fails
 *
 * @remarks none
 *
 * @attention none
 */
IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr);

/**
 * @fn void IMP_OSD_DestroyRgn(IMPRgnHandle handle)
 *
 * Destroying an OSD Area
 *
 * @param[in] prAttr Area handle, the return value of IMP_OSD_CreateRgn
 *
 * @retval none
 *
 * @remarks none
 *
 * @attention none
 */
void IMP_OSD_DestroyRgn(IMPRgnHandle handle);

/**
 * @fn int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Registering an OSD Region
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] grpNum OSD group number
 *@param[in] pgrAttr OSD group attributes are displayed
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created.
 *
 * @attention none
 */
int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum)
 *
 * Deregistering an OSD Region
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] grpNum OSD group number
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created and the corresponding region has been registered.
 *
 * @attention none
 */
int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum);

/**
 * @fn int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * Setting region Properties
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] prAttr OSD Area Attributes
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that the corresponding region has been created.
 *
 * @attention none
 */
int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs)
 *
 * Set the region properties and validity time
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] prAttr OSD Regional attribute
 * @param[in] prTs Effect of time
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that the corresponding region has been created.
 *
 * @attention none
 */
int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs);

/**
 * @fn int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * Get region properties
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[out] prAttr OSD Regional attribute
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that the corresponding region has been created.
 *
 * @attention none
 */
int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData)
 *
 *Update region data attributes for OSD_REG_BITMAP and OSD_REG_PIC region types only
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] prAttrData OSD Area data attributes
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To call this API, the corresponding region must have been created and the region property has been set to OSD_REG_BITMAP or OSD_REG_PIC.
 *
 * @attention none
 */
int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData);

/**
 * @fn int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Example Set the area properties of an OSD group
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] grpNum OSD group number.
 * @param[in] pgrAttr Area attributes of the OSD group
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created and the region has been created and registered.
 *
 * @attention none
 */
int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * Obtain the area attributes of the OSD group
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] grpNum OSD group number.
 * @param[out] pgrAttr Area attributes of the OSD group
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created and the region has been created and registered.
 *
 * @attention none
 */
int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag)
 *
 * Sets whether the group area is displayed
 *
 * @param[in] handle Area handle, the return value of IMP_OSD_CreateRgn
 * @param[in] grpNum OSD group number
 * @param[in] showFlag Enable or disable the OSD group area display function
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created and the region has been created and registered.
 *
 * @attention none
 */
int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag);

/**
 * @fn int IMP_OSD_Start(int grpNum)
 *
 * The OSD group is displayed
 *
 * @param[in] grpNum OSD group number.
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created.
 *
 * @attention none
 */
int IMP_OSD_Start(int grpNum);

/**
 * @fn int IMP_OSD_Stop(int grpNum)
 *
 * Disable the OSD group display function
 *
 * @param[in] grpNum OSD group number.
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks To invoke this API, ensure that the OSD group has been created.
 *
 * @attention none
 */
int IMP_OSD_Stop(int grpNum);

/**
 * @fn int IMP_OSD_SetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int bosdshow);
 *
 * Set the interface for an ISP to draw OSD attributes
 *
 * @param[in] sensornum The lens label，prAttr OSD Area Attributes，bosdshow Switch the OSD flag. This flag is 0, indicating that only ISPosd can be drawn.If the value is 1, both ISPosd, IPU, DBox, and OSD can be drawn.
 * This interface supports only the OSD_REG_ISP_PIC and OSD_REG_ISP_LINE_RECT types
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that IMP_ISP_EnableTuning has been invoked
 *
 * @attention none
 */
int IMP_OSD_SetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int bosdshow);

/**
 * @fn int IMP_OSD_GetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int *pbosdshow);
 *
 * Obtain the interface used by the ISP to draw OSD attributes
 *
 * @param[in] sensornum This interface can only get IMPOSDIspDraw attribute. Pbosdshow can get THE OSD flag of ISP switch\
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remarks Calling this API requires that IMP_ISP_EnableTuning has been invoked
 *
 * @attention none。
 */
int IMP_OSD_GetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int *pbosdshow);

/**
 * @fn int IMP_OSD_SetGroupCallback(int grpNum, OSD_Frame_CALLBACK callback)
 *
 * Set OSD CallBack,Can be used to get OSD Frame
 *
 * @param[in] grpNum 	OSDGroup
 * @param[in] callback 	CallBack
 *
 * @retval 0 success
 * @retval non-0 failure
 *
 * @remark none
 *
 * @attention Called after IMP_OSD_CreateGroup and before IMP_FrameSource_EnableChn
 */
int IMP_OSD_SetGroupCallback(int grpNum, OSD_Frame_CALLBACK callback);


/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_OSD_H__ */
