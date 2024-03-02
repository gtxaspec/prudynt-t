/*
 * IMP Decoder func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_DECODER_H__
#define __IMP_DECODER_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * IMP Decoder header file
 */

/**
 * @defgroup IMP_Decoder
 * @ingroup imp
 * @brief Video Decoder modules, only support JEPG decode for now.
 * @{
 */

/**
 * Attribute of Decoder
 */
typedef struct {
	IMPPayloadType		decType;		/**< Stream payload type */
	uint32_t			maxWidth;		/**< Max width of frame */
	uint32_t			maxHeight;		/**< Max hight of frame */
	IMPPixelFormat		pixelFormat;	/**< Pixel format of Output frame */
	uint32_t			nrKeepStream;	/**< Number of frames in Decoder FIFO */
	uint32_t			frmRateNum;		/**< The number of time units within a second, time unitis its unit. The numerator of framerate */
	uint32_t			frmRateDen;		/**< The number of time units in a frame, time unit is its unit. The denominator of framerate */
} IMPDecoderAttr;

/**
 * Attribute of Decoder Channel
 */
typedef struct {
	IMPDecoderAttr		decAttr;		/**< Decoder attribute */
} IMPDecoderCHNAttr;

/**
 * Attribute of decode frame
 */
typedef struct {
	int					i_payload;		/**< Length of decode frame */
	uint8_t				*p_payload;		/**< Pointer to decode frame */
	int64_t				timeStamp;		/**< Timestamp of decode frame */
} IMPDecoderNal;

/**
 * Attribute of decode stream
 */
typedef struct {
	IMPDecoderNal	decoderNal; /**< decode stream data structure */
} IMPDecoderStream;

/**
 * @fn int IMP_Decoder_CreateChn(int decChn, const IMPDecoderCHNAttr *attr)
 *
 * Create Decoder channel.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 * @param[in] attr Pointer to Decoder Channel attribute
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 * @attention None.
 */
int IMP_Decoder_CreateChn(int decChn, const IMPDecoderCHNAttr *attr);

/**
 * @fn int IMP_Decoder_DestroyChn(int decChn)
 *
 * Destroy Decoder channel.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 * @attention None.
 */
int IMP_Decoder_DestroyChn(int decChn);

/**
 * @fn int IMP_Decoder_StartRecvPic(int decChn)
 *
 * Decoder channel start recieve pictures.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks First Open decoding Channel to receive the image then start decoding.
 *
 * @attention Failed if the channel isn't created.
 */
int IMP_Decoder_StartRecvPic(int decChn);

/**
 * @fn int IMP_Decoder_StopRecvPic(int decChn)
 *
 * Decoder channel stop recieve pictures.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks Stop decoding the Channel to receive image.
 *
 * @attention Failed if the channel isn't created.
 */
int IMP_Decoder_StopRecvPic(int decChn);

/**
 * @fn int IMP_Decoder_SendStreamTimeout(int decChn, IMPDecoderStream *stream, uint32_t timeoutMsec)
 *
 * Send frame to Decoder channel.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 * @param[in] stream Pointer to the Data stream structure to be decoded
 * @param[in] timeoutMsec Decode timeout value(msec).
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 *
 * @attention Failed if the channel isn't created.
 */
int IMP_Decoder_SendStreamTimeout(int decChn, IMPDecoderStream *stream, uint32_t timeoutMsec);

/**
 * @fn int IMP_Decoder_PollingFrame(int decChn, uint32_t timeoutMsec)
 *
 * Polling Decoder channel, return when decoding finished or timeout.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 * @param[in] timeoutMsec Wait timeout value(msec).
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 *
 * @attention Failed if the channel isn't created.
 */
int IMP_Decoder_PollingFrame(int decChn, uint32_t timeoutMsec);

/**
 * @fn int IMP_Decoder_GetFrame(int decChn, IMPFrameInfo **frame)
 *
 * Get the decoded output frame.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 * @param[out] frame Pointer to output frame's pointer
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 *
 * @attention The memory of output buffer is malloced in Decoder, So the input argument is a pointer of a pointer(pointer's address).

The decoding stream buffer is applied by the decoder, and the current function only needs to be introduced into the structure body pointer.
 */
int IMP_Decoder_GetFrame(int decChn, IMPFrameInfo **frame);

/**
 * @fn int IMP_Decoder_ReleaseFrame(int decChn, IMPFrameInfo *frame)
 *
 * Release the decoded output frame.
 *
 * @param[in] decChn Channel Num. Value range: [0, @ref NR_MAX_DEC_CHN - 1]
 * @param[in] frame Pointer to output frame
 *
 * @retval 0 Success
 * @retval OtherValues Failure
 *
 * @remarks None.
 *
 * @attention None.
 */
int IMP_Decoder_ReleaseFrame(int decChn, IMPFrameInfo *frame);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_DECODER_H__ */
