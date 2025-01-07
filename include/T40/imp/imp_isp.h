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
 * ISP功能开关
 */
typedef enum {
	IMPISP_TUNING_OPS_MODE_DISABLE,			/**< 不使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_ENABLE,			/**< 使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsMode;

/**
 * ISP功能模式
 */
typedef enum {
	IMPISP_TUNING_OPS_TYPE_AUTO,			/**< 该模块的操作为自动模式 */
	IMPISP_TUNING_OPS_TYPE_MANUAL,			/**< 该模块的操作为手动模式 */
	IMPISP_TUNING_OPS_TYPE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsType;

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
 * int32_t ret = 0;
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
 * int32_t ret = 0;
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
* 摄像头标号
*/
typedef enum {
	IMPVI_MAIN = 0,            /**< 主摄像头 */
	IMPVI_SEC = 1,             /**< 次摄像头 */
	IMPVI_THR = 2,             /**< 第三摄像头 */
	IMPVI_BUTT,                /**< 用于判断参数有效性的值，必须小于此值 */
} IMPVI_NUM;

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
	int32_t addr;		/**< I2C地址 */
	int32_t i2c_adapter_id;	/**< I2C控制器 */
} IMPI2CInfo;

/**
* 摄像头控制总线类型是SPI时，需要配置的参数结构体
*/
typedef struct {
	char modalias[32];              /**< SPI设备名字，必须与摄像头驱动中struct spi_device_id中name变量一致 */
	int32_t bus_num;		/**< SPI总线地址 */
} IMPSPIInfo;

/**
* 摄像头输入数据接口枚举
*/
typedef enum {
	IMPISP_SENSOR_VI_MIPI_CSI0 = 0,		/**< MIPI CSI0 接口 */
	IMPISP_SENSOR_VI_MIPI_CSI1 = 1,		/**< MIPI CSI1 接口 */
	IMPISP_SENSOR_VI_DVP = 2,			/**< MDVP接口 */
	IMPISP_SENSOR_VI_BUTT = 3,			/**< 用于判断参数有效性的值，必须小于此值 */
} IMPSensorVinType;

/**
* 摄像头输入时钟源选择
*/
typedef enum {
	IMPISP_SENSOR_MCLK0 = 0,		/**< MCLK0时钟源 */
	IMPISP_SENSOR_MCLK1 = 1,		/**< MCLK1时钟源 */
	IMPISP_SENSOR_MCLK2 = 2,		/**< MCLK2时钟源 */
	IMPISP_SENSOR_MCLK_BUTT = 3,	/**< 用于判断参数有效性的值，必须小于此值 */
} IMPSensorMclk;

#define GPIO_PA(n) (0 * 32 + (n))
#define GPIO_PB(n) (1 * 32 + (n))
#define GPIO_PC(n) (2 * 32 + (n))
#define GPIO_PD(n) (3 * 32 + (n))

/**
* 摄像头注册信息结构体
*/
typedef struct {
	char name[32];					/**< 摄像头名字 */
	IMPSensorControlBusType cbus_type;              /**< 摄像头控制总线类型 */
	union {
		IMPI2CInfo i2c;				/**< I2C总线信息 */
		IMPSPIInfo spi;				/**< SPI总线信息 */
	};
	int rst_gpio;		/**< 摄像头reset接口链接的GPIO */
	int pwdn_gpio;		/**< 摄像头power down接口链接的GPIO */
	int power_gpio;		/**< 摄像头power 接口链接的GPIO，注意：现在没有启用该参数 */
	unsigned short sensor_id;               /**< 摄像头ID号 */
	IMPSensorVinType video_interface;	/**< 摄像头数据输入接口 */
	IMPSensorMclk mclk;			/**< 摄像头Mclk时钟源 */
	int default_boot;			/**< 摄像头默认启动setting */
} IMPSensorInfo;

/**
 * @fn int32_t IMP_ISP_Open(void)
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
int32_t IMP_ISP_Open(void);

/**
 * @fn int32_t IMP_ISP_Close(void)
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
int32_t IMP_ISP_Close(void);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath(IMPVI_NUM num, char *path)
 *
 * 设置ISP bin文件默认路径
 *
 * @param[in] num   需要添加sensor的标号
 * @param[in] path  需要设置的bin文件路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置用户自定义ISP启动时Bin文件的绝对路径。
 *
 * @attention 这个函数必须在添加sensor之前、打开ISP之后被调用。
 */
int32_t IMP_ISP_SetDefaultBinPath(IMPVI_NUM num, char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath(IMPVI_NUM num, char *path)
 *
 * 获取ISP bin文件默认路径
 *
 * @param[in] num	需要添加sensor的标号
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
int32_t IMP_ISP_GetDefaultBinPath(IMPVI_NUM num, char *path);

/**
 * @fn int32_t IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 添加一个sensor，用于向ISP模块提供数据源
 *
 * @param[in] num   需要添加sensor的标号
 * @param[in] pinfo 需要添加sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 添加一个摄像头，用于提供图像。
 *
 * @attention 在使用这个函数之前，必须保证摄像头驱动已经注册进内核.
 */
int32_t IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo);

/**
 * @fn int32_t IMP_ISP_DelSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 删除一个sensor
 *
 * @param[in] num   需要删除sensor的标号
 * @param[in] pinfo 需要删除sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 删除一个摄像头。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经停止工作，即调用了IMP_ISP_DisableSensor函数.
 * @attention 多摄系统中，必须按照IMPVI_NUM 0-1-2顺序全部添加完成才能继续调用其他API接口，
 */
int32_t IMP_ISP_DelSensor(IMPVI_NUM num, IMPSensorInfo *pinfo);

/**
 * @fn int32_t IMP_ISP_EnableSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 使能一个sensor
 *
 * @param[in] num   需要使能的sensor的标号
 * @param[in] pinfo 需要使能的sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 使能一个摄像头。
 *
 * @attention 在使用这个函数之前，必须保证此摄像头已经被添加，即调用了IMP_ISP_AddSensor函数.
 */
int32_t IMP_ISP_EnableSensor(IMPVI_NUM num, IMPSensorInfo *info);

/**
 * @fn int32_t IMP_ISP_DisableSensor(IMPVI_NUM num)
 *
 * 关闭一个sensor
 *
 * @param[in] num 需要关闭的sensor的标号
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 关闭一个摄像头，使之停止传输图像, 这样FrameSource无法输出图像，同时ISP也不能进行效果调试。
 *
 * @attention 在使用这个函数之前，必须保证所有FrameSource都已停止输出图像，同时效果调试也在不使能态.
 */
int32_t IMP_ISP_DisableSensor(IMPVI_NUM num);

/**
* 双摄缓存模式结构体
*/
typedef enum {
	IMPISP_DUALSENSOR_SIGLE_BYPASS_MODE = 0,    /**< 关闭ISP的双摄模块 */
	IMPISP_DUALSENSOR_DUAL_DIRECT_MODE,			/**< 双摄像头不缓存 */
	IMPISP_DUALSENSOR_DUAL_SELECT_MODE,		    /**< 双摄像头选择其中某一个摄像头输出 */
	IMPISP_DUALSENSOR_DUAL_SINGLECACHED_MODE,	/**< 双摄像头仅单摄缓存 */
	IMPISP_DUALSENSOR_DUAL_ALLCACHED_MODE,		/**< 双摄像头都缓存 */
	IMPISP_DUALSENSOR_MODE_BUTT,				/**< 用于判断参数有效性的值，必须大于此值 */
} IMPISPDualSensorMode;

/**
* 双摄输出模式
*/
typedef struct {
	IMPISPTuningOpsMode en;                 /**< 双摄输出模式使能 */
	uint32_t switch_con;                    /**< 双摄输出模式组 */
	uint32_t switch_con_num;                /**< 双摄输出模式组内帧数 */
} IMPISPDualSensorSwitchAttr;

/**
* 挂载的sensor数量结构体
*/
typedef enum {
	IMPISP_TOTAL_ONE = 1,                   /**< 总共挂载一个sensor */
	IMPISP_TOTAL_TWO,                       /**< 总共挂载两个sensor */
	IMPISP_TOTAL_THR,                       /**< 总共挂载三个sensor */
	IMPISP_TOTAL_BUTT,                      /**< 用于判断参数有效性的值，必须大于此值 */
} IMPISPSensorNum;

/**
* 双摄拼接模式结构体
*/
typedef enum{
	IMPISP_NOT_JOINT = 0,                  /**< 双摄不使能拼接模式 */
	IMPISP_MAIN_ON_THE_LEFT,               /**< 双摄使能拼接模式，主摄图像在左边 */
	IMPISP_MAIN_ON_THE_RIGHT,              /**< 双摄使能拼接模式，主摄图像在右边 */
	IMPISP_MIAN_ON_THE_ABOVE,              /**< 双摄使能拼接模式，主摄图像在上边 */
	IMPISP_MAIN_ON_THE_UNDER,              /**< 双摄使能拼接模式，主摄图像在下边 */
	IMPISP_MAIN_JOINT_BUTT,                /**< 用于判断参数有效性的值，必须大于此值 */
} IMPISPDualSensorSplitJoint;

/**
* 多摄系统参数结构体
*/
typedef struct  {
	IMPISPSensorNum sensor_num;		 		/**< 总共挂载sensor数量 */
	IMPISPDualSensorMode dual_mode;		 		/**< 双摄缓存模式 */
	IMPISPDualSensorSwitchAttr dual_mode_switch;	/**< 双摄输出模式 */
	IMPISPDualSensorSplitJoint joint_mode;	 		/**< 双摄图像的拼接模式 */
} IMPISPCameraInputMode;

/**
 * @fn int32_t IMP_ISP_SetCameraInputMode(IMPISPCameraInputMode *mode)
 *
 * 设置多摄系统的参数
 *
 * @param[in] mode 需要配置的多摄系统参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置多摄系统的参数
 *
 * @attention 此函数必须在添加sensor之前调用，即先调用此函数，然后调用IMP_ISP_AddSensor逐个添加sensor
 */
int32_t IMP_ISP_SetCameraInputMode(IMPISPCameraInputMode *mode);

/**
 * @fn int32_t IMP_ISP_GetCameraInputMode(IMPISPCameraInputMode *mode)
 *
 * 获取多摄系统的参数
 *
 * @param[in] mode 需要获取的多摄系统参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 获取多摄系统的参数
 *
 * @attention 此函数必须在IMP_ISP_SetCameraInputMode之后调用
 */
int32_t IMP_ISP_GetCameraInputMode(IMPISPCameraInputMode *mode);

/**
 * @fn int32_t IMP_ISP_SetCameraInputSelect(IMPVI_NUM vinum)
 *
 * 设置多摄系统选择模式下输出某个摄像头数据
 *
 * @param[in] vinum 选择的摄像头
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置多摄系统的参数
 *
 * @attention 此函数必须在IMP_ISP_AddSensor之后调用
 * @attention 此函数只能在IMPISP_DUALSENSOR_DUAL_SELECT_MODE模式下调用
 */
int32_t IMP_ISP_SetCameraInputSelect(IMPVI_NUM vinum);

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable)
 *
 * ISP模块是否bypass
 *
 * @param[in] num       对应sensor的标号
 * @param[in] enable    是否bypass输出模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 使能sensor的bypass功能，需要在AddSensor之前就调用此接口。
 * @remark 关闭sensor的bypass功能，需要在DisableSensor之前就调用此接口。
 * @remark 详细使用流程请参考sample-ISP-Bypass。
 *
 * @attention 在使用这个函数之前，必须保证ISP模块是开启的.
 */
int32_t IMP_ISP_Tuning_SetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable)
 *
 * ISP模块是否bypass
 *
 * @param[in] num       对应sensor的标号
 * @param[out] enable    是否bypass输出模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 无。
 *
 * @attention 在使用这个函数之前，必须保证ISP模块是开启的.
 */
int32_t IMP_ISP_Tuning_GetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable);

/**
 * @fn int32_t IMP_ISP_WDR_ENABLE(IMPVI_NUM num, IMPISPTuningOpsMode *mode)
 *
 * 开关ISP WDR.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mode  ISP WDR 模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数第一次调用必须在添加sensor之前，即先调用此函数，然后调用IMP_ISP_AddSensor逐个添加sensor
 */
int32_t IMP_ISP_WDR_ENABLE(IMPVI_NUM num, IMPISPTuningOpsMode *mode);

/**
 * @fn IMP_ISP_WDR_ENABLE_GET(IMPVI_NUM num, IMPISPTuningOpsMode *mode)
 *
 * 获取ISP WDR 模式.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mode ISP WDR 模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_WDR_ENABLE_GET(IMPVI_NUM num, IMPISPTuningOpsMode *mode);

/**
 * @fn int32_t IMP_ISP_EnableTuning(void)
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
int32_t IMP_ISP_EnableTuning(void);

/**
 * @fn int32_t IMP_ISP_DisableTuning(void)
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
int32_t IMP_ISP_DisableTuning(void);

/**
 * @fn int32_t IMP_ISP_SetSensorRegister(IMPVI_NUM num, uint32_t *reg, uint32_t *value)
 *
 * 设置sensor一个寄存器的值
 *
 * @param[in] num   对应sensor的标号
 * @param[in] reg   寄存器地址
 * @param[in] value 寄存器值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接设置一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int32_t IMP_ISP_SetSensorRegister(IMPVI_NUM num, uint32_t *reg, uint32_t *value);

/**
 * @fn int32_t IMP_ISP_GetSensorRegister(IMPVI_NUM num, uint32_t *reg, uint32_t *value)
 *
 * 获取sensor一个寄存器的值
 *
 * @param[in] num   对应sensor的标号
 * @param[in] reg   寄存器地址
 * @param[out] value 寄存器值的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接获取一个sensor寄存器的值。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int32_t IMP_ISP_GetSensorRegister(IMPVI_NUM num, uint32_t *reg, uint32_t *value);

/**
 * Sensor属性参数
 */
typedef struct {
	uint32_t hts;       /**< sensor hts */
	uint32_t vts;       /**< sensor vts */
	uint32_t fps;       /**< sensor 帧率 */
	uint32_t width;     /**< sensor输出宽度 */
	uint32_t height;    /**< sensor输出的高度 */
} IMPISPSENSORAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_GetSensorAttr(IMPVI_NUM num, IMPISPSENSORAttr *attr)
 *
 * 获取sensor参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr sensor属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetSensorAttr(IMPVI_NUM num, IMPISPSENSORAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetSensorFPS(IMPVI_NUM num, uint32_t *fps_num, uint32_t *fps_den)
 *
 * 设置摄像头输出帧率
 *
 * @param[in] num       对应sensor的标号
 * @param[in] fps_num   设定帧率的分子参数
 * @param[in] fps_den   设定帧率的分母参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetSensorFPS(IMPVI_NUM num, uint32_t *fps_num, uint32_t *fps_den);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSensorFPS(IMPVI_NUM num, uint32_t *fps_num, uint32_t *fps_den)
 *
 * 获取摄像头输出帧率
 *
 * @param[in] num       对应sensor的标号
 * @param[in] fps_num   获取帧率分子参数的指针
 * @param[in] fps_den   获取帧率分母参数的指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 * @attention 在使能帧通道开始传输数据之前必须先调用此函数获取摄像头默认帧率。
 */
int32_t IMP_ISP_Tuning_GetSensorFPS(IMPVI_NUM num, uint32_t *fps_num, uint32_t *fps_den);

/**
 * @fn int32_t IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void))
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
int32_t IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void));

/**
 * ISP Wait Frame irq 参数。
 */

typedef enum {
	IMPISP_IRQ_FD = 0,	/**< 帧结束 */
	IMPISP_IRQ_FS = 1,	/**< 帧开始(预留) */
} IMPISPIrqType;

/**
 * ISP Wait Frame 参数。
 */
typedef struct {
	uint32_t timeout;		/**< 超时时间，单位ms */
	uint64_t cnt;			/**< Frame统计 */
	IMPISPIrqType irqtype;          /**< Frame中断类型*/
} IMPISPWaitFrameAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_WaitFrameDone(IMPVI_NUM num, IMPISPWaitFrameAttr *attr)
 * 等待帧结束
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr 等待帧结束属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_WaitFrameDone(IMPVI_NUM num, IMPISPWaitFrameAttr *attr);

/**
 * ISP抗闪频功能模式结构体。
 */
typedef enum {
	IMPISP_ANTIFLICKER_DISABLE_MODE,	/**< 不使能ISP抗闪频功能 */
	IMPISP_ANTIFLICKER_NORMAL_MODE,         /**< 使能ISP抗闪频功能的正常模式，即曝光最小值为第一个step，不能达到sensor的最小值 */
	IMPISP_ANTIFLICKER_AUTO_MODE,           /**< 使能ISP抗闪频功能的自动模式，最小曝光可以达到sensor曝光的最小值 */
	IMPISP_ANTIFLICKER_BUTT,                /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPAntiflickerMode;

/**
 * ISP抗闪频属性参数结构体。
 */
typedef struct {
	IMPISPAntiflickerMode mode;             /**< ISP抗闪频功能模式选择 */
	uint8_t freq;                           /**< 设置抗闪的工频 */
} IMPISPAntiflickerAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr)
 *
 * 设置ISP抗闪频属性
 *
 * @param[in] num   对应sensor的标号
 * @param[in] pattr 设置参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr)
 *
 * 获得ISP抗闪频属性
 *
 * @param[in] num   对应sensor的标号
 * @param[out] pattr 获取参数值指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr);

/**
 * HV Flip 模式
 */
typedef enum {
	IMPISP_FLIP_NORMAL_MODE = 0,	/**< 正常模式 */
	IMPISP_FLIP_H_MODE,				/**< ISP镜像模式 */
	IMPISP_FLIP_V_MODE,				/**< ISP翻转模式 */
	IMPISP_FLIP_HV_MODE,			/**< ISP镜像并翻转模式 */
	IMPISP_FLIP_SENSOR_H_MODE,		/**< Sensor镜像模式 */
	IMPISP_FLIP_SENSOR_V_MODE,		/**< Sensor翻转模式 */
	IMPISP_FLIP_SENSOR_HV_MODE,		/**< Sensor镜像并翻转模式 */
	IMPISP_FLIP_IHSV_MODE,			/**< ISP镜像Sensor翻转模式 */
	IMPISP_FLIP_SHIV_MODE,			/**< sensor镜像ISP翻转模式 */
	IMPISP_FLIP_MODE_BUTT,          /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPHVFLIP;

/**
 * @fn int32_t IMP_ISP_Tuning_SetHVFLIP(IMPVI_NUM num, IMPISPHVFLIP *hvflip)
 *
 * 设置HV Flip的模式.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] hvflip    镜像翻转模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetHVFLIP(IMPVI_NUM num, IMPISPHVFLIP *hvflip);

/**
 * @fn int32_t IMP_ISP_Tuning_GetHVFlip(IMPVI_NUM num, IMPISPHVFLIP *hvflip)
 *
 * 获取HV Flip的模式.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] hvflip   HV Flip模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetHVFlip(IMPVI_NUM num, IMPISPHVFLIP *hvflip);

/**
 * ISP 工作模式配置，正常模式或夜视模式。
 */
typedef enum {
	IMPISP_RUNNING_MODE_DAY = 0,        /**< 正常模式 */
	IMPISP_RUNNING_MODE_NIGHT = 1,      /**< 夜视模式 */
	IMPISP_RUNNING_MODE_CUSTOM = 2,     /**< 定制模式 */
	IMPISP_RUNNING_MODE_BUTT,           /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPRunningMode;

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *mode)
 *
 * 设置ISP工作模式，正常模式或夜视模式或者定制；默认为正常模式。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mode  运行模式参数
 *
 * @remark ISP工作模式，如果选用定制模式需要添加特殊的bin文件，此模式可用于星光夜视的专门调节
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
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_MAIN, &mode);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPRunningMode error !\n");
		return -1;
	}
 *
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *mode);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *pmode)
 *
 * 获取ISP工作模式。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] pmode 操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *pmode);

/**
 * @fn int32_t IMP_ISP_Tuning_SetBrightness(IMPVI_NUM num, unsigned char *bright)
 *
 * 设置ISP 综合效果图片亮度
 *
 * @param[in] num       对应sensor的标号
 * @param[in] bright    图片亮度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetBrightness(IMPVI_NUM num, unsigned char *bright);

/**
 * @fn int32_t IMP_ISP_Tuning_GetBrightness(IMPVI_NUM num, unsigned char *pbright)
 *
 * 获取ISP 综合效果图片亮度
 *
 * @param[in] num         对应sensor的标号
 * @param[out] pbright    图片亮度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetBrightness(IMPVI_NUM num, unsigned char *pbright);

/**
 * @fn int32_t IMP_ISP_Tuning_SetContrast(IMPVI_NUM num, unsigned char *contrast)
 *
 * 设置ISP 综合效果图片对比度
 *
 * @param[in] num 对应sensor的标号
 * @param[in] contrast 图片对比度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetContrast(IMPVI_NUM num, unsigned char *contrast);

/**
 * @fn int32_t IMP_ISP_Tuning_GetContrast(IMPVI_NUM num, unsigned char *pcontrast)
 *
 * 获取ISP 综合效果图片对比度
 *
 * @param[in] num        对应sensor的标号
 * @param[out] pcontrast 图片对比度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetContrast(IMPVI_NUM num, unsigned char *pcontrast);

 /**
  * @fn int32_t IMP_ISP_Tuning_SetSharpness(IMPVI_NUM num, unsigned char *sharpness)
 *
 * 设置ISP 综合效果图片锐度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] sharpness     图片锐度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetSharpness(IMPVI_NUM num, unsigned char *sharpness);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSharpness(IMPVI_NUM num, unsigned char *psharpness)
 *
 * 获取ISP 综合效果图片锐度
 *
 * @param[in] num         对应sensor的标号
 * @param[out] psharpness 图片锐度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetSharpness(IMPVI_NUM num, unsigned char *psharpness);

/**
 * @fn int32_t IMP_ISP_Tuning_SetSaturation(IMPVI_NUM num, unsigned char *saturation)
 *
 * 设置ISP 综合效果图片饱和度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] saturation    图片饱和度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */

int32_t IMP_ISP_Tuning_SetSaturation(IMPVI_NUM num, unsigned char *saturation);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSaturation(IMPVI_NUM num, unsigned char *psaturation)
 *
 * 获取ISP 综合效果图片饱和度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] saturation    图片饱和度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetSaturation(IMPVI_NUM num, unsigned char *psaturation);

/**
 * @fn int32_t IMP_ISP_Tuning_SetBcshHue(IMPVI_NUM num, unsigned char *hue)
 *
 * 设置图像的色调
 *
 * @param[in] num 对应sensor的标号
 * @param[in] hue 图像的色调参考值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128正向调节色调，小于128反向调节色调，调节范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetBcshHue(IMPVI_NUM num, unsigned char *hue);

/**
 * @fn int32_t IMP_ISP_Tuning_GetBcshHue(IMPVI_NUM num, unsigned char *hue)
 *
 * 获取图像的色调值。
 *
 * @param[in] num   对应sensor的标号
 * @param[out] hue  图像的色调参数指针。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128代表正向调节色调，小于128代表反向调节色调，范围0~255。
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetBcshHue(IMPVI_NUM num, unsigned char *hue);

/**
 * ISP各个模块旁路开关
 */
typedef union {
	uint32_t key;                          /**< 各个模块旁路开关 */
	struct {
		uint32_t bitBypassBLC : 1;      /**< [0] */
		uint32_t bitBypassLSC : 1;      /**< [1] */
		uint32_t bitBypassAWB0 : 1;     /**< [2] */
		uint32_t bitBypassWDR : 1;      /**< [3] */
		uint32_t bitBypassDPC : 1;      /**< [4] */
		uint32_t bitBypassGIB : 1;      /**< [5] */
		uint32_t bitBypassAWB1 : 1;     /**< [6] */
		uint32_t bitBypassADR : 1;      /**< [7] */
		uint32_t bitBypassDMSC : 1;     /**< [8] */
		uint32_t bitBypassCCM : 1;      /**< [9] */
		uint32_t bitBypassGAMMA : 1;    /**< [10] */
		uint32_t bitBypassDEFOG : 1;    /**< [11] */
		uint32_t bitBypassCSC : 1;      /**< [12] */
		uint32_t bitBypassMDNS : 1;     /**< [13] */
		uint32_t bitBypassYDNS : 1;     /**< [14] */
		uint32_t bitBypassBCSH : 1;     /**< [15] */
		uint32_t bitBypassCLM : 1;      /**< [16] */
		uint32_t bitBypassYSP : 1;      /**< [17] */
		uint32_t bitBypassSDNS : 1;     /**< [18] */
		uint32_t bitBypassCDNS : 1;     /**< [19] */
		uint32_t bitBypassHLDC : 1;     /**< [20] */
		uint32_t bitBypassLCE : 1;      /**< [21] */
		uint32_t bitRsv : 10;           /**< [22 ~ 30] */
	};
} IMPISPModuleCtl;

/**
 * @fn int32_t IMP_ISP_Tuning_SetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule)
 *
 * 设置ISP各个模块bypass功能
 *
 * @param[in] num           对应sensor的标号
 * @param[in] ispmodule     ISP各个模块bypass功能.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule);

/**
 * @fn int32_t IMP_ISP_Tuning_GetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule)
 *
 * 获取ISP各个模块bypass功能.
 *
 * @param[in] num           对应sensor的标号
 * @param[out] ispmodule    ISP各个模块bypass功能
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule);

/**
 * ISP 模块强度配置数组的下标
 */
typedef enum {
	IMP_ISP_MODULE_SINTER = 0, /**< 2D降噪下标 */
	IMP_ISP_MODULE_TEMPER,     /**< 3D降噪下标 */
	IMP_ISP_MODULE_DRC,        /**< 数字宽动态下标*/
	IMP_ISP_MODULE_DPC,        /**< 动态去坏点下标 (ps：默认强度128，强度越小，坏点越明显；强度越大，去坏点能力越好)*/
	IMP_ISP_MODULE_DEFOG,	   /**< 去雾模块的强度下标 */
	IMP_ISP_MODULE_BUTT,       /**< 用于判断参数有效性的值，必须大于此值 */
} IMPISPModuleRatioArrayList;

/**
 * ISP 模块强度配置单元
 */
typedef struct {
	IMPISPTuningOpsMode en;     /**< 模块强度配置功能使能 */
	uint8_t ratio;              /**< 模块强度配置功能强度，128为默认强度，大于128增加强度，小于128降低强度 */
} IMPISPRatioUnit;

/**
 * ISP 模块强度配置
 */
typedef struct {
	IMPISPRatioUnit ratio_attr[16];  /**< 各个模块强度配置功能属性 */
} IMPISPModuleRatioAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio)
 *
 * 设置各个模块的强度。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] ratio 各个模块的强度配置功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio)
 *
 * 获取各个模块的强度。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] ratio        各个模块的强度配置功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio);

/**
 * ISP CSC转换矩阵标准与模式结构体
 */
typedef enum {
	IMP_ISP_CG_BT601_FULL,          /**< BT601 full range */
	IMP_ISP_CG_BT601_LIMITED,       /**< BT601 非full range */
	IMP_ISP_CG_BT709_FULL,          /**< BT709 full range */
	IMP_ISP_CG_BT709_LIMITED,       /**< BT709 非full range */
	IMP_ISP_CG_USER,                /**< 用户自定义模式 */
	IMP_ISP_CG_BUTT,                /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPCSCColorGamut;

/**
 * ISP CSC转换矩阵结构体
 */
typedef struct {
	float CscCoef[9];               /**< 3x3矩阵 */
	unsigned char CscOffset[2];     /**< [0] UV偏移值 [1] Y偏移值*/
	unsigned char CscClip[4];       /**< 分别为Y最大值，Y最大值，UV最大值，UV最小值 */
} IMPISPCscMatrix;

/**
 * ISP CSC属性结构体
 */
typedef struct {
	IMPISPCSCColorGamut ColorGamut;     /**< RGB转YUV的标准矩阵 */
	IMPISPCscMatrix Matrix;             /**< 客户自定义的转换矩阵 */
} IMPISPCSCAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc)
 *
 * 设置CSC属性.
 *
 * @param[in] num 对应sensor的标号
 * @param[in] csc CSC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc)
 *
 * 获取CSC属性.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] csc      CSC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc);

/**
 * ISP 颜色矩阵属性
 */
typedef struct {
	IMPISPTuningOpsMode ManualEn;       /**< 手动CCM使能 */
	IMPISPTuningOpsMode SatEn;          /**< 手动模式下饱和度使能 */
	float ColorMatrix[9];               /**< 颜色矩阵 */
} IMPISPCCMAttr;
/**
 * @fn int32_t IMP_ISP_Tuning_SetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm)
 *
 * 设置CCM属性.
 *
 * @param[in] num 对应sensor的标号
 * @param[in] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm)
 *
 * 获取CCM属性.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] ccm      CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm);

/**
 * ISP Gamma模式枚举
 */
typedef enum {
	IMP_ISP_GAMMA_CURVE_DEFAULT,    /**< 默认gamma模式 */
	IMP_ISP_GAMMA_CURVE_SRGB,       /**< 标准SRGB gamma模式 */
	IMP_ISP_GAMMA_CURVE_HDR,        /**< HDR Gamma模式 */
	IMP_ISP_GAMMA_CURVE_USER,       /**< 用户自定义gamma模式 */
	IMP_ISP_GAMMA_CURVE_BUTT,       /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPGammaCurveType;

/**
 * gamma属性结构体
 */
typedef struct {
	IMPISPGammaCurveType Curve_type; /**< gamma模式 */
	uint16_t gamma[129];		/**< gamma参数数组，有129个点 */
} IMPISPGammaAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma)
 *
 * 设置GAMMA参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] gamma gamma参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma);

/**
 * @fn int32_t IMP_ISP_Tuning_GetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma)
 *
 * 获取GAMMA参数.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] gamma    gamma参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma);

/**
 * 统计值直方图统计色域结构体
 */
typedef enum {
	IMP_ISP_HIST_ON_RAW,    /**< Raw域 */
	IMP_ISP_HIST_ON_YUV,    /**< YUV域 */
} IMPISPHistDomain;

/**
 * 统计范围结构体
 */
typedef struct {
	unsigned int start_h;   /**< 横向起始点，单位为pixel AF统计值横向起始点：[1 ~ width]，且取奇数 */
	unsigned int start_v;   /**< 纵向起始点，单位为pixel AF统计值垂直起始点 ：[3 ~ height]，且取奇数 */
	unsigned char node_h;   /**< 横向统计区域块数 [默认:15]*/
	unsigned char node_v;   /**< 纵向统计区域块数 [默认:15]*/
} IMPISP3AStatisLocation;

/**
 * AE统计值属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode ae_sta_en;  /**< AE统计功能开关*/
	IMPISP3AStatisLocation local;   /**< AE统计位置 */
	IMPISPHistDomain hist_domain;   /**< AE统计色域 */
	unsigned char histThresh[4];    /**< AE直方图的分段 */
} IMPISPAEStatisAttr;

/**
 * AWB统计值属性结构体
 */
typedef enum {
	IMP_ISP_AWB_ORIGIN,     /**< 原始统计值 */
	IMP_ISP_AWB_LIMITED,    /**< 加限制条件后的统计值 */
} IMPISPAWBStatisMode;

/**
 * AWB统计值属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode awb_sta_en;         /**< AWB统计功能开关*/
	IMPISP3AStatisLocation local;		/**< AWB统计范围 */
	IMPISPAWBStatisMode mode;		/**< AWB统计属性 */
} IMPISPAWBStatisAttr;

typedef struct{
	unsigned char thlow1; /*Ldg低亮度起始阈值【取值范围：0~255】*/
	unsigned char thlow2; /*Ldg低亮度终止阈值【取值范围：0~255】*/
	unsigned short slplow; /*Ldg低亮度斜率 [取值范围:0~4095]*/
	unsigned char gainlow; /*Ldg低亮度增益 【取值范围：0~255】*/
	unsigned char thhigh1; /*Ldg高亮度起始阈值 【取值范围：0~255】*/
	unsigned char thhigh2; /*Ldg高亮度终止阈值 【取值范围：0~255】*/
	unsigned short slphigh; /*Ldg高亮度斜率 【取值范围：0~4095】*/
	unsigned char gainhigh; /*Ldg高亮度增益 【取值范围：0~255】*/
}isp_af_ldg_info;

typedef struct{
	unsigned char fir0;
	unsigned char fir1;
	unsigned char iir0;
	unsigned char iir1;
}isp_af_ldg_en;
/**
 * AF统计属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode af_sta_en;      /**< AF统计功能开关*/
	IMPISP3AStatisLocation local;       /**< AF统计范围 */
	unsigned char af_metrics_shift;     /**< AF统计值缩小参数 默认是0，1代表缩小2倍*/
	unsigned short af_delta;            /**< AF统计低通滤波器的权重 [0 ~ 64]*/
	unsigned short af_theta;            /**< AF统计高通滤波器的权重 [0 ~ 64]*/
	unsigned short af_hilight_th;       /**< AF高亮点统计阈值 [0 ~ 255]*/
	unsigned short af_alpha_alt;        /**< AF统计低通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
	unsigned short af_belta_alt;        /**< AF统计高通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
	isp_af_ldg_en ldg_en;               /**< AF LDG模块的使能位*/
	isp_af_ldg_info fir0;               /**< FIR0滤波器的LDG*/
	isp_af_ldg_info fir1;               /**< FIR1滤波器的LDG*/
	isp_af_ldg_info iir0;               /**< IIR0滤波器的LDG*/
	isp_af_ldg_info iir1;               /**< IIR1滤波器的LDG*/
} IMPISPAFStatisAttr;

/**
 * 统计信息属性结构体
 */
typedef struct {
	IMPISPAEStatisAttr ae;      /**< AE 统计信息属性 */
	IMPISPAWBStatisAttr awb;    /**< AWB 统计信息属性 */
	IMPISPAFStatisAttr af;      /**< AF 统计信息属性 */
} IMPISPStatisConfig;

/**
 * @fn int32_t IMP_ISP_Tuning_SetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config)
 *
 * 设置统计信息参数.
 *
 * @param[in] num               对应sensor的标号
 * @param[in] statis_config    统计信息属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config);

/**
 * @fn int32_t IMP_ISP_Tuning_GetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config)
 *
 * 获取统计信息参数.
 *
 * @param[in] num               对应sensor的标号
 * @param[out] statis_config    统计信息属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config);

/**
 * 权重信息
 */
typedef struct {
	unsigned char weight[15][15];    /**< 各区域权重信息 [0 ~ 8]*/
} IMPISPWeight;

/**
 * AE权重信息
 */
typedef struct {
	IMPISPTuningOpsMode roi_enable;    /**< 感兴趣区域权重设置使能 */
	IMPISPTuningOpsMode weight_enable; /**< 全局权重设置使能 */
	IMPISPWeight ae_roi;               /**< 感兴趣区域权重值（0 ~ 8）*/
	IMPISPWeight ae_weight;            /**< 全局权重值 （0 ~ 8）*/
} IMPISPAEWeightAttr;

/**
* @fn int32_t IMP_ISP_Tuning_SetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight)
*
* 设置统计信息参数.
*
* @param[in] num                对应sensor的标号
* @param[out] ae_weight         权重信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_SetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight);

/**
* @fn int32_t IMP_ISP_Tuning_GetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight)
*
* 获取统计信息参数.
*
* @param[in] num                对应sensor的标号
* @param[out] ae_weight         权重信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight);

/**
 * 各区域统计信息
 */
typedef struct {
	uint32_t statis[15][15];    /**< 各区域统计信息*/
}  __attribute__((packed, aligned(1))) IMPISPStatisZone;

/**
 * AE统计信息
 */
typedef struct {
	unsigned short ae_hist_5bin[5];         /**< AE统计直方图bin值 [0 ~ 65535]*/
	uint32_t ae_hist_256bin[256];           /**< AE统计直方图bin值, 为每个bin的实际pixel数量*/
	IMPISPStatisZone ae_statis;             /**< AE统计信息 */
}  __attribute__((packed, aligned(1))) IMPISPAEStatisInfo;

/**
* @fn int32_t IMP_ISP_Tuning_GetAeStatistics(IMPVI_NUM num, IMPISPAEStatisInfo *ae_statis)
*
* 获取AE统计信息.
*
* @param[in] num                对应sensor的标号
* @param[out] ae_statis         AE统计信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAeStatistics(IMPVI_NUM num, IMPISPAEStatisInfo *ae_statis);

/**
 * AE曝光时间单位
 */
typedef enum {
	ISP_CORE_EXPR_UNIT_LINE,			/**< 单位为曝光行 */
	ISP_CORE_EXPR_UNIT_US,				/**< 单位为微秒 */
} IMPISPAEIntegrationTimeUnit;

/**
 * AE曝光信息
 */
typedef struct {
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
	IMPISPTuningOpsType AeMode;                         /**< AE Freezen使能 */
	IMPISPTuningOpsType AeIntegrationTimeMode;          /**< AE曝光手动模式使能 */
	IMPISPTuningOpsType AeAGainManualMode;              /**< AE Sensor 模拟增益手动模式使能 */
	IMPISPTuningOpsType AeDGainManualMode;              /**< AE Sensor数字增益手动模式使能 */
	IMPISPTuningOpsType AeIspDGainManualMode;	    /**< AE ISP 数字增益手动模式使能 */
	uint32_t AeIntegrationTime;                         /**< AE手动模式下的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	IMPISPTuningOpsType AeMinIntegrationTimeMode;       /**< 预留 */
	IMPISPTuningOpsType AeMinAGainMode;                 /**< AE最小模拟增益使能位 */
	IMPISPTuningOpsType AeMinDgainMode;                 /**< 预留 */
	IMPISPTuningOpsType AeMinIspDGainMode;              /**< 预留 */
	IMPISPTuningOpsType AeMaxIntegrationTimeMode;       /**< AE最大曝光使能位 */
	IMPISPTuningOpsType AeMaxAGainMode;                 /**< AE最大sensor模拟增益使能位 */
	IMPISPTuningOpsType AeMaxDgainMode;                 /**< AE最大sensor数字增益使能位 */
	IMPISPTuningOpsType AeMaxIspDGainMode;              /**< AE最大ISP数字增益使能位 */
	uint32_t AeMinIntegrationTime;                      /**< AE最小曝光时间 */
	uint32_t AeMinAGain;                                /**< AE最小sensor模拟增益 */
	uint32_t AeMinDgain;                                /**< AE最小sensor数字增益 */
	uint32_t AeMinIspDGain;                             /**< AE最小ISP数字增益 */
	uint32_t AeMaxIntegrationTime;                      /**< AE最大曝光时间 */
	uint32_t AeMaxAGain;                                /**< AE最大sensor模拟增益 */
	uint32_t AeMaxDgain;                                /**< AE最大sensor数字增益 */
	uint32_t AeMaxIspDGain;                             /**< AE最大ISP数字增益 */

	/* WDR模式下短帧的AE 手动模式属性*/
	IMPISPTuningOpsType AeShortMode;                    /**< AE Freezen使能 */
	IMPISPTuningOpsType AeShortIntegrationTimeMode;     /**< AE曝光手动模式使能 */
	IMPISPTuningOpsType AeShortAGainManualMode;         /**< AE Sensor 模拟增益手动模式使能 */
	IMPISPTuningOpsType AeShortDGainManualMode;         /**< AE Sensor数字增益手动模式使能 */
	IMPISPTuningOpsType AeShortIspDGainManualMode;      /**< AE ISP 数字增益手动模式使能 */
	uint32_t AeShortIntegrationTime;                    /**< AE手动模式下的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	IMPISPTuningOpsType AeShortMinIntegrationTimeMode;  /**< 预留 */
	IMPISPTuningOpsType AeShortMinAGainMode;            /**< AE最小模拟增益使能位 */
	IMPISPTuningOpsType AeShortMinDgainMode;            /**< 预留 */
	IMPISPTuningOpsType AeShortMinIspDGainMode;         /**< 预留 */
	IMPISPTuningOpsType AeShortMaxIntegrationTimeMode;  /**< AE最大曝光使能位 */
	IMPISPTuningOpsType AeShortMaxAGainMode;            /**< AE最大sensor模拟增益使能位 */
	IMPISPTuningOpsType AeShortMaxDgainMode;            /**< AE最大sensor数字增益使能位 */
	IMPISPTuningOpsType AeShortMaxIspDGainMode;         /**< AE最大ISP数字增益使能位 */
	uint32_t AeShortMinIntegrationTime;                 /**< AE最小曝光时间 */
	uint32_t AeShortMinAGain;                           /**< AE最小sensor模拟增益 */
	uint32_t AeShortMinDgain;                           /**< AE最小sensor数字增益 */
	uint32_t AeShortMinIspDGain;                        /**< AE最小ISP数字增益 */
	uint32_t AeShortMaxIntegrationTime;                 /**< AE最大曝光时间 */
	uint32_t AeShortMaxAGain;                           /**< AE最大sensor模拟增益 */
	uint32_t AeShortMaxDgain;                           /**< AE最大sensor数字增益 */
	uint32_t AeShortMaxIspDGain;                        /**< AE最大ISP数字增益 */

	uint32_t TotalGainDb;                               /**< AE total gain，单位为db */
	uint32_t TotalGainDbShort;                          /**< AE 短帧 total gain, 单位为db */
	uint32_t ExposureValue;                             /**< AE 曝光值，为integration time x again x dgain */
	uint32_t EVLog2;                                    /**< AE 曝光值，此值经过log运算 */
} IMPISPAEExprInfo;

/**
* @fn int32_t IMP_ISP_Tuning_SetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo)
*
* 获取AE统计信息.
*
* @param[in] num                对应sensor的标号
* @param[in] exprinfo           AE曝光信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_SetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo);

/**
* @fn int32_t IMP_ISP_Tuning_GetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo)
*
* 获取AE统计信息.
*
* @param[in] num                对应sensor的标号
* @param[out] exprinfo          AE曝光信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo);

/**
 * AE场景模式状态
 */
typedef enum {
	IMP_ISP_AE_SCENCE_AUTO,        /**< 自动模式 */
	IMP_ISP_AE_SCENCE_DISABLE,     /**< 关闭此场景模式 */
	TISP_AE_SCENCE_ROI_ENABLE,     /**< ROI 使能此场景模式 */
	TISP_AE_SCENCE_GLOBAL_ENABLE,  /**< GLOBAL 使能此场景模式 */
	IMP_ISP_AE_SCENCE_BUTT,        /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPAEScenceMode;

/**
 * AE场景模式属性
 */
typedef struct {
	IMPISPAEScenceMode AeHLCEn;            /**< AE 强光抑制功能使能 */
	unsigned char AeHLCStrength;           /**< AE 强光抑制强度（0 ~ 10）*/
	IMPISPAEScenceMode AeBLCEn;            /**< AE 背光补偿功能使能 */
	unsigned char AeBLCStrength;           /**< AE 背光补偿强度（0 ~ 10） */
	IMPISPAEScenceMode AeTargetCompEn;     /**< AE 目标亮度补偿使能 */
	uint32_t AeTargetComp;                 /**< AE 目标亮度调节强度（0 ~ 255，小于128变暗，大于128变亮） */
	IMPISPAEScenceMode AeStartEn;          /**< AE 起始点功能使能 */
	uint32_t AeStartEv;                    /**< AE 起始点EV值 */

	uint32_t luma;                         /**< AE Luma值 */
	uint32_t luma_scence;                  /**< AE 场景Luma值 */
        bool stable;                           /**< AE 收敛状态 */   
        uint32_t target;                       /**< 当前的目标亮度 */
        uint32_t ae_mean;                      /**< 叠加权重之后，AE的当前的统计平均值 */
} IMPISPAEScenceAttr;

/**
* @fn int32_t IMP_ISP_Tuning_SetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr)
*
* 设置AE场景模式.
*
* @param[in] num                    对应sensor的标号
* @param[in] scenceattr             AE场景模式设置
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_SetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr);

/**
* @fn int32_t IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr)
*
* 获取AE场景模式信息.
*
* @param[in] num                    对应sensor的标号
* @param[out] scenceattr            AE场景模式设置
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr);

/**
 * AWB统计信息
 */
typedef struct {
	IMPISPStatisZone awb_r;    /**< AWB R通道统计值 */
	IMPISPStatisZone awb_g;    /**< AWB G通道统计值 */
	IMPISPStatisZone awb_b;    /**< AWB B通道统计值 */
} IMPISPAWBStatisInfo;

/**
* @fn int32_t IMP_ISP_Tuning_GetAwbStatistics(IMPVI_NUM num, IMPISPAWBStatisInfo *awb_statis)
*
* 获取AWB统计值.
*
* @param[in]    num                    对应sensor的标号
* @param[out]   awb_statis             awb统计信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAwbStatistics(IMPVI_NUM num, IMPISPAWBStatisInfo *awb_statis);

/**
 * 白平衡增益属性
 */
typedef struct {
	uint32_t rgain;     /**< 白平衡R通道增益 */
	uint32_t bgain;     /**< 白平衡B通道增益 */
} IMPISPAWBGain;

/**
 * AWB 全局统计信息
 */
typedef struct {
	IMPISPAWBGain statis_weight_gain;	/**< 白平衡全局加权统计值 */
	IMPISPAWBGain statis_gol_gain;		/**< 白平衡全局统计值 */
} IMPISPAWBGlobalStatisInfo;

/**
* @fn int32_t IMP_ISP_Tuning_GetAwbGlobalStatistics(IMPVI_NUM num, IMPISPAWBGlobalStatisInfo *awb_statis)
*
* 获取AWB统计值.
*
* @param[in]    num                    对应sensor的标号
* @param[out]   awb_statis             awb统计信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAwbGlobalStatistics(IMPVI_NUM num, IMPISPAWBGlobalStatisInfo *awb_statis);

/**
 * 白平衡模式
 */
typedef enum {
	ISP_CORE_WB_MODE_AUTO = 0,			/**< 自动模式 */
	ISP_CORE_WB_MODE_MANUAL,			/**< 手动模式 */
	ISP_CORE_WB_MODE_DAY_LIGHT,			/**< 晴天 */
	ISP_CORE_WB_MODE_CLOUDY,			/**< 阴天 */
	ISP_CORE_WB_MODE_INCANDESCENT,                  /**< 白炽灯 */
	ISP_CORE_WB_MODE_FLOURESCENT,                   /**< 荧光灯 */
	ISP_CORE_WB_MODE_TWILIGHT,			/**< 黄昏 */
	ISP_CORE_WB_MODE_SHADE,				/**< 阴影 */
	ISP_CORE_WB_MODE_WARM_FLOURESCENT,              /**< 暖色荧光灯 */
	ISP_CORE_WB_MODE_COLORTEND,			/**< 自定义模式 */
} IMPISPAWBMode;

/**
 * 白平衡自定义模式属性
 */
typedef struct {
	IMPISPTuningOpsMode customEn;   /**< 白平衡自定义模式使能 */
	IMPISPAWBGain gainH;            /**< 白平衡高色温通道增益偏移 */
	IMPISPAWBGain gainM;            /**< 白平衡中色温通道增益偏移 */
	IMPISPAWBGain gainL;            /**< 白平衡低色温通道增益偏移 */
	uint32_t ct_node[4];            /**< 白平衡通道增益偏移的节点 */
} IMPISPAWBCustomModeAttr;

/**
 * 白平衡属性
 */
typedef struct isp_core_wb_attr{
	IMPISPAWBMode mode;                     /**< 白平衡模式 */
	IMPISPAWBGain gain_val;			/**< 白平衡通道增益，手动模式时有效 */
	IMPISPTuningOpsMode awb_frz;            /**< 白平衡frzzen 使能*/
	unsigned int ct;                        /**< 白平衡当前色温值 */
	IMPISPAWBCustomModeAttr custom;         /**< 白平衡自定义模式属性 */
	IMPISPTuningOpsMode awb_start_en;       /**< 白平衡收敛起始点使能 */
	IMPISPAWBGain awb_start;                /**< 白平衡收敛起始点 */
} IMPISPWBAttr;

/**
* @fn int32_t IMP_ISP_Tuning_SetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr)
*
* 设置AWB属性
*
* @param[in]    num                    对应sensor的标号
* @param[in]    attr                   awb属性
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_SetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr);

/**
* @fn int32_t IMP_ISP_Tuning_GetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr)
*
* 获取AWB属性
*
* @param[in]    num                    对应sensor的标号
* @param[out]   attr                   awb属性
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight)
 *
 * 设置AWB统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[in] awb_weight    各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight)
 *
 * 获取AWB统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] awb_weight   各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight);

/**
 * AF statistics info each area
 */
typedef struct {
	IMPISPStatisZone Af_Fir0;
	IMPISPStatisZone Af_Fir1;
	IMPISPStatisZone Af_Iir0;
	IMPISPStatisZone Af_Iir1;
	IMPISPStatisZone Af_YSum;           /*高亮点的亮度*/
	IMPISPStatisZone Af_HighLumaCnt;    /*高亮点的个数*/
} IMPISPAFStatisInfo;
/**
 * @fn IMP_ISP_Tuning_GetAfStatistics(IMPVI_NUM num, IMPISPAFStatisInfo *af_statis)
 *
 * 获取AF统计值。
 *
 * @param[in]   num             对应sensor的标号
 * @param[out]  af_statis       AF统计值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAfStatistics(IMPVI_NUM num, IMPISPAFStatisInfo *af_statis);

/**
 * AF全局统计值信息
 */
 typedef struct {
	 uint32_t af_metrics;       /**< AF主统计值*/
	 uint32_t af_metrics_alt;   /**< AF次统计值*/
	 uint8_t af_frame_num;      /**< AF帧数*/
	 uint32_t af_wl;            /*低通滤波器输出的统计值*/
	 uint32_t af_wh;            /*高通滤波器输出的统计值*/
 } IMPISPAFMetricsInfo;

/**
 * @fn int32_t IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_NUM num, IMPISPAFMetricsInfo *metric)
 *
 * 获取AF统计值。
 *
 * @param[in]   num             对应sensor的标号
 * @param[out]  metric          AF全局统计值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_NUM num, IMPISPAFMetricsInfo *metric);

/**
 * @fn int32_t IMP_ISP_Tuning_SetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight)
 *
 * 设置AF统计区域的权重。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight);
/**
 * @fn int32_t IMP_ISP_Tuning_GetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight)
 *
 * 获取AF统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] af_weight    各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight);

/**
 * ISP AutoZoom Attribution
 */
typedef struct {
	int32_t zoom_chx_en[3];     /**< 数字自动对焦功能通道使能 */
	int32_t zoom_left[3];       /**< 自动对焦区域横向起始点，需要小于原始图像的宽度 */
	int32_t zoom_top[3];        /**< 自动对焦区域纵向起始点，需要小于原始图像的高度 */
	int32_t zoom_width[3];      /**< 自动对焦区域的宽度 */
	int32_t zoom_height[3];     /**< 自动对焦区域的高度 */
} IMPISPAutoZoom;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom)
 *
 * 设置自动对焦功能的属性。
 *
 * @param[in] num           对应sensor的标号
 * @param[in] ispautozoom   自动对焦功能的属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom)
 *
 * 获取自动对焦功能的属性。
 *
 * @param[in] num               对应sensor的标号
 * @param[out] ispautozoom      自动对焦功能的属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom);

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
typedef struct color_value {
	struct {
		unsigned char r_value;	/**< R 值 */
		unsigned char g_value;	/**< G 值 */
		unsigned char b_value;	/**< B 值 */
	} mask_argb;			/**< RGB */
	struct {
		unsigned char y_value;	/**< Y 值 */
		unsigned char u_value;	/**< U 值 */
		unsigned char v_value;	/**< V 值 */
	} mask_ayuv;			/**< YUV */
} IMP_ISP_COLOR_VALUE;

/**
 * 每个通道的填充属性
 */
typedef struct isp_mask_block_par {
	unsigned char mask_en;          /**< 填充使能 */
	unsigned short mask_pos_top;    /**< 填充位置y坐标*/
	unsigned short mask_pos_left;   /**< 填充位置x坐标  */
	unsigned short mask_width;      /**< 填充数据宽度 */
	unsigned short mask_height;     /**< 填充数据高度 */
	IMPISP_MASK_TYPE mask_type;		/**< 填充数据类型 */
	IMP_ISP_COLOR_VALUE mask_value;  /**< 填充数据值 */
} IMPISP_MASK_BLOCK_PAR;

/**
 * 填充参数
 */
typedef struct {
	IMPISP_MASK_BLOCK_PAR mask_chx[3][4];	/**< 通道填充参数 */
} IMPISPMASKAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetMask(IMPVI_NUM num, IMPISPMASKAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mask  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetMask(IMPVI_NUM num, IMPISPMASKAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_GetMask(IMPVI_NUM num, IMPISPMASKAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetMask(IMPVI_NUM num, IMPISPMASKAttr *mask);

/**
 * 填充图片类型
 */
typedef enum {
	IMP_ISP_OSD_PIC_FILE,	/**< 图片文件 */
	IMP_ISP_OSD_PIC_ARRAY,	/**< 图片数组 */
	IMP_ISP_OSD_PIC_BUTT,	/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPOSDPicType;

/**
 * 填充图片参数
 */
typedef struct {
	uint8_t  osd_enable;    /**< 填充功能使能 */
	uint16_t osd_left;      /**< 填充横向起始点 */
	uint16_t osd_top;       /**< 填充纵向起始点 */
	uint16_t osd_width;     /**< 填充宽度 */
	uint16_t osd_height;    /**< 填充高度 */
	char *osd_image;		/**< 填充图片(文件路径/数组) */
	IMPISPOSDPicType osd_image_type; /**< 填充图片类型 */
	uint16_t osd_stride;    /**< 填充图片的对其宽度, 以字节为单位，例如320x240的RGBA8888图片osd_stride=320*4 */
} IMPISPOSDPicAttr;

/**
 * 填充图片格式
 */
typedef enum {
	IMP_ISP_PIC_ARGB_8888,  /**< ARGB8888 */
	IMP_ISP_PIC_ARGB_1555,  /**< ARBG1555 */
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
 * 填充功能通道属性
 */
typedef struct {
	IMPISPPICTYPE osd_type;                        /**< 填充图片类型  */
	IMPISPARGBType osd_argb_type;                  /**< 填充格式  */
	IMPISPTuningOpsMode osd_pixel_alpha_disable;   /**< 填充像素Alpha禁用功能使能  */
	IMPISPOSDPicAttr pic[8];                       /**< 填充图片属性，每个通道最多可以填充8张图片 */
} IMPISPOSDChxAttr;

/**
 * 填充功能属性
 */
typedef struct {
	IMPISPOSDChxAttr osd_chx[2];       /**< 通道0/1的填充属性 */
} IMPISPOSDAttr;

typedef struct {
	int chx;
	int pic_num;
	IMPISPPICTYPE osd_type;                        /**< 填充图片类型  */
	IMPISPARGBType osd_argb_type;                  /**< 填充格式  */
	IMPISPTuningOpsMode osd_pixel_alpha_disable;   /**< 填充像素Alpha禁用功能使能  */
	IMPISPOSDPicAttr pic;
} IMPISPSingleOSDAttr;

/**
 * @fn int32_t IMP_ISP_GetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_GetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr);

/**
 * 画窗功能属性
 */
typedef struct {
	uint8_t  wind_enable;           /**< 画窗功能使能 */
	uint16_t wind_left;             /**< 画窗功能横向起始点 */
	uint16_t wind_top;              /**< 画窗功能纵向起始点 */
	uint16_t wind_width;            /**< 画窗宽度 */
	uint16_t wind_height;           /**< 画窗高度 */
	IMP_ISP_COLOR_VALUE wind_color; /**< 画窗颜色 */
	uint8_t  wind_line_width;           /**< 窗口边框宽度 */
	uint8_t  wind_alpha;            /**< 宽口边框alpha（3bit） */
}IMPISPDrawWindAttr;

/**
 * 画四角窗功能属性
 */
typedef struct {
	uint8_t  rang_enable;           /**< 画四角窗功能使能 */
	uint16_t rang_left;             /**< 画四角窗功能横向起始点 */
	uint16_t rang_top;              /**< 画四角窗功能纵向起始点 */
	uint16_t rang_width;            /**< 画四角窗宽度 */
	uint16_t rang_height;           /**< 画四角窗高度 */
	IMP_ISP_COLOR_VALUE rang_color; /**< 画四角窗颜色 */
	uint8_t  rang_line_width;       /**< 画四角窗边框宽度 */
	uint8_t  rang_alpha;            /**< 四角窗边框alpha （3bit） */
	uint16_t rang_extend;           /**< 四角窗边框长度 */
} IMPISPDrawRangAttr;

/**
 * 画线功能属性
 */
typedef struct {
	uint8_t  line_enable;               /**< 画线功能使能 */
	uint16_t line_startx;               /**< 画线横向起始点 */
	uint16_t line_starty;               /**< 画线纵向起始点 */
	uint16_t line_endx;                 /**< 画线横向结束点 */
	uint16_t line_endy;                 /**< 画线纵向结束点 */
	IMP_ISP_COLOR_VALUE line_color;     /**< 线条颜色 */
	uint8_t  line_width;                /**< 线宽 */
	uint8_t  line_alpha;                /**< 线条Alpha值 */
} IMPISPDrawLineAttr;

/**
 * 画图功能类型
 */
typedef enum {
	IMP_ISP_DRAW_LINE,              /**< 画线 */
	IMP_ISP_DRAW_RANGE,             /**< 画四角窗 */
	IMP_ISP_DRAW_WIND,              /**< 画框 */
} IMPISPDrawType;

/**
 * 画图功能属性
 */
typedef struct {
	IMPISPDrawType type;                /**< 画图类型 */
	IMPISP_MASK_TYPE color_type;		/**< 填充数据类型 */
	union {
		IMPISPDrawWindAttr wind;    /**< 画框属性 */
		IMPISPDrawRangAttr rang;    /**< 画四角窗属性 */
		IMPISPDrawLineAttr line;    /**< 画线属性 */
	} draw_cfg;                         /**< 画图属性 */
} IMPISPDrawUnitAttr;

/**
 * 通道画图功能属性
 */
typedef struct {
	IMPISPDrawUnitAttr draw_chx[2][16];   /**< channel 0/1画图属性，最多可以画16个 */
} IMPISPDrawAttr;

/**
 * @fn int32_t IMP_ISP_GetDrawAttr(IMPVI_NUM num, IMPISPDrawAttr *attr)
 *
 * 获取绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_GetDrawAttr(IMPVI_NUM num, IMPISPDrawAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetDrawAttr(IMPVI_NUM num, IMPISPDrawAttr *attr)
 *
 * 设置绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetDrawAttr(IMPVI_NUM num, IMPISPDrawAttr *attr);

/**
 * Face AE功能属性
 */
typedef struct {
	IMPISPTuningOpsMode enable; /**< Face AE功能开关 */
	unsigned int left;   /**< Face AE区域左起始点 */
	unsigned int top;    /**< Face AE区域上起始点 */
	unsigned int right;  /**< Face AE区域右结束点 */
	unsigned int bottom; /**< Face AE区域下结束点 */
	unsigned int target; /**< Face AE区域目标亮度 */
} IMPISPFaceAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_GetFaceAe(IMPVI_NUM num, IMPISPFaceAttr *gaeattr)
 *
 * 获取人脸曝光功能参数.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] gaeattr  face ae参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetFaceAe(IMPVI_NUM num, IMPISPFaceAttr *gaeattr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetFaceAe(IMPVI_NUM num, IMPISPFaceAttr *saeattr)
 *
 * 设置人脸曝光功能参数.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] saeattr  face ae参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetFaceAe(IMPVI_NUM num, IMPISPFaceAttr *saeattr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetFaceAwb(IMPVI_NUM num, IMPISPFaceAttr *gawbattr)
 *
 * 获取人脸曝光功能参数.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] gawbattr  face awb参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetFaceAwb(IMPVI_NUM num, IMPISPFaceAttr *gawbattr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetFaceAwb(IMPVI_NUM num, IMPISPFaceAttr *sawbattr)
 *
 * 设置人脸曝光功能参数.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] sawbattr  face awb参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetFaceAwb(IMPVI_NUM num, IMPISPFaceAttr *sawbattr);

/**
 * 客户自定义自动曝光库的AE初始属性
 */
typedef struct {
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */

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
	IMPISPAEStatisAttr AeStatis;						/**< AE统计属性 */
} IMPISPAeInitAttr;

/**
 * 客户自定义自动曝光库的AE信息
 */
typedef struct {
	IMPISPAEStatisInfo ae_info;							/**< AE统计值 */
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
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
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeShortIntegrationTime;                    /**< AE手动模式下的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t luma;			       				/**< AE Luma值 */
	uint32_t luma_scence;		       				/**< AE 场景Luma值 */
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
 * @fn int32_t IMP_ISP_SetAeAlgoFunc(IMPVI_NUM num, IMPISPAeAlgoFunc *ae_func)
 *
 * 客户自定义自动曝光库的注册接口
 *
 * @param[in] num       对应sensor的标号
 * @param[in] ae_func   客户自定义AE库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)接口之后立刻调用。
 */
int32_t IMP_ISP_SetAeAlgoFunc(IMPVI_NUM num, IMPISPAeAlgoFunc *ae_func);

/**
 * 客户自定义自动白平衡库的AWB初始属性
 */
typedef struct {
	IMPISPAWBStatisAttr AwbStatis;							/**< AWB的初始统计属性 */
} IMPISPAwbInitAttr;

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
	IMPISPAWBStatisInfo awb_statis;						/**< 白平衡区域统计值 */
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
	void *priv_data;																		/**< 私有数据地址 */
	int (*open)(void *priv_data, IMPISPAwbInitAttr *AwbInitAttr);                           /**< 自定义AWB库开始接口 */
	void (*close)(void *priv_data);                                                         /**< 自定义AWB库关闭接口 */
	void (*handle)(void *priv_data, const IMPISPAwbInfo *AwbInfo, IMPISPAwbAttr *AwbAttr);  /**< 自定义AWB库的处理接口 */
	int (*notify)(void *priv_data, IMPISPAwbNotify notify, void *data);                     /**< 自定义AWB库的通知接口 */
} IMPISPAwbAlgoFunc;

/**
 * @fn int32_t IMP_ISP_SetAwbAlgoFunc(IMPVI_NUM num, IMPISPAwbAlgoFunc *awb_func)
 *
 * 客户自定义自动白平衡库的注册接口.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] awb_func  客户自定义AWB库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)接口之后立刻调用。
 */
int32_t IMP_ISP_SetAwbAlgoFunc(IMPVI_NUM num, IMPISPAwbAlgoFunc *awb_func);

/**
 * Tuning bin文件功能属性
 */
typedef struct {
	IMPISPTuningOpsMode enable;	 /**< Switch bin功能开关 */
	char bname[64];				 /**< bin文件的绝对路径 */
} IMPISPBinAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SwitchBin(IMPVI_NUM num, IMPISPBinAttr *attr)
 *
 * 切换Bin文件.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] attr     需要切换的bin文件属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SwitchBin(IMPVI_NUM num, IMPISPBinAttr *attr);

/**
 * WDR输出模式
 */
typedef enum {
        IMPISP_WDR_OUTPUT_MODE_FUS_FRAME,	/**< 混合模式 */
        IMPISP_WDR_OUTPUT_MODE_LONG_FRAME,	/**< 长帧模式 */
        IMPISP_WDR_OUTPUT_MODE_SHORT_FRAME,	/**< 短帧模式 */
        IMPISP_WDR_OUTPUT_MODE_BUTT,		/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPWdrOutputMode;

/**
 * @fn int32_t IMP_ISP_Tuning_SetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode)
 *
 * 设置WDR图像输出模式。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] mode	输出模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode);

/**
 * @fn int32_t IMP_ISP_Tuning_GetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode)
 *
 * 获取WDR图像输出模式。
 *
 * @param[in] num       对应sensor的标号
 * @param[out] mode	输出模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode);

/**
 * 丢帧参数
 */
typedef struct {
	IMPISPTuningOpsMode enable;	/**< 使能标志 */
        uint8_t lsize;			/**< 总数量(范围:0~31) */
        uint32_t fmark;			/**< 位标志(1输出，0丢失) */
} IMPISPFrameDrop;

/**
 * 丢帧属性
 */
typedef struct {
	IMPISPFrameDrop fdrop[3];	/**< 各个通道的丢帧参数 */
} IMPISPFrameDropAttr;

/**
 * @fn int32_t IMP_ISP_SetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr)
 *
 * 设置丢帧属性。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr	丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_SetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr)
 *
 * 获取丢帧属性。
 *
 * @param[in] num       对应sensor的标号
 * @param[out] attr	丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_GetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr);

/**
 * 缩放模式
 */
typedef enum {
        IMPISP_SCALER_FITTING_CRUVE,
        IMPISP_SCALER_FIXED_WEIGHT,
        IMPISP_SCALER_BUTT,
} IMPISPScalerMode;

/**
 * 缩放效果参数
 */
typedef struct {
        uint8_t chx;		/*通道 0~2*/
        IMPISPScalerMode mode;	/*缩放方法*/
        uint8_t level;		/*缩放清晰度等级 范围0~128*/
} IMPISPScalerLvAttr;

/**
 * @fn IMP_ISP_SetScalerLv(IMPVI_NUM num, IMPISPScalerLvAttr *attr)
 *
 * Set Scaler 缩放的方法及等级.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr	mscaler等级属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetScalerLv(IMPVI_NUM num, IMPISPScalerLvAttr *attr);

/**
 * @fn IMP_ISP_StartNightMode(IMPVI_NUM num)
 *
 * 起始夜视模式.
 *
 * @param[in] num       对应sensor的标号
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 使用范围：IMP_ISP_Open之后，IMP_ISP_AddSensor之前。
 */
int32_t IMP_ISP_StartNightMode(IMPVI_NUM num);

/**
 * @fn int32_t IMP_ISP_GetSingleOSDAttr(IMPVI_NUM num, IMPISPSingleOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_GetSingleOSDAttr(IMPVI_NUM num, IMPISPSingleOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetSingleOSDAttr(IMPVI_NUM num, IMPISPSingleOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetSingleOSDAttr(IMPVI_NUM num, IMPISPSingleOSDAttr *attr);

/**
 * raw属性和buffer属性。
 */
typedef  struct {
	uint32_t paddr;		/*  buffer paddr */
	uint32_t vaddr;		/*  buffer vaddr */
	uint32_t buf_size;	/*  buffer size */
}IMPISPRAWBUF;

typedef struct {
	IMPISPRAWBUF buf;			/* buf属性 */
	uint32_t raw_width;         /* sensor width */
	uint32_t raw_height;        /* sensor height */
	uint8_t raw_bit;            /* 输入参数仅支持 8,16 */ /* 8:get 8bit raw，16:get 16bit raw */
	unsigned short sensor_id;   /* 摄像头ID号 */
} IMPISPGETRawiConfig;

/**
 * @fn IMP_ISP_GetRaw(IMPVI_NUM num, IMPISPGETRawiConfig *attr)
 *
 * 应用层获取Raw图.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr      raw 属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_GetRaw(IMPVI_NUM num, IMPISPGETRawiConfig *attr);

/**
 * @fn IMP_ISP_SetPreDqtime(IMPVI_NUM num, uint32_t *dqtime)
 *
 * 设置单帧调度时软中断的触发延迟时间.
 *
 * @param[in] num       对应sensor的标号(预留)
 * @param[in] dqbuf     raw 属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 由于该接口不区分主次摄，所以num参数无需设置。
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_SetPreDqtime(IMPVI_NUM num, uint32_t *dqtime);

typedef struct {
        int16_t strength;       /**< 畸变矫正的强度[范围0~255，默认为128]*/
        int16_t width;          /**< 图像宽度 (set时此参数无需设置)*/
        int16_t height;         /**< 图像高度 (set时此参数无需设置)*/
        int16_t center_w;       /**< 图像畸变水平光学中心 取值:[width/2] (ps:如果目前使用分辨率是2560*1440 光学中心就是1280*720)*/
        int16_t center_h;       /**< 图像畸变垂直光学中心 取值:[height/2]*/
} IMPISPHLDCAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPHLDCAttr(IMPVI_NUM num, IMPISPHLDCAttr *hldc)
 *
 * 设置HLDC属性.
 *
 * @param[in] num 对应sensor的标号
 * @param[in] hldc HLDC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 *
 *1、分辨率大于1080p需要先set进去值再打开HLDC模块
 *2、这个HLDC在效果bin文件里面一开始默认状态是关闭 (ps：此时调用IMP_ISP_Tuning_GetModuleControl ，返回值是1，1代表bypass使能，相对应HLDC关闭)
 *3、请直接IMP_ISP_Tuning_GetHLDCAttr（图像宽高是1920*1080默认值），SET（三个值：强度，光学中心），GET（此时get到的图像宽高是你当前真实的分辨率）
 *4、强度越小，图像拉的越宽，强度越大，图像相对拉的越窄;128这个强度是默认的，开机第一次强度不要设置128，可以先设置其他值后，可再设置128（128就是不进行畸变矫正，可以选择关闭HLDC效果是一样的）
 */
int32_t IMP_ISP_Tuning_SetHLDCAttr(IMPVI_NUM num, IMPISPHLDCAttr *hldc);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPHLDCAttr(IMPVI_NUM num, IMPISPHLDCAttr *hldc)
 *
 * 获取HLDC属性.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] hldc      HLDC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetHLDCAttr(IMPVI_NUM num, IMPISPHLDCAttr *hldc);

/**
 * mjpeg固定对比度
 */
typedef struct {
	uint8_t mode;         /*当置1的时候，参数生效；参数为0，参数不生效*/
	uint8_t range_low;    /*可设置为16, 增加对比度效果，可增加该值*/
	uint8_t range_high;   /*可设置为235*/
} IMPISPFixedContrastAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetFixedContraster(IMPISPFixedContrastAttr *attr)
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
int32_t IMP_ISP_Tuning_SetFixedContraster(IMPVI_NUM num, IMPISPFixedContrastAttr *attr);

#define MAXSUPCHNMUN 2	/*ISPOSD最大支持两个通道，0为主码流通道，1为次码流通道*/
#define MAXISPOSDPIC 8	/*ISPOSD每个通道支持绘制的最大图片个数,目前最大只支持8个*/

typedef enum {
	IMP_ISP_OSD_RGN_FREE,	/*ISPOSD区域未创建或者释放*/
	IMP_ISP_OSD_RGN_BUSY,	/*ISPOSD区域已创建*/
}IMPIspOsdRngStat;

typedef enum {
	ISP_OSD_REG_INV		  = 0, /**< 未定义的 */
	ISP_OSD_REG_PIC 	  = 1, /**< ISP绘制图片*/
}IMPISPOSDType;
typedef struct IMPISPOSDNode IMPISPOSDNode;

/*ISPOSD属性集合*/
typedef struct {
	IMPISPOSDType type;
	union {
		IMPISPSingleOSDAttr stsinglepicAttr;/*pic 类型的ISPOSD*/
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
 * @remarks 无。
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
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
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
int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
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
int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_ShowOsdRgn( int chn,int handle, int showFlag)
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
int IMP_ISP_Tuning_ShowOsdRgn(int chn,int handle, int showFlag);

/**
 * @fn int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle)
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
