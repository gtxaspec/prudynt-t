/*
 * System utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SU_BASE_H__
#define __SU_BASE_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * Sysutils Basic Functions header files
 */

/**
 * @defgroup sysutils System Utils
 */

/**
 * @defgroup Sysutils_Base
 * @ingroup sysutils
 * @brief System Basic Functions. 
 * @{
 */

/**
 * Magic device ID
 */
#define DEVICE_ID_MAGIC     "53ef"

/**
 * Length of magic device ID 
 */
#define DEVICE_ID_MAGIC_LEN 4

/**
 * Length of device ID
 */
#define DEVICE_ID_LEN       32

/**
 * Maxnum length of Device model,Device ID and Firmware version
 */
#define MAX_INFO_LEN        64

/**
 * Device Model
 */
typedef struct {
	char chr[MAX_INFO_LEN];		/**< Device Model strings */
} SUModelNum;

/**
 * Device software version
 */
typedef struct {
	char chr[MAX_INFO_LEN];		/**< Device software version strings*/
} SUVersion;

/**
 * Device ID. Device ID is a unique value, different values between the CPU chip differences
 */
typedef union {
	char chr[MAX_INFO_LEN];		/**< Device ID in string */
	uint8_t hex[MAX_INFO_LEN];	/**< Device ID in binary */
} SUDevID;

/**
 * System time structure
 */
typedef struct {
	int sec;	/**< Second,Range：0~59 */
	int min;	/**< Minute,Range：0~59 */
	int hour;	/**< Hour,Range：0~23 */
	int mday;	/**< Day,Range：1~31 */
	int mon;	/**< Month,Range：1~12 */
	int year;	/**< Year,Range：>1900 */
} SUTime;

/**
 * @fn int SU_Base_GetModelNumber(SUModelNum *modelNum)
 *
 * Get device model.
 *
 * @param[out] modelNum Device model structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_Base_GetModelNumber(SUModelNum *modelNum);

/**
 * @fn int SU_Base_GetVersion(SUVersion *version)
 *
 * Get device version.
 *
 * @param[out] version Device version structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_Base_GetVersion(SUVersion *version);

/**
 * @fn int SU_Base_GetDevID(SUDevID *devID)
 *
 * Get device ID.
 *
 * @param[out] devID Device ID structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks The device ID of each CPU is unique.
 *
 * @attention None.
 */
int SU_Base_GetDevID(SUDevID *devID);

/**
 * @fn int SU_Base_GetTime(SUTime *time)
 *
 * Get system time.
 *
 * @param[in] time System time structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_Base_GetTime(SUTime *time);

/**
 * @fn int SU_Base_SetTime(SUTime *time)
 *
 * Set system time.
 *
 * @param[out] time System time structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention System time parameters should be in a reasonable range, otherwise the function will call failure.
 */
int SU_Base_SetTime(SUTime *time);

/**
 * @fn int SU_Base_SUTime2Raw(SUTime *suTime, uint32_t *rawTime)
 *
 * Converts the time of the SUTime type to Raw time in seconds..
 *
 * @param[in] suTime System time structure pointer.
 * @param[out] rawTime Raw time(Count from the date 1970-01-01 00:00:00).
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks This function can be used to set the relative number of seconds alarm.
 *
 * @attention None.
 */
int SU_Base_SUTime2Raw(SUTime *suTime, uint32_t *rawTime);

/**
 * @fn int SU_Base_Raw2SUTime(uint32_t *rawTime, SUTime *suTime)
 *
 * Converts the time of the  Raw time in seconds to SUTime type.
 *
 * @param[in] rawTime Raw time(Count from time 1970-01-01 00:00:00).
 * @param[out] suTime System time structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks This function can be used to set the relative number of seconds alarm.
 *
 * @attention None.
 */
int SU_Base_Raw2SUTime(uint32_t *rawTime, SUTime *suTime);

/**
 * @fn int SU_Base_SetAlarm(SUTime *time)
 *
 * Set Alarm time.
 *
 * @param[in] time System time structure pointer.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks Temporarily support alarm time setting within 24 hours.
 *
 * @attention System time structure parameters should be in a reasonable time Range, otherwise the function call Failure.
 */
int SU_Base_SetAlarm(SUTime *time);

/**
 * @fn int SU_Base_GetAlarm(SUTime *time)
 *
 * Get the current Alarm time.
 *
 * @param[out] time .
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_Base_GetAlarm(SUTime *time);

/**
 * @fn int SU_Base_EnableAlarm()
 *
 * Enable Alarm.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks Before calling this function, please call SU_Base_GetAlarm (SUTime * time) to set the alarm time.
 *
 * @attention If alarm time before the current system time ,this function will return failure.
 */
int SU_Base_EnableAlarm(void);

/**
 * @fn int SU_Base_DisableAlarm()
 *
 * Disable Alarm.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_Base_DisableAlarm(void);

/**
 * @fn int SU_Base_PollingAlarm(uint32_t timeoutMsec)
 *
 * Wait Alarm.
 *
 * @param[in] timeout,unit：ms.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks After calling this function, the program will enter the blocked state until the alarm response or timeout Exit.
 *
 * @attention None.
 */
int SU_Base_PollingAlarm(uint32_t timeoutMsec);

/**
 * @fn int SU_Base_Shutdown(void)
 *
 * Shutdown device.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks After calling this function the device will shut down immediately and turn off the main power.
 *
 * @attention Before calling this function make sure that all files have been saved.
 */
int SU_Base_Shutdown(void);

/**
 * @fn int SU_Base_Reboot(void)
 *
 * Reboot device.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks  The device will immediately reboot after calling this function.
 *
 * @attention Before calling this function make sure that all files have been saved.
 */
int SU_Base_Reboot(void);

/**
 * @fn int SU_Base_Suspend(void)
 *
 * Suspend device.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks After calling this function the device will immediately enter suspend ,when the function exits normally indicates that the system wake.
 *
 * @attention None.
 */
int SU_Base_Suspend();

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_BASE_H__ */
