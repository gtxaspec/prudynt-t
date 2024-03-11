/*
 * IMP ISP header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_ISP_H__
#define __IMP_ISP_H__

#include <stdbool.h>
#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * ISP模块头文件
 */

/**
 * @defgroup IMP_ISP
 * @ingroup imp
 * @brief 图像信号处理单元。主要包含图像效果设置、模式切换以及Sensor的注册添加删除等操作
 *
 * ISP模块与数据流无关，不需要进行Bind，仅作用于效果参数设置及Sensor控制。
 *
 * ISP模块的使能步骤如下：
 * @code
 * int ret = 0;
 * ret = IMP_ISP_Open(); // step.1 创建ISP模块
 * if(ret < 0){
 *     printf("Failed to ISPInit\n");
 *     return -1;
 * }
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * sensor.cbus_type = SENSOR_CONTROL_INTERFACE_I2C; // OR SENSOR_CONTROL_INTERFACE_SPI
 * sensor.i2c = {
 * 	.type = "xxx", // I2C设备名字，必须和sensor驱动中struct i2c_device_id中的name一致。
 *	.addr = xx,	//I2C地址
 *	.i2c_adapter_id = xx, //sensor所在的I2C控制器ID
 * }
 * OR
 * sensor.spi = {
 *	.modalias = "xx", //SPI设备名字，必须和sensor驱动中struct spi_device_id中的name一致。
 *	.bus_num = xx, //SPI总线地址
 * }
 * ret = IMP_ISP_AddSensor(&sensor); //step.2 添加一个sensor，在此操作之前sensor驱动已经添加到内核。
 * if (ret < 0) {
 *     printf("Failed to Register sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableSensor(void); //step.3 使能sensor，现在sensor开始输出图像。
 * if (ret < 0) {
 *     printf("Failed to EnableSensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableTuning(); //step.4 使能ISP tuning, 然后才能调用ISP调试接口。
 * if (ret < 0) {
 *     printf("Failed to EnableTuning\n");
 *     return -1;
 * }
 *
 * 调试接口请参考ISP调试接口文档。 //step.5 效果调试。
 *
 * @endcode
 * ISP模块的卸载步骤如下：
 * @code
 * int ret = 0;
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * ret = IMP_ISP_DisableTuning(); //step.1 关闭ISP tuning
 * if (ret < 0) {
 *     printf("Failed to disable tuning\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DisableSensor(); //step.2 关闭sensor，现在sensor停止输出图像；在此操作前FrameSource必须全部关闭。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DelSensor(&sensor); //step.3 删除sensor，在此操作前sensor必须关闭。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_Close(); //step.4 清理ISP模块，在此操作前所有sensor都必须被删除。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 * @endcode
 * 更多使用方法请参考Samples
 * @{
 */

/**
* 摄像头控制总线类型枚举
*/
typedef enum {
	TX_SENSOR_CONTROL_INTERFACE_I2C = 1,	/**< I2C控制总线 */
	TX_SENSOR_CONTROL_INTERFACE_SPI,	/**< SPI控制总线 */
} IMPSensorControlBusType;

/**
* 摄像头控制总线类型是I2C时，需要配置的参数结构体
*/
typedef struct {
	char type[20];		/**< I2C设备名字，必须与摄像头驱动中struct i2c_device_id中name变量一致 */
	int addr;		/**< I2C地址 */
	int i2c_adapter_id;	/**< I2C控制器 */
} IMPI2CInfo;
/**
* 摄像头控制总线类型是SPI时，需要配置的参数结构体
*/
typedef struct {
	char modalias[32];	/**< SPI设备名字，必须与摄像头驱动中struct spi_device_id中name变量一致 */
	int bus_num;		/**< SPI总线地址 */
} IMPSPIInfo;

/**
* 摄像头注册信息结构体
*/
typedef struct {
	char name[32];					/**< 摄像头名字 */
	uint16_t sensor_id;				    	/**< 摄像头ID  */
	IMPSensorControlBusType cbus_type;	/**< 摄像头控制总线类型 */
	union {
		IMPI2CInfo i2c;				/**< I2C总线信息 */
		IMPSPIInfo spi;				/**< SPI总线信息 */
	};
	unsigned short rst_gpio;		/**< 摄像头reset接口链接的GPIO，注意：现在没有启用该参数 */
	unsigned short pwdn_gpio;		/**< 摄像头power down接口链接的GPIO，注意：现在没有启用该参数 */
	unsigned short power_gpio;		/**< 摄像头power 接口链接的GPIO，注意：现在没有启用该参数 */
} IMPSensorInfo;

/**
 * @fn int IMP_ISP_Open(void)
 *
 * 打开ISP模块
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 创建ISP模块，准备向ISP添加sensor，并开启ISP效果调试功能。
 *
 * @attention 这个函数必须在添加sensor之前被调用。
 */
int IMP_ISP_Open(void);

/**
 * @fn int IMP_ISP_Close(void)
 *
 * 关闭ISP模块
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark ISP模块，ISP模块不再工作。
 *
 * @attention 在使用这个函数之前，必须保证所有FrameSource和效果调试功能已经关闭，所有sensor都已被卸载.
 */
int IMP_ISP_Close(void);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath(char *path)
 *
 * 设置ISP bin文件默认路径
 *
 * @param[in] path  需要设置的bin文件路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置用户自定义ISP启动时Bin文件的绝对路径。
 *
 * @attention 这个函数必须在添加sensor之前、打开ISP之后被调用。
 */
int32_t IMP_ISP_SetDefaultBinPath(char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath(char *path)
 *
 * 获取ISP bin文件默认路径
 *
 * @param[out] path	需要获取的bin文件路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 获取用户自定义ISP启动时Bin文件的绝对路径。
 *
 * @attention 这个函数必须在添加sensor之后被调用。
 * @attention 一次只能获取单个ISP的bin文件路径属性。
 */
int32_t IMP_ISP_GetDefaultBinPath(char *path);

/**
 * @fn int IMP_ISP_AddSensor(IMPSensorInfo *pinfo)
 *
 * 添加一个sensor，用于向ISP模块提供数据源
 *
 * @param[in] pinfo 需要添加sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 添加一个摄像头，用于提供图像。
 *
 * @attention 在使用这个函数之前，必须保证摄像头驱动已经注册进内核.
 */
int IMP_ISP_AddSensor(IMPSensorInfo *pinfo);

/**
 * @fn int IMP_ISP_DelSensor(IMPSensorInfo *pinfo)
 *
 * 删除一个sensor
 *
 * @param[in] pinfo 需要删除sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 删除一个摄像头。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经停止工作，即调用了IMP_ISP_DisableSensor函数.
 */
int IMP_ISP_DelSensor(IMPSensorInfo *pinfo);

/**
 * @fn int IMP_ISP_EnableSensor(void)
 *
 * 使能一个sensor
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 使能一个摄像头，使之开始传输图像, 这样FrameSource才能输出图像，同时ISP才能进行效果调试。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被添加进ISP模块.
 */
int IMP_ISP_EnableSensor(void);

/**
 * @fn int IMP_ISP_DisableSensor(void)
 *
 * 不使能一个sensor
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 不使能一个摄像头，使之停止传输图像, 这样FrameSource无法输出图像，同时ISP也不能进行效果调试。
 *
 * @attention 在使用这个函数之前，必须保证所有FrameSource都已停止输出图像，同时效果调试也在不使能态.
 */
int IMP_ISP_DisableSensor(void);

/**
 * @fn int IMP_ISP_SetSensorRegister(uint32_t reg, uint32_t value)
 *
 * 设置sensor一个寄存器的值
 *
 * @param[in] reg 寄存器地址
 *
 * @param[in] value 寄存器值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接设置一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int IMP_ISP_SetSensorRegister(uint32_t reg, uint32_t value);

/**
 * @fn int IMP_ISP_GetSensorRegister(uint32_t reg, uint32_t *value)
 *
 * 获取sensor一个寄存器的值
 *
 * @param[in] reg 寄存器地址
 *
 * @param[in] value 寄存器值的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接获取一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int IMP_ISP_GetSensorRegister(uint32_t reg, uint32_t *value);

/**
 * ISP功能开关
 */
typedef enum {
	IMPISP_TUNING_OPS_MODE_DISABLE,			/**< 不使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_ENABLE,			/**< 使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsMode;

/**
 * ISP功能选用开关
 */
typedef enum {
	IMPISP_TUNING_OPS_TYPE_AUTO,			/**< 该模块的操作为自动模式 */
	IMPISP_TUNING_OPS_TYPE_MANUAL,			/**< 该模块的操作为手动模式 */
	IMPISP_TUNING_OPS_TYPE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsType;

typedef struct {
	unsigned int zone[15][15];    /**< 各区域信息*/
} __attribute__((packed, aligned(1))) IMPISPZone;

/**
 * @fn int IMP_ISP_EnableTuning(void)
 *
 * 使能ISP效果调试功能
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor被执行且返回成功.
 */
int IMP_ISP_EnableTuning(void);

/**
 * @fn int IMP_ISP_DisableTuning(void)
 *
 * 不使能ISP效果调试功能
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证在不使能sensor之前，先不使能ISP效果调试（即调用此函数）.
 */
int IMP_ISP_DisableTuning(void);

/**
 * @fn int IMP_ISP_Tuning_SetSensorFPS(uint32_t fps_num, uint32_t fps_den)
 *
 * 设置摄像头输出帧率
 *
 * @param[in] fps_num 设定帧率的分子参数
 * @param[in] fps_den 设定帧率的分母参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetSensorFPS(uint32_t fps_num, uint32_t fps_den);

/**
 * @fn int IMP_ISP_Tuning_GetSensorFPS(uint32_t *fps_num, uint32_t *fps_den)
 *
 * 获取摄像头输出帧率
 *
 * @param[in] fps_num 获取帧率分子参数的指针
 * @param[in] fps_den 获取帧率分母参数的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 * @attention 在使能帧通道开始传输数据之前必须先调用此函数获取摄像头默认帧率。
 */
int IMP_ISP_Tuning_GetSensorFPS(uint32_t *fps_num, uint32_t *fps_den);

/**
 * ISP抗闪频属性参数结构体。
 */
typedef enum {
	IMPISP_ANTIFLICKER_DISABLE,	/**< 不使能ISP抗闪频功能 */
	IMPISP_ANTIFLICKER_50HZ,	/**< 使能ISP抗闪频功能, 并设置频率为50HZ */
	IMPISP_ANTIFLICKER_60HZ,	/**< 使能ISP抗闪频功能，并设置频率为60HZ */
	IMPISP_ANTIFLICKER_BUTT,	/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPAntiflickerAttr;

/**
 * @fn int IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISPAntiflickerAttr attr)
 *
 * 设置ISP抗闪频属性
 *
 * @param[in] attr 设置参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISPAntiflickerAttr attr);

/**
 * @fn int IMP_ISP_Tuning_GetAntiFlickerAttr(IMPISPAntiflickerAttr *pattr)
 *
 * 获得ISP抗闪频属性
 *
 * @param[in] pattr 获取参数值指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetAntiFlickerAttr(IMPISPAntiflickerAttr *pattr);

/**
 * @fn int IMP_ISP_Tuning_SetBrightness(unsigned char bright)
 *
 * 设置ISP 综合效果图片亮度
 *
 * @param[in] bright 图片亮度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetBrightness(unsigned char bright);

/**
 * @fn int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright)
 *
 * 获取ISP 综合效果图片亮度
 *
 * @param[in] bright 图片亮度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright);

/**
 * @fn int IMP_ISP_Tuning_SetContrast(unsigned char contrast)
 *
 * 设置ISP 综合效果图片对比度
 *
 * @param[in] contrast 图片对比度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetContrast(unsigned char contrast);

/**
 * @fn int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast)
 *
 * 获取ISP 综合效果图片对比度
 *
 * @param[in] contrast 图片对比度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast);

 /**
 * @fn int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness)
 *
 * 设置ISP 综合效果图片锐度
 *
 * @param[in] sharpness 图片锐度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness);

/**
 * @fn int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness)
 *
 * 获取ISP 综合效果图片锐度
 *
 * @param[in] sharpness 图片锐度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness);

/**
 * @fn int IMP_ISP_Tuning_SetBcshHue(unsigned char hue)
 *
 * 设置图像的色调
 *
 * @param[in] hue 图像的色调参考值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128正向调节色调，小于128反向调节色调，调节范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetBcshHue(unsigned char hue);

/**
 * @fn int IMP_ISP_Tuning_GetBcshHue(unsigned char *hue)
 *
 * 获取图像的色调值。
 *
 * @param[out] hue 图像的色调参数指针。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128代表正向调节色调，小于128代表反向调节色调，范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetBcshHue(unsigned char *hue);

/**
 * @fn int IMP_ISP_Tuning_SetSaturation(unsigned char sat)
 *
 * 设置ISP 综合效果图片饱和度
 *
 * @param[in] sat 图片饱和度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetSaturation(unsigned char sat);

/**
 * @fn int IMP_ISP_Tuning_GetSaturation(unsigned char *psat)
 *
 * 获取ISP 综合效果图片饱和度
 *
 * @param[in] sat 图片饱和度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetSaturation(unsigned char *psat);

/**
 * @fn int IMP_ISP_Tuning_SetISPBypass(IMPISPTuningOpsMode enable)
 *
 * ISP模块是否bypass
 *
 * @param[in] enable 是否bypass输出模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 无
 *
 * @attention 在使用这个函数之前，必须保证ISP模块是关闭的.
 */
int IMP_ISP_Tuning_SetISPBypass(IMPISPTuningOpsMode enable);

/**
 * @fn int IMP_ISP_Tuning_GetTotalGain(uint32_t *gain)
 *
 * 获取ISP输出图像的整体增益值
 *
 * @param[in] gain 获取增益值参数的指针,其数据存放格式为[24.8]，高24bit为整数，低8bit为小数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetTotalGain(uint32_t *gain);

/**
 * 设置ISP图像镜面效果功能是否使能
 *
 * @fn int IMP_ISP_Tuning_SetISPHflip(IMPISPTuningOpsMode mode)
 *
 * @param[in] mode 是否使能镜面效果
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetISPHflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPHflip(IMPISPTuningOpsMode *pmode)
 *
 * 获取ISP图像镜面效果功能的操作状态
 *
 * @param[in] pmode 操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPHflip(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPVflip(IMPISPTuningOpsMode mode)
 *
 * 设置ISP图像上下反转效果功能是否使能
 *
 * @param[in] mode 是否使能图像上下反转
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetISPVflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPVflip(IMPISPTuningOpsMode *pmode)
 *
 * 获取ISP图像上下反转效果功能的操作状态
 *
 * @param[in] pmode 操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPVflip(IMPISPTuningOpsMode *pmode);

/**
 * ISP 工作模式配置，正常模式或夜视模式。
 */
typedef enum {
	IMPISP_RUNNING_MODE_DAY = 0,				/**< 正常模式 */
	IMPISP_RUNNING_MODE_NIGHT = 1,				/**< 夜视模式 */
	IMPISP_RUNNING_MODE_BUTT,					/**< 最大值 */
} IMPISPRunningMode;

/**
 * @fn int IMP_ISP_Tuning_SetISPRunningMode(IMPISPRunningMode mode)
 *
 * 设置ISP工作模式，正常模式或夜视模式；默认为正常模式。
 *
 * @param[in] mode运行模式参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * 示例：
 * @code
 * IMPISPRunningMode mode;
 *
 *	if( it is during a night now){
		mode = IMPISP_RUNNING_MODE_NIGHT
	}else{
		mode = IMPISP_RUNNING_MODE_DAY;
	}
	ret = IMP_ISP_Tuning_SetISPRunningMode(mode);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPRunningMode error !\n");
		return -1;
	}
 *
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetISPRunningMode(IMPISPRunningMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPRunningMode(IMPISPRunningMode *pmode)
 *
 * 获取ISP工作模式，正常模式或夜视模式。
 *
 * @param[in] pmode操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPRunningMode(IMPISPRunningMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPCustomMode(IMPISPTuningOpsMode mode)
 *
 * 使能ISP Custom Mode，加载另外一套效果参数.
 *
 * @param[in] mode Custom 模式，使能或者关闭
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetISPCustomMode(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPCustomMode(IMPISPTuningOpsMode mode)
 *
 * 获取ISP Custom Mode的状态.
 *
 * @param[out] mode Custom 模式，使能或者关闭
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPCustomMode(IMPISPTuningOpsMode *pmode);

/**
 * gamma
 */
typedef struct {
	uint16_t gamma[129];		/**< gamma参数数组，有129个点 */
} IMPISPGamma;

/**
* @fn int IMP_ISP_Tuning_SetGamma(IMPISPGamma *gamma)
*
* 设置GAMMA参数.
* @param[in] gamma gamma参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_SetGamma(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_GetGamma(IMPISPGamma *gamma)
*
* 获取GAMMA参数.
* @param[out] gamma gamma参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetGamma(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_SetAeComp(int comp)
*
* 设置AE补偿。AE补偿参数可以调整图像AE target，范围为[0-255].
* @param[in] comp AE补偿参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_SetAeComp(int comp);

/**
* @fn int IMP_ISP_Tuning_GetAeComp(int *comp)
*
* 获取AE补偿。
* @param[out] comp AE补偿参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetAeComp(int *comp);

/**
* @fn int IMP_ISP_Tuning_GetAeLuma(int *luma)
*
* 获取画面平均亮度。
*
* @param[out] luma AE亮度参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetAeLuma(int *luma);

/**
 * @fn int IMP_ISP_Tuning_SetAeFreeze(IMPISPTuningOpsMode mode)
 *
 * 使能AE Freeze功能.
 *
 * @param[in] mode AE Freeze功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetAeFreeze(IMPISPTuningOpsMode mode);

/**
 * 曝光模式
 */
enum isp_core_expr_mode {
	ISP_CORE_EXPR_MODE_AUTO,			/**< 自动模式 */
	ISP_CORE_EXPR_MODE_MANUAL,			/**< 手动模式 */
};

/**
 * 曝光单位
 */
enum isp_core_expr_unit {
	ISP_CORE_EXPR_UNIT_LINE,			/**< 行 */
	ISP_CORE_EXPR_UNIT_US,				/**< 毫秒 */
};

/**
 * 曝光参数
 */
typedef union isp_core_expr_attr{
	struct {
		enum isp_core_expr_mode mode;		/**< 设置的曝光模式 */
		enum isp_core_expr_unit unit;		/**< 设置的曝光单位 */
		uint16_t time;
	} s_attr;
	struct {
		enum isp_core_expr_mode mode;			/**< 获取的曝光模式 */
		uint16_t integration_time;		/**< 获取的曝光时间，单位为行 */
		uint16_t integration_time_min;	/**< 获取的曝光最小时间，单位为行 */
		uint16_t integration_time_max;	/**< 获取的曝光最大时间，单位为行 */
		uint16_t one_line_expr_in_us;		/**< 获取的一行曝光时间对应的微妙数 */
	} g_attr;
}IMPISPExpr;


/**
 * @fn int IMP_ISP_Tuning_SetExpr(IMPISPExpr *expr)
 *
 * 设置AE参数。
 *
 * @param[in] expr AE参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetExpr(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_GetExpr(IMPISPExpr *expr)
 *
 * 获取AE参数。
 *
 * @param[out] expr AE参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetExpr(IMPISPExpr *expr);

/**
 * 曝光统计区域选择
 */
typedef union isp_core_ae_roi_select{
	struct {
		unsigned endy :8;                   /**< 结束点y坐标 (0 ~ 255)*/
		unsigned endx :8;                   /**< 结束点x坐标 (0 ~ 255)*/
		unsigned starty :8;                 /**< 起始点y坐标 (0 ~ 255)*/
		unsigned startx :8;                 /**< 起始点x坐标 (0 ~ 255)*/
	};
	uint32_t value;
} IMPISPAERoi;

/**
 * 白平衡模式
 */
enum isp_core_wb_mode {
	ISP_CORE_WB_MODE_AUTO = 0,			/**< 自动模式 */
	ISP_CORE_WB_MODE_MANUAL,			/**< 手动模式 */
	ISP_CORE_WB_MODE_DAY_LIGHT,			/**< 晴天 */
	ISP_CORE_WB_MODE_CLOUDY,			/**< 阴天 */
	ISP_CORE_WB_MODE_INCANDESCENT,		/**< 白炽灯 */
	ISP_CORE_WB_MODE_FLOURESCENT,		/**< 荧光灯 */
	ISP_CORE_WB_MODE_TWILIGHT,			/**< 黄昏 */
	ISP_CORE_WB_MODE_SHADE,				/**< 阴影 */
	ISP_CORE_WB_MODE_WARM_FLOURESCENT,	/**< 暖色荧光灯 */
	ISP_CORE_WB_MODE_CUSTOM,	/**< 自定义模式 */
};

/**
 * 白平衡参数
 */
typedef struct isp_core_wb_attr{
	enum isp_core_wb_mode mode;		/**< 白平衡模式，分为自动与手动模式 */
	uint16_t rgain;			/**< 红色增益，手动模式时有效 */
	uint16_t bgain;			/**< 蓝色增益，手动模式时有效 */
}IMPISPWB;

/**
 * @fn int IMP_ISP_Tuning_SetWB(IMPISPWB *wb)
 *
 * 设置白平衡功能设置。可以设置自动与手动模式，手动模式主要通过设置rgain、bgain实现。
 *
 * @param[in] wb 设置的白平衡参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetWB(IMPISPWB *wb);

/**
 * @fn int IMP_ISP_Tuning_GetWB(IMPISPWB *wb)
 *
 * 获取白平衡功能设置。
 *
 * @param[out] wb 获取的白平衡参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_Statis(IMPISPWB *wb)
 *
 * 获取白平衡统计值。
 *
 * @param[out] wb 获取的白平衡统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB_Statis(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_GOL_Statis(IMPISPWB *wb)
 *
 * 获取白平衡全局统计值。
 *
 * @param[out] wb 获取的白平衡全局统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB_GOL_Statis(IMPISPWB *wb);

/**
 * Cluster 模式白平衡参数
 */
typedef struct {
	IMPISPTuningOpsMode ClusterEn;     /* Cluster 模式白平衡使能 */
	IMPISPTuningOpsMode ToleranceEn;   /* AWB 收敛容忍使能 */
	unsigned int tolerance_th;         /* AWB 收敛容忍阈值，取值范围为0~64*/
	unsigned int awb_cluster[7];      /* Cluster 模式白平衡参数 */
}IMPISPAWBCluster;

/**
 * int IMP_ISP_Tuning_SetAwbClust(IMPISPAWBCluster *awb_cluster);
 *
 * 设置CLuster AWB模式的参数。
 *
 * @param[in] CLuster AWB 模式的参数，包括使能、阈值等，awb_cluster[]设置，请咨询Tuning人员。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbClust(IMPISPAWBCluster *awb_cluster);

/**
 * @fn int IMP_ISP_Tuning_GetAwbClust(IMPISPAWBCluster *awb_cluster)
 *
 * 获取CLuster AWB模式下的参数。
 *
 * @param[out] CLuster AWB 模式的参数，包括使能、阈值等。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbClust(IMPISPAWBCluster *awb_cluster);

/*
 *  ISP 白平衡色温倾向参数
 */
typedef struct {
    unsigned int trend_array[6];   /* 高中低色温下的Rgain与Bgain的offset */
}IMPISPAWBCtTrend;

/**
 * int IMP_ISP_Tuning_SetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);
 *
 * 通过rgain与bgain的offset，设置不同色温下的色温偏向。
 *
 * @param[in] ct_trend 包含高中低三个色温下的rgain、bgain offset
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

/**
 * int IMP_ISP_Tuning_GetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);
 *
 * 获取不同色温下的色温偏向，即rgain offset与bgain offset，
 *
 * @param[out] ct_trend 包含高中低三个色温下的rgain、bgain offset
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

/**
 * ISP WB COEFFT parameter structure.
 */
typedef struct isp_core_rgb_coefft_wb_attr {
		unsigned short rgb_coefft_wb_r;
		unsigned short rgb_coefft_wb_g;
		unsigned short rgb_coefft_wb_b;

}IMPISPCOEFFTWB;

/**
 * @fn IMP_ISP_Tuning_Awb_GetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * 获取sensor AWB RGB通道偏移参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_Awb_GetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn IMP_ISP_Tuning_Awb_SetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * 设置sensor可以设置AWB RGB通道偏移参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 *
 * 示例：
 * @code
 * IMPISPCOEFFTWB isp_core_rgb_coefft_wb_attr;
 *
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r=x;
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g=y;
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b=z;
 *IMP_ISP_Tuning_Awb_SetRgbCoefft(&isp_core_rgb_coefft_wb_attr);
 if(ret){
 IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_Awb_SetRgbCoefft error !\n");
 return -1;
 }
*/
int IMP_ISP_Tuning_Awb_SetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn int IMP_ISP_Tuning_SetMaxAgain(uint32_t gain)
 *
 * 设置sensor可以设置最大Again。
 *
 * @param[in] gain sensor可以设置的最大again.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMaxAgain(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain)
 *
 * 获取sensor可以设置最大Again。
 *
 * @param[out] gain sensor可以设置的最大again.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain)
 *
 * 设置ISP可以设置的最大Dgain。
 *
 * @param[in] ISP Dgain 可以设置的最大dgain.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain)
 *
 * 获取ISP设置的最大Dgain。
 *
 * @param[out] ISP Dgain 可以得到设置的最大的dgain.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void))
 *
 * 设置视频丢失功能。当出现sensor与主板的连接线路出现问题时，设置的回调函数会被执行。
 *
 * @param[in] cb 回调函数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void));

/**
 * @fn int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength)
 *
 * 设置强光抑制强度。
 *
 * @param[in] strength 强光抑制强度参数.取值范围为［0-10], 0表示关闭功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength)
 *
 * 获取强光抑制的强度。
 *
 * @param[out] strength 可以得到设置的强光抑制的强度.0表示关闭此功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetBacklightComp(uint32_t strength)
 *
 * 设置背光补偿强度。
 *
 * @param[in] strength 背光补偿强度参数.取值范围为［0-10], 0表示关闭功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetBacklightComp(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetBacklightComp(uint32_t *strength)
 *
 * 获取背光补偿的强度。
 *
 * @param[out] strength 可以得到设置的背光补偿的强度.0表示关闭此功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetBacklightComp(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio)
 *
 * 设置3D降噪强度。
 *
 * @param[in] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]. *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio);

/**
 * @fn int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio)
 *
 * 设置2D降噪强度。
 *
 * @param[in] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255].
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio);

/**
 * ISP EV 参数。
 */
typedef struct {
	uint32_t ev;			/**< 曝光值 */
	uint32_t expr_us;		/**< 曝光时间 */
	uint32_t ev_log2;		/**<log格式曝光时间 */
	uint32_t again;			/**< 模拟增益 */
	uint32_t dgain;			/**< 数字增益 */
	uint32_t gain_log2;		/**< log格式增益 */
}IMPISPEVAttr;

/**
* @fn int IMP_ISP_Tuning_GetEVAttr(IMPISPEVAttr *attr)
*
* 获取EV属性。
* @param[out] attr EV属性参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetEVAttr(IMPISPEVAttr *attr);

/**
* @fn int IMP_ISP_Tuning_EnableMovestate(void)
*
* 当sensor在运动时，设置ISP进入运动态。
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_EnableMovestate(void);

/**
* @fn int IMP_ISP_Tuning_DisableMovestate(void)
*
* 当sensor从运动态恢复为静止态，设置ISP不使能运动态。
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_Tuning_EnableMovestate已被调用。
*/
int IMP_ISP_Tuning_DisableMovestate(void);

/**
* 模式选择选项
*/
typedef enum {
	IMPISP_TUNING_MODE_AUTO,    /**< 该模块的操作为自动模式 */
	IMPISP_TUNING_MODE_MANUAL,    /**< 该模块的操作为手动模式 */
	IMPISP_TUNING_MODE_RANGE,    /**< 该模块的操作为设置范围模式 */
	IMPISP_TUNING_MODE_BUTT,    /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningMode;

/**
* 权重信息
*/
typedef struct {
	unsigned char weight[15][15];    /**< 各区域权重信息 [0 ~ 8]*/
} IMPISPWeight;

/**
 * @fn int IMP_ISP_Tuning_SetAeWeight(IMPISPWeight *ae_weight)
 *
 * 设置AE统计区域的权重。
 *
 * @param[in] ae_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeWeight(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAeWeight(IMPISPWeight *ae_weight)
 *
 * 获取AE统计区域的权重。
 *
 * @param[out] ae_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeWeight(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_GetROI(IMPISPWeight *roi_weight)
 *
 * 获取AE感兴趣区域，用于场景判断。
 *
 * @param[out] roi_weight AE感兴趣区域权重。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_AE_GetROI(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_SetROI(IMPISPWeight *roi_weight)
 *
 * 获取AE感兴趣区域，用于场景判断。
 *
 * @param[in] roi_weight AE感兴趣区域权重。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_AE_SetROI(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_SetAwbWeight(IMPISPWeight *awb_weight)
 *
 * 设置AWB统计区域的权重。
 *
 * @param[in] awb_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbWeight(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbWeight(IMPISPWeight *awb_weight)
 *
 * 获取AWB统计区域的权重。
 *
 * @param[out] awb_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbWeight(IMPISPWeight *awb_weight);

/**
* AWB统计值
*/
typedef struct {
	unsigned char zone_r[225];    /**< 15*15块，RGB三个通道在每个块的亮度统计平均值*/
	unsigned char zone_g[225];    /**< 15*15块，RGB三个通道在每个块的亮度统计平均值*/
	unsigned char zone_b[225];    /**< 15*15块，RGB三个通道在每个块的亮度统计平均值*/
} IMPISPAWBZone;
/**
 * @fn int IMP_ISP_Tuning_GetAwbZone(IMPISPAWBZONE *awb_zone)
 *
 * 获取WB在每个块，不同通道的统计平均值。
 *
 * @param[out] awb_zone 白平衡统计信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbZone(IMPISPAWBZone *awb_zone);

/**
 * AWB algorithm
 */
typedef enum {
	IMPISP_AWB_ALGO_NORMAL = 0, /*常规模式，用有效点来做统计*/
	IMPISP_AWB_ALGO_GRAYWORLD, /*灰色世界模式，所有像素点都用来做统计*/
	IMPISP_AWB_ALGO_REWEIGHT, /*偏向模式，不同色温重新设置权重*/
} IMPISPAWBAlgo;

/**
 * @fn int IMP_ISP_Tuning_SetWB_ALGO(IMPISPAWBALGO wb_algo)
 *
 * 设置AWB统计的模式。
 *
 * @param[in] wb_algo AWB统计的不同模式。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetWB_ALGO(IMPISPAWBAlgo wb_algo);

/**
* AE统计值参数
*/
typedef struct {
	unsigned char ae_histhresh[4];    /**< AE统计直方图bin边界 [0 ~ 255]*/
	unsigned short ae_hist[5];    /**< AE统计直方图bin值 [0 ~ 65535]*/
	unsigned char ae_stat_nodeh;    /**< 水平方向有效统计区域个数 [0 ~ 15]*/
	unsigned char ae_stat_nodev;    /**< 垂直方向有效统计区域个数 [0 ~ 15]*/
} IMPISPAEHist;

/**
 * AE统计值参数
 */
typedef struct {
	unsigned int ae_hist[256];    /**< AE统计直方图256 bin值*/
} IMPISPAEHistOrigin;

/**
 * @fn int IMP_ISP_Tuning_SetAeHist(IMPISPAEHist *ae_hist)
 *
 * 设置AE统计相关参数。
 *
 * @param[in] ae_hist AE统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeHist(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist(IMPISPAEHist *ae_hist)
 *
 * 获取AE统计值。
 *
 * @param[out] ae_hist AE统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeHist(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Origin(IMPISPAEHistOrigin *ae_hist)
 *
 * 获取AE 256 bin统计值。
 *
 * @param[out] ae_hist AE统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeHist_Origin(IMPISPAEHistOrigin *ae_hist);

/**
* AWB统计值
*/
struct isp_core_awb_sta_info{
	unsigned short r_gain;    /**< AWB加权r/g平均值 [0 ~ 4095]*/
	unsigned short b_gain;    /**< AWB加权b/g平均值 [0 ~ 4095]*/
	unsigned int awb_sum;    /**< 用于AWB统计的像素数 [0 ~ 4294967295]*/
};
/**
* AWB统计模式
*/
enum isp_core_awb_stats_mode{
	IMPISP_AWB_STATS_LEGACY_MODE = 0,    /**< 延迟模式 */
	IMPISP_AWB_STATS_CURRENT_MODE = 1,    /**< 当前模式 */
	IMPISP_AWB_STATS_MODE_BUTT,
};
/**
* AWB统计值参数
*/
typedef struct {
	struct isp_core_awb_sta_info awb_stat;    /**< AWB统计值 */
	enum isp_core_awb_stats_mode awb_stats_mode;    /**< AWB统计模式 */
	unsigned short awb_whitelevel;    /**< AWB统计数值上限 [0 ~ 1023]*/
	unsigned short awb_blacklevel;    /**< AWB统计数值下限 [0 ~ 1023]*/
	unsigned short cr_ref_max;    /**< AWB统计白点区域r/g最大值 [0 ~ 4095]*/
	unsigned short cr_ref_min;    /**< AWB统计白点区域r/g最小值 [0 ~ 4095]*/
	unsigned short cb_ref_max;    /**< AWB统计白点区域b/g最大值  [0 ~ 4095]*/
	unsigned short cb_ref_min;    /**< AWB统计白点区域b/g最大值  [0 ~ 4095]*/
	unsigned char awb_stat_nodeh;    /**< 水平方向有效统计区域个数 [0 ~ 15]*/
	unsigned char awb_stat_nodev;    /**< 垂直方向有效统计区域个数 [0 ~ 15]*/
} IMPISPAWBHist;

/**
 * @fn int IMP_ISP_Tuning_GetAwbHist(IMPISPAWBHist *awb_hist)
 *
 * 获取AWB统计值。
 *
 * @param[out] awb_hist AWB统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbHist(IMPISPAWBHist *awb_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAwbHist(IMPISPAWBHist *awb_hist)
 *
 * 设置AWB统计相关参数。
 *
 * @param[in] awb_hist AWB统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbHist(IMPISPAWBHist *awb_hist);

struct isp_core_af_sta_info{
	unsigned int af_metrics;     /**< AF主统计值*/
	unsigned int af_metrics_alt; /**< AF次统计值*/
	unsigned int af_wl;          /*低通滤波器输出的统计值*/
	unsigned int af_wh;          /*高通滤波器输出的统计值*/
};

/**
* 滤波器的使能位
*/
struct isp_af_ldg_en_info{
	unsigned char fir0_en;
	unsigned char fir1_en;
	unsigned char iir0_en;
	unsigned char iir1_en;
};

struct isp_ldg_info{
	unsigned char thlow1;   /*Ldg低亮度起始阈值【取值范围：0~255】*/
	unsigned char thlow2;   /*Ldg低亮度终止阈值【取值范围：0~255】*/
	unsigned short slplow;  /*Ldg低亮度斜率 [取值范围:0~4095]*/
	unsigned char gainlow;  /*Ldg低亮度增益 【取值范围：0~255】*/
	unsigned char thhigh1;  /*Ldg高亮度起始阈值 【取值范围：0~255】*/
	unsigned char thhigh2;  /*Ldg高亮度终止阈值 【取值范围：0~255】*/
	unsigned short slphigh; /*Ldg高亮度斜率 【取值范围：0~4095】*/
	unsigned char gainhigh; /*Ldg高亮度增益 【取值范围：0~255】*/
};

/**
* AF统计值参数
*/
typedef struct {
	struct isp_core_af_sta_info af_stat; /*统计值 (只读)*/
	unsigned char af_enable;             /**< AF功能开关*/
	unsigned char af_metrics_shift;      /**< AF统计值缩小参数 默认是0，1代表缩小2倍*/
	unsigned short af_delta;             /**< AF统计低通滤波器的权重 [0 ~ 64]*/
	unsigned short af_theta;             /**< AF统计高通滤波器的权重 [0 ~ 64]*/
	unsigned short af_hilight_th;        /**< AF高亮点统计阈值 [0 ~ 255]*/
	unsigned short af_alpha_alt;         /**< AF统计低通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
	unsigned short af_belta_alt;         /**< AF统计高通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
	unsigned char  af_hstart;            /**< AF统计值横向起始点：[1-width]，且取奇数*/
	unsigned char  af_vstart;            /**< AF统计值垂直起始点 ：[3-height]，且取奇数*/
	unsigned char  af_stat_nodeh;        /**< 水平方向统计区域个数 [默认值15]，整个画幅的统计窗口H数目 */
	unsigned char  af_stat_nodev;        /**< 垂直方向统计区域个数 [默认值15]，整个画幅的统计窗口V数目 */
	unsigned char  af_frame_num;         /*当前帧数(只读)*/
	struct isp_af_ldg_en_info  ldg_en;   /**< LDG模块使能*/
	struct isp_ldg_info af_fir0;
	struct isp_ldg_info af_fir1;
	struct isp_ldg_info af_iir0;
	struct isp_ldg_info af_iir1;
} IMPISPAFHist;


/**
 * @fn IMP_ISP_Tuning_GetAFMetrices(unsigned int *metric);
 *
 * 获取AF统计值。
 *
 * @param[out] metric AF统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAFMetrices(unsigned int *metric);

/**
 * @fn int IMP_ISP_Tuning_GetAfHist(IMPISPAFHist *af_hist);
 *
 * 获取AF统计值。
 *
 * @param[out] af_hist AF统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfHist(IMPISPAFHist *af_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAfHist(IMPISPAFHist *af_hist)
 *
 * 设置AF统计相关参数。
 *
 * @param[in] af_hist AF统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAfHist(IMPISPAFHist *af_hist);
/**
 * @fn int IMP_ISP_Tuning_SetAfWeight(IMPISPWeight *af_weight)
 *
 * 设置AF统计区域的权重。
 *
 * @param[in] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAfWeight(IMPISPWeight *af_weigh);
/**
 * @fn int IMP_ISP_Tuning_GetAfWeight(IMPISPWeight *af_weight)
 *
 * 获取AF统计区域的权重。
 *
 * @param[out] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfWeight(IMPISPWeight *af_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAfZone(IMPISPZone *af_zone)
 *
 * 获取AF各个zone的统计值。
 *
 * @param[out] af_zone AF各个区域的统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfZone(IMPISPZone *af_zone);

/**
 * ISP Wait Frame 参数。
 */
typedef struct {
	uint32_t timeout;		/**< 超时时间，单位ms */
	uint64_t cnt;			/**< Frame统计 */
}IMPISPWaitFrameAttr;

/**
* @fn int IMP_ISP_Tuning_WaitFrame(IMPISPWaitFrameAttr *attr)
* 等待帧结束
*
* @param[out] attr 等待帧结束属性
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_WaitFrame(IMPISPWaitFrameAttr *attr);

/**
 * AE Min
 */
typedef struct {
	unsigned int min_it;  /**< AE最小曝光 */
	unsigned int min_again;	 /**< AE 最小模拟增益 */
	unsigned int min_it_short; /**< AE短帧的最小曝光 */
	unsigned int min_again_short; /**< AE 短帧的最小模拟增益 */
} IMPISPAEMin;

/**
 * @fn int IMP_ISP_Tuning_SetAeMin(IMPISPAEMin *ae_min)
 *
 * 设置AE最小值参数。
 *
 * @param[in] ae_min AE最小值参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeMin(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_GetAeMin(IMPISPAEMin *ae_min)
 *
 * 获取AE最小值参数。
 *
 * @param[out] ae_min AE最小值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeMin(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max)
 *
 * 设置AE最大值参数。
 *
 * @param[in] it_max AE最大值参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max)
 *
 * 获取AE最大值参数。
 *
 * @param[out] it_max AE最大值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAeZone(IMPISPZone *ae_zone)
 *
 * 获取AE各个zone的Y值。
 *
 * @param[out] ae_zone AE各个区域的Y值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeZone(IMPISPZone *ae_zone);

/*
 *  ISP AE Target Parameters
 */
typedef struct {
	unsigned int at_list[10]; /*ae target list 目标亮度表*/
} IMPISPAETargetList;

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList(IMPISPAETargetList *target_list)
 *
 * 设置AE的目标亮度表
 *
 * @param[in] target_list  目标亮度表
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeTargetList(IMPISPAETargetList *target_list);

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList(IMPISPAETargetList *target_list)
 *
 * 获取AE当前的目标亮度表
 *
 * @param[out] target_list  目标亮度表
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeTargetList(IMPISPAETargetList *target_list);

/**
 * ISP Module Control
 */
typedef union {
	unsigned int key;
	struct {
		unsigned int bitBypassBLC : 1; /* [0]  */
		unsigned int bitBypassGIB : 1; /* [1]  */
		unsigned int bitBypassAG : 1; /* [2]  */
		unsigned int bitBypassDPC : 1; /* [4]  */
		unsigned int bitBypassRDNS : 1; /* [5]	*/
		unsigned int bitBypassLSC : 1; /* [6]  */
		unsigned int bitBypassADR : 1; /* [7]	 */
		unsigned int bitBypassDMSC : 1; /* [8]	 */
		unsigned int bitBypassCCM : 1; /* [9]  */
		unsigned int bitBypassGAMMA : 1; /* [10]  */
		unsigned int bitBypassDEFOG : 1; /* [11]	 */
		unsigned int bitBypassCSC : 1; /* [12]	 */
		unsigned int bitBypassCLM : 1; /* [13]	 */
		unsigned int bitBypassSP : 1; /* [14]  */
		unsigned int bitBypassYDNS : 1; /* [15]	 */
		unsigned int bitBypassBCSH : 1; /* [16]	 */
		unsigned int bitBypassSDNS : 1; /* [17]	 */
		unsigned int bitBypassHLDC : 1; /* [18]	 */
		unsigned int bitRsv : 12; /* [19 ~ 30]	*/
		unsigned int bitBypassMDNS : 1; /* [31]  */
	};
} IMPISPModuleCtl;

/**
 * @fn int IMP_ISP_Tuning_SetModuleControl(IMPISPModuleCtl *ispmodule)
 *
 * 设置ISP各个模块bypass功能
 *
 * @param[in] ispmodule ISP各个模块bypass功能.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetModuleControl(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_GetModuleControl(IMPISPModuleCtl *ispmodule)
 *
 * 获取ISP各个模块bypass功能.
 *
 * @param[out] ispmodule ISP各个模块bypass功能
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetModuleControl(IMPISPModuleCtl *ispmodule);

/**
 * ISP Front Crop Attribution
 */
typedef struct {
	bool fcrop_enable;
	unsigned int fcrop_top;
	unsigned int fcrop_left;
	unsigned int fcrop_width;
	unsigned int fcrop_height;
} IMPISPFrontCrop;

/**
 * @fn int IMP_ISP_Tuning_SetFrontCrop(IMPISPFrontCrop *ispfrontcrop)
 *
 * 设置ISP前Crop的位置
 *
 * @param[in] ispfrontcrop 前Crop参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetFrontCrop(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_GetFrontCrop(IMPISPFrontCrop *ispfrontcrop)
 *
 * 获取前Crop参数.
 *
 * @param[out] ispfrontcrop 前Crop参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetFrontCrop(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_SetDPC_Strength(unsigned int strength)
 *
 * 设置DPC强度.
 *
 * @param[in] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetDPC_Strength(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *strength)
 *
 * 获取DPC强度.
 *
 * @param[out] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio)
 *
 * 设置DRC强度值.
 *
 * @param[in] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio)
 *
 * 获取DRC强度值.
 *
 * @param[out] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio);

/**
 * HV Flip 模式
 */
typedef enum {
	IMPISP_FLIP_NORMAL_MODE = 0,	/**< 正常模式 */
	IMPISP_FLIP_H_MODE = 1,	   /**< 镜像模式 */
	IMPISP_FLIP_V_MODE = 2,		/**< 翻转模式 */
	IMPISP_FLIP_HV_MODE = 3,	/**< 镜像并翻转模式 */
	IMPISP_FLIP_MODE_BUTT,
} IMPISPHVFLIP;

/**
 * @fn int IMP_ISP_Tuning_SetHVFLIP(IMPISPHVFLIP hvflip)
 *
 * 设置HV Flip的模式.
 *
 * @param[in] hvflip HV Flip模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetHVFLIP(IMPISPHVFLIP hvflip);

/**
 * @fn int IMP_ISP_Tuning_GetHVFlip(IMPISPHVFLIP *hvflip)
 *
 * 获取HV Flip的模式.
 *
 * @param[out] hvflip HV Flip模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetHVFlip(IMPISPHVFLIP *hvflip);

/**
 * 填充数据类型
 */
typedef enum {
	IMPISP_MASK_TYPE_RGB = 0, /**< RGB */
	IMPISP_MASK_TYPE_YUV = 1, /**< YUV */
} IMPISP_MASK_TYPE;

/**
 * 填充数据
 */
typedef union mask_value {
	struct {
		unsigned char Red; /**< R 值 */
		unsigned char Green; /**< G 值 */
		unsigned char Blue; /**< B 值 */
	} mask_rgb; /**< RGB*/
	struct {
		unsigned char y_value; /**< Y 值 */
		unsigned char u_value; /**< U 值 */
		unsigned char v_value; /**< V 值 */
	} mask_ayuv; /**< YUV*/
} IMP_ISP_MASK_VALUE;

/**
 * 每个通道的填充属性
 */
typedef struct {
	unsigned char mask_en;/**< 填充使能 */
	unsigned short mask_pos_top;/**< 填充位置y坐标*/
	unsigned short mask_pos_left;/**< 填充位置x坐标  */
	unsigned short mask_width;/**< 填充数据宽度 */
	unsigned short mask_height;/**< 填充数据高度 */
	IMP_ISP_MASK_VALUE mask_value;/**< 填充数据值 */
} IMPISP_MASK_BLOCK_PAR;

/**
 * 填充参数
 */
typedef struct {
	IMPISP_MASK_BLOCK_PAR chn0[4];/**< 通道0填充参数 */
	IMPISP_MASK_BLOCK_PAR chn1[4];/**< 通道1填充参数 */
	IMPISP_MASK_BLOCK_PAR chn2[4];/**< 通道3填充参数 */
	IMPISP_MASK_TYPE mask_type;/**< 填充数据类型 */
} IMPISPMASKAttr;

/**
 * @fn int IMP_ISP_Tuning_SetMask(IMPISPMASKAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMask(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetMask(IMPISPMASKAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMask(IMPISPMASKAttr *mask);

/**
 * Sensor属性参数
 */
typedef struct {
	unsigned int hts;/**< sensor hts */
	unsigned int vts;/**< sensor vts */
	unsigned int fps;/**< sensor 帧率 */
	unsigned int width;/**< sensor输出宽度 */
	unsigned int height;/**< sensor输出的高度 */
} IMPISPSENSORAttr;
/**
 * @fn int IMP_ISP_Tuning_GetSensorAttr(IMPISPSENSORAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[out] attr sensor属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetSensorAttr(IMPISPSENSORAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_EnableDRC(IMPISPTuningOpsMode mode)
 *
 * 使能DRC功能.
 *
 * @param[out] mode DRC功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_EnableDRC(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_EnableDefog(IMPISPTuningOpsMode mode)
 *
 * 使能Defog功能.
 *
 * @param[out] mode Defog功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_EnableDefog(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetAwbCt(unsigned int *ct)
 *
 * 设置AWB色温值.
 *
 * @param[in] ct AWB色温值.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbCt(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_GetAWBCt(unsigned int *ct)
 *
 * 获取AWB色温值.
 *
 * @param[out] ct AWB色温值.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAWBCt(unsigned int *ct);

/**
 * ISP 颜色矩阵属性
 */
typedef struct {
	IMPISPTuningOpsMode ManualEn;   /* 手动CCM使能 */
	IMPISPTuningOpsMode SatEn;      /* 手动模式下饱和度使能 */
	float ColorMatrix[9];              /* 颜色矩阵 */
} IMPISPCCMAttr;
/**
 * @fn int IMP_ISP_Tuning_SetCCMAttr(IMPISPCCMAttr *ccm)
 *
 * 设置CCM属性.
 *
 * @param[in] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetCCMAttr(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_GetCCMAttr(IMPISPCCMAttr *ccm)
 *
 * 获取CCM属性.
 *
 * @param[out] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetCCMAttr(IMPISPCCMAttr *ccm);

/**
 * ISP AE 手动模式属性
 */
typedef struct {
	/* 线性模式下长帧的AE 手动模式属性*/
	IMPISPTuningOpsMode AeFreezenEn;    /* AE Freezen使能 */
	IMPISPTuningOpsMode AeItManualEn;	/* AE曝光手动模式使能 */
	unsigned int AeIt;			   /* AE手动模式下的曝光值，单位是曝光行 */
	IMPISPTuningOpsMode AeAGainManualEn;	   /* AE Sensor 模拟增益手动模式使能 */
	unsigned int AeAGain;			      /* AE Sensor 模拟增益值，单位是倍数 x 1024 */
	IMPISPTuningOpsMode AeDGainManualEn;	   /* AE Sensor数字增益手动模式使能 */
	unsigned int AeDGain;			      /* AE Sensor数字增益值，单位是倍数 x 1024 */
	IMPISPTuningOpsMode AeIspDGainManualEn;	      /* AE ISP 数字增益手动模式使能 */
	unsigned int AeIspDGain;			 /* AE ISP 数字增益值，单位倍数 x 1024*/
} IMPISPAEAttr;
/**
 * @fn int IMP_ISP_Tuning_SetAeAttr(IMPISPAEAttr *ae)
 *
 * 设置AE手动模式属性.
 *
 * @param[in] ae AE手动模式属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeAttr(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeAttr(IMPISPAEAttr *ae)
 *
 * 获取AE手动模式属性.
 *
 * @param[out] ae AE手动模式属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 * @attention 在使用这个函数之前，需要先将IMPISPAEAttr结构体初始化为0，然后配置相应的属性。
 */
int IMP_ISP_Tuning_GetAeAttr(IMPISPAEAttr *ae);

/*
 * AE 收敛相关参数
 */
typedef struct {
	bool stable;			/*AE收敛状态，1:代表稳定 0:正在收敛*/
	unsigned int target;    /*当前的目标亮度*/
	unsigned int ae_mean;   /*叠加权重之后，AE的当前的统计平均值*/
}IMPISPAEState;

/**
 * @fn int IMP_ISP_Tuning_GetAeState(IMPISPAEState *ae_state)
 *
 * 获取AE收敛相关的状态参数.
 *
 * @param[out] ae AE的收敛状态.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeState(IMPISPAEState *ae_state);

/**
 * 缩放模式
 */
typedef enum {
	IMP_ISP_SCALER_METHOD_FITTING_CURVE,
	IMP_ISP_SCALER_METHOD_FIXED_WEIGHT,
	IMP_ISP_SCALER_METHOD_BUTT,
} IMPISPScalerMethod;

/**
 * 缩放效果参数
 */
typedef struct {
	unsigned char channel;			/*通道 0~2*/
	IMPISPScalerMethod method;		/*缩放方法*/
	unsigned char level;			/*缩放清晰度等级 范围0~128*/
} IMPISPScalerLv;

/**
 * @fn int IMP_ISP_Tuning_SetScalerLv(IMPISPScalerLv *scaler_level)
 *
 * Set Scaler 缩放的方法及等级.
 *
 * @param[in] mscaler 参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetScalerLv(IMPISPScalerLv *scaler_level);

/**
 * 客户自定义自动曝光库的AE初始属性
 */
typedef struct {
	enum isp_core_expr_unit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */

	/* WDR模式下长帧或者线性模式下的AE属性*/
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeMinIntegrationTime;                      /**< AE最小曝光时间 */
	uint32_t AeMinAGain;                                /**< AE最小sensor模拟增益 */
	uint32_t AeMinDgain;                                /**< AE最小sensor数字增益 */
	uint32_t AeMinIspDGain;                             /**< AE最小ISP数字增益 */
	uint32_t AeMaxIntegrationTime;                      /**< AE最大曝光时间 */
	uint32_t AeMaxAGain;                                /**< AE最大sensor模拟增益 */
	uint32_t AeMaxDgain;                                /**< AE最大sensor数字增益 */
	uint32_t AeMaxIspDGain;                             /**< AE最大ISP数字增益 */

	/* WDR模式下短帧的AE属性*/
	uint32_t AeShortIntegrationTime;                    /**< AE的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeShortMinIntegrationTime;                 /**< AE最小曝光时间 */
	uint32_t AeShortMinAGain;                           /**< AE最小sensor模拟增益 */
	uint32_t AeShortMinDgain;                           /**< AE最小sensor数字增益 */
	uint32_t AeShortMinIspDGain;                        /**< AE最小ISP数字增益 */
	uint32_t AeShortMaxIntegrationTime;                 /**< AE最大曝光时间 */
	uint32_t AeShortMaxAGain;                           /**< AE最大sensor模拟增益 */
	uint32_t AeShortMaxDgain;                           /**< AE最大sensor数字增益 */
	uint32_t AeShortMaxIspDGain;                        /**< AE最大ISP数字增益 */

	uint32_t fps;                                       /**< sensor 帧率 */
	IMPISPAEHist AeStatis;						/**< AE统计属性 */
} IMPISPAeInitAttr;

/**
 * 客户自定义自动曝光库的AE信息
 */
typedef struct {
	IMPISPZone ae_zone;                                 /**< AE各个区域统计值 */
	IMPISPAEHistOrigin ae_hist_256bin;                  /**< AE的256bin统计直方图 */
	IMPISPAEHist ae_hist;                               /**< AE的5bin统计直方图 */
	enum isp_core_expr_unit AeIntegrationTimeUnit;      /**< AE曝光时间单位 */
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/
	uint32_t AeShortIntegrationTime;                    /**< AE的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t Wdr_mode;									/**< 当前是否WDR模式*/
	IMPISPSENSORAttr sensor_attr;						/**< Sensor基本属性*/
}  __attribute__((packed, aligned(1))) IMPISPAeInfo;

/**
 * 客户自定义自动曝光库的AE属性
 */
typedef struct {
	uint32_t change;                                    /**< 是否更新AE参数 */
	enum isp_core_expr_unit AeIntegrationTimeUnit;      /**< AE曝光时间单位 */
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeShortIntegrationTime;                    /**< AE手动模式下的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t luma;			       					/**< AE Luma值 */
	uint32_t luma_scence;		       				    /**< AE 场景Luma值 */
} IMPISPAeAttr;

/**
 * 客户自定义自动曝光库的AE通知属性
 */
typedef enum {
	IMPISP_AE_NOTIFY_FPS_CHANGE,						/**< 帧率变更 */
} IMPISPAeNotify;

/**
 * 客户自定义自动曝光库的AE回调函数
 */
typedef struct {
	void *priv_data;																	/**< 私有数据地址 */
	int (*open)(void *priv_data, IMPISPAeInitAttr *AeInitAttr);                         /**< 自定义AE库开始接口 */
	void (*close)(void *priv_data);													    /**< 自定义AE库关闭接口 */
	void (*handle)(void *priv_data, const IMPISPAeInfo *AeInfo, IMPISPAeAttr *AeAttr);  /**< 自定义AE库的处理接口 */
	int (*notify)(void *priv_data, IMPISPAeNotify notify, void *data);                  /**< 自定义AE库的通知接口 */
} IMPISPAeAlgoFunc;

/**
 * 客户自定义自动曝光库的注册接口
 */
int32_t IMP_ISP_SetAeAlgoFunc(IMPISPAeAlgoFunc *ae_func);
/**
 * @fn int32_t IMP_ISP_SetAeAlgoFunc(IMPISPAeAlgoFunc *ae_func)
 *
 * 客户自定义自动曝光库的注册接口.
 *
 * @param[in] ae_func   客户自定义AE库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor接口之后立刻调用。
 */

 /**
 * 客户自定义自动白平衡库的AWB信息
 */
typedef struct {
	uint32_t cur_r_gain;								/**< 白平衡R通道增益 */
	uint32_t cur_b_gain;								/**< 白平衡B通道增益 */
	uint32_t r_gain_statis;								/**< 白平衡全局统计值r_gain */
	uint32_t b_gain_statis;								/**< 白平衡全局加权统计值b_gain */
	uint32_t r_gain_wei_statis;							/**< 白平衡全局加权统计值r_gain */
	uint32_t b_gain_wei_statis;							/**< 白平衡全局加权统计值b_gain */
	IMPISPAWBZone awb_statis;						/**< 白平衡区域统计值 */
}__attribute__((packed, aligned(1))) IMPISPAwbInfo;

/**
 * 客户自定义自动白平衡库的AWB属性
 */
typedef struct {
	uint32_t change;					/**< 是否更新AWB参数 */
	uint32_t r_gain;						/**< AWB参数 r_gain */
	uint32_t b_gain;						/**< AWB参数 b_gain */
	uint32_t ct;						    /**< 当前色温 */
} IMPISPAwbAttr;

/**
 * 客户自定义自动白平衡库的AWB通知属性
 */
typedef enum {
	IMPISP_AWB_NOTIFY_MODE_CHANGE,           /**< 当前AWB模式变化 */
} IMPISPAwbNotify;

/**
 * 客户自定义自动白平衡库的AWB回调函数
 */
typedef struct {
	void *priv_data;									/**< 私有数据地址 */
	int (*open)(void *priv_data);                           /**< 自定义AWB库开始接口 */
	void (*close)(void *priv_data);                                                         /**< 自定义AWB库关闭接口 */
	void (*handle)(void *priv_data, const IMPISPAwbInfo *AwbInfo, IMPISPAwbAttr *AwbAttr);  /**< 自定义AWB库的处理接口 */
	int (*notify)(void *priv_data, IMPISPAwbNotify notify, void *data);                     /**< 自定义AWB库的通知接口 */
} IMPISPAwbAlgoFunc;

/**
 * 客户自定义自动白平衡库的注册接口
 */
int32_t IMP_ISP_SetAwbAlgoFunc(IMPISPAwbAlgoFunc *awb_func);
/**
 * @fn int32_t IMP_ISP_SetAwbAlgoFunc(IMPISPAwbAlgoFunc *awb_func)
 *
 * 客户自定义自动白平衡库的注册接口.
 *
 * @param[in] awb_func  客户自定义AWB库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor接口之后立刻调用。
 */

/**
 *	黑电平校正功能属性
 */
typedef struct {
	unsigned int black_level_r;     /**< R通道 */
	unsigned int black_level_gr;    /**< GR通道 */
	unsigned int black_level_gb;    /**< GB通道 */
	unsigned int black_level_b;     /**< B通道 */
	unsigned int black_level_ir;    /**< IR通道 */
} IMPISPBlcAttr;

/**
 * @fn int IMP_ISP_Tuning_GetBlcAttr(IMPISPBlcAttr *blc)
 *
 * 获取BLC的相关属性.
 *
 * @param[out] blc blc功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 * @attention 在使用这个函数之前，需要先将IMPISPAEAttr结构体初始化为0。
 */
int IMP_ISP_Tuning_GetBlcAttr(IMPISPBlcAttr *blc);

/**
 * @fn int32_t IMP_ISP_Tuning_SetDefog_Strength(uint8_t *ratio)
 *
 * 设置Defog模块的强度。
 *
 * @param[in] ratio  Defog强度.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetDefog_Strength(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDefog_Strength(uint8_t *ratio)
 *
 * 获取Defog模块的强度。
 *
 * @param[in] ratio  Defog强度.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetDefog_Strength(uint8_t *ratio);

typedef enum {
        ISP_CSC_CG_BT601_FULL,    /**< BT601 full range */
        ISP_CSC_CG_BT601_CLIP,    /**< BT601 非full range */
        ISP_CSC_CG_BT709_FULL,    /**< BT709 full range */
        ISP_CSC_CG_BT709_CLIP,    /**< BT709 非full range */
        ISP_CSC_CG_USER,          /**< 用户自定义模式 */
	IMP_CSC_CG_BUTT,          /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPCscCgMode;

typedef struct {
        int csc_coef[9];	/**< 3x3矩阵 */
        int csc_offset[2];	/**< [0] UV偏移值 [1] Y偏移值 */
        int csc_y_clip[2];	/**< Y最大值，Y最大值 */
        int csc_c_clip[2];	/**< UV最大值，UV最小值 */
} IMPISPCscParam;

typedef struct {
        IMPISPCscCgMode mode;	/**< 模式 */
        IMPISPCscParam csc_par;	/**< 自定义转换矩阵 */
} IMPISPCscAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetCsc_Attr(IMPISPCscAttr *attr)
 *
 * 设置CSC模块功能属性。
 *
 * @param[in] attr CSC模块属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetCsc_Attr(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCsc_Attr(IMPISPCscAttr *attr)
 *
 * 获取CSC模块功能属性。
 *
 * @param[in] attr CSC模块属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetCsc_Attr(IMPISPCscAttr *attr);

/**
 * 丢帧参数
 */
typedef struct {
        IMPISPTuningOpsMode enable;     /**< 使能标志 */
        uint8_t lsize;                  /**< 总数量(范围:0~31) */
        uint32_t fmark;                 /**< 位标志(1输出，0丢失) */
} IMPISPFrameDrop;

/**
 * 丢帧属性
 */
typedef struct {
        IMPISPFrameDrop fdrop[3];       /**< 各个通道的丢帧参数 */
} IMPISPFrameDropAttr;

/**
 * @fn int32_t IMP_ISP_SetFrameDrop(IMPISPFrameDropAttr *attr)
 *
 * 设置丢帧属性。
 *
 * @param[in] attr      丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_SetFrameDrop(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop(IMPISPFrameDropAttr *attr)
 *
 * 获取丢帧属性。
 *
 * @param[out] attr     丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_GetFrameDrop(IMPISPFrameDropAttr *attr);

/**
 * mjpeg固定对比度
 */
typedef struct {
        uint8_t mode;
	uint8_t range_low;
	uint8_t range_high;
} IMPISPFixedContrastAttr;

/**
 * @fn int32_t IMP_ISP_SetFixedContraster(IMPISPFixedContrastAttr *attr)
 *
 * mjpeg设置固定对比度。
 *
 * @param[out] attr	属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetFixedContraster(IMPISPFixedContrastAttr *attr);

/*
 * VIC中断里设置GPIO状态
 */
typedef struct {
	uint16_t gpio_num[10];		/** gpio端口 */
	uint16_t gpio_sta[10];		/** gpio状态 */
    uint16_t free;
} IMPISPGPIO;

/**
* @fn int32_t IMP_ISP_SET_GPIO_INIT_OR_FREE(IMPISPGPIO *attr);
*
* 申请或者释放GPIO资源
* @param[in] gpio_num 需要申请或释放的GPIO端口，以0xFF结束
* @param[in] gpio_sta 申请GPIO的初始化状态，0：低 1：高
* @param[in] free 0：申请 1：释放
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @remark gpio_num[10]={20,21,0xff},gpio_sta[10]= {1,0} 初始化PA20输出高PA21输出低
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_SET_GPIO_INIT_OR_FREE(IMPISPGPIO *attr);

/**
* @fn int32_t IMP_ISP_SET_GPIO_STA(IMPISPGPIO *attr)
*
* 下一个VIC DONE设置GPIO状态.
*
* @param[in] gpio_num 需要设置的GPIO端口，以0xFF结束
* @param[in] gpio_sta GPIO状态，0：低 1：高
*
* @remark gpio_num[10]={20,21,0xff},gpio_sta[10]= {1,0} PA20设置高，PA21设置低
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_SET_GPIO_STA(IMPISPGPIO *attr);

/**
 * ISP AutoZoom Attribution
 */
typedef struct {
	int chan;              /** <通道号> */
	int scaler_enable;     /** <使能缩放功能> */
	int scaler_outwidth;   /** <缩放后输出的宽度> */
	int scaler_outheight;  /** <缩放后输出的高度> */
	int crop_enable;       /** <使能裁剪功能> **/
	int crop_left;         /** <裁剪起始地址横坐标> */
	int crop_top;          /** <裁剪起始地址纵坐标> */
	int crop_width;        /** <缩放后输出的宽度> */
	int crop_height;       /** <缩放后输出的高度> */
} IMPISPAutoZoom;


/**
 * @fn int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom)
 *
 * 设置自动聚焦的参数
 *
 * @param[in] 自动聚焦配置参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor被执行且返回成功.
 */
int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom);

///**
// * 填充数据类型
// */
//typedef enum {
//        IMPISP_MASK_TYPE_RGB = 0, /**< RGB */
//        IMPISP_MASK_TYPE_YUV = 1, /**< YUV */
//} IMPISP_MASK_TYPE;

/**
 * 填充数据
 */
typedef struct color_value {
	struct {
		unsigned char r_value;	/**< R 值 */
		unsigned char g_value;	/**< G 值 */
		unsigned char b_value;	/**< B 值 */
	} argb;						/**< RGB */
	struct {
		unsigned char y_value;	/**< Y 值 */
		unsigned char u_value;	/**< U 值 */
		unsigned char v_value;	/**< V 值 */
	} ayuv;						/**< YUV */
} IMP_ISP_COLOR_VALUE;

/**
 * 每个通道的填充属性
 */
typedef struct isp_mask_block_par {
	uint8_t chx;					/**< 通道号(范围: 0~2) */
	uint8_t pinum;					/**< 块号(范围: 0~3) */
	uint8_t mask_en;				/**< 填充使能 */
	uint16_t mask_pos_top;			/**< 填充位置y坐标*/
	uint16_t mask_pos_left;			/**< 填充位置x坐标  */
	uint16_t mask_width;			/**< 填充数据宽度 */
	uint16_t mask_height;			/**< 填充数据高度 */
	IMPISP_MASK_TYPE mask_type;		/**< 填充数据类型 */
	IMP_ISP_COLOR_VALUE mask_value; /**< 填充数据值 */
} IMPISPMaskBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetMaskBlock(IMPISPMaskBlockAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mask  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr block;
 *
 * if (en) {
 *      block.chx = 0;
 *      block.pinum = 0;
 *      block.mask_en = 1;
 *      block.mask_pos_top = 10;
 *      block.mask_pos_left = 100;
 *      block.mask_width = 200;
 *      block.mask_height = 200;
 *      block.mask_type = IMPISP_MASK_TYPE_YUV;
 *      block.mask_value.ayuv.y_value = 100;
 *      block.mask_value.ayuv.u_value = 100;
 *      block.mask_value.ayuv.v_value = 100;
 * } else {
 *      block.mask_en = 0;
 * }
 *
 * ret = IMP_ISP_Tuning_SetMaskBlock(&block);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetMaskBlock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetMaskBlock(IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_GetMaskBlock(IMPISPMaskBlockAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr attr;
 *
 * attr.chx = 0;
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetMaskBlock(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetMaskBlock error !\n");
 * 	return -1;
 * }
 * printf("chx:%d, pinum:%d, en:%d\n", attr.chx, attr.pinum, attr.mask_en);
 * if (attr.mask_en) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetMaskBlock(IMPISPMaskBlockAttr *mask);

/**
 * 填充图片格式
 */
typedef enum {
	IMP_ISP_PIC_ARGB_8888,  /**< ARGB8888 */
	IMP_ISP_PIC_ARGB_1555,  /**< ARBG1555 */
	IMP_ISP_PIC_ARGB_1100,  /**< AC 2bit */
} IMPISPPICTYPE;

/**
 * 填充格式
 */
typedef enum {
	IMP_ISP_ARGB_TYPE_BGRA = 0,
	IMP_ISP_ARGB_TYPE_GBRA,
	IMP_ISP_ARGB_TYPE_BRGA,
	IMP_ISP_ARGB_TYPE_RBGA,
	IMP_ISP_ARGB_TYPE_GRBA,
	IMP_ISP_ARGB_TYPE_RGBA,

	IMP_ISP_ARGB_TYPE_ABGR = 8,
	IMP_ISP_ARGB_TYPE_AGBR,
	IMP_ISP_ARGB_TYPE_ABRG,
	IMP_ISP_ARGB_TYPE_AGRB,
	IMP_ISP_ARGB_TYPE_ARBG,
	IMP_ISP_ARGB_TYPE_ARGB,
} IMPISPARGBType;

/**
 * 填充图片参数
 */
typedef struct {
    uint8_t  pinum;			/**< 块号(范围: 0~7) */
	uint8_t  osd_enable;    /**< 填充功能使能 */
	uint16_t osd_left;      /**< 填充横向起始点 */
	uint16_t osd_top;       /**< 填充纵向起始点 */
	uint16_t osd_width;     /**< 填充宽度 */
	uint16_t osd_height;    /**< 填充高度 */
	char *osd_image;		/**< 填充图片首地址 */
	uint16_t osd_stride;    /**< 填充图片的对其宽度, 以字节为单位，例如320x240的RGBA8888图片osd_stride=320*4 */
} IMPISPOSDBlockAttr;

/**
 * 填充功能通道属性
 */
typedef struct {
	IMPISPPICTYPE osd_type;                        /**< 填充图片类型  */
	IMPISPARGBType osd_argb_type;                  /**< 填充格式  */
	IMPISPTuningOpsMode osd_pixel_alpha_disable;   /**< 填充像素Alpha禁用功能使能  */
} IMPISPOSDAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDAttr(IMPISPOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * attr.osd_type = IMP_ISP_PIC_ARGB_8888;
 * attr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
 * attr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *
 * if(ret){
 * 	IMP_LOG_ERR(TAG, " error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDAttr(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDAttr(IMPISPOSDAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetOSDAttr(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDAttr error !\n");
 * 	return -1;
 * }
 * printf("type:%d, argb_type:%d, mode:%d\n", attr.osd_type,
 * attr.osd_argb_type, attr.osd_pixel_alpha_disable);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDAttr(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDBlock(IMPISPOSDBlockAttr *attr)
 *
 * 设置OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr block;
 *
 * block.pinum = pinum;
 * block.osd_enable = enable;
 * block.osd_left = left / 2 * 2;
 * block.osd_top = top / 2 * 2;
 * block.osd_width = width;
 * block.osd_height = height;
 * block.osd_image = image;
 * block.osd_stride = stride;
 *
 * ret = IMP_ISP_Tuning_SetOSDBlock(&block);
 * if(ret){
 * 	imp_log_err(tag, "imp_isp_tuning_setosdblock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDBlock(IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock(IMPISPOSDBlockAttr *attr)
 *
 * 获取OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetOSDBlock(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, en:%d\n", attr.pinum, attr.osd_enable);
 * if (attr.osd_enable) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDBlock(IMPISPOSDBlockAttr *attr);

/**
 * 画窗功能属性
 */
typedef struct {
	uint8_t  enable;           /**< 画窗功能使能 */
	uint16_t left;             /**< 画窗功能横向起始点 */
	uint16_t top;              /**< 画窗功能纵向起始点 */
	uint16_t width;            /**< 画窗宽度 */
	uint16_t height;           /**< 画窗高度 */
	IMP_ISP_COLOR_VALUE color; /**< 画窗颜色 */
	uint8_t  line_width;	   /**< 窗口边框宽度 */
	uint8_t  alpha;            /**< 宽口边框alpha（3bit） */
}IMPISPDrawWindAttr;

/**
 * 画四角窗功能属性
 */
typedef struct {
	uint8_t  enable;           /**< 画四角窗功能使能 */
	uint16_t left;             /**< 画四角窗功能横向起始点 */
	uint16_t top;              /**< 画四角窗功能纵向起始点 */
	uint16_t width;            /**< 画四角窗宽度 */
	uint16_t height;           /**< 画四角窗高度 */
	IMP_ISP_COLOR_VALUE color; /**< 画四角窗颜色 */
	uint8_t  line_width;       /**< 画四角窗边框宽度 */
	uint8_t  alpha;            /**< 四角窗边框alpha （3bit） */
	uint16_t extend;           /**< 四角窗边框长度 */
} IMPISPDrawRangAttr;

/**
 * 画线功能属性
 */
typedef struct {
	uint8_t  enable;               /**< 画线功能使能 */
	uint16_t startx;               /**< 画线横向起始点 */
	uint16_t starty;               /**< 画线纵向起始点 */
	uint16_t endx;                 /**< 画线横向结束点 */
	uint16_t endy;                 /**< 画线纵向结束点 */
	IMP_ISP_COLOR_VALUE color;     /**< 线条颜色 */
	uint8_t  width;                /**< 线宽 */
	uint8_t  alpha;                /**< 线条Alpha值 */
} IMPISPDrawLineAttr;

/**
 * 画图功能类型
 */
typedef enum {
	IMP_ISP_DRAW_WIND,              /**< 画框 */
	IMP_ISP_DRAW_RANGE,             /**< 画四角窗 */
	IMP_ISP_DRAW_LINE,              /**< 画线 */
} IMPISPDrawType;

/**
 * 画图功能属性
 */
typedef struct {
	uint8_t pinum;                      /**< 块号(范围: 0~19) */
	IMPISPDrawType type;                /**< 画图类型 */
	IMPISP_MASK_TYPE color_type;		/**< 填充数据类型 */
	union {
		IMPISPDrawWindAttr wind;		/**< 画框属性 */
		IMPISPDrawRangAttr rang;		/**< 画四角窗属性 */
		IMPISPDrawLineAttr line;		/**< 画线属性 */
	} cfg;								/**< 画图属性 */
} IMPISPDrawBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetDrawBlock(IMPISPDrawBlockAttr *attr)
 *
 * 设置绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)2;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.line.enable = en;
 * block.cfg.line.startx = left / 2 * 2;
 * block.cfg.line.starty = top / 2 * 2;
 * block.cfg.line.endx = w / 2 * 2;
 * block.cfg.line.endy = h / 2 * 2;
 * block.cfg.line.color.ayuv.y_value = y;
 * block.cfg.line.color.ayuv.u_value = u;
 * block.cfg.line.color.ayuv.v_value = v;
 * block.cfg.line.width = lw / 2 * 2;
 * block.cfg.line.alpha = alpha;
 * IMP_ISP_Tuning_SetDrawBlock(&block);
 *
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)0;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.wind.enable = en;
 * block.cfg.wind.left = left / 2 * 2;
 * block.cfg.wind.top = top / 2 * 2;
 * block.cfg.wind.width = w / 2 * 2;
 * block.cfg.wind.height = h / 2 * 2;
 * block.cfg.wind.color.ayuv.y_value = y;
 * block.cfg.wind.color.ayuv.u_value = u;
 * block.cfg.wind.color.ayuv.v_value = v;
 * block.cfg.wind.line_width = lw / 2 * 2;
 * block.cfg.wind.alpha = alpha;
 *
 * IMP_ISP_Tuning_SetDrawBlock(&block);
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)1;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.rang.enable = en;
 * block.cfg.rang.left = left / 2 * 2;
 * block.cfg.rang.top = top / 2 * 2;
 * block.cfg.rang.width = w / 2 * 2;
 * block.cfg.rang.height = h / 2 * 2;
 * block.cfg.rang.color.ayuv.y_value = y;
 * block.cfg.rang.color.ayuv.u_value = u;
 * block.cfg.rang.color.ayuv.v_value = v;
 * block.cfg.rang.line_width = lw / 2 * 2;
 * block.cfg.rang.alpha = alpha;
 * block.cfg.rang.extend = extend / 2 * 2;
 *
 * IMP_ISP_Tuning_SetDrawBlock(&block);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetDrawBlock(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDrawBlock(IMPISPDrawBlockAttr *attr)
 *
 * 获取绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPDrawBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetDrawBlock(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetDrawBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, type:%d, color type:%d\n", attr.pinum, attr.type, attr.color_type);
 * switch (attr.type) {
 *      case IMP_ISP_DRAW_WIND:
 *          printf("enable:%d\n", attr.wind.enable);
 *          if (attr.wind.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_RANGE:
 *          printf("enable:%d\n", attr.rang.enable);
 *          if (attr.rang.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_LINE:
 *          printf("enable:%d\n", attr.line.enable);
 *          if (attr.line.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      default:
 *          break;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetDrawBlock(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath_Sec(char *path)
 *
 * 设置ISP bin文件默认路径
 *
 * @param[in] path  需要设置的bin文件路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置用户自定义ISP启动时Bin文件的绝对路径。
 *
 * @attention 这个函数必须在添加sensor之前、打开ISP之后被调用。
 */
int32_t IMP_ISP_SetDefaultBinPath_Sec(char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath_Sec(char *path)
 *
 * 获取ISP bin文件默认路径
 *
 * @param[out] path	需要获取的bin文件路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 获取用户自定义ISP启动时Bin文件的绝对路径。
 *
 * @attention 这个函数必须在添加sensor之后被调用。
 * @attention 一次只能获取单个ISP的bin文件路径属性。
 */
int32_t IMP_ISP_GetDefaultBinPath_Sec(char *path);

/**
 * @fn int IMP_ISP_SetSensorRegister_Sec(uint32_t reg, uint32_t value)
 *
 * 设置sensor一个寄存器的值
 *
 * @param[in] reg 寄存器地址
 *
 * @param[in] value 寄存器值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接设置一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int IMP_ISP_SetSensorRegister_Sec(uint32_t reg, uint32_t value);

/**
 * @fn int IMP_ISP_GetSensorRegister_Sec(uint32_t reg, uint32_t *value)
 *
 * 获取sensor一个寄存器的值
 *
 * @param[in] reg 寄存器地址
 *
 * @param[in] value 寄存器值的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接获取一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int IMP_ISP_GetSensorRegister_Sec(uint32_t reg, uint32_t *value);

/**
 * @fn int IMP_ISP_Tuning_SetSensorFPS_Sec(uint32_t fps_num, uint32_t fps_den)
 *
 * 设置摄像头输出帧率
 *
 * @param[in] fps_num 设定帧率的分子参数
 * @param[in] fps_den 设定帧率的分母参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetSensorFPS_Sec(uint32_t fps_num, uint32_t fps_den);

/**
 * @fn int IMP_ISP_Tuning_GetSensorFPS_Sec(uint32_t *fps_num, uint32_t *fps_den)
 *
 * 获取摄像头输出帧率
 *
 * @param[in] fps_num 获取帧率分子参数的指针
 * @param[in] fps_den 获取帧率分母参数的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 * @attention 在使能帧通道开始传输数据之前必须先调用此函数获取摄像头默认帧率。
 */
int IMP_ISP_Tuning_GetSensorFPS_Sec(uint32_t *fps_num, uint32_t *fps_den);

/**
 * @fn int IMP_ISP_Tuning_SetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr attr)
 *
 * 设置ISP抗闪频属性
 *
 * @param[in] attr 设置参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr attr);

/**
 * @fn int IMP_ISP_Tuning_GetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr *pattr)
 *
 * 获得ISP抗闪频属性
 *
 * @param[in] pattr 获取参数值指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr *pattr);

/**
 * @fn int IMP_ISP_Tuning_SetBrightness_Sec(unsigned char bright)
 *
 * 设置ISP 综合效果图片亮度
 *
 * @param[in] bright 图片亮度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetBrightness_Sec(unsigned char bright);

/**
 * @fn int IMP_ISP_Tuning_GetBrightness_Sec(unsigned char *pbright)
 *
 * 获取ISP 综合效果图片亮度
 *
 * @param[in] bright 图片亮度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetBrightness_Sec(unsigned char *pbright);

/**
 * @fn int IMP_ISP_Tuning_SetContrast_Sec(unsigned char contrast)
 *
 * 设置ISP 综合效果图片对比度
 *
 * @param[in] contrast 图片对比度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetContrast_Sec(unsigned char contrast);

/**
 * @fn int IMP_ISP_Tuning_GetContrast_Sec(unsigned char *pcontrast)
 *
 * 获取ISP 综合效果图片对比度
 *
 * @param[in] contrast 图片对比度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetContrast_Sec(unsigned char *pcontrast);

 /**
 * @fn int IMP_ISP_Tuning_SetSharpness_Sec(unsigned char sharpness)
 *
 * 设置ISP 综合效果图片锐度
 *
 * @param[in] sharpness 图片锐度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetSharpness_Sec(unsigned char sharpness);

/**
 * @fn int IMP_ISP_Tuning_GetSharpness_Sec(unsigned char *psharpness)
 *
 * 获取ISP 综合效果图片锐度
 *
 * @param[in] sharpness 图片锐度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetSharpness_Sec(unsigned char *psharpness);

/**
 * @fn int IMP_ISP_Tuning_SetBcshHue_Sec(unsigned char hue)
 *
 * 设置图像的色调
 *
 * @param[in] hue 图像的色调参考值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128正向调节色调，小于128反向调节色调，调节范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetBcshHue_Sec(unsigned char hue);

/**
 * @fn int IMP_ISP_Tuning_GetBcshHue_Sec(unsigned char *hue)
 *
 * 获取图像的色调值。
 *
 * @param[out] hue 图像的色调参数指针。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128代表正向调节色调，小于128代表反向调节色调，范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetBcshHue_Sec(unsigned char *hue);

/**
 * @fn int IMP_ISP_Tuning_SetSaturation_Sec(unsigned char sat)
 *
 * 设置ISP 综合效果图片饱和度
 *
 * @param[in] sat 图片饱和度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetSaturation_Sec(unsigned char sat);

/**
 * @fn int IMP_ISP_Tuning_GetSaturation_Sec(unsigned char *psat)
 *
 * 获取ISP 综合效果图片饱和度
 *
 * @param[in] sat 图片饱和度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_GetSaturation_Sec(unsigned char *psat);

/**
 * @fn int IMP_ISP_Tuning_GetTotalGain_Sec(uint32_t *gain)
 *
 * 获取ISP输出图像的整体增益值
 *
 * @param[in] gain 获取增益值参数的指针,其数据存放格式为[24.8]，高24bit为整数，低8bit为小数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetTotalGain_Sec(uint32_t *gain);


/**
 * @fn int IMP_ISP_Tuning_SetISPRunningMode_Sec(IMPISPRunningMode mode)
 *
 * 设置ISP工作模式，正常模式或夜视模式；默认为正常模式。
 *
 * @param[in] mode运行模式参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * 示例：
 * @code
 * IMPISPRunningMode mode;
 *
 *	if( it is during a night now){
		mode = IMPISP_RUNNING_MODE_NIGHT
	}else{
		mode = IMPISP_RUNNING_MODE_DAY;
	}
	ret = IMP_ISP_Tuning_SetISPRunningMode_Sec(mode);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPRunningMode error !\n");
		return -1;
	}
 *
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetISPRunningMode_Sec(IMPISPRunningMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPRunningMode_Sec(IMPISPRunningMode *pmode)
 *
 * 获取ISP工作模式，正常模式或夜视模式。
 *
 * @param[in] pmode操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPRunningMode_Sec(IMPISPRunningMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPCustomMode_Sec(IMPISPTuningOpsMode mode)
 *
 * 使能ISP Custom Mode，加载另外一套效果参数.
 *
 * @param[in] mode Custom 模式，使能或者关闭
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetISPCustomMode_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPCustomMode_Sec(IMPISPTuningOpsMode mode)
 *
 * 获取ISP Custom Mode的状态.
 *
 * @param[out] mode Custom 模式，使能或者关闭
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetISPCustomMode_Sec(IMPISPTuningOpsMode *pmode);

/**
* @fn int IMP_ISP_Tuning_SetGamma_Sec(IMPISPGamma *gamma)
*
* 设置GAMMA参数.
* @param[in] gamma gamma参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_SetGamma_Sec(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_GetGamma_Sec(IMPISPGamma *gamma)
*
* 获取GAMMA参数.
* @param[out] gamma gamma参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetGamma_Sec(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_SetAeComp_Sec(int comp)
*
* 设置AE补偿。AE补偿参数可以调整图像AE target，范围为[0-255].
* @param[in] comp AE补偿参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_SetAeComp_Sec(int comp);

/**
* @fn int IMP_ISP_Tuning_GetAeComp_Sec(int *comp)
*
* 获取AE补偿。
* @param[out] comp AE补偿参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetAeComp_Sec(int *comp);

/**
* @fn int IMP_ISP_Tuning_GetAeLuma_Sec(int *luma)
*
* 获取画面平均亮度。
*
* @param[out] luma AE亮度参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetAeLuma_Sec(int *luma);

/**
 * @fn int IMP_ISP_Tuning_SetAeFreeze_Sec(IMPISPTuningOpsMode mode)
 *
 * 使能AE Freeze功能.
 *
 * @param[in] mode AE Freeze功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetAeFreeze_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetExpr_Sec(IMPISPExpr *expr)
 *
 * 设置AE参数。
 *
 * @param[in] expr AE参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetExpr_Sec(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_GetExpr_Sec(IMPISPExpr *expr)
 *
 * 获取AE参数。
 *
 * @param[out] expr AE参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetExpr_Sec(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_SetWB_Sec(IMPISPWB *wb)
 *
 * 设置白平衡功能设置。可以设置自动与手动模式，手动模式主要通过设置rgain、bgain实现。
 *
 * @param[in] wb 设置的白平衡参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetWB_Sec(IMPISPWB *wb);

/**
 * @fn int IMP_ISP_Tuning_GetWB_Sec(IMPISPWB *wb)
 *
 * 获取白平衡功能设置。
 *
 * @param[out] wb 获取的白平衡参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB_Sec(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_Statis_Sec(IMPISPWB *wb)
 *
 * 获取白平衡统计值。
 *
 * @param[out] wb 获取的白平衡统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB_Statis_Sec(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_GOL_Statis_Sec(IMPISPWB *wb)
 *
 * 获取白平衡全局统计值。
 *
 * @param[out] wb 获取的白平衡全局统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetWB_GOL_Statis_Sec(IMPISPWB *wb);

/**
 * int IMP_ISP_Tuning_SetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);
 *
 * 设置CLuster AWB模式的参数。
 *
 * @param[in] CLuster AWB 模式的参数，包括使能、阈值等，awb_cluster[]设置，请咨询Tuning人员。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);

/**
 * @fn int IMP_ISP_Tuning_GetAwbClust_Sec(IMPISPAWBCluster *awb_cluster)
 *
 * 获取CLuster AWB模式下的参数。
 *
 * @param[out] CLuster AWB 模式的参数，包括使能、阈值等。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);

/**
 * int IMP_ISP_Tuning_SetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);
 *
 * 通过rgain与bgain的offset，设置不同色温下的色温偏向。
 *
 * @param[in] ct_trend 包含高中低三个色温下的rgain、bgain offset
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);

/**
 * int IMP_ISP_Tuning_GetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);
 *
 * 获取不同色温下的色温偏向，即rgain offset与bgain offset，
 *
 * @param[out] ct_trend 包含高中低三个色温下的rgain、bgain offset
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);

/**
 * @fn IMP_ISP_Tuning_Awb_GetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * 获取sensor AWB RGB通道偏移参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_Awb_GetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn IMP_ISP_Tuning_Awb_SetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * 设置sensor可以设置AWB RGB通道偏移参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 *
 * 示例：
 * @code
 * IMPISPCOEFFTWB isp_core_rgb_coefft_wb_attr;
 *
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_r=x;
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_g=y;
 *isp_core_rgb_coefft_wb_attr.rgb_coefft_wb_b=z;
 *IMP_ISP_Tuning_Awb_SetRgbCoefft_Sec(&isp_core_rgb_coefft_wb_attr);
 if(ret){
 IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_Awb_SetRgbCoefft error !\n");
 return -1;
 }
*/
int IMP_ISP_Tuning_Awb_SetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn int IMP_ISP_Tuning_SetMaxAgain_Sec(uint32_t gain)
 *
 * 设置sensor可以设置最大Again。
 *
 * @param[in] gain sensor可以设置的最大again.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMaxAgain_Sec(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxAgain_Sec(uint32_t *gain)
 *
 * 获取sensor可以设置最大Again。
 *
 * @param[out] gain sensor可以设置的最大again.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMaxAgain_Sec(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetMaxDgain_Sec(uint32_t gain)
 *
 * 设置ISP可以设置的最大Dgain。
 *
 * @param[in] ISP Dgain 可以设置的最大dgain.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMaxDgain_Sec(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxDgain_Sec(uint32_t *gain)
 *
 * 获取ISP设置的最大Dgain。
 *
 * @param[out] ISP Dgain 可以得到设置的最大的dgain.0表示1x，32表示2x，依次类推。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMaxDgain_Sec(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetHiLightDepress_Sec(uint32_t strength)
 *
 * 设置强光抑制强度。
 *
 * @param[in] strength 强光抑制强度参数.取值范围为［0-10], 0表示关闭功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetHiLightDepress_Sec(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetHiLightDepress_Sec(uint32_t *strength)
 *
 * 获取强光抑制的强度。
 *
 * @param[out] strength 可以得到设置的强光抑制的强度.0表示关闭此功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetHiLightDepress_Sec(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetBacklightComp_Sec(uint32_t strength)
 *
 * 设置背光补偿强度。
 *
 * @param[in] strength 背光补偿强度参数.取值范围为［0-10], 0表示关闭功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetBacklightComp_Sec(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetBacklightComp_Sec(uint32_t *strength)
 *
 * 获取背光补偿的强度。
 *
 * @param[out] strength 可以得到设置的背光补偿的强度.0表示关闭此功能。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetBacklightComp_Sec(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetTemperStrength_Sec(uint32_t ratio)
 *
 * 设置3D降噪强度。
 *
 * @param[in] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]. *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetTemperStrength_Sec(uint32_t ratio);

/**
 * @fn int IMP_ISP_Tuning_SetSinterStrength_Sec(uint32_t ratio)
 *
 * 设置2D降噪强度。
 *
 * @param[in] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255].
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetSinterStrength_Sec(uint32_t ratio);

/**
* @fn int IMP_ISP_Tuning_GetEVAttr_Sec(IMPISPEVAttr *attr)
*
* 获取EV属性。
* @param[out] attr EV属性参数
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_GetEVAttr_Sec(IMPISPEVAttr *attr);

/**
* @fn int IMP_ISP_Tuning_EnableMovestate_Sec(void)
*
* 当sensor在运动时，设置ISP进入运动态。
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_EnableMovestate_Sec(void);

/**
* @fn int IMP_ISP_Tuning_DisableMovestate_Sec(void)
*
* 当sensor从运动态恢复为静止态，设置ISP不使能运动态。
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_Tuning_EnableMovestate已被调用。
*/
int IMP_ISP_Tuning_DisableMovestate_Sec(void);

/**
 * @fn int IMP_ISP_Tuning_SetAeWeight_Sec(IMPISPWeight *ae_weight)
 *
 * 设置AE统计区域的权重。
 *
 * @param[in] ae_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeWeight_Sec(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAeWeight_Sec(IMPISPWeight *ae_weight)
 *
 * 获取AE统计区域的权重。
 *
 * @param[out] ae_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeWeight_Sec(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_GetROI_Sec(IMPISPWeight *roi_weight)
 *
 * 获取AE感兴趣区域，用于场景判断。
 *
 * @param[out] roi_weight AE感兴趣区域权重。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_AE_GetROI_Sec(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_SetROI_Sec(IMPISPWeight *roi_weight)
 *
 * 获取AE感兴趣区域，用于场景判断。
 *
 * @param[in] roi_weight AE感兴趣区域权重。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_AE_SetROI_Sec(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_SetAwbWeight_Sec(IMPISPWeight *awb_weight)
 *
 * 设置AWB统计区域的权重。
 *
 * @param[in] awb_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbWeight_Sec(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbWeight_Sec(IMPISPWeight *awb_weight)
 *
 * 获取AWB统计区域的权重。
 *
 * @param[out] awb_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbWeight_Sec(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbZone_Sec(IMPISPAWBZONE *awb_zone)
 *
 * 获取WB在每个块，不同通道的统计平均值。
 *
 * @param[out] awb_zone 白平衡统计信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbZone_Sec(IMPISPAWBZone *awb_zone);

/**
 * @fn int IMP_ISP_Tuning_SetWB_ALGO_Sec(IMPISPAWBALGO wb_algo)
 *
 * 设置AWB统计的模式。
 *
 * @param[in] wb_algo AWB统计的不同模式。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */

int IMP_ISP_Tuning_SetWB_ALGO_Sec(IMPISPAWBAlgo wb_algo);

/**
 * @fn int IMP_ISP_Tuning_SetAeHist_Sec(IMPISPAEHist *ae_hist)
 *
 * 设置AE统计相关参数。
 *
 * @param[in] ae_hist AE统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeHist_Sec(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Sec(IMPISPAEHist *ae_hist)
 *
 * 获取AE统计值。
 *
 * @param[out] ae_hist AE统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeHist_Sec(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Origin_Sec(IMPISPAEHistOrigin *ae_hist)
 *
 * 获取AE 256 bin统计值。
 *
 * @param[out] ae_hist AE统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeHist_Origin_Sec(IMPISPAEHistOrigin *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAwbHist_Sec(IMPISPAWBHist *awb_hist)
 *
 * 获取AWB统计值。
 *
 * @param[out] awb_hist AWB统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAwbHist_Sec(IMPISPAWBHist *awb_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAwbHist_Sec(IMPISPAWBHist *awb_hist)
 *
 * 设置AWB统计相关参数。
 *
 * @param[in] awb_hist AWB统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbHist_Sec(IMPISPAWBHist *awb_hist);

/**
 * @fn IMP_ISP_Tuning_GetAFMetrices_Sec(unsigned int *metric);
 *
 * 获取AF统计值。
 *
 * @param[out] metric AF统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAFMetrices_Sec(unsigned int *metric);

/**
 * @fn int IMP_ISP_Tuning_GetAfHist_Sec(IMPISPAFHist *af_hist);
 *
 * 获取AF统计值。
 *
 * @param[out] af_hist AF统计值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfHist_Sec(IMPISPAFHist *af_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAfHist_Sec(IMPISPAFHist *af_hist)
 *
 * 设置AF统计相关参数。
 *
 * @param[in] af_hist AF统计相关参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAfHist_Sec(IMPISPAFHist *af_hist);
/**
 * @fn int IMP_ISP_Tuning_SetAfWeight_Sec(IMPISPWeight *af_weight)
 *
 * 设置AF统计区域的权重。
 *
 * @param[in] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAfWeight_Sec(IMPISPWeight *af_weigh);
/**
 * @fn int IMP_ISP_Tuning_GetAfWeight_Sec(IMPISPWeight *af_weight)
 *
 * 获取AF统计区域的权重。
 *
 * @param[out] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfWeight_Sec(IMPISPWeight *af_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAfZone_Sec(IMPISPZone *af_zone)
 *
 * 获取AF各个zone的统计值。
 *
 * @param[out] af_zone AF各个区域的统计值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAfZone_Sec(IMPISPZone *af_zone);

/**
* @fn int IMP_ISP_Tuning_WaitFrame_Sec(IMPISPWaitFrameAttr *attr)
* 等待帧结束
*
* @param[out] attr 等待帧结束属性
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int IMP_ISP_Tuning_WaitFrame_Sec(IMPISPWaitFrameAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_SetAeMin_Sec(IMPISPAEMin *ae_min)
 *
 * 设置AE最小值参数。
 *
 * @param[in] ae_min AE最小值参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeMin_Sec(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_GetAeMin_Sec(IMPISPAEMin *ae_min)
 *
 * 获取AE最小值参数。
 *
 * @param[out] ae_min AE最小值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeMin_Sec(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_SetAe_IT_MAX_Sec(unsigned int it_max)
 *
 * 设置AE最大值参数。
 *
 * @param[in] it_max AE最大值参数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAe_IT_MAX_Sec(unsigned int it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAE_IT_MAX_Sec(unsigned int *it_max)
 *
 * 获取AE最大值参数。
 *
 * @param[out] it_max AE最大值信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAE_IT_MAX_Sec(unsigned int *it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAeZone_Sec(IMPISPZone *ae_zone)
 *
 * 获取AE各个zone的Y值。
 *
 * @param[out] ae_zone AE各个区域的Y值。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeZone_Sec(IMPISPZone *ae_zone);

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList_Sec(IMPISPAETargetList *target_list)
 *
 * 设置AE的目标亮度表
 *
 * @param[in] target_list  目标亮度表
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeTargetList_Sec(IMPISPAETargetList *target_list);

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList_Sec(IMPISPAETargetList *target_list)
 *
 * 获取AE当前的目标亮度表
 *
 * @param[out] target_list  目标亮度表
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeTargetList_Sec(IMPISPAETargetList *target_list);

/**
 * @fn int IMP_ISP_Tuning_SetModuleControl_Sec(IMPISPModuleCtl *ispmodule)
 *
 * 设置ISP各个模块bypass功能
 *
 * @param[in] ispmodule ISP各个模块bypass功能.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetModuleControl_Sec(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_GetModuleControl_Sec(IMPISPModuleCtl *ispmodule)
 *
 * 获取ISP各个模块bypass功能.
 *
 * @param[out] ispmodule ISP各个模块bypass功能
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetModuleControl_Sec(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_SetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop)
 *
 * 设置ISP前Crop的位置
 *
 * @param[in] ispfrontcrop 前Crop参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_GetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop)
 *
 * 获取前Crop参数.
 *
 * @param[out] ispfrontcrop 前Crop参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_SetDPC_Strength_Sec(unsigned int strength)
 *
 * 设置DPC强度.
 *
 * @param[in] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetDPC_Strength_Sec(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDPC_Strength_Sec(unsigned int *strength)
 *
 * 获取DPC强度.
 *
 * @param[out] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetDPC_Strength_Sec(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetDRC_Strength_Sec(unsigned int ratio)
 *
 * 设置DRC强度值.
 *
 * @param[in] strength 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetDRC_Strength_Sec(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDRC_Strength_Sec(unsigned int *ratio)
 *
 * 获取DRC强度值.
 *
 * @param[out] ratio 强度调节比例.默认值为128,如果设置大于128则增加强度，小于128降低强度.取值范围为［0-255]
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetDRC_Strength_Sec(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetHVFLIP_Sec(IMPISPHVFLIP hvflip)
 *
 * 设置HV Flip的模式.
 *
 * @param[in] hvflip HV Flip模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetHVFLIP_Sec(IMPISPHVFLIP hvflip);

/**
 * @fn int IMP_ISP_Tuning_GetHVFlip_Sec(IMPISPHVFLIP *hvflip)
 *
 * 获取HV Flip的模式.
 *
 * @param[out] hvflip HV Flip模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetHVFlip_Sec(IMPISPHVFLIP *hvflip);

/**
 * @fn int IMP_ISP_Tuning_SetMask_Sec(IMPISPMASKAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetMask_Sec(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetMask_Sec(IMPISPMASKAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetMask_Sec(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetSensorAttr_Sec(IMPISPSENSORAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[out] attr sensor属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetSensorAttr_Sec(IMPISPSENSORAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_EnableDRC_Sec(IMPISPTuningOpsMode mode)
 *
 * 使能DRC功能.
 *
 * @param[out] mode DRC功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_EnableDRC_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_EnableDefog_Sec(IMPISPTuningOpsMode mode)
 *
 * 使能Defog功能.
 *
 * @param[out] mode Defog功能使能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_EnableDefog_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetAwbCt_Sec(unsigned int *ct)
 *
 * 设置AWB色温值.
 *
 * @param[in] ct AWB色温值.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAwbCt_Sec(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_GetAWBCt_Sec(unsigned int *ct)
 *
 * 获取AWB色温值.
 *
 * @param[out] ct AWB色温值.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAWBCt_Sec(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_SetCCMAttr_Sec(IMPISPCCMAttr *ccm)
 *
 * 设置CCM属性.
 *
 * @param[in] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetCCMAttr_Sec(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_GetCCMAttr_Sec(IMPISPCCMAttr *ccm)
 *
 * 获取CCM属性.
 *
 * @param[out] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetCCMAttr_Sec(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_SetAeAttr_Sec(IMPISPAEAttr *ae)
 *
 * 设置AE手动模式属性.
 *
 * @param[in] ae AE手动模式属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_SetAeAttr_Sec(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeAttr_Sec(IMPISPAEAttr *ae)
 *
 * 获取AE手动模式属性.
 *
 * @param[out] ae AE手动模式属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 * @attention 在使用这个函数之前，需要先将IMPISPAEAttr结构体初始化为0，然后配置相应的属性。
 */
int IMP_ISP_Tuning_GetAeAttr_Sec(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeState_Sec(IMPISPAEState *ae_state)
 *
 * 获取AE收敛相关的状态参数.
 *
 * @param[out] ae AE的收敛状态.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int IMP_ISP_Tuning_GetAeState_Sec(IMPISPAEState *ae_state);

/**
 * @fn int IMP_ISP_Tuning_SetScalerLv_Sec(IMPISPScalerLv *scaler_level)
 *
 * Set Scaler 缩放的方法及等级.
 *
 * @param[in] mscaler 参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int IMP_ISP_Tuning_SetScalerLv_Sec(IMPISPScalerLv *scaler_level);

/**
 * @fn int IMP_ISP_Tuning_GetBlcAttr_Sec(IMPISPBlcAttr *blc)
 *
 * 获取BLC的相关属性.
 *
 * @param[out] blc blc功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 * @attention 在使用这个函数之前，需要先将IMPISPAEAttr结构体初始化为0。
 */
int IMP_ISP_Tuning_GetBlcAttr_Sec(IMPISPBlcAttr *blc);

/**
 * @fn int32_t IMP_ISP_Tuning_SetDefog_Strength_Sec(uint8_t *ratio)
 *
 * 设置Defog模块的强度。
 *
 * @param[in] ratio  Defog强度.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetDefog_Strength_Sec(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDefog_Strength_Sec(uint8_t *ratio)
 *
 * 获取Defog模块的强度。
 *
 * @param[in] ratio  Defog强度.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetDefog_Strength_Sec(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_SetCsc_Attr_Sec(IMPISPCscAttr *attr)
 *
 * 设置CSC模块功能属性。
 *
 * @param[in] attr CSC模块属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetCsc_Attr_Sec(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCsc_Attr_Sec(IMPISPCscAttr *attr)
 *
 * 获取CSC模块功能属性。
 *
 * @param[in] attr CSC模块属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetCsc_Attr_Sec(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetFrameDrop_Sec(IMPISPFrameDropAttr *attr)
 *
 * 设置丢帧属性。
 *
 * @param[in] attr      丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_SetFrameDrop_Sec(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop_Sec(IMPISPFrameDropAttr *attr)
 *
 * 获取丢帧属性。
 *
 * @param[out] attr     丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_GetFrameDrop_Sec(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetFixedContraster_Sec(IMPISPFixedContrastAttr *attr)
 *
 * mjpeg设置固定对比度。
 *
 * @param[out] attr	属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetFixedContraster_Sec(IMPISPFixedContrastAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_SetAutoZoom_Sec(IMPISPAutoZoom *ispautozoom)
 *
 * 设置自动聚焦的参数
 *
 * @param[in] 自动聚焦配置参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor被执行且返回成功.
 */
int IMP_ISP_Tuning_SetAutoZoom_Sec(IMPISPAutoZoom *ispautozoom);

/**
 * @fn int32_t IMP_ISP_Tuning_SetMaskBlock_Sec(IMPISPMaskBlockAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mask  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr block;
 *
 * if (en) {
 *      block.chx = 0;
 *      block.pinum = 0;
 *      block.mask_en = 1;
 *      block.mask_pos_top = 10;
 *      block.mask_pos_left = 100;
 *      block.mask_width = 200;
 *      block.mask_height = 200;
 *      block.mask_type = IMPISP_MASK_TYPE_YUV;
 *      block.mask_value.ayuv.y_value = 100;
 *      block.mask_value.ayuv.u_value = 100;
 *      block.mask_value.ayuv.v_value = 100;
 * } else {
 *      block.mask_en = 0;
 * }
 *
 * ret = IMP_ISP_Tuning_SetMaskBlock_Sec(&block);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetMaskBlock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetMaskBlock_Sec(IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_GetMaskBlock_Sec(IMPISPMaskBlockAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr attr;
 *
 * attr.chx = 0;
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetMaskBlock_Sec(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetMaskBlock error !\n");
 * 	return -1;
 * }
 * printf("chx:%d, pinum:%d, en:%d\n", attr.chx, attr.pinum, attr.mask_en);
 * if (attr.mask_en) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetMaskBlock_Sec(IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDAttr_Sec(IMPISPOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * attr.osd_type = IMP_ISP_PIC_ARGB_8888;
 * attr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
 * attr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *
 * if(ret){
 * 	IMP_LOG_ERR(TAG, " error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDAttr_Sec(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDAttr_Sec(IMPISPOSDAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetOSDAttr_Sec(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDAttr error !\n");
 * 	return -1;
 * }
 * printf("type:%d, argb_type:%d, mode:%d\n", attr.osd_type,
 * attr.osd_argb_type, attr.osd_pixel_alpha_disable);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDAttr_Sec(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDBlock_Sec(IMPISPOSDBlockAttr *attr)
 *
 * 设置OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr block;
 *
 * block.pinum = pinum;
 * block.osd_enable = enable;
 * block.osd_left = left / 2 * 2;
 * block.osd_top = top / 2 * 2;
 * block.osd_width = width;
 * block.osd_height = height;
 * block.osd_image = image;
 * block.osd_stride = stride;
 *
 * ret = IMP_ISP_Tuning_SetOSDBlock_Sec(&block);
 * if(ret){
 * 	imp_log_err(tag, "imp_isp_tuning_setosdblock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDBlock_Sec(IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock_Sec(IMPISPOSDBlockAttr *attr)
 *
 * 获取OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetOSDBlock_Sec(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, en:%d\n", attr.pinum, attr.osd_enable);
 * if (attr.osd_enable) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDBlock_Sec(IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetDrawBlock_Sec(IMPISPDrawBlockAttr *attr)
 *
 * 设置绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)2;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.line.enable = en;
 * block.cfg.line.startx = left / 2 * 2;
 * block.cfg.line.starty = top / 2 * 2;
 * block.cfg.line.endx = w / 2 * 2;
 * block.cfg.line.endy = h / 2 * 2;
 * block.cfg.line.color.ayuv.y_value = y;
 * block.cfg.line.color.ayuv.u_value = u;
 * block.cfg.line.color.ayuv.v_value = v;
 * block.cfg.line.width = lw / 2 * 2;
 * block.cfg.line.alpha = alpha;
 * IMP_ISP_Tuning_SetDrawBlock_Sec(&block);
 *
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)0;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.wind.enable = en;
 * block.cfg.wind.left = left / 2 * 2;
 * block.cfg.wind.top = top / 2 * 2;
 * block.cfg.wind.width = w / 2 * 2;
 * block.cfg.wind.height = h / 2 * 2;
 * block.cfg.wind.color.ayuv.y_value = y;
 * block.cfg.wind.color.ayuv.u_value = u;
 * block.cfg.wind.color.ayuv.v_value = v;
 * block.cfg.wind.line_width = lw / 2 * 2;
 * block.cfg.wind.alpha = alpha;
 *
 * IMP_ISP_Tuning_SetDrawBlock_Sec(&block);
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)1;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.rang.enable = en;
 * block.cfg.rang.left = left / 2 * 2;
 * block.cfg.rang.top = top / 2 * 2;
 * block.cfg.rang.width = w / 2 * 2;
 * block.cfg.rang.height = h / 2 * 2;
 * block.cfg.rang.color.ayuv.y_value = y;
 * block.cfg.rang.color.ayuv.u_value = u;
 * block.cfg.rang.color.ayuv.v_value = v;
 * block.cfg.rang.line_width = lw / 2 * 2;
 * block.cfg.rang.alpha = alpha;
 * block.cfg.rang.extend = extend / 2 * 2;
 *
 * IMP_ISP_Tuning_SetDrawBlock_Sec(&block);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetDrawBlock_Sec(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDrawBlock_Sec(IMPISPDrawBlockAttr *attr)
 *
 * 获取绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPDrawBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetDrawBlock_Sec(&attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetDrawBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, type:%d, color type:%d\n", attr.pinum, attr.type, attr.color_type);
 * switch (attr.type) {
 *      case IMP_ISP_DRAW_WIND:
 *          printf("enable:%d\n", attr.wind.enable);
 *          if (attr.wind.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_RANGE:
 *          printf("enable:%d\n", attr.rang.enable);
 *          if (attr.rang.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_LINE:
 *          printf("enable:%d\n", attr.line.enable);
 *          if (attr.line.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      default:
 *          break;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetDrawBlock_Sec(IMPISPDrawBlockAttr *attr);

#define MAXSUPCHNMUN 2  /*ISPOSD最大支持一个通道*/
#define MAXISPOSDPIC 8  /*ISPOSD每个通道支持绘制的最大图片个数,目前最大只支持8个*/

/**
 * 区域状态
 */
typedef enum {
	IMP_ISP_OSD_RGN_FREE,   /*ISPOSD区域未创建或者释放*/
	IMP_ISP_OSD_RGN_BUSY,   /*ISPOSD区域已创建*/
}IMPIspOsdRngStat;

/**
 * 模式选择
 */
typedef enum {
	ISP_OSD_REG_INV       = 0, /**< 未定义的 */
	ISP_OSD_REG_PIC       = 1, /**< ISP绘制图片*/
}IMPISPOSDType;
typedef struct IMPISPOSDNode IMPISPOSDNode;

/**
 * 填充功能通道属性
 */
typedef struct {
	int chx;
	int sensornum;
	IMPISPOSDAttr chnOSDAttr;					  /**< 填充功能通道属性 */
	IMPISPOSDBlockAttr pic;                       /**< 填充图片属性，每个通道最多可以填充8张图片 */
} IMPISPOSDSingleAttr;

/**
 * ISPOSD属性集合
 */
typedef struct {
	IMPISPOSDType type;
	union {
	IMPISPOSDSingleAttr stsinglepicAttr;		/*pic 类型的ISPOSD*/
	};
}IMPIspOsdAttrAsm;

/**
 * @fn int IMP_ISP_Tuning_SetOsdPoolSize(int size)
 *
 * 创建ISPOSD使用的rmem内存大小
 *
 * @param[in]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_SetOsdPoolSize(int size);

/**
 * @fn int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 创建ISPOSD区域
 *
 * @param[in] chn通道号，IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 设置ISPOSD 通道区域的属性
 *
 * @param[in] sensor num，handle号 IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 获取ISPOSD 通道号中的区域属性
 *
 * @param[in] sensor num，handle号，IMPOSDRgnCreateStat 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_ShowOsdRgn( int chn,int handle, int showFlag)
 *
 * 设置ISPOSD通道号中的handle对应的显示状态
 *
 * @param[in] sensor num，handle号，showFlag显示状态(0:关闭显示，1:开启显示)
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_ShowOsdRgn(int chn,int handle, int showFlag);

/**
 * @fn int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle)
 *
 * 销毁通道中对应的handle节点
 *
 * @param[in] sensor num，handle号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remark 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_ISP_H__ */
