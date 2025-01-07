#ifndef __IMP_DMIC_H
#define __IMP_DMIC_H

#include <stdint.h>
#include "imp_audio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

typedef enum {
	DMIC_SAMPLE_RATE_8000 = 8000, /**8KHz sampling rate*/
	DMIC_SAMPLE_RATE_16000 = 16000, /*16KHz sampling rate*/
	DMIC_SAMPLE_RATE_48000 = 48000, /*48KHz sampling rate*/
} IMPDmicSampleRate;

typedef enum {
	DMIC_BIT_WIDTH_16 = 16, /**<16 bit sampling precision*/
} IMPDmicBitWidth;

typedef enum {
	DMIC_SOUND_MODE_MONO = 1,       /*Single channel*/
	DMIC_SOUND_MODE_STEREO = 2,		/*Double channel*/
} IMPDmicSoundMode;

/*DMIC input device attribut*/
typedef struct {
	IMPDmicSampleRate samplerate; /**< DMIC sampling rate */
	IMPDmicBitWidth bitwidth;     /**<DMIC sampling precision */
	IMPDmicSoundMode soundmode;   /*audio channel mode*/
	int frmNum;					 /*Number of cached frames*/
	int numPerFrm;				 /*Number of sample points per frame */
	int chnCnt;					/*Number of channels supported*/
} IMPDmicAttr;

/**
 DMIC frame structure.
 */
typedef struct {
	IMPDmicBitWidth bitwidth;
	IMPDmicSoundMode soundmode;
	uint32_t *virAddr;
	uint32_t phyAddr;
	int64_t timeStamp;
	int seq;
	int len;
} IMPDmicFrame;

/*DMIC audio channel frame structure*/
typedef struct {
	IMPDmicFrame rawFrame;  /*four dmic frame raw data*/
	IMPDmicFrame aecFrame;  /*one of dmic frame after aec processing*/
} IMPDmicChnFrame;

/*DMIC channel parameter structure*/
typedef struct {
	int usrFrmDepth;  /**<DMIC audio frame buffer depth*/
	int Rev;		  /**<retain**/
} IMPDmicChnParam;

typedef struct {
	int dmic_id;
	int dmic_en;
} DmicXInfo;

/**
@int IMP_DMIC_SetUserInfo(int dmicDevId, int aecDmicId, int need_aec);
 *Set up information about the user requirements of the MAC array;
 * @param[in] dmicDevId  MAC array device number.
 * @param[in] aecDmicId  Dmic ID number of MAC array as aec processing.
 * @param[in] need_aec   Whether the user needs to do echo cancellation.(need_aec: 0:no need 1:need)
 * @retval 0 success
 * @retval non-0 failure.
 */
int IMP_DMIC_SetUserInfo(int dmicDevId, int aecDmicId, int need_aec);

/**
@fn int IMP_DMIC_SetPubAttr(int dmicDevId, IMPDmicAttr *attr);
 Set MAC array input device attribute.
 * @param[in] dmicDevId  MAC array audio device number.
 * @param[in] attr  MAC array audio device attribute pointer
 *
 * @retval 0 success
 * @retval non-0 failure.
 *
 * @attention Need to be called before IMP_DMIC_Enable.
 */
int IMP_DMIC_SetPubAttr(int dmicDevId, IMPDmicAttr *attr);

/**
@fn int IMP_DMIC_GetPubAttr(int dmicDevId, IMPDmicAttr *attr);
 *
 Get MAC array input device attribute.
 * @param[in] dmicDevId  MAC array audio device number.
 * @param[in] attr  MAC array audio device attribute pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_DMIC_GetPubAttr(int dmicDevId, IMPDmicAttr *attr);

/**
@fn int IMP_DMIC_Enable(int dmicDevId);
 *
 * Enable microphone array audio input device;
 * Enable MAC array audio device.
 * @param[in] dmicDevId MAC array audio device number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_DMIC_Enable(int dmicDevId);

/**
@fn int IMP_DMIC_Disable(int dmicDevId);
 *
 * Diable MAC array audio device.
 * @param[in] dmicDevId MAC array audio device number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks none.
 * @attention none.
 */
int IMP_DMIC_Disable(int dmicDevId);

/**
@fn int IMP_DMIC_EnableChn(int dmicDevId, int dmicChnId);
 *
 * Enable MAC arrayu audio channel
 *
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number
 *
 * @retval 0 success.
 * @retval non-0 failure
 *
 * @remarks no.
 *
 * @attention Must first enable MAC array device.
 */
int IMP_DMIC_EnableChn(int dmicDevId, int dmicChnId);

/**
@fn int IMP_DMIC_DisableChn(int dmicDevId, int dmicChnId);
 *
 * Disable MAC array audio channel
 *
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number
 *
 * @retval 0 success.
 * @retval non-0 failure
 * @remarks no.
 *
 * @attention It supports the use of IMP_DMIC_EnableChn.
 */
int IMP_DMIC_DisableChn(int dmicDevId, int dmicChnId);
/**
@fn int IMP_DMIC_SetChnParam(int dmicDevId, int dmicChnId, IMPDmicChnParam *chnParam);
 *
 * Set MAC array audio channel parameters.
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number
 * @param[in] chnParam MAC array channel frame structure pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks none.
 *
 * @attention Supporting the use of IMP_DMIC_EnableChn.
 */
int IMP_DMIC_SetChnParam(int dmicDevId, int dmicChnId, IMPDmicChnParam *chnParam);

/**
@fn int IMP_DMIC_GetChnParam(int dmicDevId, int dmicChnId, IMPDmicChnParam *chnParam);
 *
 * Set MAC array audio channel parameters.
 *
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number
 * @param[in] chnParam MAC array channel frame structure pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 * @remarks no.
 *
 * @attention no.
 */
int IMP_DMIC_GetChnParam(int dmicDevId, int dmicChnId, IMPDmicChnParam *chnParam);

/**
* @fn int IMP_DMIC_GetFrame(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm, IMPBlock block);
*
* Get Mac array channel audio frame.
*
* @param[in] dmicDevId MAC array audio device number.
* @param[in] dmicChnId MAC array audio channel number
* @param[out] chnFrm   MAC array channel audio frame structure pointer.
* @param[in] block block Blocking / non blocking identifier.
*
* @retval 0 success.
* @retval non-0 failure.
*
*/
int IMP_DMIC_GetFrame(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm, IMPBlock block);

/**
* @fn int IMP_DMIC_ReleaseFrame(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm);
*
* Release Mac array channel audio frame.
*
* @param[in] dmicDevId MAC array audio device number.
* @param[in] dmicChnId MAC array audio channel number
* @param[out] chnFrm   MAC array channel audio frame structure pointer.
*
* @retval 0 success.
* @retval non-0 failure.
*
* @remarks no.
*
* @attention It supports the use of IMP_DMIC_GetFrame.
*/
int IMP_DMIC_ReleaseFrame(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm);

/**
* @fn int IMP_DMIC_EnableAecRefFrame(int dmicDevId, int dmicChnId, int audioAoDevId, int aoChn);
*
* Open access reference frame.
* @param[in] dmicDevId MAC array audio device number.
* @param[in] dmicChnId MAC array audio channel number
* @param[in] audioAoDevId audio output device number.
* @param[in] aoChn audio output channel number.
*
* @retval 0 success.
* @retval non-0 failure.
*
* @remarks Use this current interface before using IMP_DMIC_GetFrameAndRef.
*
* @attention no.
*/
int IMP_DMIC_EnableAecRefFrame(int dmicDevId, int dmicChnId, int audioAoDevId, int aoChn);

/**
* @fn int IMP_DMIC_DisableAecRefFrame(int dmicDevId, int dmicChnId, int audioAoDevId, int aoChn);
*
* Open access reference frame.
* @param[in] dmicDevId MAC array audio device number.
* @param[in] dmicChnId MAC array audio channel number
* @param[in] audioAoDevId audio output device number.
* @param[in] aoChn audio output channel number.
*
* @retval 0 success.
* @retval non-0 failure.
*
* @remarks Use this current interface before using IMP_DMIC_GetFrameAndRef.
*
* @attention no.
*/
int IMP_DMIC_DisableAecRefFrame(int dmicDevId, int dmicChnId, int audioAoDevId, int aoChn);
/**
* @fn int IMP_DMIC_GetFrameAndRef(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm, IMPDmicFrame *ref, IMPBlock block);
* Get audio frame and output reference frame.
*
* @param[in] dmicDevId MAC array audio device number.
* @param[in] dmicChnId MAC array audio channel number
* @param[out] chnFrm   MAC array channel audio frame structure pointer.
* @param[out] ref reference frame structure pointer.
* @param[in] block block Blocking / non blocking identifier.
*
* @retval 0 success.
* @retval non-0 failure.
**/
int IMP_DMIC_GetFrameAndRef(int dmicDevId, int dmicChnId, IMPDmicChnFrame *chnFrm, IMPDmicFrame *ref, IMPBlock block);

/**
 ** @fn int IMP_DMIC_EnableAec(int dmicDevId, int dmicChnId, int aoDevId, int aoChId);
 *
 * Enable audio echo cancellation feature of the specified audio input and audio output.
 *
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number
 * @param[in] aoDevId Need to perform audio echo cancellation of the audio output device number.
 * @param[in] aoChn Need to perform audio echo cancellation of the audio output channel number.
 * @retval 0 success.
 * @retval non-0 failure.
 */
int IMP_DMIC_EnableAec(int dmicDevId, int dmicChnId, int aoDevId, int aoChId);
/**
  * @fn int IMP_DMIC_PollingFrame(int dmicDevId, int dmicChnId, unsigned int timeout_ms);
  * Polling encoded audio stream cache.
  * @param[in] dmicDevId MAC array audio device number.
  * @param[in] dmicChnId MAC array audio channel number
  * @param[in] timeout_ms Polling timeout time.
  *
  * @retval 0 success.
  * @retval non-0 failure.
  *
  * @remarks no.
  *
  * @attention Use the interface before using IMP_DMIC_GetFrame, and when the interface is called successfully, then the audio data is ready, and you can use IMP_DMIC_GetFrame to get audio data.

  */
int IMP_DMIC_DisableAec(int dmicDevId, int dmicChnId);
/**
 * @fn int IMP_DMIC_DisableAec(int dmicDevId, int dmicChnId);
 * close AEC function.
 * @param[in] dmicDevId MAC array audio device number.
 * @param[in] dmicChnId MAC array audio channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention IMP_DMIC_DisableAec should be matche with IMP_DMIC_EnableAec
 *
 */
int IMP_DMIC_PollingFrame(int dmicDevId, int dmicChnId, unsigned int timeout_ms);

/**
*	@fn int IMP_DMIC_SetVol(int dmicDevId, int dmicChnId, int dmicVol);
*   Set audio input volume.
*
*   @param[in] dmicDevId MAC array audio device number.
*   @param[in] dmicChnId MAC array audio channel number
*   @param[in] dmic vol of MAC array
*   @retval 0 success.
*   @retval non-0 failure.
*	@remarks volume in the range of [-30 ~ 120]. - 30 represents mute, 120 is to amplify the sound of to 30dB, step 0.5dB.
*	@remarks 60 is to set the volume to a critical point. In this case, the software does not increase or decrease the volume, when the volume          value is less than 60, for each drop of 1, the volume is decreased by 0.5dB; when the volume value is greater than 60, for each rise of 1, the volume is increased by 0.5dB.
*/
int IMP_DMIC_SetVol(int dmicDevId, int dmicChnId, int dmicVol);
/**
*	@fn int IMP_DMIC_SetVol(int dmicDevId, int dmicChnId, int dmicVol);
*   Set audio input volume.
*
*   @param[in] dmicDevId MAC array audio device number.
*   @param[in] dmicChnId MAC array audio channel number
*   @param[in] dmic vol of MAC array.
*   @retval 0 success.
*   @retval non-0 failure.
*	@remarks no.
*	@attention no.
*/
int IMP_DMIC_GetVol(int dmicDevId, int dmicChnId, int *dmicVol);


/**
*  @fn int IMP_DMIC_SetGain(int dmicDevId, int dmicChnId, int dmicGain);
*  Set MAC array dmic input gain.
*
*   @param[in] dmicDevId MAC array audio device number.
*   @param[in] dmicChnId MAC array audio channel number
*   @param[out] dmic of MAC array input gain, range [0 ~ 31].
*
*   @retval 0 success.
*   @retval non-0 failure.
*/
int IMP_DMIC_SetGain(int dmicDevId, int dmicChnId, int dmicGain);

/**
*  @fn int IMP_DMIC_GetGain(int dmicDevId, int dmicChnId, int *dmicGain);
*  Get MAC array dmic input gain.
*
*   @param[in] dmicDevId MAC array audio device number.
*   @param[in] dmicChnId MAC array audio channel number
*
*   @retval 0 success.
*   @retval non-0 failure.
*
*   @remarks no.
*/
int IMP_DMIC_GetGain(int dmicDevId, int dmicChnId, int *dmicGain);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

