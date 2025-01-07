/*
 * SU ADC header file.
 *
 * Copyright (C) 2015 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SU_ADC_H__
#define __SU_ADC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * ADC module include header
 */

/**
 * @defgroup Sysutils_ADC
 * @ingroup sysutils
 * @brief Analog to digital conversion module
 *
 * Please refer to the use of Samples
 * @{
 */

/**
 * @fn int SU_ADC_Init(void);
 *
 * ADC module init
 *
 * @retval 0 Success
 * @retval Non-0 Failure，return error code
 *
 * @remark Before using the ADC, be sure to call this function.
 *
 * @attention None
 */
int SU_ADC_Init(void);

/**
 * @fn int SU_ADC_Exit(void);
 *
 * ADC module deinit
 *
 * @retval 0 Success
 * @retval Non-0 Failure,return error code
 *
 * @remark After not using ADC,be sure to call this function.
 *
 * @attention None
 */
int SU_ADC_Exit(void);

/**
 * @fn int SU_ADC_EnableChn(uint32_t chn_num);
 *
 * Enable an ADC channel
 *
 * @param[in] chn_num The channel number you want to use.
 *
 * @retval 0 Success
 * @retval Non-0 Failure,retrun error code
 *
 * @remark None
 *
 * @attention None
 */
int SU_ADC_EnableChn(uint32_t chn_num);

/**
 * @fn int SU_ADC_DisableChn(uint32_t chn_num);
 *
 * Disable an ADC channel
 *
 * @param[in] chn_num The channel num you want do stop.
 *
 * @retval 0 Success
 * @retval Non-0 Failure,return error code.
 *
 * @remark None
 *
 * @attention None
 */
int SU_ADC_DisableChn(uint32_t chn_num);

/**
 * @fn int SU_ADC_GetChnValue(uint32_t chn_num, int *value);
 *
 * Get the ADC value of the channel chn_num
 *
 * @param[in] chn_num The channel number
 *
 * @param[out] value ADC value obtained
 *
 * @retval 0 Success
 * @retval Non-0 Failure,return error code
 *
 * @remark None
 *
 * @attention None
 */
int SU_ADC_GetChnValue(uint32_t chn_num, int *value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __SU_ADC_H__ */
