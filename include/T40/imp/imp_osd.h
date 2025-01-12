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
 * IMP OSD模块头文件
 */


/**
 * @defgroup IMP_OSD
 * @ingroup imp
 * @brief OSD模块，可在视频流上叠加图片、位图、直线、矩形框
 *
 * @section osd_summary 1 模块介绍
 * OSD的全称是On-Screen Display。模块的功能就是在每张片源中叠加线条、图片等信息。
 *
 * @section osd_concept 2 相关概念
 * @subsection osd_region 2.1 Region
 * Region即是叠加区域，在API中简称Rgn。每个Region具有一定的图像信息，可以经过OSD模块叠加后，与背景图像合为一张图片。
 * 对于图片的叠加，还可以实现Alpha效果。关于各种叠加类型的详细介绍请参考@ref osd_region_type。
 *
 * @subsection osd_region_type 2.1 Region type
 * Region有几种类型，分别为：\n
 * OSD_REG_LINE：直线\n
 * OSD_REG_RECT：矩形框\n
 * OSD_REG_BITMAP：位图\n
 * OSD_REG_COVER：遮挡\n
 * OSD_REG_PIC：图片\n
 * 其中，位图与图片的区别是，位图仅进行像素的单色覆盖，而图片是RGBA图像的Alpha叠加。
 *
 * @section osd_fun 3 模块功能
 * OSD模块支持线条、矩形框、位图叠加、矩形遮挡和图片叠加。
 * 线条、矩形框和位图采用软件实现；矩形遮挡和图片叠加采用硬件实现。
 *
 * @section osd_use 4 模块使用
 * OSD的使用一般有以下几个步骤
 * 1. 创建OSD组
 * 2. 绑定OSD组到系统中
 * 3. 创建OSD区域
 * 4. 注册OSD区域到OSD组中
 * 5. 设置OSD组区域属性和区域属性
 * 6. 设置OSD功能开关
 * @{
 */

/**
 * 出错返回值
 */
#define INVHANDLE		(-1)

/**
 * OSD 区域句柄
 */
typedef int IMPRgnHandle;

/**
 * OSD颜色类型，颜色格式为bgra
 */
typedef enum {
	OSD_RED,	/* red */
	OSD_BLACK, 	/* black */
	OSD_GREEN, 	/* green */
	OSD_YELLOW, /* yellow */
}IMPOsdColour;

typedef enum {
     OSD_IPU_BLACK   = 0xff000000, /**< 黑色 */
     OSD_IPU_WHITE   = 0xffffffff, /**< 白色 */
     OSD_IPU_RED     = 0xffff0000, /**< 红色 */
     OSD_IPU_GREEN   = 0xff00ff00, /**< 绿色 */
     OSD_IPU_BLUE    = 0xff0000ff, /**< 蓝色 */
}IMPIpuColour;

/**
 * OSD区域类型
 */
typedef enum {
	OSD_REG_INV					= 0, /**< 未定义的 */
	OSD_REG_HORIZONTAL_LINE		= 1, /**< 横线 */
	OSD_REG_VERTICAL_LINE		= 2, /**< 竖线 */
	OSD_REG_RECT		= 3, /**< 矩形 */
	OSD_REG_FOUR_CORNER_RECT  = 4, /**< 四角矩形 */
	OSD_REG_BITMAP		= 5, /**< 点阵图片 */
	OSD_REG_COVER		= 6, /**< 矩形遮挡 */
	OSD_REG_PIC			= 7, /**< 图片，适合用作Logo或时间戳 */
	OSD_REG_PIC_RMEM	= 8, /**< 图片，适合用作Logo或时间戳, 使用RMEM内存 */
	OSD_REG_SLASH       = 9, /*斜线*/
	OSD_REG_ISP_PIC 	  = 10, /*ISP绘制图片*/
	OSD_REG_ISP_LINE_RECT = 11, /*ISP绘制线或框*/
	OSD_REG_ISP_COVER	  = 12, /*ISP绘制矩形遮挡*/
	OSD_REG_MOSAIC	  = 13, /*马赛克*/
} IMPOsdRgnType;

/**
 * OSD区域线和矩形数据
 */
typedef struct {
	uint32_t		color;			/**< 颜色，支持IMPOsdColour枚举类型颜色 */
	uint32_t		linewidth;		/**< 线宽 */
	uint32_t		linelength;		/**< 线长 */
	uint32_t		rectlinelength; /**< 半边框的线长*/
} lineRectData;

/**
 * OSD区域遮挡数据
 */
typedef struct {
	uint32_t		color;			/**< 颜色，只支持bgra颜色格式 */
} coverData;


/**
 * OSD区域图片数据
 */
typedef struct {
	void				*pData;			/**< 图片数据指针 */
} picData;

/**
 * OSD区域属性数据
 */
typedef union {
	void				*bitmapData;		/**< 点阵数据 */
	lineRectData			lineRectData;		/**< 线、矩形数据 */
	coverData			coverData;		/**< 遮挡数据 */
	picData				picData;		/**< 图片数据 */
} IMPOSDRgnAttrData;

/**
 * OSD Font大小数据
*/
typedef struct {
	unsigned int fontWidth;
	unsigned int fontHeight;
} IMPOSDFontSizeAttrData;

/**
   * OSD Font属性数据
 */
typedef struct {
	unsigned int			invertColorSwitch;		/**< 时间戳反色使能开关 */
	unsigned int			luminance;				/**< 亮度参考 默认190*/
	unsigned int			length;					/**< 字体长度*/
	IMPOSDFontSizeAttrData  data;					/**< 时间戳字符尺寸属性数据 */
	unsigned int			istimestamp;			/**< 图片数据是否为时间戳 */
	unsigned int			colType[20];			/**< 主码流时间戳各字符反色标识 */
} IMPOSDFontAttrData;

typedef struct {
	IMPISPDrawAttr stDrawAttr;		/*ISP绘制线、框属性*/
	IMPISPOSDAttr  stpicAttr;		/*ISP绘制图片属性*/
	IMPISPMASKAttr stCoverAttr;		/*ISP绘制矩形遮挡*/
}IMPOSDIspDraw;


/**
 * OSD马赛克属性
 */
typedef struct mosaicPointAttr{
	int x;
	int y;
	int mosaic_width;
	int mosaic_height;
	int frame_width;
	int frame_height;
	int mosaic_min_size;
} IMPOSDMosaicAttr;

/**
 * OSD区域属性
 */
typedef struct {
	IMPOsdRgnType		type;			/**< OSD区域类型 */
	IMPRect				rect;			/**< 矩形数据 */
	IMPLine				line;			/**< 直线数据*/
	IMPPixelFormat		fmt;			/**< 点格式 */
	IMPOSDRgnAttrData	data;			/**< OSD区域属性数据 */
	IMPOSDIspDraw		osdispdraw;		/**< ISP绘制OSD的相关属性*/
	IMPOSDFontAttrData  fontData;		/**< OSD Font属性数据 */
	IMPOSDMosaicAttr	mosaicAttr;		/**< 马赛克数据 */
} IMPOSDRgnAttr;

/**
 * OSD区域生效时间戳
 */
typedef struct {
	uint64_t ts;						/**< 时间戳 */
	uint64_t minus;						/**< 下限 */
	uint64_t plus;						/**< 上限 */
} IMPOSDRgnTimestamp;

/**
 * OSD组区域属性
 */
typedef struct {
	int					show;			/**< 是否显示 */
	IMPPoint			offPos;			/**< 显示起始坐标 */
	float				scalex;			/**< 缩放x参数 */
	float				scaley;			/**< 缩放y参数 */
	int					gAlphaEn;		/**< Alpha开关 */
	int					fgAlhpa;		/**< 前景Alpha */
	int					bgAlhpa;		/**< 背景Alpha */
	int					layer;			/**< 显示层 */
} IMPOSDGrpRgnAttr;

/*
 * osd 区域创建的状态
 * */
typedef struct stRgnCreateStat{
    int status;                         /**< osd区域创建的状态：0、未创建，1、区域已创建 */
}IMPOSDRgnCreateStat;

/*
 * osd 区域注册的状态
 * */
typedef struct stRgnRigsterStat{
	int status;                         /**< osd组区域注册的状态：0、未注册，1、区域已注册 */
}IMPOSDRgnRegisterStat;


/**
 * @fn int IMP_OSD_SetMosaic(unsigned char *frame_virAddr, IMPOSDMosaicAttr *mosaicAttr);
 *
 * OSD 马赛克
 *
 *
 * @retval  0 成功
 * @retval -1 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_SetMosaic(unsigned char *frame_virAddr, IMPOSDMosaicAttr *mosaicAttr);

/**
 * @fn int IMP_OSD_GetRegionLuma(IMPRgnHandle handle,IMPOSDRgnAttr *prAttr)
 *
 * OSD 时间戳反色
 *
 *
 * @retval  0 成功
 * @retval -1 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_GetRegionLuma(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_RgnCreate_Query(IMPRgnHandle handle,IMPOSDRgnCreateStat *pstStatus)
 *
 * 查询osd区域创建的状态
 *
 * @param[in] handle号，IMPOSDRgnCreateStat 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_RgnCreate_Query(IMPRgnHandle handle,IMPOSDRgnCreateStat *pstStatus);

/**
 * @fn int IMP_OSD_RgnRegister_Query(IMPRgnHandle handle,int grpNum,IMPOSDRgnRegisterStat *pstStatus)
 *
 * 查询osd区域注册状态
 *
 * @param[in] handle 号，grpNum OSD组号,取值范围: [0, @ref NR_MAX_OSD_GROUPS - 1]，IMPOSDRgnRegisterStat 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_RgnRegister_Query(IMPRgnHandle handle,int grpNum,IMPOSDRgnRegisterStat *pstStatus);


/**
 * @fn int IMP_OSD_CreateGroup(int grpNum)
 *
 * 创建OSD组
 *
 * @param[in] grpNum OSD组号,取值范围: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_OSD_CreateGroup(int grpNum);

/**
 * @fn int IMP_OSD_DestroyGroup(int grpNum)
 *
 * 销毁OSD组
 *
 * @param[in] grpNum OSD组号,取值范围: [0, @ref NR_MAX_OSD_GROUPS - 1]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的组已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_DestroyGroup(int grpNum);

/**
 * @fn int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to)
 *
 * 将OSD组加入到系统中
 *
 * @param[in] from OSD单元
 * @param[in] to 系统中其他单元
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 新版本的SDK中建议使用Bind的方式将OSD串到系统的数据流中，Attach的方式不再推荐使用，但是API保留，方便兼容以往版本的软件。
 * 具体可以参考@ref bind 的的示例
 *
 * @attention 无。
 */
int IMP_OSD_AttachToGroup(IMPCell *from, IMPCell *to);

/**
 * @fn IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr)
 *
 * 创建OSD区域
 *
 * @param[in] prAttr OSD区域属性
 *
 * @retval 大于等于0 成功
 * @retval 小于0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
IMPRgnHandle IMP_OSD_CreateRgn(IMPOSDRgnAttr *prAttr);

/**
 * @fn void IMP_OSD_DestroyRgn(IMPRgnHandle handle)
 *
 * 销毁OSD区域
 *
 * @param[in] prAttr 区域句柄，IMP_OSD_CreateRgn的返回值
 *
 * @retval 无
 *
 * @remarks 无。
 *
 * @attention 无。
 */
void IMP_OSD_DestroyRgn(IMPRgnHandle handle);

/**
 * @fn int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * 注册OSD区域
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] grpNum OSD组号
 * @param[in] pgrAttr OSD组显示属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_RegisterRgn(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum)
 *
 * 注销OSD区域
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] grpNum OSD组号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建,对应的区域已经注册。
 *
 * @attention 无。
 */
int IMP_OSD_UnRegisterRgn(IMPRgnHandle handle, int grpNum);

/**
 * @fn int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * 设置区域属性
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] prAttr OSD区域属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的区域已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_SetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs)
 *
 * 设置区域属性和生效时间
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] prAttr OSD区域属性
 * @param[in] prTs 生效时间
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的区域已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_SetRgnAttrWithTimestamp(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr, IMPOSDRgnTimestamp *prTs);

/**
 * @fn int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr)
 *
 * 获取区域属性
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[out] prAttr OSD区域属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的区域已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_GetRgnAttr(IMPRgnHandle handle, IMPOSDRgnAttr *prAttr);

/**
 * @fn int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData)
 *
 * 更新区域数据属性，只针对OSD_REG_BITMAP和OSD_REG_PIC的区域类型
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] prAttrData OSD区域数据属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的区域已经创建且区域属性已经设置成OSD_REG_BITMAP或OSD_REG_PIC。
 *
 * @attention 无。
 */
int IMP_OSD_UpdateRgnAttrData(IMPRgnHandle handle, IMPOSDRgnAttrData *prAttrData);

/**
 * @fn int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * 设置OSD组区域属性
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] grpNum OSD组号
 * @param[in] pgrAttr OSD组区域属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建,区域已经创建并注册。
 *
 * @attention 无。
 */
int IMP_OSD_SetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr)
 *
 * 获取OSD组区域属性
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] grpNum OSD组号
 * @param[out] pgrAttr OSD组区域属性
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建,区域已经创建并注册。
 *
 * @attention 无。
 */
int IMP_OSD_GetGrpRgnAttr(IMPRgnHandle handle, int grpNum, IMPOSDGrpRgnAttr *pgrAttr);

/**
 * @fn int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag)
 *
 * 设置组区域是否显示
 *
 * @param[in] handle 区域句柄，IMP_OSD_CreateRgn的返回值
 * @param[in] grpNum OSD组号
 * @param[in] showFlag OSD组区域显示开关
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建,区域已经创建并注册。
 *
 * @attention 无。
 */
int IMP_OSD_ShowRgn(IMPRgnHandle handle, int grpNum, int showFlag);

/**
 * @fn int IMP_OSD_Start(int grpNum)
 *
 * 设置开始OSD组的显示
 *
 * @param[in] grpNum OSD组号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_Start(int grpNum);

/**
 * @fn int IMP_OSD_Stop(int grpNum)
 *
 * 设置停止OSD组的显示
 *
 * @param[in] grpNum OSD组号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求对应的OSD组已经创建。
 *
 * @attention 无。
 */
int IMP_OSD_Stop(int grpNum);

/**
 * @fn int IMP_OSD_SetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int bosdshow);
 *
 * 设置ISP绘制OSD的属性接口
 *
 * @param[in] sensornum 镜头标号，prAttr OSD区域属性，bosdshow 开关osd标记，该标志为0，表示只可绘制ISPosd，为1，表示既可绘制ISPosd又可绘制ipu/dbox/软件OSD
 * 该接口只支持OSD_REG_ISP_PIC和OSD_REG_ISP_LINE_RECT类型
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求，IMP_ISP_EnableTuning已被调用
 *
 * @attention 无。
 */
int IMP_OSD_SetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int bosdshow);

/**
 * @fn int IMP_OSD_GetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int *pbosdshow);
 *
 * 获取ISP绘制OSD的属性接口
 *
 * @param[in] sensornum 镜头标号，prAttr，OSD区域属性，该接口只支持获取IMPOSDIspDraw的属性 ，pbosdshow 获取ISP开关osd的标记\
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 调用此API时要求，IMP_ISP_EnableTuning已被调用
 *
 * @attention 无。
 */
int IMP_OSD_GetRgnAttr_ISP(int sensornum,IMPOSDRgnAttr *prAttr,int *pbosdshow);


/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_OSD_H__ */
