/*
 * Cipher utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SU_CIPHER_H__
#define __SU_CIPHER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * Sysutils  The Encryption and Decryption management header file
 */

/**
 * @defgroup Sysutils_Cipher
 * @ingroup sysutils
 * @brief Encryption and Decryption manage
 * @{
 */

/**
 * Select an encryption algorithm.
 * @remarks It supports two encryption algorithms AES and DES.
 */
typedef enum IN_UNF_CIPHER_ALG_E
{
	IN_UNF_CIPHER_ALG_AES = 0x0,
	IN_UNF_CIPHER_ALG_DES = 0x1
} IN_UNF_CIPHER_ALG;

/**
 * Select the encryption mode.
 * @remarks It supports two encryption modes AES and DES.
 */
typedef enum IN_UNF_CIPHER_WORK_MODE_E
{
	IN_UNF_CIPHER_WORK_MODE_ECB = 0x0,
	IN_UNF_CIPHER_WORK_MODE_CBC = 0x1,
	IN_UNF_CIPHER_WORK_MODE_OTHER = 0x2
} IN_UNF_CIPHER_WORK_MODE;

/**
 * Select the encryption key length used.
 * @remarks By hardware limitations, at this stage only supports 128 bit  KEY length.
 */
typedef enum IN_UNF_CIPHER_KEY_LENGTH_E
{
	IN_UNF_CIPHER_KEY_AES_128BIT = 0x0,
} IN_UNF_CIPHER_KEY_LENGTH;

/**
 * Select the data length of encryption algorithm at a process.
 * @remarks By hardware limitations, at this stage only supports 128 bit  KEY length.
 */
typedef enum IN_UNF_CIPHER_BIT_WIDTH_E
{
	IN_UNF_CIPHER_BIT_WIDTH_128BIT = 0x0,
} IN_UNF_CIPHER_BIT_WIDTH;

/**
 * Select the encryption process control structure.
 */
typedef struct IN_UNF_CIPHER_CTRL_S
{
	unsigned int key[4];				/**< KEY used in a encryption*/
	unsigned int IV[4];					/**< IV Vector used in a encryption*/
	unsigned int enDataLen;				/**< Total length of data to be processed*/
	IN_UNF_CIPHER_ALG enAlg;			/**< Encryption algorithm used in processing data */
	IN_UNF_CIPHER_BIT_WIDTH enBitWidth;	/**< Data length of Encryption algorithm in once process */
	IN_UNF_CIPHER_WORK_MODE enWorkMode;	/**< Encryption algorithm  mode used to process data*/
	IN_UNF_CIPHER_KEY_LENGTH enKeyLen;	/**< KEY length of the encryption algorithm */
} IN_UNF_CIPHER_CTRL;

/**
 * @fn int SU_CIPHER_Init(void)
 *
 * Open encryption module.
 *
 * @param	None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_CIPHER_Init(void);

/**
 * @fn int SU_CIPHER_DES_Init(void)
 *
 * Open des module.
 *
 * @param	none.
 *
 * @retval 0 success.
 * @retval none-0 failure.
 *
 * @remarks none.
 *
 * @attention none.
 */
int SU_CIPHER_DES_Init(void);

/**
 * @fn int SU_CIPHER_Exit(void)
 *
 * Close encryption module.
 *
 * @param	None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_CIPHER_Exit(void);

/**
 * @fn int SU_CIPHER_DES_Exit(void)
 *
 * Close des module.
 *
 * @param	None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_CIPHER_DES_Exit(void);

/**
 * @fn int SU_CIPHER_DES_Test(void)
 *
 * Invoke the des module interface test.
 *
 * @param	None.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int SU_CIPHER_DES_Test(void);

/**
 * @fn int SU_CIPHER_CreateHandle(void)
 *
 * Get a encryption module handle.
 *
 * @param	None.
 *
 * @retval  success: Return the handle.
 * @retval  Failure: retval <  0.
 *
 * @remarks None.
 *
 * @attention This function can be called multiple times, once for each call will return a handle.
 * If this function called N times,then N times SU_CIPHER_DestroyHandle() called should be to do
 * to destroy all handles we have got.
 *
 */
int SU_CIPHER_CreateHandle(void);

/**
 * @fn int SU_CIPHER_DestroyHandle(int fd)
 *
 * Destroy a encryption module handle.
 *
 * @param[in] fd Handle need to be destroy
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention This function can be called multiple times, each call will destroy one handle.
 *
 */
int SU_CIPHER_DestroyHandle(int fd);

/**
 * @fn int SU_CIPHER_ConfigHandle(int hCipher, IN_UNF_CIPHER_CTRL* Ctrl)
 *
 * Config encryption module.
 *
 * @param[in] hCipher The handle need to be configed.
 * @param[in] Ctrl Configuration information structure.
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention  None.
 *
 */
int SU_CIPHER_ConfigHandle(int hCipher, IN_UNF_CIPHER_CTRL* Ctrl);

/**
 * @fn int SU_CIPHER_Encrypt(int hCipher, unsigned int * srcAddr, unsigned int * dstAddr, unsigned int dataLen)
 *
 * Start encrypt data.
 *
 * @param[in] hCipher Handle to be operated..
 * @param[in] srcAddr Source address of required encrypted data.
 * @param[in] dstAddr Target address to store encrypted data.
 * @param[in] dataLen Data length needed to be processed..
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention Encrypted data length dataLen maximum not more than 1Mbyte (1024*1024).
 *
 */
int SU_CIPHER_Encrypt(int hCipher, unsigned int * srcAddr, unsigned int * dstAddr, unsigned int dataLen);

/**
 * @fn int SU_CIPHER_Decrypt(int hCipher, unsigned int * srcAddr, unsigned int * dstAddr, unsigned int dataLen);
 *
 * Start decrypt data.
 *
 * @param[in] hCipher Handle to be operated..
 * @param[in] srcAddr Source address of required decrypted data.
 * @param[in] dstAddr Target address to store decrypted data.
 * @param[in] dataLen Data length needed to be processed..
 *
 * @retval 0 Success.
 * @retval Non-0 Failure.
 *
 * @remarks None.
 *
 * @attention Decrypted data length dataLen maximum not more than 1Mbyte (1024*1024).
 *
 */
int SU_CIPHER_Decrypt(int hCipher, unsigned int * srcAddr, unsigned int * dstAddr, unsigned int dataLen);

/**
 * Error Code.
 */
#define REINIT				-10	/**< Repeat initialization */
#define INIT_FAILED			-11	/**< Initializatie failed*/
#define FAILED_GETHANDLE	-12 /**< Get handle failed*/
#define INVALID_PARA		-13	/**< Invalid parameter*/
#define SET_PARA_FAILED		-14 /**< Set parameters fail*/
#define FAILURE				-15 /**< Operate failure*/
#define SET_DATALEN_ERR		-16	/**< Set data length error*/
#define EXIT_ERR			-17	/**< Module exit error*/
#define UNINIT				-18	/**< Module not initialization*/
#define FAILED_DESHANDLE	-19 /**< Destroy handle failed*/

/**
 * @}
 */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SU_CIPHER_H__ */
