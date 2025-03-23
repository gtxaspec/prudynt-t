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
 * The header file of ISP
 */

/**
 * @defgroup IMP_ISP
 * @ingroup imp
 * @brief Image signal processing unit. It contains several key function, for example, image effects setting, night scene, sensor's operations and so on.
 *
 * ISP module is not related to the data flow, so no need to process Bind, Only used for effect parameters configuration and sensor controls.
 *
 * The ISP manipulation is as follow:
 * @code
 * int ret = 0;
 * ret = IMP_ISP_Open(); // step.1  create ISP module
 * if(ret < 0){
 *     printf("Failed to ISPInit\n");
 *     return -1;
 * }
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * sensor.cbus_type = SENSOR_CONTROL_INTERFACE_I2C; // OR SENSOR_CONTROL_INTERFACE_SPI
 * sensor.i2c = {
 * 	.type = "xxx", // I2C sets the name, this name has to be the same as the name of the sensor drivers in struct i2c_device_id.
 *	.addr = xx,	// the I2C address
 *	.i2c_adapter_id = xx, // The value is the I2C adapter ID.
 * }
 * OR
 * sensor.spi = {
 *	.modalias = "xx", // SPI sets the name, this name has to be the same as the name of the sensor drivers in struct i2c_device_id.
 *	.bus_num = xx, // It is the address of SPI bus.
 * }
 * ret = IMP_ISP_AddSensor(&sensor); //step.2, add a sensor. Before the function is called, the sensor driver has to be registered into kernel.
 * if (ret < 0) {
 *     printf("Failed to Register sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableSensor(void); //step.3, Enable sensor and sensor starts to output image.
 * if (ret < 0) {
 *     printf("Failed to EnableSensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableTuning(); //step.4, Enable ISP tuning, then you can use ISP debug interface.
 * if (ret < 0) {
 *     printf("Failed to EnableTuning\n");
 *     return -1;
 * }
 *
 * Debug interface, please refer to the ISP debug interface documentation //step.5 Effect of debugging.
 *
 * @endcode
 * The process which uninstall(disable)ISP is as follows:
 * @code
 * int ret = 0;
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * ret = IMP_ISP_DisableTuning(); //step.1 Turn off ISP tuning
 * if (ret < 0) {
 *     printf("Failed to disable tuning\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DisableSensor(); //step.2, Turn off sensor, Note that sensor will stop output pictures, so that all FrameSource should be closed.
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DelSensor(&sensor); //step.3, Delete sensor, before that step, the sensor has to be stopped.
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_Close(); //step.4, After deleting all sensors, you can run this interface to clean up the ISP module.
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 * @endcode
 * There are more examples in the samples.
 * @{
 */

/**
* The enum is types of sensor control bus.
*/
typedef enum {
	TX_SENSOR_CONTROL_INTERFACE_I2C = 1,	/**< I2C control bus */
	TX_SENSOR_CONTROL_INTERFACE_SPI,	/**< SPI control bus */
} IMPSensorControlBusType;

/**
* Defines I2C bus information
*/
typedef struct {
	char type[20];		/**< Set the name, the value must be match with sensor name in 'struct i2c_device_id' */
	int addr;		/**< the I2C address */
	int i2c_adapter_id;	/**< I2C adapter ID */
} IMPI2CInfo;
/**
* Defines SPI bus information
*/
typedef struct {
	char modalias[32];	/**< Set the name, the value must be match with sensor name in 'struct i2c_device_id' */
	int bus_num;		/**< Address of SPI bus */
} IMPSPIInfo;

/**
* Defines the information of sensor
*/
typedef struct {
	char name[32];					/**< the sensor name */
	uint16_t sensor_id;				    	/**< the camera ID  */
	IMPSensorControlBusType cbus_type;		/**< the sensor control bus type */
	union {
		IMPI2CInfo i2c;				/**< I2C bus information */
		IMPSPIInfo spi;				/**< SPI bus information */
	};
	unsigned short rst_gpio;		/**< The reset pin of sensor, but it is invalid now. */
	unsigned short pwdn_gpio;		/**< The power down pin of sensor, but it is invalid now. */
	unsigned short power_gpio;		/**< The power pin of sensor, but it is invalid now. */
} IMPSensorInfo;

/**
 * @fn int IMP_ISP_Open(void)
 *
 * Open the ISP module
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark After calling the function,it first creates the ISP module, then prepares to add sensor to ISP, and starts the ISP effect debugging function.
 *
 * @attention Before adding sensor image, this function must be called firstly.
 */
int IMP_ISP_Open(void);

/**
 * @fn int IMP_ISP_Close(void)
 *
 * Close the ISP module
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark After calling the function, ISP will stop working.
 *
 * @attention Before calling this function, make sure that all FrameSources and effect debugging functions are off(disabled), and all sensors are deleted.
 */
int IMP_ISP_Close(void);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath(char *path)
 *
 * Sets the default path to the ISP bin file.
 *
 * @param[in] path  The bin file path property to set.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Sets the absolute path to the Bin file when the user-defined ISP is started.
 *
 * @attention This function must be called before adding the sensor and after opening the ISP.
 */
int32_t IMP_ISP_SetDefaultBinPath(char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath(char *path)
 *
 * Gets the default path to the ISP bin file.
 *
 * @param[out] path  The bin file path property to get.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Get the absolute path to the Bin file when the user-defined ISP is started.
 *
 * @attention This function must be called after the sensor is added.
 * @attention Only bin file path attributes for a single ISP can be retrieved at a time.
 */
int32_t IMP_ISP_GetDefaultBinPath(char *path);

/**
 * @fn int IMP_ISP_AddSensor(IMPSensorInfo *pinfo)
 *
 * Add a sensor into ISP module.
 *
 * @param[in] pinfo The pointer for the sensor information.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The sensor will be used to capture image.
 *
 * @attention Before using this function, you must ensure that the camera driver has been registered into the kernel.
 */
int IMP_ISP_AddSensor(IMPSensorInfo *pinfo);

/**
 * @fn int IMP_ISP_DelSensor(IMPSensorInfo *pinfo)
 *
 * Delete a sensor from ISP module.
 *
 * @param[in] pinfo The pointer for the sensor information
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Delete a sensor(image sensor which would be a camera)
 *
 * @attention Before using this function, you must ensure that the sensor has been stopped working, use IMP_ISP_DisableSensor function to do so.
 */
int IMP_ISP_DelSensor(IMPSensorInfo *pinfo);

/**
 * @fn int IMP_ISP_EnableSensor(void)
 *
 * Enable the registered sensor.
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Enable the registered sensor, then FrameSource can ouput image and ISP tuns on the image effects.
 *
 * @attention Before using this function, you must ensure that the sensor is already registered into ISP module.
 */
int IMP_ISP_EnableSensor(void);

/**
 * @fn int IMP_ISP_DisableSensor(void)
 *
 * Disable the running sensor.
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark  if a sensor is not used, FrameSource and ISP won't be working either.
 *
 * @attention Before using this function, you must ensure that the Framesource and ISP have stopped working.
 */
int IMP_ISP_DisableSensor(void);

/**
 * @fn int IMP_ISP_SetSensorRegister(uint32_t reg, uint32_t value)
 *
 * Set the value of a register of a sensor.
 *
 * @param[in] reg 	The address of the register.
 *
 * @param[in] value 	The value of the register.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Value of a register can be directly set.
 *
 * @attention Before using this function, you must ensure that the sensor is working, so it will be able to be configured or set.
 */
int IMP_ISP_SetSensorRegister(uint32_t reg, uint32_t value);

/**
 * @fn int IMP_ISP_GetSensorRegister(uint32_t reg, uint32_t *value)
 *
 * Obtain a value of the register of sensor.
 *
 * @param[in] reg 	The address of the register.
 *
 * @param[in] value 	The pointer of register value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark You can directly obtain the value of the sensor's register.
 *
 * @attention Before using this function, you must ensure that the sensor is working.
 */
int IMP_ISP_GetSensorRegister(uint32_t reg, uint32_t *value);

/**
 * ISP OPTION MODE parameter structure.
 */

typedef enum {
	IMPISP_TUNING_OPS_MODE_DISABLE,			/**< DISABLE mode of the current module */
	IMPISP_TUNING_OPS_MODE_ENABLE,			/**< ENABLE mode of the current module */
	IMPISP_TUNING_OPS_MODE_BUTT,			/**< effect paramater, parameters have to be less than this value*/
} IMPISPTuningOpsMode;

/**
 * ISP MODE property parameter structure.
 */
typedef enum {
	IMPISP_TUNING_OPS_TYPE_AUTO,			/**< AUTO mode of the current module*/
	IMPISP_TUNING_OPS_TYPE_MANUAL,			/**< MANUAL mode of the current module*/
	IMPISP_TUNING_OPS_TYPE_BUTT,			/**< effect paramater, parameters have to be less than this value*/
} IMPISPTuningOpsType;

typedef struct {
	unsigned int zone[15][15];    /**< zone info*/
}  __attribute__((packed, aligned(1))) IMPISPZone;

/**
 * ISP AutoZoom Attribution
 */
typedef struct {
	int chan;           /** <channel num> */
	int scaler_enable;  /** <scaler function enable> */
	int scaler_outwidth;/** <output picture width after scaler> */
	int scaler_outheight;/** <output picture height after scaler> */
	int crop_enable;      /** <crop function enable> */
	int crop_left;        /** <crop starting abscissa> */
	int crop_top;         /** <crop starting ordinate> */
	int crop_width;       /** <output width after crop> */
	int crop_height;      /** <output height after crop> */
} IMPISPAutoZoom;

/**
 * @fn int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom)
 *
 * setting Auto zoom parameters
 *
 * @param[in] Setting parameters for changing resolution
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom);

/**
 * @fn int IMP_ISP_EnableTuning(void)
 *
 * Enable effect debugging of ISP
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int IMP_ISP_EnableTuning(void);

/**
 * @fn int IMP_ISP_DisableTuning(void)
 *
 * Disable effect debugging of ISP
 *
 * @param none
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention First you must ensure that ISP is no longer working, then stop the sensor, after that you can use this function.
 */
int IMP_ISP_DisableTuning(void);

/**
 * @fn int IMP_ISP_Tuning_SetSensorFPS(uint32_t fps_num, uint32_t fps_den)
 *
 * Set the FPS of enabled sensor.
 *
 * @param[in] fps_num 	The numerator value of FPS.
 * @param[in] fps_den 	The denominator value of FPS.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 */
int IMP_ISP_Tuning_SetSensorFPS(uint32_t fps_num, uint32_t fps_den);

/**
 * @fn int IMP_ISP_Tuning_GetSensorFPS(uint32_t *fps_num, uint32_t *fps_den)
 *
 * Get the FPS of enabled sensor.
 *
 * @param[in] fps_num The pointer for numerator value of FPS.
 * @param[in] fps_den The pointer for denominator value of FPS.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 * @attention Before starting data transmission in a Channel, you must first call this function in order to obtain the sensor's default FPS.
 */
int IMP_ISP_Tuning_GetSensorFPS(uint32_t *fps_num, uint32_t *fps_den);

/**
 * ISP Anti-flicker property parameter structure.
 */
typedef enum {
	IMPISP_ANTIFLICKER_DISABLE,	/**< Disable antiflicker module */
	IMPISP_ANTIFLICKER_50HZ,	/**< Enable antiflicker module and set the frequency to 50HZ */
	IMPISP_ANTIFLICKER_60HZ,	/**< Enable antiflicker module and set the frequencye to 60HZ */
	IMPISP_ANTIFLICKER_BUTT,	/**< effect parameter, parameters have to be less than this value*/
} IMPISPAntiflickerAttr;

/**
 * @fn int IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISPAntiflickerAttr attr)
 *
 * Set the antiflicker parameter.
 *
 * @param[in] attr 	The value for antiflicker mode
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before calling this function, make sure that ISP debugging function is working.
 */
int IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISPAntiflickerAttr attr);

/**
 * @fn int IMP_ISP_Tuning_GetAntiFlickerAttr(IMPISPAntiflickerAttr *pattr)
 *
 * Get the mode of antiflicker
 *
 * @param[in] pattr The pointer for antiflicker mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before calling this function, make sure that ISP debugging function is working.
 */
int IMP_ISP_Tuning_GetAntiFlickerAttr(IMPISPAntiflickerAttr *pattr);

/**
 * @fn int IMP_ISP_Tuning_SetBrightness(unsigned char bright)
 *
 * Set the brightness of image effect.
 *
 * @param[in] bright The value for brightness.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 that means increase brightness, and less than 128 that means decrease brightness.\n
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetBrightness(unsigned char bright);

/**
 * @fn int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright)
 *
 * Get the brightness of image effect.
 *
 * @param[in] pbright The pointer for brightness value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase brightness), and less than 128 (decrease brightness).\n
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetBrightness(unsigned char *pbright);

/**
 * @fn int IMP_ISP_Tuning_SetContrast(unsigned char contrast)
 *
 * Set the contrast of image effect.
 *
 * @param[in] contrast 		The value for contrast.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase contrast), and less than 128 (decrease contrast).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetContrast(unsigned char contrast);

/**
 * @fn int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast)
 *
 * Get the contrast of image effect.
 *
 * @param[in] pcontrast 	The pointer for contrast.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase contrast), and less than 128 (decrease contrast).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetContrast(unsigned char *pcontrast);

/**
 * @fn int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness)
 *
 * Set the sharpness of image effect.
 *
 * @param[in] sharpness 	The value for sharpening strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase sharpening), and less than 128 (decrease sharpening).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetSharpness(unsigned char sharpness);

/**
 * @fn int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness)
 *
 * Get the sharpness of image effect.
 *
 * @param[in] psharpness 	The pointer for sharpness strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase sharpening), and less than 128 (decrease sharpening).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSharpness(unsigned char *psharpness);

/**
 * @fn int IMP_ISP_Tuning_SetBcshHue(unsigned char hue)
 *
 * Set the hue of image color.
 *
 * @param[in] hue The value of hue, range from 0 to 255, default 128.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 that means increase hue, and less than 128 that means decrease hue.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetBcshHue(unsigned char hue);

/**
 * @fn int IMP_ISP_Tuning_GetBcshHue(unsigned char *hue)
 *
 * Get the hue of image color.
 *
 * @param[in] hue The pointer for hue value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase hue), and less than 128 (decrease hue).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetBcshHue(unsigned char *hue);

/**
 * @fn int IMP_ISP_Tuning_SetSaturation(unsigned char sat)
 *
 * Set the saturation of image effect.
 *
 * @param[in] sat 	The value for saturation strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark  The default value is 128, more than 128 (increase saturation), and less than 128 (decrease saturation).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetSaturation(unsigned char sat);

/**
 * @fn int IMP_ISP_Tuning_GetSaturation(unsigned char *psat)
 *
 * Get the saturation of image effect.
 *
 * @param[in] psat	 The pointer for saturation strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark  The default value is 128, more than 128 (increase saturation), and less than 128 (decrease saturation).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSaturation(unsigned char *psat);

/**
 * @fn int IMP_ISP_Tuning_SetISPBypass(IMPISPTuningOpsMode enable)
 *
 * Control ISP modules.
 *
 * @param[in] enable 	bypass output mode (yes / no)
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark none
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetISPBypass(IMPISPTuningOpsMode enable);

/**
 * @fn int IMP_ISP_Tuning_GetTotalGain(uint32_t *gain)
 *
 * Get the overall gain value of the ISP output image
 *
 * @param[in] gain 	The pointer of total gain value, its format is [24.8], 24 (integer), 8(decimal)
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 */
int IMP_ISP_Tuning_GetTotalGain(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetISPHflip(IMPISPTuningOpsMode mode)
 *
 * Set ISP image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] mode 	The hflip (enable/disable).
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPHflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPHflip(IMPISPTuningOpsMode *pmode)
 *
 * Get ISP image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the hflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPHflip(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPVflip(IMPISPTuningOpsMode mode)
 *
 * Set ISP image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] mode 	The vflip enable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPVflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPVflip(IMPISPTuningOpsMode *pmode)
 *
 * Get ISP image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the vflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPVflip(IMPISPTuningOpsMode *pmode);


/**
 * @fn int IMP_ISP_Tuning_SetSensorHflip(IMPISPTuningOpsMode mode);
 *
 * Set Sensor image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] mode 	The hflip (enable/disable).
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSensorHflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetSensorHflip(IMPISPTuningOpsMode *pmode)
 *
 * Get Sensor image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the hflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetSensorHflip(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetSensorVflip(IMPISPTuningOpsMode mode);
 *
 * Set Sensor image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] mode 	The vflip enable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSensorVflip(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetSensorVflip(IMPISPTuningOpsMode *pmode);
 *
 * Get Sensor image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the vflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetSensorVflip(IMPISPTuningOpsMode *pmode);
/**
 * Defines the enumeration of ISP working mode.
 */
typedef enum {
	IMPISP_RUNNING_MODE_DAY = 0,				/**< ISP day mode */
	IMPISP_RUNNING_MODE_NIGHT = 1,				/**< ISP night mode */
	IMPISP_RUNNING_MODE_BUTT,				/**< maximum value */
} IMPISPRunningMode;

/**
 * @fn int IMP_ISP_Tuning_SetISPRunningMode(IMPISPRunningMode mode)
 *
 * Set ISP running mode, normal mode or night vision mode; default mode: normal mode.
 *
 * @param[in] mode  running mode parameter
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * Example:
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
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPRunningMode(IMPISPRunningMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPRunningMode(IMPISPRunningMode *pmode)
 *
 * Get ISP running mode, normal mode or night vision mode;
 *
 * @param[in] pmode The pointer of the running mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPRunningMode(IMPISPRunningMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPCustomMode(IMPISPTuningOpsMode mode)
 *
 * Enable ISP custom mode, load another set of parameters.
 *
 * @param[in] mode Custom mode, enable or disable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetISPCustomMode(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPCustomMode(IMPISPTuningOpsMode mode)
 *
 * get ISP custom mode
 *
 * @param[out] mode Custom mode, enable or disable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetISPCustomMode(IMPISPTuningOpsMode *pmode);

/**
 * Defines the attribute of gamma.
 */
typedef struct {
	uint16_t gamma[129];		/**< The array of gamma attribute has 129 elements */
} IMPISPGamma;

/**
* @fn int IMP_ISP_Tuning_SetGamma(IMPISPGamma *gamma)
*
* Sets the attributes of ISP gamma.
*
* @param[in] gamma 	The pointer of the attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_SetGamma(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_GetGamma(IMPISPGamma *gamma)
*
* Obtains the attributes of gamma.
*
* @param[out] gamma 	The address of the attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetGamma(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_SetAeComp(int comp)
*
* Setting AE compensation.AE compensation parameters can adjust the target of the image AE.
* the recommended value range is from 0 to 255.
*
* @param[in] comp 	compensation parameter.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_SetAeComp(int comp);

/**
* @fn int IMP_ISP_Tuning_GetAeComp(int *comp)
*
* Obtains the compensation of AE.
*
* @param[out] comp 	The pointer of the compensation.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetAeComp(int *comp);

/**
* @fn int IMP_ISP_Tuning_GetAeLuma(int *luma)
*
* Obtains the AE luma of current frame.
*
* @param[out] luma AE luma parameter.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetAeLuma(int *luma);

/**
 * @fn int IMP_ISP_Tuning_SetAeFreeze(IMPISPTuningOpsMode mode)
 *
 * AE Freeze.
 *
 * @param[in] mode AE Freeze mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeFreeze(IMPISPTuningOpsMode mode);

/**
 * exposure mode.
 */
enum isp_core_expr_mode {
	ISP_CORE_EXPR_MODE_AUTO,			/**< Auto exposure mode */
	ISP_CORE_EXPR_MODE_MANUAL,			/**< Manual exposure mode */
};

/**
 * exposure unit.
 */
enum isp_core_expr_unit {
	ISP_CORE_EXPR_UNIT_LINE,			/**< The unit is integration line */
	ISP_CORE_EXPR_UNIT_US,				/**< The unit is millisecond */
};

/**
 * exposure parameters.
 */
typedef union isp_core_expr_attr{
	struct {
		enum isp_core_expr_mode mode;		/**< set the exposure mode */
		enum isp_core_expr_unit unit;		/**< set the exposure unit */
		uint16_t time;
	} s_attr;
	struct {
		enum isp_core_expr_mode mode;			/**< exposure mode obtained */
		uint16_t integration_time;				/**< The integration time, the unit is line. */
		uint16_t integration_time_min;			/**< The min value of integration time, the unit is line. */
		uint16_t integration_time_max;			/**< The max value of integration time, the unit is line. */
		uint16_t one_line_expr_in_us;			/**< A integration line correspond to the time (ms) */
	} g_attr;
}IMPISPExpr;


/**
 * @fn int IMP_ISP_Tuning_SetExpr(IMPISPExpr *expr)
 *
 * Set AE attributes.
 *
 * @param[in] expr 	The pointer for exposure attributes.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetExpr(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_GetExpr(IMPISPExpr *expr)
 *
 * Get AE attributes.
 *
 * @param[in] expr The pointer for exposure attributes.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetExpr(IMPISPExpr *expr);

/**
 * exposure region of interest.
 */
typedef union isp_core_ae_roi_select{
	struct {
		unsigned endy :8;                   /**< y coordinate(end), the range is from 0 to 255.*/
		unsigned endx :8;                   /**< x coordinate(end), the range is from 0 to 255.*/
		unsigned starty :8;                 /**< y coordinate(start), the range is from 0 to 255.*/
		unsigned startx :8;                 /**< x coordinate(start), the range is from 0 to 255.*/
	};
	uint32_t value;
} IMPISPAERoi;

/**
 * White balance mode.
 */
enum isp_core_wb_mode {
	ISP_CORE_WB_MODE_AUTO = 0,			/**< Auto WB mode */
	ISP_CORE_WB_MODE_MANUAL,			/**< Manual WB mode */
	ISP_CORE_WB_MODE_DAY_LIGHT,			/**< Day-light mode */
	ISP_CORE_WB_MODE_CLOUDY,			/**< Cloudy day mode */
	ISP_CORE_WB_MODE_INCANDESCENT,		/**< Incandescent mode */
	ISP_CORE_WB_MODE_FLOURESCENT,		/**< Fluorescent mode */
	ISP_CORE_WB_MODE_TWILIGHT,			/**< Twilight mode */
	ISP_CORE_WB_MODE_SHADE,				/**< Shade mode */
	ISP_CORE_WB_MODE_WARM_FLOURESCENT,	/**< Warm color fluorescent mode */
	ISP_CORE_WB_MODE_CUSTOM,	/**< Custom mode */
};

/**
 * White balance attributes.
 */
typedef struct isp_core_wb_attr{
	enum isp_core_wb_mode mode;			/**< The mode for WB; auto and manual mode */
	uint16_t rgain;					/**< red gain attribute, manual mode is effective*/
	uint16_t bgain;					/**< blue gain attribute, manual mode is effective*/
}IMPISPWB;

/**
 * @fn int IMP_ISP_Tuning_SetWB(IMPISPWB *wb)
 *
 * Set the white balance function settings. You can set the automatic and manual mode, manual mode is achieved mainly through setting of bgain, rgain.
 *
 * @param[in] wb 	The pointer for WB attribute.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetWB(IMPISPWB *wb);

/**
 * @fn int IMP_ISP_Tuning_GetWB(IMPISPWB *wb)
 *
 * Get the white balance function settings
 *
 * @param[in] wb 	The pointer for WB attribute.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_Statis(IMPISPWB *wb)
 *
 * Get the white balance statistic value.
 *
 * @param[out] wb 	The pointer for the statistic.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB_Statis(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_GOL_Statis(IMPISPWB *wb)
 *
 * Get the white balance global statistic value.
 *
 * @param[out] wb 	The pointer for the statistic.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB_GOL_Statis(IMPISPWB *wb);

/*
 * ISP AWB CLuster Mode Parameters structure
 */
typedef struct {
	IMPISPTuningOpsMode ClusterEn;      /* Cluster AWB Enable ctrl*/
	IMPISPTuningOpsMode ToleranceEn;    /* AWB Tolerance mode Enable ctrl */
	unsigned int tolerance_th;          /* AWB Tolerance Threshold, range 0~64*/
	unsigned int awb_cluster[7];        /* Cluster AWB Parameters Array*/
}IMPISPAWBCluster;

/**
 * @fn int IMP_ISP_Tuning_SetAwbClust(IMPISPAWBCluster *awb_cluster);
 *
 * Set Cluster AWB mode Parameters.
 *
 * @param[in] awb_cluster  contains cluster awb mode parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetAwbClust(IMPISPAWBCluster *awb_cluster);

/**
 * @fn int IMP_ISP_Tuning_GetAwbClust(IMPISPAWBCluster *awb_cluster);
 *
 * Get Cluster AWB mode Parameters.
 *
 * @param[out] awb_cluster  contains cluster awb mode parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetAwbClust(IMPISPAWBCluster *awb_cluster);

/*
 *  ISP AWB Ct Trend Parameters
 */
typedef struct {
	unsigned int trend_array[6];	/* rg offset & bg offset of hight middle low ct */
}IMPISPAWBCtTrend;

/**
 * @fn int IMP_ISP_Tuning_SetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

 *
 * Set rg bg offset under different ct.
 *
 * @param[in] ct_trend  contains ct offset parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

/**
 * @fn int IMP_ISP_Tuning_GetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

 *
 * Get rg bg offset under different ct.
 *
 * @param[out] ct_trend  contains current ct offset parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetAwbCtTrend(IMPISPAWBCtTrend *ct_trend);

/**
 * ISP WB COEFFT parameter structure.
 */
typedef struct isp_core_rgb_coefft_wb_attr {
	unsigned short rgb_coefft_wb_r;      /**< rgain offset */
	unsigned short rgb_coefft_wb_g;      /**< ggain offset */
	unsigned short rgb_coefft_wb_b;      /**< bgain offset */

}IMPISPCOEFFTWB;

/**
 * @fn int IMP_ISP_Tuning_Awb_GetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);
 *
 * Set the AWB r g b channel offset source in ISP.
 *
 * @param[out] isp_wb_attr  The pointer for the attributes
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_Awb_GetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn int IMP_ISP_Tuning_Awb_SetRgbCoefft(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * Sets the Max value of sensor color r g b.
 *
 * @param[in] gain  The value for sensor sensor color r g b..
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
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
 * Sets the Max value of sensor analog gain.
 *
 * @param[in] gain  The value for sensor analog gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetMaxAgain(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain)
 *
 * Get the Max value of sensor analog gain.
 *
 * @param[in] gain  The pointer for sensor analog gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetMaxAgain(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain)
 *
 * Set the Max value of sensor Digital gain.
 *
 * @param[in] gain 	The pointer for sensor digital gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 * Sets the Max value of isp digital gain.。
 *
 * @param[in] gain The value for isp digital gain. The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @attention When the interface is called, 'IMP_ISP_EnableTuning' has returned successfully.
 */
int IMP_ISP_Tuning_SetMaxDgain(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain)
 *
 * Get the Max value of sensor Digital gain.
 *
 * @param[out] gain 	The pointer for sensor digital gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain);

/**
 * Obtains the Max value of isp digital gain.
 *
 * @param[out] gain The pointer for isp digital gain. The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @attention When the interface is called, 'IMP_ISP_EnableTuning' has returned successfully.
 */
int IMP_ISP_Tuning_GetMaxDgain(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void))
 *
 * Set the video loss function. When there is a problem with the connection line of the sensor board, the callback function will be executed.
 *
 * @param[in] cb 	The pointer for callback function.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void));

/**
 * @fn int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength)
 *
 * Set highlight intensity controls.
 *
 * @param[in] strength 	Highlight control parameter, the value range is [0-10], set to 0 means disable the current function.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetHiLightDepress(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength)
 *
 * Get the strength of high light depress.
 *
 * @param[out] strength 	The pointer for hilight depress strength.
 * The value of 0 corresponds to disable.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetHiLightDepress(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetBacklightComp(uint32_t strength)
 *
 * Set backlight intensity controls.
 *
 * @param[in] strength 	Backlight control parameter, the value range is [0-10], set to 0 means disable the current function.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetBacklightComp(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetBacklightComp(uint32_t *strength)
 *
 * Get the strength of backlight compensation.
 *
 * @param[out] strength 	The pointer for backlight compensation strength.
 * The value of 0 corresponds to disable.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetBacklightComp(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio)
 *
 * Set 3D noise reduction intensity
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the temper value. If it is less than 128, that means decreaing the temper value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetTemperStrength(uint32_t ratio);

/**
 * @fn int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio)
 *
 * Set 2D noise reduction intensity
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the sinter value. If it is less than 128, that means decreaing the sinter value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSinterStrength(uint32_t ratio);

/**
 * attributes of ISP exposure value.
 */
typedef struct {
	uint32_t ev;			/**< exposure value*/
	uint32_t expr_us;		/**< exposure time in millisecond */
	uint32_t ev_log2;		/**< exposure time in log2 format */
	uint32_t again;			/**< Analog gain */
	uint32_t dgain;			/**< Digital gain */
	uint32_t gain_log2;		/**< Gain in log2 format */
}IMPISPEVAttr;

/**
* @fn int IMP_ISP_Tuning_GetEVAttr(IMPISPEVAttr *attr)
*
* Obtains the attributes of exposure value.
* @param[out] attr 	The pointer for attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetEVAttr(IMPISPEVAttr *attr);

/**
* @fn int IMP_ISP_Tuning_EnableMovestate(void)
*
* When the sensor will motion, it should be called.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_EnableMovestate(void);

/**
* @fn IMP_ISP_Tuning_DisableMovestate(void)
*
* When the sensor is from motion to still, it should be called.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_Tuning_EnableMovestate' is working properly.
*/
int IMP_ISP_Tuning_DisableMovestate(void);

/**
 * Mode selection
 */
typedef enum {
	IMPISP_TUNING_MODE_AUTO,    /**< AUTO mode of the current module */
	IMPISP_TUNING_MODE_MANUAL,    /**< MANUAL mode of the current module */
	IMPISP_TUNING_MODE_RANGE,    /**< Set the range of current module */
	IMPISP_TUNING_MODE_BUTT,    /**< effect paramater, parameters have to be less than this value */
} IMPISPTuningMode;

/**
 * Weight information
 */
typedef struct {
	unsigned char weight[15][15];	 /**< The weight info of each zone [0 ~ 8]*/
} IMPISPWeight;

/**
 * @fn int IMP_ISP_Tuning_SetAeWeight(IMPISPWeight *ae_weight)
 *
 * Set zone weighting for AE target
 *
 * @param[in] ae_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeWeight(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAeWeight(IMPISPWeight *ae_weight)
 *
 * Get zone weighting for AE target
 *
 * @param[out] ae_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeWeight(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_GetROI(IMPISPWeight *roi_weight)
 *
 * Set roi weighting for AE SCENE judgement
 *
 * @param[out] roi_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_AE_GetROI(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_SetROI(IMPISPWeight *roi_weight)
 *
 * Set roi weighting for AE SCENE judgement
 *
 * @param[in] roi_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_AE_SetROI(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_SetAwbWeight(IMPISPWeight *awb_weight)
 *
 * Set zone weighting for AWB
 *
 * @param[in] awb_weight awb weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbWeight(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbWeight(IMPISPWeight *awb_weight)
 *
 * Get zone weighting for AWB
 *
 * @param[out] awb_weight awb weight。
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAwbWeight(IMPISPWeight *awb_weight);

/**
 * AWB statistics
 */
typedef struct {
	unsigned char zone_r[225]; /**< 15*15 statistical average of each zone in R channel*/
	unsigned char zone_g[225]; /**< 15*15 statistical average of each zone in G channel*/
	unsigned char zone_b[225]; /**< 15*15 statistical average of each zone in B channel*/
} IMPISPAWBZone;
/**
 * @fn int IMP_ISP_Tuning_GetAwbZone(IMPISPAWBZONE *awb_zone)
 *
 * Get WB zone statistical average in R G B channel
 *
 * @param[out] awb_zone wb statistics。
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAwbZone(IMPISPAWBZone *awb_zone);

/**
 * AWB algorithm
 */
typedef enum {
	IMPISP_AWB_ALGO_NORMAL = 0, /*normal mode, use effective pixels for statistics*/
	IMPISP_AWB_ALGO_GRAYWORLD, /*grayworld mode, use all pixels for statistics*/
	IMPISP_AWB_ALGO_REWEIGHT, /*reweight for different color temperature*/
} IMPISPAWBAlgo;

/**
 * @fn int IMP_ISP_Tuning_SetWB_ALGO(IMPISPAWBALGO wb_algo)
 *
 * Set AWB algorithm for different application situation
 *
 * @param[in] awb algorithm
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */

int IMP_ISP_Tuning_SetWB_ALGO(IMPISPAWBAlgo wb_algo);

/**
 * AE statistics
 */
typedef struct {
	unsigned char ae_histhresh[4];	  /**< AE Histogram threshold for bin boundary.[0 ~ 255] */
	unsigned short ae_hist[5];    /**< Normalized histogram results for bin.[0 ~ 65535] */
	unsigned char ae_stat_nodeh;	/**< Number of active zones horizontally for AE stats collection.[0 ~ 15]*/
	unsigned char ae_stat_nodev;	/**< Number of active zones vertically for AE stats collection.[0 ~ 15]*/
} IMPISPAEHist;

/**
 * AE statistics
 */
typedef struct {
	unsigned int ae_hist[256];    /**< AE histogram results for 256 bin*/
} IMPISPAEHistOrigin;

/**
 * @fn int IMP_ISP_Tuning_SetAeHist(IMPISPAEHist *ae_hist)
 *
 * Set AE statistics parameters
 *
 * @param[in] ae_hist AE statictics parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeHist(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist(IMPISPAEHist *ae_hist)
 *
 * Get AE statistics information.
 *
 * @param[out] ae_hist AE statistics
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeHist(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Origin(IMPISPAEHistOrigin *ae_hist)
 *
 * Get AE 256 bin statistics information.
 *
 * @param[out] ae_hist AE statistics
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeHist_Origin(IMPISPAEHistOrigin *ae_hist);

/**
 * AWB statistics
 */
struct isp_core_awb_sta_info{
	unsigned short r_gain;	  /**< AWB statistics R/G color ratio output [0 ~ 4095] 4.8bit fixed-point */
	unsigned short b_gain;	  /**< AWB statistics B/G color ratio output [0 ~ 4095] 4.8bit fixed-point */
	unsigned int awb_sum;	 /**< Number of pixels used for AWB statistics [0 ~ 4294967295] */
};
/**
 * AWB statictis mode
 */
enum isp_core_awb_stats_mode{
	IMPISP_AWB_STATS_LEGACY_MODE = 0,    /**< Legacy mode */
	IMPISP_AWB_STATS_CURRENT_MODE = 1,    /**< Current mode */
	IMPISP_AWB_STATS_MODE_BUTT,
};
/**
 * AWB statictis parameters
 */
typedef struct {
	struct isp_core_awb_sta_info awb_stat;	  /**< AWB statistics */
	enum isp_core_awb_stats_mode awb_stats_mode;	/**< AWB statistic mode */
	unsigned short awb_whitelevel;	  /**< Upper limit of valid data for AWB [0 ~ 1023]*/
	unsigned short awb_blacklevel;	  /**< lower limit of valid data for AWB [0 ~ 1023]*/
	unsigned short cr_ref_max;    /**< Maximum value of R/G for white region [0 ~ 4095] 4.8bit fixed-point*/
	unsigned short cr_ref_min;    /**< Minimum value of R/G for white region [0 ~ 4095] 4.8bit fixed-point */
	unsigned short cb_ref_max;    /**< Maximum value of B/G for white region [0 ~ 4095] 4.8bit fixed-point  */
	unsigned short cb_ref_min;    /**< Minimum value of B/G for white region [0 ~ 4095] 4.8bit fixed-point  */
	unsigned char awb_stat_nodeh;	 /**< Number of active zones horizontally for AWB stats collection.[0 ~ 15] */
	unsigned char awb_stat_nodev;	 /**< Number of active zones vertically for AWB stats collection.[0 ~ 15] */
} IMPISPAWBHist;

/**
 * @fn int IMP_ISP_Tuning_GetAwbHist(IMPISPAWBHist *awb_hist)
 *
 * Set AWB statistic parameters
 *
 * @param[out] awb_hist AWB statistic parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, IMP_ISP_EnableTuning is called.
 */
int IMP_ISP_Tuning_GetAwbHist(IMPISPAWBHist *awb_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAwbHist(IMPISPAWBHist *awb_hist)
 *
 * Get AWB Statistics
 *
 * @param[out] awb_hist AWB statistic
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbHist(IMPISPAWBHist *awb_hist);

/**
 * AF statistics
 */
struct isp_core_af_sta_info{
	unsigned int af_metrics;    /**< The integrated and normalized measure of contrast*/
	unsigned int af_metrics_alt;	/**< The integrated and normalized measure of contrast - with alternative threshold*/
	unsigned int af_wl;
	unsigned int af_wh;
};

struct isp_af_ldg_en_info{
	unsigned char fir0_en;
	unsigned char fir1_en;
	unsigned char iir0_en;
	unsigned char iir1_en;
};

struct isp_ldg_info{
	unsigned char thlow1;   /*Ldg start【0~255】*/
	unsigned char thlow2;   /*Ldg end【0~255】*/
	unsigned short slplow;  /*Ldgslp【0~4095】*/
	unsigned char gainlow;  /*Ldggain 【0~255】*/
	unsigned char thhigh1;  /*【0~255】*/
	unsigned char thhigh2;  /*【0~255】*/
	unsigned short slphigh; /*【0~4095】*/
	unsigned char gainhigh; /*【0~255】*/
};

/**
 * AF statistics
 */
typedef struct {
	struct isp_core_af_sta_info af_stat;	/**< AF statistics*/
	unsigned char af_enable;    /**< AF enable*/
	unsigned char af_metrics_shift;	   /**< Metrics scaling factor 0x0 is default */
	unsigned short af_delta;    /**< AF statistics low pass fliter weight [0 ~ 64]*/
	unsigned short af_theta;    /**< AF statistics high pass fliter weight [0 ~ 64]*/
	unsigned short af_hilight_th;    /**< AF high light threshold [0 ~ 255]*/
	unsigned short af_alpha_alt;    /**< AF statistics H and V direction weight [0 ~ 64]*/
	unsigned short af_belta_alt;
	unsigned char  af_hstart;    /**< AF statistic pixel start by horizontal:[1 ~ width], must be odd number*/
	unsigned char  af_vstart;    /**< AF statistic pixel start by vertical:[3 ~ height], must be odd number*/
	unsigned char  af_stat_nodeh;    /**< Number of zones horizontally for AF stats [1 ~ 15] */
	unsigned char  af_stat_nodev;    /**< Number of zones vertically for AF stats [1 ~ 15] */
	unsigned char  af_frame_num;
	struct isp_af_ldg_en_info  ldg_en;
	struct isp_ldg_info af_fir0;
	struct isp_ldg_info af_fir1;
	struct isp_ldg_info af_iir0;
	struct isp_ldg_info af_iir1;
} IMPISPAFHist;

/**
 * @fn int int IMP_ISP_Tuning_GetAFMetrices(unsigned int *metric);
 *
 * Get AF statistic metric
 *
 * @param[in] metric AF statistic metric
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAFMetrices(unsigned int *metric);

/**
 * @fn int IMP_ISP_Tuning_GetAfHist(IMPISPAFHist *af_hist);
 *
 * Set AF statistic parameters
 *
 * @param[out] af_hist AF statistic parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfHist(IMPISPAFHist *af_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAfHist(IMPISPAFHist *af_hist)
 *
 * Get AF statistics
 *
 * @param[in] af_hist AF statistics parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAfHist(IMPISPAFHist *af_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAfWeight(IMPISPWeight *af_weight)
 *
 * Set zone weighting for AF
 *
 * @param[in] af_weight af weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAfWeight(IMPISPWeight *af_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAfWeight(IMPISPWeight *af_weight)
 *
 * Get zone weighting for AF
 *
 * @param[in] af_weight af weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfWeight(IMPISPWeight *af_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAfZone(IMPISPZone *af_zone)
 *
 * Get AF zone metric information.
 *
 * @param[out] af_zone AF metric info per zone
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfZone(IMPISPZone *af_zone);

/**
 * ISP Wait Frame Params。
 */
typedef struct {
	uint32_t timeout;		/**< timeout，unit ms */
	uint64_t cnt;			/**< Frame num */
}IMPISPWaitFrameAttr;
/**
 * @fn int IMP_ISP_Tuning_WaitFrame(IMPISPWaitFrameAttr *attr)
 * Wait frame done
 *
 * @param[out] attr frame done parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_WaitFrame(IMPISPWaitFrameAttr *attr);

/**
 * AE Min
 */
typedef struct {
	unsigned int min_it;  /**< AE min integration time */
	unsigned int min_again;	 /**< AE min analog gain */
	unsigned int min_it_short; /**< AE min integration time on short frame */
	unsigned int min_again_short; /**< AE min analog gain on short frame */
} IMPISPAEMin;

/**
 * @fn int IMP_ISP_Tuning_SetAeMin(IMPISPAEMin *ae_min)
 *
 * Set AE Min parameters
 *
 * @param[in] ae_min AE min parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeMin(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_GetAeMin(IMPISPAEMin *ae_min)
 *
 * Get AE min information.
 *
 * @param[out] ae_min AE min parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeMin(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max)
 *
 * Set AE Max parameters
 *
 * @param[in] it_max AE max it parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAe_IT_MAX(unsigned int it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max)
 *
 * Get AE max information.
 *
 * @param[out] ae_max AE max it parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAE_IT_MAX(unsigned int *it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAeZone(IMPISPZone *ae_zone)
 *
 * Get AE zone y information.
 *
 * @param[out] ae_zone AE y info per zone
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeZone(IMPISPZone *ae_zone);

/*
 *  ISP AE Target Parameters
 */
typedef struct {
	unsigned int at_list[10]; /*ae target list*/
} IMPISPAETargetList;

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList(IMPISPAETargetList *target_list)
 *
 * Set  AE target List
 *
 * @param[in] at_list  ae target list.
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
 * Get  AE target List
 *
 * @param[out] at_list  ae target list.
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
 * Set ISP Module control
 *
 * @param[in] ispmodule ISP Module control.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetModuleControl(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_GetModuleControl(IMPISPModuleCtl *ispmodule)
 *
 * Get ISP Module control.
 *
 * @param[out] ispmodule ISP Module control
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
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
 * Set Front Crop attribution
 *
 * @param[in] ispfrontcrop IFront Crop attribution.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetFrontCrop(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_GetFrontCrop(IMPISPFrontCrop *ispfrontcrop)
 *
 * Get Front Crop Attribution.
 *
 * @param[out] ispfrontcrop IFront Crop attribution.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetFrontCrop(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_SetDPC_Strength(unsigned int ratio)
 *
 * Set DPC Strength.
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the dpc value. If it is less than 128, that means decreaing the dpc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetDPC_Strength(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *strength)
 *
 * Get DPC Strength.
 *
 * @param[out] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the dpc value. If it is less than 128, that means decreaing the dpc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetDPC_Strength(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio)
 *
 * Set DRC Strength.
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the drc value. If it is less than 128, that means decreaing the drc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetDRC_Strength(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio)
 *
 * Get DRC Strength.
 *
 * @param[out] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the drc value. If it is less than 128, that means decreaing the drc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetDRC_Strength(unsigned int *ratio);

/**
 * HFlip and VFlip parameters
 */
typedef enum {
	IMPISP_FLIP_NORMAL_MODE = 0,	/**< normal mode */
	IMPISP_FLIP_H_MODE = 1,	   /**< only mirror mode */
	IMPISP_FLIP_V_MODE = 2,	 /**< only flip mode */
	IMPISP_FLIP_HV_MODE = 3, /**< mirror & flip mode */
	IMPISP_FLIP_MODE_BUTT,
} IMPISPHVFLIP;

/**
 * @fn int IMP_ISP_Tuning_SetHVFLIP(IMPISPHVFLIP hvflip)
 *
 * Set HV Flip mode.
 *
 * @param[in] hvflip HV Flip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetHVFLIP(IMPISPHVFLIP hvflip);

/**
 * @fn int IMP_ISP_Tuning_GetHVFlip(IMPISPHVFLIP *hvflip)
 *
 * Get HV Flip mode.
 *
 * @param[out] hvflip hvflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetHVFlip(IMPISPHVFLIP *hvflip);

/**
 * fill data type of Mask parameters
 */
typedef enum {
	IMPISP_MASK_TYPE_RGB = 0, /**< RGB type */
	IMPISP_MASK_TYPE_YUV = 1, /**< YUV type */
} IMPISP_MASK_TYPE;

/**
 * fill data value of Mask parameters
 */
typedef union mask_value {
	struct {
		unsigned char Red; /**< R offset of RGB type */
		unsigned char Green; /**< G offset of RGB type */
		unsigned char Blue; /**< B offset of RGB type */
	} mask_rgb; /**< RGB type */
	struct {
		unsigned char y_value; /**< Y offset of YUV type */
		unsigned char u_value; /**< U offset of YUV type */
		unsigned char v_value; /**< V offset of YUV type */
	} mask_ayuv; /**< YUV type */
} IMP_ISP_MASK_VALUE;

/**
 * Mask parameters of each channel
 */
typedef struct {
	unsigned char mask_en;/**< mask enable */
	unsigned short mask_pos_top;/**< y of mask position */
	unsigned short mask_pos_left;/**< x of mask position  */
	unsigned short mask_width;/**< mask block width */
	unsigned short mask_height;/**< mask block height */
	IMP_ISP_MASK_VALUE mask_value;/**< mask value */
} IMPISP_MASK_BLOCK_PAR;

/**
 * Mask parameters
 */
typedef struct {
	IMPISP_MASK_BLOCK_PAR chn0[4];/**< chan0 mask attr */
	IMPISP_MASK_BLOCK_PAR chn1[4];/**< chan1 mask attr */
	IMPISP_MASK_BLOCK_PAR chn2[4];/**< chan2 mask attr */
	IMPISP_MASK_TYPE mask_type;/**< mask type */
} IMPISPMASKAttr;

/**
 * @fn int IMP_ISP_Tuning_SetMask(IMPISPMASKAttr *mask)
 *
 * Set Mask Attr.
 *
 * @param[in] mask Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetMask(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetMask(IMPISPMASKAttr *mask)
 *
 * Get Mask Attr.
 *
 * @param[out] mask Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetMask(IMPISPMASKAttr *mask);

/**
 * Sensor attr parameters
 */
typedef struct {
	unsigned int hts;/**< sensor hts */
	unsigned int vts;/**< sensor vts */
	unsigned int fps;/**< sensor fps: */
	unsigned int width;/**< sensor width*/
	unsigned int height;/**< sensor height*/
} IMPISPSENSORAttr;
/**
 * @fn int IMP_ISP_Tuning_GetSensorAttr(IMPISPSENSORAttr *attr)
 *
 * Get Sensor Attr.
 *
 * @param[out] attr Sensor attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSensorAttr(IMPISPSENSORAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_EnableDRC(IMPISPTuningOpsMode mode)
 *
 * Enable DRC.
 *
 * @param[out] mode DRC ENABLE mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_EnableDRC(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_EnableDefog(IMPISPTuningOpsMode mode)
 *
 * Enable Defog.
 *
 * @param[out] mode Defog ENABLE mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_EnableDefog(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetAwbCt(unsigned int *ct)
 *
 * set awb color temp.
 *
 * @param[out] ct AWB color temp value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbCt(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_GetAWBCt(unsigned int *ct)
 *
 * Get AWB color temp.
 *
 * @param[out] ct AWB color temp value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAWBCt(unsigned int *ct);

/**
 * ISP CCM Attr
 */
typedef struct {
	IMPISPTuningOpsMode ManualEn;    /* CCM Manual enable ctrl */
	IMPISPTuningOpsMode SatEn;       /* CCM Saturation enable ctrl */
	float ColorMatrix[9];               /* color matrix on manual mode */
} IMPISPCCMAttr;
/**
 * @fn int IMP_ISP_Tuning_SetCCMAttr(IMPISPCCMAttr *ccm)
 *
 * set ccm attr
 *
 * @param[out] ccm ccm attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetCCMAttr(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_GetCCMAttr(IMPISPCCMAttr *ccm)
 *
 * Get CCM Attr.
 *
 * @param[out] ccm ccm attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetCCMAttr(IMPISPCCMAttr *ccm);

/**
 * ISP AE Attr
 */
typedef struct {
	/* Ae Manual Attr On Linear Mode */
	IMPISPTuningOpsMode AeFreezenEn;    /* Ae Freezen Manual enable ctrl */
	IMPISPTuningOpsMode AeItManualEn;	/* Ae Integration time Manual enable ctrl */
	unsigned int AeIt;			   /* Ae Integration time value */
	IMPISPTuningOpsMode AeAGainManualEn;	   /* Ae Sensor Analog Gain Manual enable ctrl */
	unsigned int AeAGain;			      /* Ae Sensor Analog Gain value */
	IMPISPTuningOpsMode AeDGainManualEn;	   /* Ae Sensor Digital Gain Manual enable ctrl */
	unsigned int AeDGain;			      /* Ae Sensor Digital Gain value */
	IMPISPTuningOpsMode AeIspDGainManualEn;	      /* Ae Isp Digital Gain Manual enable ctrl */
	unsigned int AeIspDGain;			 /* Ae Isp Digital Gain value */
} IMPISPAEAttr;
/**
 * @fn int IMP_ISP_Tuning_SetAeAttr(IMPISPAEAttr *ae)
 *
 * set Ae attr
 *
 * @param[out] ae ae manual attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 * @attention Before set the attr, you must memset the IMPISPAEAttr structure to 0, and then set the corresponding attr.
 */
int IMP_ISP_Tuning_SetAeAttr(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeAttr(IMPISPAEAttr *ae)
 *
 * Get Ae Attr.
 *
 * @param[out] ae ae manual attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeAttr(IMPISPAEAttr *ae);

/**
 * AE state info
 */
typedef struct {
	bool stable;			/*ae stable info, 1:stable  0:converging*/
	unsigned int target;    /*current ae target*/
	unsigned int ae_mean;   /*current ae statistical weighted average value*/
}IMPISPAEState;

/**
 * @fn int IMP_ISP_Tuning_GetAeState(IMPISPAEState *ae_state)
 *
 * Get Ae State info.
 *
 * @param[out] ae ae state info.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeState(IMPISPAEState *ae_state);

/**
 * Scaler method
 */
typedef enum {
	IMP_ISP_SCALER_METHOD_FITTING_CURVE,
	IMP_ISP_SCALER_METHOD_FIXED_WEIGHT,
	IMP_ISP_SCALER_METHOD_BUTT,
} IMPISPScalerMethod;

/**
 * Scaler effect params
 */
typedef struct {
	unsigned char channel;			/*channel 0~2*/
	IMPISPScalerMethod method;		/*scaler method*/
	unsigned char level;			/*scaler level range 0~128*/
} IMPISPScalerLv;

/**
 * @fn int IMP_ISP_Tuning_SetScalerLv(IMPISPScalerLv *scaler_level)
 *
 * Set Scaler method and level.
 *
 * @param[in] mscaler opt.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetScalerLv(IMPISPScalerLv *scaler_level);

/**
 * 3th custom AE library init Attribution
 */
typedef struct {
	enum isp_core_expr_unit AeIntegrationTimeUnit;  /**< AE integration time unit */
	uint32_t AeIntegrationTime;                         /**< AE integration time value */
	uint32_t AeAGain;				    /**< AE sensor analog gain value */
	uint32_t AeDGain;				    /**< AE sensor analog gain value */
	uint32_t AeIspDGain;				    /**< AE ISP digital gain value */

	uint32_t AeMinIntegrationTime;                      /**< AE min integration time */
	uint32_t AeMinAGain;				    /**< AE min sensor analog gain */
	uint32_t AeMinDgain;				    /**< AE min sensor digital gain */
	uint32_t AeMinIspDGain;				    /**< AE min isp digital gain */
	uint32_t AeMaxIntegrationTime;			    /**< AE max integration time */
	uint32_t AeMaxAGain;				    /**< AE max sensor analog gain */
	uint32_t AeMaxDgain;				    /**< AE max sensor digital gain */
	uint32_t AeMaxIspDGain;				    /**< AE max isp digital gain */

	/* AE Manual mode attr for short frame on WDR mode*/
	uint32_t AeShortIntegrationTime;                    /**< AE integration time value */
	uint32_t AeShortAGain;				    /**< AE sensor analog gain value */
	uint32_t AeShortDGain;				    /**< AE sensor digital gain value */
	uint32_t AeShortIspDGain;			    /**< AE ISP digital gain value */

	uint32_t AeShortMinIntegrationTime;                 /**< AE min integration time */
	uint32_t AeShortMinAGain;			    /**< AE min sensor analog gain */
	uint32_t AeShortMinDgain;			    /**< AE min sensor digital gain */
	uint32_t AeShortMinIspDGain;			    /**< AE min isp digital gain */
	uint32_t AeShortMaxIntegrationTime;		    /**< AE max integration time */
	uint32_t AeShortMaxAGain;			    /**< AE max sensor analog gain */
	uint32_t AeShortMaxDgain;			    /**< AE max sensor digital gain */
	uint32_t AeShortMaxIspDGain;			    /**< AE max isp digital gain */

	uint32_t fps;                                       /**< sensor fps 16/16 */
	IMPISPAEHist AeStatis;                        /**< Ae statis attrbution */
} IMPISPAeInitAttr;

/**
 * 3th custom AE library AE information
 */
typedef struct {
	IMPISPZone ae_zone;                             /**< AE statis each zone */
	IMPISPAEHistOrigin ae_hist_256bin;              /**< AE 256 bin hist */
	IMPISPAEHist ae_hist;                           /**< AE 5 bin hist and attribution */
	enum isp_core_expr_unit AeIntegrationTimeUnit;  /**< AE integration time unit */
	uint32_t AeIntegrationTime;                         /**< AE integration time value */
	uint32_t AeAGain;				    /**< AE sensor analog gain value */
	uint32_t AeDGain;				    /**< AE sensor digital gain value */
	uint32_t AeIspDGain;				    /**< AE ISP digital gain value */
	uint32_t AeShortIntegrationTime;                    /**< AE integration time value */
	uint32_t AeShortAGain;				    /**< AE sensor analog gain value */
	uint32_t AeShortDGain;				    /**< AE sensor digital gain value */
	uint32_t AeShortIspDGain;			    /**< AE ISP digital gain value */

	uint32_t Wdr_mode;                                  /**< WDR mode or not */
	IMPISPSENSORAttr sensor_attr;                       /**< sensor attribution */
}  __attribute__((packed, aligned(1))) IMPISPAeInfo;

/**
 * 3th custom AE library AE attribution
 */
typedef struct {
	uint32_t change;                                    /**< change AE attr or not */
	enum isp_core_expr_unit AeIntegrationTimeUnit;  /**< AE integration time unit */

	uint32_t AeIntegrationTime;                         /**< AE integration time value */
	uint32_t AeAGain;				    /**< AE sensor analog gain value */
	uint32_t AeDGain;				    /**< AE sensor digital gain value */
	uint32_t AeIspDGain;				    /**< AE ISP digital gain value */

	/* AE Manual mode attr for short frame on WDR mode*/
	uint32_t AeShortIntegrationTime;                    /**< AE integration time value */
	uint32_t AeShortAGain;				    /**< AE sensor analog gain value */
	uint32_t AeShortDGain;				    /**< AE sensor digital gain value */
	uint32_t AeShortIspDGain;			    /**< AE ISP digital gain value */

	uint32_t luma;                                      /**< AE Luma value */
	uint32_t luma_scence;                               /**< AE scence Luma value */
} IMPISPAeAttr;

/**
 * 3th custom AE library AE notify attribution
 */
typedef enum {
	IMPISP_AE_NOTIFY_FPS_CHANGE,                        /* AE notify the fps change*/
} IMPISPAeNotify;

typedef struct {
	void *priv_data;								       	/* private data addr*/
	int (*open)(void *priv_data, IMPISPAeInitAttr *AeInitAttr);                              /* AE open function for 3th custom library*/
	void (*close)(void *priv_data);                                                         /* AE close function for 3th custom library*/
	void (*handle)(void *priv_data, const IMPISPAeInfo *AeInfo, IMPISPAeAttr *AeAttr);      /* AE handle function for 3th custom library*/
	int (*notify)(void *priv_data, IMPISPAeNotify notify, void* data);                      /* AE notify function for 3th custom library*/
} IMPISPAeAlgoFunc;

/**
 * @fn int32_t IMP_ISP_SetAeAlgoFunc(IMPISPAeAlgoFunc *ae_func)
 *
 * the callback functions interface for 3th custom AE library.
 *
 * @param[in]  ae_func     the callback functions.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that IMP_ISP_AddSensor have be called.
 */
int32_t IMP_ISP_SetAeAlgoFunc(IMPISPAeAlgoFunc *ae_func);


/**
 * 3th custom AWB library AWB init information
 */
typedef struct {
	uint32_t cur_r_gain;						/* current awb r-gain*/
	uint32_t cur_b_gain;						/* current awb b-gain*/
	uint32_t r_gain_statis;						/* current awb r-gain of global statis info*/
	uint32_t b_gain_statis;						/* current awb b-gain of global statis info*/
	uint32_t r_gain_wei_statis;					/* current awb r-gain of global weighted statis info*/
	uint32_t b_gain_wei_statis;					/* current awb b-gain of global weighted statis info*/
	IMPISPAWBZone awb_statis;					/* current awb statis info for each zone*/
}__attribute__((packed, aligned(1))) IMPISPAwbInfo;

/**
 * 3th custom AWB library AWB attribution
 */
typedef struct {
	uint32_t change;					/* change awb attribution or not*/
	uint32_t r_gain;					/* awb attribution of r-gain*/
	uint32_t b_gain;					/* awb attribution of b-gain*/
	uint32_t ct;						/* awb color temp*/
} IMPISPAwbAttr;

/**
 * 3th custom AWB library AWB notify attribution
 */
typedef enum {
	IMPISP_AWB_NOTIFY_MODE_CHANGE,
} IMPISPAwbNotify;

/**
 * 3th custom AWB library callback functions
 */
typedef struct {
	void *priv_data;									/* private data addr*/
	int (*open)(void *priv_data);								/* AWB open function for 3th custom library*/
	void (*close)(void *priv_data);								/* AWB close function for 3th custom library*/
	void (*handle)(void *priv_data, const IMPISPAwbInfo *AwbInfo, IMPISPAwbAttr *AwbAttr);	/* AWB handle function for 3th custom library*/
	int (*notify)(void *priv_data, IMPISPAwbNotify notify, void *data);			/* AWB notify function for 3th custom library*/
} IMPISPAwbAlgoFunc;

int32_t IMP_ISP_SetAwbAlgoFunc(IMPISPAwbAlgoFunc *awb_func);
/**
 * @fn int32_t IMP_ISP_SetAwbAlgoFunc(IMPISPAwbAlgoFunc *awb_func)
 *
 * the callback functions interface for 3th custom AWB library.
 *
 * @param[in]  awb_func    the callback functions.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that IMP_ISP_AddSensor have be called.
 */

/**
 * black level attr
 */
typedef struct {
	unsigned int black_level_r;     /**< R channel */
	unsigned int black_level_gr;    /**< GR channel */
	unsigned int black_level_gb;    /**< GB channel */
	unsigned int black_level_b;     /**< B channel */
	unsigned int black_level_ir;    /**< IR channel */
} IMPISPBlcAttr;

/**
 * @fn int IMP_ISP_Tuning_GetBlcAttr(IMPISPBlcAttr *blc)
 *
 * Gets the associated properties of the BLC.
 *
 * @param[out] blc blc attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 * @attention Before set the attr, you must memset the IMPISPAEAttr structure to 0.
 */
int IMP_ISP_Tuning_GetBlcAttr(IMPISPBlcAttr *blc);

/**
 * @fn int32_t IMP_ISP_Tuning_SetDefog_Strength(uint8_t *ratio)
 *
 * set isp defog module ratio.
 *
 * @param[in] ratio defog module ratio.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetDefog_Strength(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDefog_Strength(uint8_t *ratio)
 *
 * @param[in] ratio defog module ratio.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetDefog_Strength(uint8_t *ratio);

typedef enum {
        ISP_CSC_CG_BT601_FULL,  /**< BT601 full range */
        ISP_CSC_CG_BT601_CLIP,  /**< BT601 not full range */
        ISP_CSC_CG_BT709_FULL,  /**< BT709 full range */
        ISP_CSC_CG_BT709_CLIP,  /**< BT709 not full range */
        ISP_CSC_CG_USER,	/**< CUSTOM mode. Only use this mode, the IMPISPCscMatrix parameters is valid. */
	IMP_CSC_CG_BUTT,           /**< effect paramater, parameters have to be less than this value */
} IMPISPCscCgMode;

typedef struct {
        int csc_coef[9];       /**< 3x3 matrix */
        int csc_offset[2];     /**< [0]:UV offset [1]:Y offset */
        int csc_y_clip[2];     /**< Y max, Y min */
        int csc_c_clip[2];     /**< UV max, UV min */
} IMPISPCscParam;

typedef struct {
        IMPISPCscCgMode mode;	/**< mode */
        IMPISPCscParam csc_par; /**< Custom Matrix */
} IMPISPCscAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetCsc_Attr(IMPISPCscAttr *attr)
 *
 * Set the properties of CSC module.
 *
 * @param[in] attr CSC module attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_Tuning_SetCsc_Attr(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCsc_Attr(IMPISPCscAttr *attr)
 *
 * Get the properties of CSC module.
 *
 * @param[out] attr CSC module attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_Tuning_GetCsc_Attr(IMPISPCscAttr *attr);

/**
 * frame drop parameter.
 */
typedef struct {
        IMPISPTuningOpsMode enable;     /**< enbale mark */
        uint8_t lsize;                  /**< sum (range:0~31) */
        uint32_t fmark;                 /**< bit mark(1 output,0 drop) */
} IMPISPFrameDrop;

/**
 * frame drop attr.
 */
typedef struct {
        IMPISPFrameDrop fdrop[3];       /**< frame drop parameters for each channel */
} IMPISPFrameDropAttr;

/**
 * @fn int32_t IMP_ISP_SetFrameDrop(IMPISPFrameDropAttr *attr)
 *
 * Set frame drop attr.
 *
 * @param[in] attr      Frame drop attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Every time (lsize+1) is accepted, (fmark invalid figure) is lost.
 * @remark Example:lsize=3,fmark=0x5(Frames 2 and 4 are lost every 4).
 *
 * @attention Before using it, make sure that 'IMP_ISP_Open' is working properly.
 */
int32_t IMP_ISP_SetFrameDrop(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop(IMPISPFrameDropAttr *attr)
 *
 * Get frame drop attr.
 *
 * @param[out] attr     Frame drop attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Every time (lsize+1) is accepted, (fmark invalid figure) is lost.
 * @remark Example:lsize=3,fmark=0x5(Frames 2 and 4 are lost every 4).
 *
 * @attention Before using it, make sure that 'IMP_ISP_Open' is working properly.
 */
int32_t IMP_ISP_GetFrameDrop(IMPISPFrameDropAttr *attr);

/**
 * mjpeg fixed contrast.
 */
typedef struct {
        uint8_t mode;
	uint8_t range_low;
	uint8_t range_high;
} IMPISPFixedContrastAttr;

/**
 * @fn int32_t IMP_ISP_SetFixedContraster(IMPISPFixedContrastAttr *attr)
 *
 * set mjpeg fixed contrast.
 *
 * @param[out] attr	attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_SetFixedContraster(IMPISPFixedContrastAttr *attr);

/*
 * set gpio level in vic done
 */
typedef struct {
	uint16_t gpio_num[10];		/** gpio num */
	uint16_t gpio_sta[10];		/** gpio state */
    uint16_t free;
} IMPISPGPIO;

/**
* @fn int32_t IMP_ISP_SET_GPIO_INIT_OR_FREE(IMPISPGPIO *attr);
*
* application and release GPIO
* @param[in] gpio_num application and release GPIO num，end mark 0xFF
* @param[in] gpio_sta GPIO state，0:low 1:high
* @param[in] free 0:application 1:release
*
* @retval 0 means success
* @retval Other values mean failure, its value is an error code.
*
* @remark gpio_num[10]={20,21,0xff},gpio_sta[10]= {1,0} means init PA20 high PA21 low
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int32_t IMP_ISP_SET_GPIO_INIT_OR_FREE(IMPISPGPIO *attr);

/**
* @fn int32_t IMP_ISP_SET_GPIO_STA(IMPISPGPIO *attr);
*
* set GPIO state in vic done.
*
* @param[in] gpio_num set GPIO num，end mark 0xFF
* @param[in] gpio_sta GPIO state，0:low 1:high
*
* @remark gpio_num[10]={20,21,0xff},gpio_sta[10]= {1,0} means PA20 high PA21 low
*
* @retval 0 means success
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_SET_GPIO_INIT_OR_FREE' is working properly.
*/
int32_t IMP_ISP_SET_GPIO_STA(IMPISPGPIO *attr);

/**
 * @fn int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom)
 *
 * Set AutoZoom Parameters
 *
 * @param[in] AutoZoom Parameters
 *
 * @retval 0 means success
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it,make sure that IMP_ISP_EnableSensor is working properly.
 */
int IMP_ISP_Tuning_SetAutoZoom(IMPISPAutoZoom *ispautozoom);


/**
* @fn int IMP_ISP_Tuning_GetAutoZoom(IMPISPAutoZoom *ispautozoom)
*
* Get the parameters for autofocus
*
* @param[in] ispautozoom autofocus configuration parameters
*
* @retval 0 Success
* @retval non-0 fails, returns an error code
*
* @attentionBefore using this function, we must ensure that IMP_ISP_Tuning_SetAutoZoom is executed and returns success.
* @attentionBefore using this function, we must ensure that IMP_ISP_EnableSensor is executed and returns success.
*/
int IMP_ISP_Tuning_GetAutoZoom(IMPISPAutoZoom *ispautozoom);

/**
 * fill data value of Mask parameters
 */
typedef union color_value {
	struct {
		unsigned char r_value;	/**< R offset of RGB type */
		unsigned char g_value;	/**< G offset of RGB type */
		unsigned char b_value;	/**< B offset of RGB type */
	} argb;			        /**< RGB type */
	struct {
		unsigned char y_value;	/**< Y offset of YUV type */
		unsigned char u_value;	/**< U offset of YUV type */
		unsigned char v_value;	/**< V offset of YUV type */
	} ayuv;			        /**< YUV type */
} IMP_ISP_COLOR_VALUE;

/**
 * Mask parameters of each channel
 */
typedef struct isp_mask_block_par {
    uint8_t chx;              /**< Channel Number (range: 0 to 2) */
    uint8_t pinum;            /**< Block NUMBER (range: 0 to 3) */
	uint8_t mask_en;          /**< mask enable */
	uint16_t mask_pos_top;    /**< y of mask position */
	uint16_t mask_pos_left;   /**< x of mask position  */
	uint16_t mask_width;      /**< mask block width */
	uint16_t mask_height;     /**< mask block height */
	IMPISP_MASK_TYPE mask_type;		/**< mask type */
	IMP_ISP_COLOR_VALUE mask_value;  /**< mask value */
} IMPISPMaskBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetMaskBlock(IMPISPMaskBlockAttr *mask)
 *
 * Set Mask Attr.
 *
 * @param[in] num       The sensor num label.
 * @param[in] mask      Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetMaskBlock(IMPISPMaskBlockAttr *mask);

/**
 *@fn int32_t IMP_ISP_Tuning_GetMaskBlock(IMPISPMaskBlockAttr *mask)
 *
 *Get Fill Parameters.
 *
 * @param[in]  num  The label of the corresponding sensor
 * @param[out] mask Fill Parameters
 *
 * @retval 0 success
 * @retval Non zero failure, return error code
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr attr;
 *
 * attr.chx = 0;
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetMaskBlock(&attr);
 * if(ret){
 *		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetMaskBlock error !\n");
 *	return -1;
 * }
 * printf("chx:%d, pinum:%d, en:%d\n", attr.chx, attr.pinum, attr.mask_en);
 * if (attr.mask_en) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention Before using this function, IMP_ ISP_ EnableTuning has been called.
 */
int32_t IMP_ISP_Tuning_GetMaskBlock(IMPISPMaskBlockAttr *mask);

/**
 * OSD picture type
 */
typedef enum {
	IMP_ISP_PIC_ARGB_8888,  /**< ARGB8888 */
	IMP_ISP_PIC_ARGB_1555,  /**< ARBG1555 */
	IMP_ISP_PIC_ARGB_1100,  /**< AC 2bit */
} IMPISPPICTYPE;

/**
 * OSD type
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
 * OSD channel attribution
 */
typedef struct {
	uint8_t pinum;                 /**< Block NUMBER (range: 0 to 7) */
	uint8_t  osd_enable;           /**< osd enable */
	uint16_t osd_left;             /**< osd area x value */
	uint16_t osd_top;              /**< osd area y value */
	uint16_t osd_width;            /**< osd area width */
	uint16_t osd_height;           /**< osd area height */
	char *osd_image;			   /**< osd picture(file path/array) */
	uint16_t osd_stride;           /**< osd stride, In bytes, for example, 320x240 RGBA8888 osd_stride=320*4. */
} IMPISPOSDBlockAttr;

typedef struct {
	IMPISPPICTYPE osd_type;
	IMPISPARGBType osd_argb_type;
	IMPISPTuningOpsMode osd_pixel_alpha_disable;
} IMPISPOSDAttr;

/**
 *@fn int32_t IMP_ISP_Tuning_SetOSDAttr(IMPISPOSDAttr *attr)
 *
 * Set Fill Parameters
 * @param[in]   num         The sensor num label.
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 *  int ret = 0;
 *  IMPISPOSDAttr attr;
 *
 *  attr.osd_type = IMP_ISP_PIC_ARGB_8888;
 *  attr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
 *  attr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *
 *  if(ret){
 *	IMP_LOG_ERR(TAG, " error !\n");
 *	return -1;
 *  }
 * @endcode
 *
 * @Before using this function, IMP_ ISP_ EnableTuning has been called
 */
int32_t IMP_ISP_Tuning_SetOSDAttr(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDAttr(IMPISPOSDAttr *attr)
 *
 * Get Fill Parameters
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 *  int ret = 0;
 *  IMPISPOSDAttr attr;
 *  ret = IMP_ISP_Tuning_GetOSDAttr(&attr);
 *  if(ret){
 *		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDAttr error !\n");
 *	return -1;
 *  }
 *  printf("type:%d, argb_type:%d, mode:%d\n", attr.osd_type,
 *  attr.osd_argb_type, attr.osd_pixel_alpha_disable);
 * @endcode
 *
 * @Before using this function, IMP_ ISP_ EnableTuning has been called
 */
int32_t IMP_ISP_Tuning_GetOSDAttr(IMPISPOSDAttr *attr);
/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock(IMPISPOSDBlockAttr *attr)
 *
 * Get osd Attr.
 *
 * @param[in]   num         The sensor num label.
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetOSDAttr(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDBlock(IMPISPOSDBlockAttr *attr)
 *
 * Set osd Attr.
 *
 * @param[in]   num         The sensor num label.
 * @param[in]   attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetOSDBlock(IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock(IMPISPOSDBlockAttr *attr)
 *
 * Get OSDBlock Parameters
 *
 * @param[in] attr
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetOSDBlock(IMPISPOSDBlockAttr *attr);

/**
 * Draw window attribution
 */
typedef struct {
	uint8_t  enable;           /**< draw window enable */
	uint16_t left;             /**< window start pixel in horizental */
	uint16_t top;              /**< window start pixel in vertical */
	uint16_t width;            /**< window width */
	uint16_t height;           /**< window height */
	IMP_ISP_COLOR_VALUE color; /**< window color */
	uint8_t  line_width;           /**< window line width */
	uint8_t  alpha;            /**< window alpha */
}IMPISPDrawWindAttr;

/**
 * Draw range attribution
 */
typedef struct {
	uint8_t  enable;              /**< draw range enable */
	uint16_t left;                /**< range start pixel in horizental */
	uint16_t top;                 /**< range start pixel in vertical */
	uint16_t width;               /**< range width */
	uint16_t height;              /**< range height */
	IMP_ISP_COLOR_VALUE color;    /**< range color */
	uint8_t  line_width;          /**< range line width */
	uint8_t  alpha;               /**< range alpha(3bit) */
	uint16_t extend;              /**< range extend */
} IMPISPDrawRangAttr;

/**
 * Draw line attribution
 */
typedef struct {
	uint8_t  enable;           /**< draw line enable */
	uint16_t startx;           /**< line start pixel in horizental */
	uint16_t starty;           /**< line start pixel in vertical */
	uint16_t endx;             /**< line width */
	uint16_t endy;             /**< line height */
	IMP_ISP_COLOR_VALUE color; /**< line color */
	uint8_t  width;            /**< line line width */
	uint8_t  alpha;            /**< line alpha */
} IMPISPDrawLineAttr;

/**
 * Draw type
 */
typedef enum {
	IMP_ISP_DRAW_WIND,      /**< Draw line */
	IMP_ISP_DRAW_RANGE,     /**< Draw range */
	IMP_ISP_DRAW_LINE,      /**< Draw window */
} IMPISPDrawType;

/**
 * Draw unit Attribution
 */
typedef struct {
	uint8_t pinum;                     /**< Block NUMBER (range: 0 to 19) */
	IMPISPDrawType type;               /**< draw type */
	IMPISP_MASK_TYPE color_type;		/**< mask type */
	union {
		IMPISPDrawWindAttr wind;   /**< draw window attr */
		IMPISPDrawRangAttr rang;   /**< draw range attr */
		IMPISPDrawLineAttr line;   /**< draw line attr */
	} cfg;                        /**< draw attr */
} IMPISPDrawBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetDrawBlock(IMPISPDrawBlockAttr *attr)
 *
 * Set draw Attr.
 *
 * @param[in]  attr        draw attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetDrawBlock(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDrawBlock(IMPISPDrawBlockAttr *attr)
 *
 * Get draw Attr.
 *
 * @param[out]   attr        draw attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetDrawBlock(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath_Sec(char *path)
 *
 * Set ISP bin file path
 *
 * @param[in] path  file path
 *
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Value of a register can be directly set.
 *
 * @attention Before using this function, you must ensure that the sensor is working, so it will be able to be configured or set.
 */
int32_t IMP_ISP_SetDefaultBinPath_Sec(char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath_Sec(char *path)
 *
 * Get ISP bin file path
 *
 * @param[out] path
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark You can directly obtain the value of the sensor's register.
 *
 * @attention Before using this function, you must ensure that the sensor is working.
 */
int32_t IMP_ISP_GetDefaultBinPath_Sec(char *path);

/**
 * @fn int IMP_ISP_SetSensorRegister_Sec(uint32_t reg, uint32_t value)
 *
 * set register value
 *
 * @param[in] reg register addr
 * @param[in] value
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int IMP_ISP_SetSensorRegister_Sec(uint32_t reg, uint32_t value);

/**
 * @fn int IMP_ISP_GetSensorRegister_Sec(uint32_t reg, uint32_t *value)
 *
 * Get sensor Register value
 *
 * @param[in] reg reg addr
 *
 * @param[out] value reg value
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
* @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int IMP_ISP_GetSensorRegister_Sec(uint32_t reg, uint32_t *value);

/**
 * @fn int IMP_ISP_Tuning_SetSensorFPS_Sec(uint32_t fps_num, uint32_t fps_den)
 *
 * Set the FPS of enabled sensor.
 *
 * @param[in] fps_num 	The numerator value of FPS.
 * @param[in] fps_den 	The denominator value of FPS.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 */
int IMP_ISP_Tuning_SetSensorFPS_Sec(uint32_t fps_num, uint32_t fps_den);

/**
 * @fn int IMP_ISP_Tuning_GetSensorFPS_Sec(uint32_t *fps_num, uint32_t *fps_den)
 *
 * Get the FPS of enabled sensor.
 *
 * @param[in] fps_num The pointer for numerator value of FPS.
 * @param[in] fps_den The pointer for denominator value of FPS.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 * @attention Before starting data transmission in a Channel, you must first call this function in order to obtain the sensor's default FPS.
 */
int IMP_ISP_Tuning_GetSensorFPS_Sec(uint32_t *fps_num, uint32_t *fps_den);

/**
 * @fn int IMP_ISP_Tuning_SetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr attr)
 *
 * Set the antiflicker parameter.
 *
 * @param[in] attr 	The value for antiflicker mode
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before calling this function, make sure that ISP debugging function is working.
 */
int IMP_ISP_Tuning_SetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr attr);

/**
 * @fn int IMP_ISP_Tuning_GetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr *pattr)
 *
 * Get the mode of antiflicker
 *
 * @param[in] pattr The pointer for antiflicker mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before calling this function, make sure that ISP debugging function is working.
 */
int IMP_ISP_Tuning_GetAntiFlickerAttr_Sec(IMPISPAntiflickerAttr *pattr);

/**
 * @fn int IMP_ISP_Tuning_SetBrightness_Sec(unsigned char bright)
 *
 * Set the brightness of image effect.
 *
 * @param[in] bright The value for brightness.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 that means increase brightness, and less than 128 that means decrease brightness.\n
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetBrightness_Sec(unsigned char bright);

/**
 * @fn int IMP_ISP_Tuning_GetBrightness_Sec(unsigned char *pbright)
 *
 * Get the brightness of image effect.
 *
 * @param[in] pbright The pointer for brightness value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase brightness), and less than 128 (decrease brightness).\n
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetBrightness_Sec(unsigned char *pbright);

/**
 * @fn int IMP_ISP_Tuning_SetContrast_Sec(unsigned char contrast)
 *
 * Set the contrast of image effect.
 *
 * @param[in] contrast 		The value for contrast.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase contrast), and less than 128 (decrease contrast).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetContrast_Sec(unsigned char contrast);

/**
 * @fn int IMP_ISP_Tuning_GetContrast_Sec(unsigned char *pcontrast)
 *
 * Get the contrast of image effect.
 *
 * @param[in] pcontrast 	The pointer for contrast.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase contrast), and less than 128 (decrease contrast).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetContrast_Sec(unsigned char *pcontrast);

/**
 * @fn int IMP_ISP_Tuning_SetSharpness_Sec(unsigned char sharpness)
 *
 * Set the sharpness of image effect.
 *
 * @param[in] sharpness 	The value for sharpening strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase sharpening), and less than 128 (decrease sharpening).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetSharpness_Sec(unsigned char sharpness);

/**
 * @fn int IMP_ISP_Tuning_GetSharpness_Sec(unsigned char *psharpness)
 *
 * Get the sharpness of image effect.
 *
 * @param[in] psharpness 	The pointer for sharpness strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase sharpening), and less than 128 (decrease sharpening).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSharpness_Sec(unsigned char *psharpness);

/**
 * @fn int IMP_ISP_Tuning_SetBcshHue_Sec(unsigned char hue)
 *
 * Set the hue of image color.
 *
 * @param[in] hue The value of hue, range from 0 to 255, default 128.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 that means increase hue, and less than 128 that means decrease hue.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetBcshHue_Sec(unsigned char hue);

/**
 * @fn int IMP_ISP_Tuning_GetBcshHue_Sec(unsigned char *hue)
 *
 * Get the hue of image color.
 *
 * @param[in] hue The pointer for hue value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark The default value is 128, more than 128 (increase hue), and less than 128 (decrease hue).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetBcshHue_Sec(unsigned char *hue);

/**
 * @fn int IMP_ISP_Tuning_SetSaturation_Sec(unsigned char sat)
 *
 * Set the saturation of image effect.
 *
 * @param[in] sat 	The value for saturation strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark  The default value is 128, more than 128 (increase saturation), and less than 128 (decrease saturation).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetSaturation_Sec(unsigned char sat);

/**
 * @fn int IMP_ISP_Tuning_GetSaturation_Sec(unsigned char *psat)
 *
 * Get the saturation of image effect.
 *
 * @param[in] psat	 The pointer for saturation strength.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark  The default value is 128, more than 128 (increase saturation), and less than 128 (decrease saturation).
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSaturation_Sec(unsigned char *psat);

/**
 * @fn int IMP_ISP_Tuning_GetTotalGain_Sec(uint32_t *gain)
 *
 * Get the overall gain value of the ISP output image
 *
 * @param[in] gain 	The pointer of total gain value, its format is [24.8], 24 (integer), 8(decimal)
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, make sure that 'IMP_ISP_EnableSensor' and 'IMP_ISP_EnableTuning' are working properly.
 */
int IMP_ISP_Tuning_GetTotalGain_Sec(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetISPHflip_Sec(IMPISPTuningOpsMode mode)
 *
 * Set ISP image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] mode 	The hflip (enable/disable).
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPHflip_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPHflipi_Sec(IMPISPTuningOpsMode *pmode)
 *
 * Get ISP image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the hflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPHflip_Sec(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPVflip_Sec(IMPISPTuningOpsMode mode)
 *
 * Set ISP image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] mode 	The vflip enable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPVflip_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPVflip_Sec(IMPISPTuningOpsMode *pmode)
 *
 * Get ISP image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the vflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPVflip_Sec(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetSensorHflip_Sec(IMPISPTuningOpsMode mode);
 *
 * Set Sensor image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] mode 	The hflip (enable/disable).
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSensorHflip_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetSensorHflip_Sec(IMPISPTuningOpsMode *pmode)
 *
 * Get Sensor image mirror(horizontal) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the hflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Left and Right flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetSensorHflip_Sec(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetSensorVflip_Sec(IMPISPTuningOpsMode mode);
 *
 * Set Sensor image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] mode 	The vflip enable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSensorVflip_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetSensorVflip_Sec(IMPISPTuningOpsMode *pmode);
 *
 * Get Sensor image mirror(vertical) effect function (enable/disable)
 *
 * @param[in] pmode The pointer for the vflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark UP and DOWN flip.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetSensorVflip_Sec(IMPISPTuningOpsMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPRunningMode_Sec(IMPISPRunningMode mode)
 *
 * Set ISP running mode, normal mode or night vision mode; default mode: normal mode.
 *
 * @param[in] mode  running mode parameter
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * Example:
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
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetISPRunningMode_Sec(IMPISPRunningMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPRunningMode_Sec(IMPISPRunningMode *pmode)
 *
 * Get ISP running mode, normal mode or night vision mode;
 *
 * @param[in] pmode The pointer of the running mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetISPRunningMode_Sec(IMPISPRunningMode *pmode);

/**
 * @fn int IMP_ISP_Tuning_SetISPCustomMode_Sec(IMPISPTuningOpsMode mode)
 *
 * Enable ISP custom mode, load another set of parameters.
 *
 * @param[in] mode Custom mode, enable or disable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetISPCustomMode_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_GetISPCustomMode_Sec(IMPISPTuningOpsMode mode)
 *
 * get ISP custom mode
 *
 * @param[out] mode Custom mode, enable or disable.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetISPCustomMode_Sec(IMPISPTuningOpsMode *pmode);

/**
* @fn int IMP_ISP_Tuning_SetGamma_Sec(IMPISPGamma *gamma)
*
* Sets the attributes of ISP gamma.
*
* @param[in] gamma 	The pointer of the attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_SetGamma_Sec(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_GetGamma_Sec(IMPISPGamma *gamma)
*
* Obtains the attributes of gamma.
*
* @param[out] gamma 	The address of the attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetGamma_Sec(IMPISPGamma *gamma);

/**
* @fn int IMP_ISP_Tuning_SetAeComp_Sec(int comp)
*
* Setting AE compensation.AE compensation parameters can adjust the target of the image AE.
* the recommended value range is from 0 to 255.
*
* @param[in] comp 	compensation parameter.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_SetAeComp_Sec(int comp);

/**
* @fn int IMP_ISP_Tuning_GetAeComp_Sec(int *comp)
*
* Obtains the compensation of AE.
*
* @param[out] comp 	The pointer of the compensation.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetAeComp_Sec(int *comp);

/**
* @fn int IMP_ISP_Tuning_GetAeLuma_Sec(int *luma)
*
* Obtains the AE luma of current frame.
*
* @param[out] luma AE luma parameter.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetAeLuma_Sec(int *luma);

/**
 * @fn int IMP_ISP_Tuning_SetAeFreeze_Sec(IMPISPTuningOpsMode mode)
 *
 * AE Freeze.
 *
 * @param[in] mode AE Freeze mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeFreeze_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetExpr_Sec(IMPISPExpr *expr)
 *
 * Set AE attributes.
 *
 * @param[in] expr 	The pointer for exposure attributes.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetExpr_Sec(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_GetExpr_Sec(IMPISPExpr *expr)
 *
 * Get AE attributes.
 *
 * @param[in] expr The pointer for exposure attributes.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetExpr_Sec(IMPISPExpr *expr);

/**
 * @fn int IMP_ISP_Tuning_SetWB_Sec(IMPISPWB *wb)
 *
 * Set the white balance function settings. You can set the automatic and manual mode, manual mode is achieved mainly through setting of bgain, rgain.
 *
 * @param[in] wb 	The pointer for WB attribute.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetWB_Sec(IMPISPWB *wb);

/**
 * @fn int IMP_ISP_Tuning_GetWB_Sec(IMPISPWB *wb)
 *
 * Get the white balance function settings
 *
 * @param[in] wb 	The pointer for WB attribute.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB_Sec(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_Statis_Sec(IMPISPWB *wb)
 *
 * Get the white balance statistic value.
 *
 * @param[out] wb 	The pointer for the statistic.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB_Statis_Sec(IMPISPWB *wb);

/**
 * @fn IMP_ISP_Tuning_GetWB_GOL_Statis_Sec(IMPISPWB *wb)
 *
 * Get the white balance global statistic value.
 *
 * @param[out] wb 	The pointer for the statistic.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetWB_GOL_Statis_Sec(IMPISPWB *wb);

/**
 * int IMP_ISP_Tuning_SetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);
 *
 * Set Cluster AWB mode Parameters.
 *
 * @param[in] awb_cluster  contains cluster awb mode parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);

/**
 * @fn int IMP_ISP_Tuning_GetAwbClust_Sec(IMPISPAWBCluster *awb_cluster)
 *
 * Get Cluster AWB mode Parameters.
 *
 * @param[out] awb_cluster  contains cluster awb mode parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetAwbClust_Sec(IMPISPAWBCluster *awb_cluster);

/**
 * int IMP_ISP_Tuning_SetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);
 *
 * Set rg bg offset under different ct.
 *
 * @param[in] ct_trend  contains ct offset parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);

/**
 * int IMP_ISP_Tuning_GetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);
 *
 * Get rg bg offset under different ct.
 *
 * @param[out] ct_trend  contains current ct offset parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetAwbCtTrend_Sec(IMPISPAWBCtTrend *ct_trend);

/**
 * @fn IMP_ISP_Tuning_Awb_GetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * Set the AWB r g b channel offset source in ISP.
 *
 * @param[out] isp_wb_attr  The pointer for the attributes
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_Awb_GetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn IMP_ISP_Tuning_Awb_SetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr)
 *
 * Sets the Max value of sensor color r g b.
 *
 * @param[in] gain  The value for sensor sensor color r g b..
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_Awb_SetRgbCoefft_Sec(IMPISPCOEFFTWB *isp_core_rgb_coefft_wb_attr);

/**
 * @fn int IMP_ISP_Tuning_SetMaxAgain_Sec(uint32_t gain)
 *
 * Sets the Max value of sensor analog gain.
 *
 * @param[in] gain  The value for sensor analog gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetMaxAgain_Sec(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxAgain_Sec(uint32_t *gain)
 *
 * Get the Max value of sensor analog gain.
 *
 * @param[in] gain  The pointer for sensor analog gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetMaxAgain_Sec(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetMaxDgain_Sec(uint32_t gain)
 *
 * Set the Max value of sensor Digital gain.
 *
 * @param[in] gain 	The pointer for sensor digital gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 * Sets the Max value of isp digital gain.。
 *
 * @param[in] gain The value for isp digital gain. The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @attention When the interface is called, 'IMP_ISP_EnableTuning' has returned successfully.
 */
int IMP_ISP_Tuning_SetMaxDgain_Sec(uint32_t gain);

/**
 * @fn int IMP_ISP_Tuning_GetMaxDgain_Sec(uint32_t *gain)
 *
 * Get the Max value of sensor Digital gain.
 *
 * @param[out] gain 	The pointer for sensor digital gain.
 * The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetMaxDgain_Sec(uint32_t *gain);

/**
 * Obtains the Max value of isp digital gain.
 *
 * @param[out] gain The pointer for isp digital gain. The value of 0 corresponds to 1x gain, 32 corresponds to 2x gain and so on.
 *
 * @retval 0 means success.
 * @retval Other values means failure, its value is an error code.
 *
 * @attention When the interface is called, 'IMP_ISP_EnableTuning' has returned successfully.
 */
int IMP_ISP_Tuning_GetMaxDgain_Sec(uint32_t *gain);

/**
 * @fn int IMP_ISP_Tuning_SetHiLightDepress_Sec(uint32_t strength)
 *
 * Set highlight intensity controls.
 *
 * @param[in] strength 	Highlight control parameter, the value range is [0-10], set to 0 means disable the current function.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetHiLightDepress_Sec(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetHiLightDepress_Sec(uint32_t *strength)
 *
 * Get the strength of high light depress.
 *
 * @param[out] strength 	The pointer for hilight depress strength.
 * The value of 0 corresponds to disable.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetHiLightDepress_Sec(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetBacklightComp_Sec(uint32_t strength)
 *
 * Set backlight intensity controls.
 *
 * @param[in] strength 	Backlight control parameter, the value range is [0-10], set to 0 means disable the current function.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetBacklightComp_Sec(uint32_t strength);

/**
 * @fn int IMP_ISP_Tuning_GetBacklightComp_Sec(uint32_t *strength)
 *
 * Get the strength of backlight compensation.
 *
 * @param[out] strength 	The pointer for backlight compensation strength.
 * The value of 0 corresponds to disable.
 *
 * @retval 0 success
 * @retval others failure
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_GetBacklightComp_Sec(uint32_t *strength);

/**
 * @fn int IMP_ISP_Tuning_SetTemperStrength_Sec(uint32_t ratio)
 *
 * Set 3D noise reduction intensity
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the temper value. If it is less than 128, that means decreaing the temper value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetTemperStrength_Sec(uint32_t ratio);

/**
 * @fn int IMP_ISP_Tuning_SetSinterStrength_Sec(uint32_t ratio)
 *
 * Set 2D noise reduction intensity
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the sinter value. If it is less than 128, that means decreaing the sinter value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_SetSinterStrength_Sec(uint32_t ratio);

/**
* @fn int IMP_ISP_Tuning_GetEVAttr_Sec(IMPISPEVAttr *attr)
*
* Obtains the attributes of exposure value.
* @param[out] attr 	The pointer for attributes.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_GetEVAttr_Sec(IMPISPEVAttr *attr);

/**
* @fn int IMP_ISP_Tuning_EnableMovestate_Sec(void)
*
* When the sensor will motion, it should be called.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
*/
int IMP_ISP_Tuning_EnableMovestate_Sec(void);

/**
* @fn int IMP_ISP_Tuning_DisableMovestate_Sec(void)
*
* When the sensor is from motion to still, it should be called.
*
* @retval 0 means success.
* @retval Other values mean failure, its value is an error code.
*
* @attention Before using it, make sure that 'IMP_ISP_Tuning_EnableMovestate' is working properly.
*/
int IMP_ISP_Tuning_DisableMovestate_Sec(void);

/**
 * @fn int IMP_ISP_Tuning_SetAeWeight_Sec(IMPISPWeight *ae_weight)
 *
 * Set zone weighting for AE target
 *
 * @param[in] ae_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeWeight_Sec(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAeWeight_Sec(IMPISPWeight *ae_weight)
 *
 * Get zone weighting for AE target
 *
 * @param[out] ae_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeWeight_Sec(IMPISPWeight *ae_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_GetROI_Sec(IMPISPWeight *roi_weight)
 *
 * Set roi weighting for AE SCENE judgement
 *
 * @param[out] roi_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_AE_GetROI_Sec(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_AE_SetROI_Sec(IMPISPWeight *roi_weight)
 *
 * Set roi weighting for AE SCENE judgement
 *
 * @param[in] roi_weight aexp weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int IMP_ISP_Tuning_AE_SetROI_Sec(IMPISPWeight *roi_weight);

/**
 * @fn int IMP_ISP_Tuning_SetAwbWeight_Sec(IMPISPWeight *awb_weight)
 *
 * Set zone weighting for AWB
 *
 * @param[in] awb_weight awb weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbWeight_Sec(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbWeight_Sec(IMPISPWeight *awb_weight)
 *
 * Get zone weighting for AWB
 *
 * @param[out] awb_weight awb weight。
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAwbWeight_Sec(IMPISPWeight *awb_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAwbZone_Sec(IMPISPAWBZONE *awb_zone)
 *
 * Get WB zone statistical average in R G B channel
 *
 * @param[out] awb_zone wb statistics。
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAwbZone_Sec(IMPISPAWBZone *awb_zone);

/**
 * @fn int IMP_ISP_Tuning_SetWB_ALGO_Sec(IMPISPAWBALGO wb_algo)
 *
 * Set AWB algorithm for different application situation
 *
 * @param[in] awb algorithm
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */

int IMP_ISP_Tuning_SetWB_ALGO_Sec(IMPISPAWBAlgo wb_algo);

/**
 * @fn int IMP_ISP_Tuning_SetAeHist_Sec(IMPISPAEHist *ae_hist)
 *
 * Set AE statistics parameters
 *
 * @param[in] ae_hist AE statictics parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeHist_Sec(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Sec(IMPISPAEHist *ae_hist)
 *
 * Get AE statistics information.
 *
 * @param[out] ae_hist AE statistics
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeHist_Sec(IMPISPAEHist *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAeHist_Origin_Sec(IMPISPAEHistOrigin *ae_hist)
 *
 * Get AE 256 bin statistics information.
 *
 * @param[out] ae_hist AE statistics
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeHist_Origin_Sec(IMPISPAEHistOrigin *ae_hist);

/**
 * @fn int IMP_ISP_Tuning_GetAwbHist_Sec(IMPISPAWBHist *awb_hist)
 *
 * Set AWB statistic parameters
 *
 * @param[out] awb_hist AWB statistic parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using this function, IMP_ISP_EnableTuning is called.
 */
int IMP_ISP_Tuning_GetAwbHist_Sec(IMPISPAWBHist *awb_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAwbHist_Sec(IMPISPAWBHist *awb_hist)
 *
 * Get AWB Statistics
 *
 * @param[out] awb_hist AWB statistic
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbHist_Sec(IMPISPAWBHist *awb_hist);

/**
 * @fn IMP_ISP_Tuning_GetAFMetrices_Sec(unsigned int *metric);
 *
 * Get AF statistic metric
 *
 * @param[in] metric AF statistic metric
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAFMetrices_Sec(unsigned int *metric);

/**
 * @fn int IMP_ISP_Tuning_GetAfHist_Sec(IMPISPAFHist *af_hist);
 *
 * Set AF statistic parameters
 *
 * @param[out] af_hist AF statistic parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfHist_Sec(IMPISPAFHist *af_hist);

/**
 * @fn int IMP_ISP_Tuning_SetAfHist_Sec(IMPISPAFHist *af_hist)
 *
 * Get AF statistics
 *
 * @param[in] af_hist AF statistics parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAfHist_Sec(IMPISPAFHist *af_hist);
/**
 * @fn int IMP_ISP_Tuning_SetAfWeight_Sec(IMPISPWeight *af_weight)
 *
 * Set zone weighting for AF
 *
 * @param[in] af_weight af weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAfWeight_Sec(IMPISPWeight *af_weigh);
/**
 * @fn int IMP_ISP_Tuning_GetAfWeight_Sec(IMPISPWeight *af_weight)
 *
 * Get zone weighting for AF
 *
 * @param[in] af_weight af weight.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfWeight_Sec(IMPISPWeight *af_weight);

/**
 * @fn int IMP_ISP_Tuning_GetAfZone_Sec(IMPISPZone *af_zone)
 *
 * Get AF zone metric information.
 *
 * @param[out] af_zone AF metric info per zone
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAfZone_Sec(IMPISPZone *af_zone);

/**
 * @fn int IMP_ISP_Tuning_WaitFrame_Sec(IMPISPWaitFrameAttr *attr)
 * Wait frame done
 *
 * @param[out] attr frame done parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_WaitFrame_Sec(IMPISPWaitFrameAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_SetAeMin_Sec(IMPISPAEMin *ae_min)
 *
 * Set AE Min parameters
 *
 * @param[in] ae_min AE min parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAeMin_Sec(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_GetAeMin_Sec(IMPISPAEMin *ae_min)
 *
 * Get AE min information.
 *
 * @param[out] ae_min AE min parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeMin_Sec(IMPISPAEMin *ae_min);

/**
 * @fn int IMP_ISP_Tuning_SetAe_IT_MAX_Sec(unsigned int it_max)
 *
 * Set AE Max parameters
 *
 * @param[in] it_max AE max it parameters.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAe_IT_MAX_Sec(unsigned int it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAE_IT_MAX_Sec(unsigned int *it_max)
 *
 * Get AE max information.
 *
 * @param[out] ae_max AE max it parameters
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAE_IT_MAX_Sec(unsigned int *it_max);

/**
 * @fn int IMP_ISP_Tuning_GetAeZone_Sec(IMPISPZone *ae_zone)
 *
 * Get AE zone y information.
 *
 * @param[out] ae_zone AE y info per zone
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeZone_Sec(IMPISPZone *ae_zone);

/**
 * @fn int IMP_ISP_Tuning_GetAeTargetList_Sec(IMPISPAETargetList *target_list)
 *
 * Set  AE target List
 *
 * @param[in] at_list  ae target list.
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
 * Get  AE target List
 *
 * @param[out] at_list  ae target list.
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
 * Set ISP Module control
 *
 * @param[in] ispmodule ISP Module control.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetModuleControl_Sec(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_GetModuleControl_Sec(IMPISPModuleCtl *ispmodule)
 *
 * Get ISP Module control.
 *
 * @param[out] ispmodule ISP Module control
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetModuleControl_Sec(IMPISPModuleCtl *ispmodule);

/**
 * @fn int IMP_ISP_Tuning_SetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop)
 *
 * Set Front Crop attribution
 *
 * @param[in] ispfrontcrop IFront Crop attribution.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_GetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop)
 *
 * Get Front Crop Attribution.
 *
 * @param[out] ispfrontcrop IFront Crop attribution.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetFrontCrop_Sec(IMPISPFrontCrop *ispfrontcrop);

/**
 * @fn int IMP_ISP_Tuning_SetDPC_Strength_Sec(unsigned int strength)
 *
 * Set DPC Strength.
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the dpc value. If it is less than 128, that means decreaing the dpc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetDPC_Strength_Sec(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDPC_Strength_Sec(unsigned int *strength)
 *
 * Get DPC Strength.
 *
 * @param[out] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the dpc value. If it is less than 128, that means decreaing the dpc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetDPC_Strength_Sec(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetDRC_Strength_Sec(unsigned int ratio)
 *
 * Set DRC Strength.
 *
 * @param[in] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the drc value. If it is less than 128, that means decreaing the drc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetDRC_Strength_Sec(unsigned int ratio);

/**
 * @fn int IMP_ISP_Tuning_GetDRC_Strength_Sec(unsigned int *ratio)
 *
 * Get DRC Strength.
 *
 * @param[out] ratio   Intensity modulation ratio. Default value is 128.If it is greater than 128, that means increaseing the drc value. If it is less than 128, that means decreaing the drc value. The value range is [0-255].
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetDRC_Strength_Sec(unsigned int *ratio);

/**
 * @fn int IMP_ISP_Tuning_SetHVFLIP_Sec(IMPISPHVFLIP hvflip)
 *
 * Set HV Flip mode.
 *
 * @param[in] hvflip HV Flip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetHVFLIP_Sec(IMPISPHVFLIP hvflip);

/**
 * @fn int IMP_ISP_Tuning_GetHVFlip_Sec(IMPISPHVFLIP *hvflip)
 *
 * Get HV Flip mode.
 *
 * @param[out] hvflip hvflip mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetHVFlip_Sec(IMPISPHVFLIP *hvflip);

/**
 * @fn int IMP_ISP_Tuning_SetMask_Sec(IMPISPMASKAttr *mask)
 *
 * Set Mask Attr.
 *
 * @param[in] mask Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetMask_Sec(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetMask_Sec(IMPISPMASKAttr *mask)
 *
 * Get Mask Attr.
 *
 * @param[out] mask Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetMask_Sec(IMPISPMASKAttr *mask);

/**
 * @fn int IMP_ISP_Tuning_GetSensorAttr_Sec(IMPISPSENSORAttr *attr)
 *
 * Get Sensor Attr.
 *
 * @param[out] attr Sensor attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetSensorAttr_Sec(IMPISPSENSORAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_EnableDRC_Sec(IMPISPTuningOpsMode mode)
 *
 * Enable DRC.
 *
 * @param[out] mode DRC ENABLE mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_EnableDRC_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_EnableDefog_Sec(IMPISPTuningOpsMode mode)
 *
 * Enable Defog.
 *
 * @param[out] mode Defog ENABLE mode.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_EnableDefog_Sec(IMPISPTuningOpsMode mode);

/**
 * @fn int IMP_ISP_Tuning_SetAwbCt_Sec(unsigned int *ct)
 *
 * set awb color temp.
 *
 * @param[out] ct AWB color temp value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetAwbCt_Sec(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_GetAWBCt_Sec(unsigned int *ct)
 *
 * Get AWB color temp.
 *
 * @param[out] ct AWB color temp value.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAWBCt_Sec(unsigned int *ct);

/**
 * @fn int IMP_ISP_Tuning_SetCCMAttr_Sec(IMPISPCCMAttr *ccm)
 *
 * set ccm attr
 *
 * @param[out] ccm ccm attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetCCMAttr_Sec(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_GetCCMAttr_Sec(IMPISPCCMAttr *ccm)
 *
 * Get CCM Attr.
 *
 * @param[out] ccm ccm attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetCCMAttr_Sec(IMPISPCCMAttr *ccm);

/**
 * @fn int IMP_ISP_Tuning_SetAeAttr_Sec(IMPISPAEAttr *ae)
 *
 * set Ae attr
 *
 * @param[out] ae ae manual attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 * @attention Before set the attr, you must memset the IMPISPAEAttr structure to 0, and then set the corresponding attr.
 */
int IMP_ISP_Tuning_SetAeAttr_Sec(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeAttr_Sec(IMPISPAEAttr *ae)
 *
 * Get Ae Attr.
 *
 * @param[out] ae ae manual attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeAttr_Sec(IMPISPAEAttr *ae);

/**
 * @fn int IMP_ISP_Tuning_GetAeState_Sec(IMPISPAEState *ae_state)
 *
 * Get Ae State info.
 *
 * @param[out] ae ae state info.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_GetAeState_Sec(IMPISPAEState *ae_state);

/**
 * @fn int IMP_ISP_Tuning_SetScalerLv_Sec(IMPISPScalerLv *scaler_level)
 *
 * Set Scaler method and level.
 *
 * @param[in] mscaler opt.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 */
int IMP_ISP_Tuning_SetScalerLv_Sec(IMPISPScalerLv *scaler_level);

/**
 * @fn int IMP_ISP_Tuning_GetBlcAttr_Sec(IMPISPBlcAttr *blc)
 *
 * Gets the associated properties of the BLC.
 *
 * @param[out] blc blc attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using that function, make sure that ISP is working properly.
 * @attention Before set the attr, you must memset the IMPISPAEAttr structure to 0.
 */
int IMP_ISP_Tuning_GetBlcAttr_Sec(IMPISPBlcAttr *blc);

/**
 * @fn int32_t IMP_ISP_Tuning_SetDefog_Strength_Sec(uint8_t *ratio)
 *
 * set isp defog module ratio.
 *
 * @param[in] ratio defog module ratio.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetDefog_Strength_Sec(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDefog_Strength_Sec(uint8_t *ratio)
 *
 * @param[in] ratio defog module ratio.
 *
 * @retval 0 means success.
 * @retval Other value means failure, its value is an error code.
 *
 * @attention Before using it, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetDefog_Strength_Sec(uint8_t *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_SetCsc_Attr_Sec(IMPISPCscAttr *attr)
 *
 * Set the properties of CSC module.
 *
 * @param[in] attr CSC module attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_Tuning_SetCsc_Attr_Sec(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCsc_Attr_Sec(IMPISPCscAttr *attr)
 *
 * Get the properties of CSC module.
 *
 * @param[out] attr CSC module attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_Tuning_GetCsc_Attr_Sec(IMPISPCscAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetFrameDrop_Sec(IMPISPFrameDropAttr *attr)
 *
 * Set frame drop attr.
 *
 * @param[in] attr      Frame drop attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Every time (lsize+1) is accepted, (fmark invalid figure) is lost.
 * @remark Example:lsize=3,fmark=0x5(Frames 2 and 4 are lost every 4).
 *
 * @attention Before using it, make sure that 'IMP_ISP_Open' is working properly.
 */
int32_t IMP_ISP_SetFrameDrop_Sec(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop_Sec(IMPISPFrameDropAttr *attr)
 *
 * Get frame drop attr.
 *
 * @param[out] attr     Frame drop attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @remark Every time (lsize+1) is accepted, (fmark invalid figure) is lost.
 * @remark Example:lsize=3,fmark=0x5(Frames 2 and 4 are lost every 4).
 *
 * @attention Before using it, make sure that 'IMP_ISP_Open' is working properly.
 */
int32_t IMP_ISP_GetFrameDrop_Sec(IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_SetFixedContraster_Sec(IMPISPFixedContrastAttr *attr)
 *
 * set mjpeg fixed contrast.
 *
 * @param[out] attr	attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention Before using it, make sure that 'IMP_ISP_EnableTuning' is working properly.
 */
int32_t IMP_ISP_SetFixedContraster_Sec(IMPISPFixedContrastAttr *attr);

/**
 * @fn int IMP_ISP_Tuning_SetAutoZoom_Sec(IMPISPAutoZoom *ispautozoom)
 *
 * SetAutoZoom
 *
 * @param[in] AutoZoomattr
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor被执行且返回成功.
 */
int IMP_ISP_Tuning_SetAutoZoom_Sec(IMPISPAutoZoom *ispautozoom);

/**
 * @fn int32_t IMP_ISP_Tuning_SetMaskBlock_Sec(IMPISPMaskBlockAttr *mask)
 *
 * Set Mask Attr.
 *
 * @param[in] num       The sensor num label.
 * @param[in] mask      Mask attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetMaskBlock_Sec(IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_GetMaskBlock_Sec(IMPISPMaskBlockAttr *mask)
 *
 *Get Fill Parameters.
 *
 * @param[in]  num  The label of the corresponding sensor
 * @param[out] mask Fill Parameters
 *
 * @retval 0 success
 * @retval Non zero failure, return error code
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr attr;
 *
 * attr.chx = 0;
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetMaskBlock_Sec(&attr);
 * if(ret){
 *		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetMaskBlock error !\n");
 *	return -1;
 * }
 * printf("chx:%d, pinum:%d, en:%d\n", attr.chx, attr.pinum, attr.mask_en);
 * if (attr.mask_en) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention Before using this function, IMP_ ISP_ EnableTuning has been called.
 */
int32_t IMP_ISP_Tuning_GetMaskBlock_Sec(IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDAttr_Sec(IMPISPOSDAttr *attr)
 *
 * Set Fill Parameters
 * @param[in]   num         The sensor num label.
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 *  int ret = 0;
 *  IMPISPOSDAttr attr;
 *
 *  attr.osd_type = IMP_ISP_PIC_ARGB_8888;
 *  attr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
 *  attr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *
 *  if(ret){
 *	IMP_LOG_ERR(TAG, " error !\n");
 *	return -1;
 *  }
 * @endcode
 *
 * @Before using this function, IMP_ ISP_ EnableTuning has been called
 */
int32_t IMP_ISP_Tuning_SetOSDAttr_Sec(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDAttr_Sec(IMPISPOSDAttr *attr)
 *
 * Get Fill Parameters
 * @param[in]   num         The sensor num label.
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 *  int ret = 0;
 *  IMPISPOSDAttr attr;
 *  ret = IMP_ISP_Tuning_GetOSDAttr_Sec(&attr);
 *  if(ret){
 *		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDAttr error !\n");
 *	return -1;
 *  }
 *  printf("type:%d, argb_type:%d, mode:%d\n", attr.osd_type,
 *  attr.osd_argb_type, attr.osd_pixel_alpha_disable);
 * @endcode
 *
 * @Before using this function, IMP_ ISP_ EnableTuning has been called
 */
int32_t IMP_ISP_Tuning_GetOSDAttr_Sec(IMPISPOSDAttr *attr);
/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock_Sec(IMPISPOSDBlockAttr *attr)
 *
 * Get osd Attr.
 *
 * @param[in]   num         The sensor num label.
 * @param[out]  attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetOSDAttr_Sec(IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDBlock_Sec(IMPISPOSDBlockAttr *attr)
 *
 * Set osd Attr.
 *
 * @param[in]   num         The sensor num label.
 * @param[in]   attr        osd block attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetOSDBlock_Sec(IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock_Sec(IMPISPOSDBlockAttr *attr)
 *
 * Get OSD Attr.
 *
 * @param[out]  attr        OSD attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
* @attention Before using that function, make sure that ISP is working properly.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_SetDrawBlock_Sec(IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDrawBlock_Sec(IMPISPDrawBlockAttr *attr)
 *
 * Set draw Attr.
 *
 * @param[out] attr draw Attr.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
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
 * @attention Before using that function, make sure that ISP is working properly.
 */
int32_t IMP_ISP_Tuning_GetDrawBlock_Sec(IMPISPDrawBlockAttr *attr);

#define MAXSUPCHNMUN 2  /*ISPOSD MAX CHN NUM*/
#define MAXISPOSDPIC 8  /*ISPOS MAX PIC NUM*/

typedef enum {
	IMP_ISP_OSD_RGN_FREE,   /*ISPOSD Not Created or Not Free*/
	IMP_ISP_OSD_RGN_BUSY,   /*ISPOSD has been Created*/
}IMPIspOsdRngStat;

typedef enum {
	ISP_OSD_REG_INV       = 0,
	ISP_OSD_REG_PIC       = 1, /**< PIC*/
}IMPISPOSDType;
typedef struct IMPISPOSDNode IMPISPOSDNode;

typedef struct {
	int chx;
	int sensornum;
	IMPISPOSDAttr chnOSDAttr;
	IMPISPOSDBlockAttr pic;
} IMPISPOSDSingleAttr;


typedef struct {
	IMPISPOSDType type;
	union {
	IMPISPOSDSingleAttr stsinglepicAttr;
	};
}IMPIspOsdAttrAsm;

/**
 * @fn int IMP_ISP_Tuning_SetOsdPoolSize(int size)
 *
 * Set OSD Rmem Size
 *
 * @param[in]
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 */
int IMP_ISP_Tuning_SetOsdPoolSize(int size);

/**
 * @fn int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * Create ISPOSD Rgn
 *
 * @param[in] chn，IMPIspOsdAttrAsm
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 */
int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * Set ISPOSD Rgn Attr
 *
 * @param[in] sensor num，handle num, IMPIspOsdAttrAsm osd attr
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 */
int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * Get ISPOSD Rgn Attr
 *
 * @param[in] sensor num，handle num，IMPOSDRgnCreateStat Attr
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 */
int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_ShowOsdRgn( int chn,int handle, int showFlag)
 *
 * Set ISPOSD show
 *
 * @param[in] sensor num，handle num，showFlag(0:close，1:open)
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 */
int IMP_ISP_Tuning_ShowOsdRgn(int chn,int handle, int showFlag);

/**
 * @fn int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle)
 *
 * Destroy Osd Rgn
 *
 * @param[in] sensor num，handle num
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 */
int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle);

typedef struct {
	IMPISPTuningOpsMode enable;	/**< Switch bin */
	char bname[128];            /**< bin file patch */
} IMPISPBinAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SwitchBin(IMPISPBinAttr *attr)
 *
 * Switch Bin files.
 *
 * @param[in] attr bin file attributes to toggle
 *
 * @retval 0 Success
 * @retval non-0 fails, returns an error code
 * @code
 * int ret = 0;
 * IMPISPBinAttr attr;
 * char name[] = "/etc/sensor/xxx-t23.bin"
 *
 * attr.enable = IMPISP_TUNING_OPS_MODE_ENABLE;
 * memcpy(attr.bname, name, sizeof(name));
 * ret = IMP_ISP_Tuning_SwitchBin( &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error ! \n");
 * 	return -1;
 *}
 * @endcode
 *
 * @attentionBefore this function is used, IMP_ISP_EnableTuning is called.
 */
int32_t IMP_ISP_Tuning_SwitchBin(IMPISPBinAttr *attr);

/**
 * Out stream state.
 */
typedef struct {
	int sensor[2];/**< The subscript is the Sensor number, with a value of 1 representing outgoing stream and 0 representing stop stream */
} IMPISPStreamState;

/**
 * Out stream check
 */
typedef struct {
	int timeout /**< unit：ms */;
	IMPISPStreamState state;
} IMPISPStreamCheck;

/**
 * @fn int32_t IMP_ISP_StreamCheck(IMPISPStreamCheck *check)
 *
 * Obtain the out stream information of the primary and secondary cameras.
 *
 * @param[in] check check.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 * IMPISPStreamCheck attr;
 * memset(&attr, 0x0, sizeof(IMPISPStreamCheck));
 * attr.timeout = 500;
 * IMP_ISP_StreamCheck(&attr);
 *
 * printf("state:%d %d\n", attr.state.sensor[0], attr.state.sensor[1]);
 * @endcode
 *
 * @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int32_t IMP_ISP_StreamCheck(IMPISPStreamCheck *check);

/**
 * @fn int32_t IMP_ISP_SetStreamOut(IMPISPStreamState *state)
 *
 * Control the main camera and secondary camera out stream.
 *
 * @param[in] state state.
 *
 * @retval 0 means success.
 * @retval Other values mean failure, its value is an error code.
 *
 * @code
 * IMPISPStreamState attr;
 *
 * memset(&attr, 0x0, sizeof(IMPISPStreamState));
 * attr.sensor[0] = 1;
 * attr.sensor[1] = 0;
 * IMP_ISP_SetStreamOut(&attr);
 * @endcode
 *
 * @attention Before using this function, you must ensure that 'IMP_ISP_EnableSensor' is working.
 */
int32_t IMP_ISP_SetStreamOut(IMPISPStreamState *state);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_ISP_H__ */
