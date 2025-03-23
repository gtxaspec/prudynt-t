/*
 * Audio utils header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_AUDIO_H__
#define __IMP_AUDIO_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * IMP audio input and output header file
 */

/**
 * @defgroup IMP_Audio
 * @ingroup imp
 * @brief audio module, including audio record and playback, audio encoding and decoding, volume and gain setting, (audio) echo cancellation, automatic gain function and so on
 *
 * @section audio_summary 1 overview
 * Audio function includes 5 modules: audio input, audio output, audio echo cancellation, audio encoding and audio decoding. \n
 * The existence of the audio input and audio output is the concept of the equipment and the channel, we all know that a MIC is a Device, and also can have multiple Channel input. \n
 * Similarly a SPK is a playback Device, and can also have multiple Channel output. \n
 * One Device supports only one Channel in this current version of the audio API. \n
 * Audio echo cancellation is located in the audio input interface, explicit explanation is embodied in the functional description. \n
 * Audio encoding, the current audio API can support such format audio coding: PT_G711A, PT_G711U and PT_G726 , when it comes to add a new encoding method, it is a must to register the encoder. \n
* Audio decoding, the current audio API can support such format audio coding: PT_G711A, PT_G711U and PT_G726 , when it comes to add a new decoding method, it is a must to register the decoder. \n

 * @section audio_function_description 2
 * The following is a detailed description of each module
* @subsection audio_in 2.1 Audio input
 * Audio input Device ID correspondence, 0: corresponding digital MIC; 1: corresponding simulation MIC \n
 * Audio input Channel, the current API only supports single channel. \n
 * Set the audio input volume, volume range: [-30 ~ 120]. - 30 represents mute, 120 represents the amplification of sound to 30dB, step 0.5dB. 60 means that the volume is set to a critical point,\n
 * in that case ,software does not increase or decrease the volume, when the volume value is less than 60, for each drop of 1, the volume is reduced by 0.5dB; when the volume value is greater than 60, for each rise of 1 the volume is increased by 0.5dB.

 * @subsection audio_out 2.2 Audio output
 * Audio output Device ID correspondence, 0: corresponding to the default SPK; 1: MIC corresponding to other SPK \n
 * Audio output Channel,the current API only supports single channel. \n
 * Set the audio output volume, volume range: [-30 ~ 120]. - 30 represents mute, 120 represents the amplification of sound to 30dB, step 0.5dB. 60 means that the volume is set to a critical point,\n
 * in that case ,software does not increase or decrease the volume, when the volume value is less than 60, for each drop of 1, the volume is reduced by 0.5dB; when the volume value is greater than 60, for each rise of 1 the volume is increased by 0.5dB.

 * @subsection audio_aec 2.3 Audio echo cancellation(AEC)
 * Audio echo cancellation function belongs to the audio input interface, so to enable echo cancellation, you must first enable audio input device and channel. \n
 * Audio echo cancellation currently supports audio sampling rate of 8K and 16K. A frame data sample number is the multiple of 10ms audio data (e.g., 8K sampling rate, the data fed into: 8000 x 2 / 100 = 160byte integer multiple).\n
 * Audio echo cancellation for different devices and different packages will have different sound effects. \n
 * Adaptive support is not available in Audio Echo Cancellation (the AEC is not automatically configured), so for different devices there will be different echo cancellation parameters, \n
 * The parameter file of the Audio echo cancellation is located in the /etc/webrtc_profile.ini configuration file. \n

 * The profile format is as follows (Main need to debug the three parameters): \n
 * [Set_Far_Frame] \n
 * Frame_V=0.3 \n
 * [Set_Near_Frame] \n
 * Frame_V=0.1 \n
 * delay_ms=150 \n
 *
 * [Set_Far_Frame] represents the remote(far-end) parameter, the SPK represents the playback data parameters. \n
 * Fram_V represents audio amplitude scaling, the playback data can be adjusted by adjusting these parameters (this is used only for echo cancellation). \n
 * [Set_Near_Frame] represents the proximal(near-end) parameter, the MIC represents record data parameters. \n
 * Fram_V represents the audio amplitude ratio, the adjustment of these parameters can adjust the amplitude of the recording data (this is only used for echo cancellation). \n
 * Delay_ms, because of the delay between software and hardware, and the position between the SPK and MIC(they are placed at a certain distance), the SPK playback data will be processed by the MIC again. So, there will be some delay because the SPK data will have some impact on the MIC data. \n
 * This time represents the time difference of the playback data in data recording. (delay to assure that there won't be any echo at all). \n

 * @subsection audio_enc 2.4 Audio encoding
 * The current audio API supports PT_G711A, PT_G711U and PT_G726 format audio coding, if you need to add a new encoding method, you need to call the IMP_AENC_RegisterEncoder interface to register the encoder.

 * @subsection audio_dec 2.5 Audio decoding
 * The current audio API supports PT_G711A, PT_G711U and PT_G726 format audio decoding, if you need to add a new decoding method, you need to call the IMP_ADEC_RegisterDecoder interface to register the decoder.
 * @{
 */

/**
 * Maximum number of audio frames
 */
#define MAX_AUDIO_FRAME_NUM 50

/**
 * Audio stream blocking type
 */
typedef enum {
	BLOCK = 0,				/**< block */
	NOBLOCK = 1,			/**< not block */
} IMPBlock;

/**
 * Audio sampling rate definition.
 */
typedef enum {
	AUDIO_SAMPLE_RATE_8000	= 8000,		/**< 8KHz sampling rate */
	AUDIO_SAMPLE_RATE_12000 = 12000,	/**< 12KHz sampling rate */
	AUDIO_SAMPLE_RATE_16000 = 16000,	/**< 16KHz sampling rate */
	AUDIO_SAMPLE_RATE_24000 = 24000,	/**< 24KHz sampling rate */
	AUDIO_SAMPLE_RATE_32000 = 32000,	/**< 32KHz sampling rate */
	AUDIO_SAMPLE_RATE_44100 = 44100,	/**< 44.1KHz sampling rate */
	AUDIO_SAMPLE_RATE_48000 = 48000,	/**< 48KHz sampling rate */
	AUDIO_SAMPLE_RATE_96000 = 96000,	/**< 96KHz sampling rate */
} IMPAudioSampleRate;

/**
 * Audio sampling precision definition.
 */
typedef enum {
	AUDIO_BIT_WIDTH_16 = 16,		/**< 16bit sampling precision*/
} IMPAudioBitWidth;

/**
 * Audio echo cancellation(AEC) channel select.
 */
typedef enum {
	AUDIO_AEC_CHANNEL_FIRST_LEFT = 0,	/**< the first channel or left channel*/
	AUDIO_AEC_CHANNEL_SECOND_RIGHT = 1,	/**< the second channel or right channel*/
	AUDIO_AEC_CHANNEL_THIRD = 2,	/**< the third channel*/
	AUDIO_AEC_CHANNEL_FOURTH = 3,	/**< the fourth channel*/
} IMPAudioAecChn;

/**
 * Audio channel mode definition.
 */
typedef enum {
	AUDIO_SOUND_MODE_MONO	= 1,	/**< Single channel*/
	AUDIO_SOUND_MODE_STEREO = 2,	/**< Double channel*/
} IMPAudioSoundMode;

/**
 * Define audio payload type enumeration.
 */
typedef enum {
	PT_PCM		= 0,
	PT_G711A	= 1,
	PT_G711U	= 2,
	PT_G726 	= 3,
	PT_AEC		= 4,
	PT_ADPCM	= 5,
	PT_MAX		= 6,
} IMPAudioPalyloadType;

/**
 * Define the decoding method.
 */
typedef enum {
	ADEC_MODE_PACK   = 0,	/**< Pack decoding*/
	ADEC_MODE_STREAM = 1,	/**< Stream decoding*/
} IMPAudioDecMode;

/**
 * Audio input and output device attribute.
 */
typedef struct {
	IMPAudioSampleRate samplerate;		/**< Audio sampling rate*/
	IMPAudioBitWidth bitwidth;			/**< Audio sampling precision*/
	IMPAudioSoundMode soundmode;		/**< Audio channel mode*/
	int frmNum;							/**< Number of cached frames, range: [2, MAX_AUDIO_FRAME_NUM]*/
	int numPerFrm;						/**< Number of sample points per frame */
	int chnCnt;							/**< Number of channels supported*/
} IMPAudioIOAttr;

/**
 * Audio frame structure.
 */
typedef struct {
	IMPAudioBitWidth bitwidth;			/**< Audio sampling precision*/
	IMPAudioSoundMode soundmode;		/**< Audio channel mode*/
	uint32_t *virAddr;					/**< Audio frame data virtual address*/
	uint32_t phyAddr;					/**< Audio frame data physical address*/
	int64_t timeStamp;					/**< Audio frame data time stamp*/
	int seq;							/**< Audio frame data serial number*/
	int len;							/**< Audio frame data length*/
} IMPAudioFrame;

/**
 * Audio channel parameter structure.
 */
typedef struct {
	int usrFrmDepth;					/**< Audio frame buffer depth*/
	IMPAudioAecChn aecChn;				/**< Audio echo cancellation(AEC) channel select*/
	int Rev;							/**< retain*/
} IMPAudioIChnParam;

/**

 */
typedef struct {
	int chnTotalNum;				/**< The total number of cached output channel*/
	int chnFreeNum;					/**< Free cache blocks*/
	int chnBusyNum;					/**< The number of cache be used*/
} IMPAudioOChnState;

/**
 * Define audio stream structure.
 */
typedef struct {
	uint8_t *stream;				/**< Data stream pointer*/
	uint32_t phyAddr;				/**< Data stream physical address*/
	int len;						/**< Audio stream length*/
	int64_t timeStamp;				/**< time stamp*/
	int seq;						/**< Audio stream serial number*/
} IMPAudioStream;

/**
 * Define audio encoding channel attribute structure.
 */
typedef struct {
	IMPAudioPalyloadType type;				/**< Audio payload data type*/
	int bufSize;							/**<  buf size, in order to frame the unit, [2 ~ MAX_AUDIO_FRAME_NUM]*/
	uint32_t *value;						/**< Protocol attribute pointer*/
} IMPAudioEncChnAttr;

/**
 * Define the encoder attribute structure.
 */
typedef struct {
	IMPAudioPalyloadType type;		/**< Encoding protocol type*/
	int maxFrmLen;					/**< Maximum code stream length*/
	char name[16];					/**< encoder name */
	int (*openEncoder)(void *encoderAttr, void
			*encoder);
	int (*encoderFrm)(void *encoder, IMPAudioFrame
			*data, unsigned char *outbuf,int *outLen);
	int (*closeEncoder)(void *encoder);
} IMPAudioEncEncoder;

/**
 * Define the decoded channel attribute structure.
 */
typedef struct {
	IMPAudioPalyloadType type;			/**< Audio decoding protocol type*/
	int bufSize;						/**< Audio decoder cache size*/
	IMPAudioDecMode mode;				/**< Decoding mode*/
	void *value;						/**< Specific protocol attribute pointer*/
} IMPAudioDecChnAttr;

/**
 * Define decoder attribute structure.
 */
typedef struct {
	IMPAudioPalyloadType type;		/**< Audio decoding protocol type*/
	char name[16];					/**< Audio encoder name*/
	int (*openDecoder)(void *decoderAttr, void
			*decoder);
	int (*decodeFrm)(void *decoder, unsigned char
			*inbuf,int inLen, unsigned short *outbuf,int
			*outLen,int *chns);
	int (*getFrmInfo)(void *decoder, void *info);
	int (*closeDecoder)(void *decoder);
} IMPAudioDecDecoder;

/**
 * Define AGC gain structure.
 */
typedef struct {
	int TargetLevelDbfs;	/**< Gain level, the value of [0, 31], this refers to the target volume level, the unit is dB, is negative value. The smaller the value, the greater the volume.*/
	int CompressionGaindB;	/**< Set the maximum gain value, [0, 90], 0 represents no gain, the greater the value, the higher the gain.*/
} IMPAudioAgcConfig;

/**
 * Defines level of noise suppression.
 */
enum Level_ns {
	NS_LOW,			/**< Low level noise suppression*/
	NS_MODERATE,	/**< Medium level noise suppression*/
	NS_HIGH,		/**< High level noise suppression*/
	NS_VERYHIGH		/**< Maximum level noise suppression*/
};

/**
 * @fn int IMP_AI_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr)
 *
 * Set audio input device attribute.
 *
 * @param[in] audioDevId Audio device number
 * @param[in] attr Audio device attribute pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks Sample code
 * @code
 * int devID = 1;
 * IMPAudioIOAttr attr;
 * attr.samplerate = AUDIO_SAMPLE_RATE_8000;
 * attr.bitwidth = AUDIO_BIT_WIDTH_16;
 * attr.soundmode = AUDIO_SOUND_MODE_MONO;
 * attr.frmNum = 20;
 * attr.numPerFrm = 400;
 * attr.chnCnt = 1;
 * ret = IMP_AI_SetPubAttr(devID, &attr);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Set Audio in %d attr err: %d\n", devID, ret);
 *		return ret;
 * }
 * @endcode
 *
 * @attention Need to be called before IMP_AI_Enable.
 */
int IMP_AI_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AI_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr)
 *
 * Get the attribute of the audio input device
 *
 * @param[in] audioDevId Audio device number
 * @param[out] attr Audio device attribute pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AI_Enable(int audioDevId)
 *
 * Enable audio input device.
 *
 * @param[in] audioDevId Audio device number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_Enable(int audioDevId);

/**
 * @fn int IMP_AI_Disable(int audioDevId)
 *
 * Disable audio input device
 *
 * @param[in] audioDevId Audio device number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention It supports the use of IMP_AI_Enable. IMP_AI_Disable must be performed before putting the system in sleeping mode.
 */
int IMP_AI_Disable(int audioDevId);

/**
 * @fn int IMP_AI_EnableChn(int audioDevId, int aiChn)
 *
 * Enable audio input channel
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn Audio input channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention Must first enable device.
 */
int IMP_AI_EnableChn(int audioDevId, int aiChn);

/**
 * @fn int IMP_AI_DisableChn(int audioDevId, int aiChn)
 *
 * Disable audio input channel
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn Audio input channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention It supports the use of IMP_AI_EnableChn.
 */
int IMP_AI_DisableChn(int audioDevId, int aiChn);

/**
 * @fn int IMP_AI_PollingFrame(int audioDevId, int aiChn, unsigned int timeout_ms)
 *
 * Polling audio stream cache.
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn Audio input channel number
 * @param[in] timeout_ms Polling timeout time.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention Use the interface before using IMP_AI_GetFrame, and when the interface is called successfully, then the audio data is ready, and you can use IMP_AI_GetFrame to get audio data.
 */
int IMP_AI_PollingFrame(int audioDevId, int aiChn, unsigned int timeout_ms);

/**
 * @fn int IMP_AI_GetFrame(int audioDevId, int aiChn, IMPAudioFrame *frm, IMPBlock block)
 *
 * Get audio frame.
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn Audio input channel number
 * @param[out] frm Audio frame structure pointer.
 * @param[in] block Blocking / non blocking identifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks Sample code
 * @code
 * IMPAudioFrame frm;
 * // Get audio frame
 * ret = IMP_AI_GetFrame(devID, chnID, &frm, BLOCK);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
 *		return ret;
 * }
 *
 * fwrite(frm.virAddr, 1, frm.len, record_file); // use audio frame data
 *
 * // Release audio frame
 * ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio release frame data error\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention no.
 */
int IMP_AI_GetFrame(int audioDevId, int aiChn, IMPAudioFrame *frm, IMPBlock block);

/**
 * @fn int IMP_AI_ReleaseFrame(int audioDevId, int aiChn, IMPAudioFrame *frm)
 *
 * Release audio frame
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn Audio input channel number
 * @param[in] frm Audio frame structure pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention It supports the use of IMP_AI_GetFrame.
 */
int IMP_AI_ReleaseFrame(int audioDevId, int aiChn, IMPAudioFrame *frm);

/**
 * @fn int IMP_AI_SetChnParam(int audioDevId, int aiChn, IMPAudioIChnParam *chnParam)
 *
 * Set audio input channel parameters.
 *
 * @param[in] audioDevId Audio device number
 * @param[in] aiChn audio input channel number
 * @param[in] chnParam audio frame structure pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * int chnID = 0;
 * IMPAudioIChnParam chnParam;
 * chnParam.usrFrmDepth = 20;
 * ret = IMP_AI_SetChnParam(devID, chnID, &chnParam);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "set ai %d channel %d attr err: %d\n", devID, chnID, ret);
 *		return ret;
 * }
 * @endcode
 *
 * @attention Supporting the use of IMP_AI_EnableChn.
 */
int IMP_AI_SetChnParam(int audioDevId, int aiChn, IMPAudioIChnParam *chnParam);

/**
 * @fn int IMP_AI_GetChnParam(int audioDevId, int aiChn, IMPAudioIChnParam *chnParam)
 *
 * Set audio input channel parameters
 *
 * @param[in] audioDevId audio device number
 * @param[in] aiChn audio input channel number
 * @param[out] chnParam audio channel parameters
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_GetChnParam(int audioDevId, int aiChn, IMPAudioIChnParam *chnParam);

/**
 * @fn int IMP_AI_EnableAec(int aiDevId, int aiChn, int aoDevId, int aoChn)
 *
 * Enable audio echo cancellation feature of the specified audio input and audio output.
 *
 * @param[in] aiDevId Need to perform audio echo cancellation of the audio input device number.
 * @param[in] aiChn Need to perform audio echo cancellation of the audio input channel number.
 * @param[in] aoDevId Need to perform audio echo cancellation of the audio output device number.
 * @param[in] aoChn Need to perform audio echo cancellation of the audio output channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks The Audio echo cancellation for different devices and different packages will have different sound effects.
 * @remarks Adaptive support is not available in Audio Echo Cancellation (the AEC is not automatically configured), so for different devices there will be different echo cancellation parameters, \n
 * @remarks Only use the functional effect is not really good.
 * @remarks The parameter file of the Audio echo cancellation is located in the /etc/webrtc_profile.ini configuration file. \n

 * @remarks The profile format is as follows (Main need to debug the three parameters): \n
 * @remarks [Set_Far_Frame] \n
 * @remarks Frame_V=0.3 \n
 * @remarks [Set_Near_Frame] \n
 * @remarks Frame_V=0.1 \n
 * @remarks delay_ms=150 \n
 *
 * @remarks [Set_Far_Frame] represents the remote(far-end) parameter, the SPK represents the playback data parameters. \n
 * @remarks Fram_V represents audio amplitude scaling, the playback data can be adjusted by adjusting these parameters (this is used only for echo cancellation). \n
 * @remarks [Set_Near_Frame] represents the proximal(near-end) parameter, the MIC represents record data parameters. \n
 * @remarks Fram_V represents the audio amplitude ratio, the adjustment of these parameters can adjust the amplitude of the recording data (this is only used for echo cancellation). \n
 * @remarks Delay_ms, because of the delay between software and hardware, and the position between the SPK and MIC(they are placed at a certain distance), the SPK playback data will be processed by the MIC again. So, there will be some delay because the SPK data will have some impact on the MIC data. \n
 * @remarks This time represents the time difference of the playback data in data recording. (delay to assure that there won't be any echo at all). \n
 *
 * @attention In fact, the interface will only check for aiDevId and aiChn, but it is better to enabele these two channels at the same time and then call the current one. \n
 * when the audio input channel is closed, the Audio Echo Cancellation feature also is turned off. In case of using it again, you will have to turn it on.
 */
int IMP_AI_EnableAec(int aiDevId, int aiChn, int aoDevId, int aoChn);

/**
@fn  int IMP_AI_Set_WebrtcProfileIni_Path(char *path)
*Set AEC profile " webrtc_profile.ini" pass.

*path parameter:It is used to set the path of the configuration file.

For example, the user wants to place the configuration file in the / system directory:                      IMP_AI_Set_WebrtcProfileIni_Path("/system");

*This function must be in   IMP_AI_EnableAec(int aiDevId, int aiChn, int aoDevId, int aoChn)before used.
*This function must be in   IMP_AI_EnableAlgo(int audioDevId, int aiChn)before used.
*This function must be in   IMP_AO_EnableAlgo(int audioDevId, int aoChn)before used.
*return value 0
*/
int IMP_AI_Set_WebrtcProfileIni_Path(char *path);


/**
 * @fn int IMP_AI_DisableAec(int aiDevId, int aiChn)
 *
 * Disable audio echo cancellation feature.
 *
 * @param[in] aiDevId Audio input device number
 * @param[in] aiChn audio input channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_DisableAec(int aiDevId, int aiChn);

/**
 * @fn int IMP_AI_EnableNs(IMPAudioIOAttr *attr, int mode)
 *
 * Enable specified audio input noise suppression.
 *
 * @param[in] attr Noise suppression is required for the audio attribute.
 * @param[in] mode Noise suppression level 0 ~ 3, see Level_ns.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks The mode parameter of noise suppression represents the noise suppression level (range [0 ~ 3]), the higher the level, the cleaner the noise suppression.
 * @remarks However, The cleaner the noise is, more details of the sound will be lost, here we have a contradiction so we need to make tradeoff while processing the noise suppression.
 *
 * @attention Audio echo cancellation contains the noise suppression function, if the audio echo cancellation is enabled, it is not required to enable the noise suppression.
 */
int IMP_AI_EnableNs(IMPAudioIOAttr *attr, int mode);

/**
 * @fn int IMP_AI_DisableNs(void)
 *
 * Disable noise suppression
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_DisableNs(void);

/**
 * @fn int IMP_AI_EnableHs()
 *
 * Enable audio howling suppression.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_EnableHs();

/**
 * @fn int IMP_AI_DisableHs(void)
 *
 * Disaable audio howling suppression.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_DisableHs(void);

/**
 * @fn int IMP_AI_EnableAgc(IMPAudioIOAttr *attr, IMPAudioAgcConfig agcConfig)
 *
 * Enable automatic gain of audio input.
 *
 * @param[in] attr Requires automatic gain of the audio attribute.
 * @param[in] agcConfig Automatic gain parameter configuration, configuration magnification.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks Note the configuration of the AGC. agcConfig amplification's main parameters have their own configuration, check the specific gain IMPAudioAgcConfig instructions.
 * @remarks Note that AGC can gain sound amplification, but if the gain parameter is not appropriate, it will lead to broken noise, please adjust carefully the parameters.
 *
 * @attention Audio echo cancellation contains the AGC function, which means it is not required to perform automatic gain if the echo cancellation is enabled.
 */
int IMP_AI_EnableAgc(IMPAudioIOAttr *attr, IMPAudioAgcConfig agcConfig);

/**
 * @fn int IMP_AI_DisableAgc(void)
 *
 * Disable AI automatic gain feature.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_DisableAgc(void);

/**
 * @fn int IMP_AO_EnableAgc(IMPAudioIOAttr *attr, IMPAudioAgcConfig agcConfig)
 *
 * Enable audio output automatic gain feature.
 *
 * @param[in] attr need automatic gain of the audio attribute.
 * @param[in] agcConfig automatic gain parameter configuration, configuration magnification.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks agcConfig amplification's main parameters have their own configuration, check the specific gain IMPAudioAgcConfig instructions.
 * @remarks Note that AGC can gain sound amplification, but if the gain parameter is not appropriate, it will lead to broken noise, please adjust carefully the parameters.
 *
 * @attention Audio echo cancellation contains the AGC function, which means it is not required to perform automatic gain if the echo cancellation is enabled.
 */
int IMP_AO_EnableAgc(IMPAudioIOAttr *attr, IMPAudioAgcConfig agcConfig);

/**
 * @fn int IMP_AO_DisableAgc(void)
 *
 * Disable AO automatic gain feature.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_DisableAgc(void);

/**
 * @fn int IMP_AI_EnableHpf(IMPAudioIOAttr *attr)
 *
 * Enable audio input for high pass filtering.
 *
 * @param[in] attr need high pass filtering of audio attribute.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention Audio echo cancellation contains the HPF function, if Audio Echo Cancellation is enabled, so HPF will be automatically enabled.
 */
int IMP_AI_EnableHpf(IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AI_SetHpfCoFrequency(int cofrequency)
 *
 * Set the cut-off frequency of high pass filtering for audio input.
 *
 * @param[in] cofrequency the cut-off frequency of high pass filter.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention while enabling the high-pass filter for audio input,setting the cut-off frequency of the high-pass filter first.
 */
int IMP_AI_SetHpfCoFrequency(int cofrequency);

/**
 * @fn int IMP_AI_DisableHpf(void)
 *
 * Disable AI high pass filtering function.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AI_DisableHpf(void);

/**
 * @fn int IMP_AO_EnableHpf(IMPAudioIOAttr *attr)
 *
 * Enable audio output for high pass filtering.
 *
 * @param[in] attr need high pass filtering of audio attribute.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention Audio echo cancellation contains the HPF function, if audio echo cancellation is enabled, so you do not need to enable HPF.
 */
int IMP_AO_EnableHpf(IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AO_SetHpfCoFrequency(int cofrequency)
 *
 * Set the cut-off frequency of high pass filtering for audio output.
 *
 * @param[in] cofrequency the cut-off frequency of high pass filter.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention while enabling the high-pass filter for audio output,setting the cut-off frequency of the high-pass filter first.
 */
int IMP_AO_SetHpfCoFrequency(int cofrequency);

/**
 * @fn int IMP_AO_DisableHpf(void)
 *
 * Disable AO high pass filtering function.
 *
 * @param no.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_DisableHpf(void);

/**
 * @fn int IMP_AO_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr)
 *
 * Set audio input and output device attribute.
 *
 * @param[in] audioDevId audio device number
 * @param[in] attr audio output device attribute pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_SetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AO_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr)
 *
 * Get audio input and output device attribute.
 *
 * @param[in] audioDevId audio device number
 * @param[out] attr audio output device attribute pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_GetPubAttr(int audioDevId, IMPAudioIOAttr *attr);

/**
 * @fn int IMP_AO_Enable(int audioDevId)
 *
 * Enable audio output device
 *
 * @param[in] audioDevId audio device number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention The IMP_AO_SetPubAttr. must be called before
 */
int IMP_AO_Enable(int audioDevId);

/**
 * @fn int IMP_AO_Disable(int audioDevId)
 *
 * Disable audio output device
 *
 * @param[in] audioDevId audio device number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_Disable(int audioDevId);

/**
 * @fn int IMP_AO_EnableChn(int audioDevId, int aoChn)
 *
 * Enable audio output channel
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_EnableChn(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_DisableChn(int audioDevId, int aoChn)
 *
 * Disable audio output channel
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_DisableChn(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_SendFrame(int audioDevId, int aoChn, IMPAudioFrame *data, IMPBlock block)
 *
 * Send audio output frame.
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 * @param[in] data audio frame sturcture pointer
 * @param[in] block Blocking / non blocking identifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * while(1) {
 *		size = fread(buf, 1, IMP_AUDIO_BUF_SIZE, play_file);
 *		if(size < IMP_AUDIO_BUF_SIZE)
 *			break;
 *
 *		IMPAudioFrame frm;
 *		frm.virAddr = (uint32_t *)buf;
 *		frm.len = size;
 *		ret = IMP_AO_SendFrame(devID, chnID, &frm, BLOCK);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "send Frame Data error\n");
 *			return ret;
 *		}
 * }
 * @endcode
 *
 * @attention no.
 */
int IMP_AO_SendFrame(int audioDevId, int aoChn, IMPAudioFrame *data, IMPBlock block);

/**
 * @fn int IMP_AO_PauseChn(int audioDevId, int aoChn)
 *
 * Pause audio output channel
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_PauseChn(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_ResumeChn(int audioDevId, int aoChn)
 *
 * Resume audio output channel
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_ResumeChn(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_ClearChnBuf(int audioDevId, int aoChn)
 *
 * Clear the current audio data cache in the audio output channel.
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_ClearChnBuf(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_QueryChnStat(int audioDevId, int aoChn, IMPAudioOChnState *status)
 *
 * Query the current audio data cache status in the audio output channel.
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 * @param[out] status Cache state structure pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
int IMP_AO_QueryChnStat(int audioDevId, int aoChn, IMPAudioOChnState *status);

/**
 * @fn int IMP_AENC_CreateChn(int aeChn, IMPAudioEncChnAttr *attr)
 *
 * Create audio encode channel.
 *
 * @param[in] aeChn channel number
 * @param[in] attr Audio encode channel attribute pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * int AeChn = 0;
 * IMPAudioEncChnAttr attr;
 * attr.type = PT_G711A;
 * attr.bufSize = 20;
 * ret = IMP_AENC_CreateChn(AeChn, &attr);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio encode create channel failed\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention The Current SDK supports PT_G711A, PT_G711U and PT_G726 encoding. \n
 * So the use of SDK encoding, only needs attr.type = PT_G711A. \n
 * How to use a custom encoder? You need to register the encoder, the sample code is in the registration interface instructions.
 */
 int IMP_AENC_CreateChn(int aeChn, IMPAudioEncChnAttr *attr);

/**
 * @fn int IMP_AENC_DestroyChn(int aeChn)
 *
 * Destory audio encode channel
 *
 * @param[in] aeChn channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention Supporting the use of IMP_AENC_CreateChn.
 */
 int IMP_AENC_DestroyChn(int aeChn);

/**
 * @fn int IMP_AENC_SendFrame(int aeChn, IMPAudioFrame *frm)
 *
 * Send audio encoding audio frame
 *
 * @param[in] aeChn channel number
 * @param[in] frm Audio frame structure pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * while(1) {
 *		// Read a frame of data
 *		ret = fread(buf_pcm, 1, IMP_AUDIO_BUF_SIZE, file_pcm);
 *		if(ret < IMP_AUDIO_BUF_SIZE)
 *			break;
 *
 *		// encode
 *		IMPAudioFrame frm;
 *		frm.virAddr = (uint32_t *)buf_pcm;
 *		frm.len = ret;
 *		ret = IMP_AENC_SendFrame(AeChn, &frm);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio encode send frame failed\n");
 *			return ret;
 *		}
 *
 *		// Get encode stream
 *		IMPAudioStream stream;
 *		ret = IMP_AENC_GetStream(AeChn, &stream, BLOCK);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio encode get stream failed\n");
 *			return ret;
 *		}
 *
 *		// Use encode stream
 *		fwrite(stream.stream, 1, stream.len, file_g711);
 *
 *		// Release encode stream
 *		ret = IMP_AENC_ReleaseStream(AeChn, &stream);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio encode release stream failed\n");
 *			return ret;
 *		}
 * }
 * @endcode
 *
 * @attention no.
 */
 int IMP_AENC_SendFrame(int aeChn, IMPAudioFrame *frm);

/**
 * @fn int IMP_AENC_PollingStream(int AeChn, unsigned int timeout_ms)
 *
 * Polling encoded audio stream cache.
 *
 * @param[in] AeChn Audio encoding input channel number.
 * @param[in] timeout_ms Polling timeout time
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention The current interface is used before using IMP_AENC_GetStream. After calling successfully the current interface, we will have the audio encoding data ready then you can use the IMP_AENC_GetStream to get the encoded data.
 */
int IMP_AENC_PollingStream(int AeChn, unsigned int timeout_ms);

/**
 * @fn int IMP_AENC_GetStream(int aeChn, IMPAudioStream *stream ,IMPBlock block)
 *
 * Get the encoded stream.
 *
 * @param[in] aeChn channel number
 * @param[in] stream Get audio encoding
 * @param[in] block Blocking / non blocking identifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks see the IMP_AENC_SendFrame function description for deeper understanding.
 *
 * @attention no.
 */
 int IMP_AENC_GetStream(int aeChn, IMPAudioStream *stream ,IMPBlock block);

/**
 * @fn int IMP_AENC_ReleaseStream(int aeChn,IMPAudioStream *stream)
 *
 * Releases the stream from the audio encoding channel.
 *
 * @param[in] aeChn channel number
 * @param[in] stream Get audio stream pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks see the IMP_AENC_SendFrame function description.
 *
 * @attention no.
 */
 int IMP_AENC_ReleaseStream(int aeChn,IMPAudioStream *stream);

/**
 * @fn int IMP_AENC_RegisterEncoder(int *handle, IMPAudioEncEncoder *encoder)
 *
 * Register encoder
 *
 * @param[in] ps32handle register handle
 * @param[in] encoder Encoder attribute structure.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * int handle_g711a = 0;
 * IMPAudioEncEncoder my_encoder;
 * my_encoder.maxFrmLen = 1024;
 * sprintf(my_encoder.name, "%s", "MY_G711A");
 * my_encoder.openEncoder = NULL; // Encoder callback function
 * my_encoder.encoderFrm = MY_G711A_Encode_Frm; // Encoder callback function
 * my_encoder.closeEncoder = NULL; // Encoder callback function
 *
 * ret = IMP_AENC_RegisterEncoder(&handle_g711a, &my_encoder);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "IMP_AENC_RegisterEncoder failed\n");
 *		return ret;
 * }
 *
 * // use encoder
 * int AeChn = 0;
 * IMPAudioEncChnAttr attr;
 * attr.type = handle_g711a; // The encoder type is equal to the value of the handle_g711a returned by the successfully registered.
 * attr.bufSize = 20;
 * ret = IMP_AENC_CreateChn(AeChn, &attr);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "imp audio encode create channel failed\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention After registration, the use of the method and the use of SDK in the encoder are all the same. (After registration, clients' methods and SDK prebuilt functions are considered as the SDK functions, that means the clients can change the content of the SDK as they wish)
 */
 int IMP_AENC_RegisterEncoder(int *handle, IMPAudioEncEncoder *encoder);

/**
 * @fn int IMP_AENC_UnRegisterEncoder(int *handle)
 *
 * Release encoder
 *
 * @param[in] ps32handle Register handle (the handle obtained at the time of registration of the encoder).
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_AENC_UnRegisterEncoder(int *handle);

/**
 * @fn int IMP_ADEC_CreateChn(int adChn, IMPAudioDecChnAttr *attr)
 *
 * Create audio decode channel
 *
 * @param[in] adChn channel number
 * @param[in] attr Channel attribute pointer.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * int adChn = 0;
 * IMPAudioDecChnAttr attr;
 * attr.type = PT_G711A;
 * attr.bufSize = 20;
 * attr.mode = ADEC_MODE_PACK;
 * ret = IMP_ADEC_CreateChn(adChn, &attr);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "imp audio decoder create channel failed\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention no.
 */
 int IMP_ADEC_CreateChn(int adChn, IMPAudioDecChnAttr *attr);

/**
 * @fn int IMP_ADEC_DestroyChn(int adChn)
 *
 * Destory audio decoding channel
 *
 * @param[in] adChn channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_ADEC_DestroyChn(int adChn);

/**
 * @fn int IMP_ADEC_SendStream(int adChn, IMPAudioStream *stream, IMPBlock block)
 *
 * Send audio stream to audio decoding channel.
 *
 * @param[in] adChn channel number
 * @param[in] stream audio stream
 * @param[in] block Blocking / non blocking identifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * while(1) {
 *		// Get the data that needs to be decoded
 *		ret = fread(buf_g711, 1, IMP_AUDIO_BUF_SIZE/2, file_g711);
 *		if(ret < IMP_AUDIO_BUF_SIZE/2)
 *			break;
 *
 *		// Send decoding stream
 *		IMPAudioStream stream_in;
 *		stream_in.stream = (uint8_t *)buf_g711;
 *		stream_in.len = ret;
 *		ret = IMP_ADEC_SendStream(adChn, &stream_in, BLOCK);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio encode send frame failed\n");
 *			return ret;
 *		}
 *
 *		// Get decoded data
 *		IMPAudioStream stream_out;
 *		ret = IMP_ADEC_GetStream(adChn, &stream_out, BLOCK);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio decoder get stream failed\n");
 *			return ret;
 *		}
 *
 *		// Use decoded data
 *		fwrite(stream_out.stream, 1, stream_out.len, file_pcm);
 *
 *		// Release decoded data
 *		ret = IMP_ADEC_ReleaseStream(adChn, &stream_out);
 *		if(ret != 0) {
 *			IMP_LOG_ERR(TAG, "imp audio decoder release stream failed\n");
 *			return ret;
 *		}
 * }
 * @endcode
 *
 * @attention no.
 */
 int IMP_ADEC_SendStream(int adChn, IMPAudioStream *stream, IMPBlock block);

/**
 * @fn int IMP_ADEC_PollingStream(int AdChn, unsigned int timeout_ms)
 *
 * Polling decode audio stream cache.
 *
 * @param[in] AdChn audio decode input channel number
 * @param[in] timeout_ms polling timeout time
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention The current interface is used before using IMP_ADEC_GetStream. After calling successfully the current interface, we will have the audio encoding data ready then you can use the IMP_ADEC_GetStream to get the encoded data.
 */
int IMP_ADEC_PollingStream(int AdChn, unsigned int timeout_ms);

/**
 * @fn int IMP_ADEC_GetStream(int adChn, IMPAudioStream *stream ,IMPBlock block)
 *
 * Get the decoded stream.
 *
 * @param[in] adChn channel number
 * @param[in] stream Get decoded stream
 * @param[in] block Blocking / non blocking identifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks See the IMP_ADEC_SendFrame function description for further instructions.
 *
 * @attention no.
 */
int IMP_ADEC_GetStream(int adChn, IMPAudioStream *stream ,IMPBlock block);

/**
 * @fn int IMP_ADEC_ReleaseStream(int adChn,IMPAudioStream *stream)
 *
 * Release of the stream from the audio decoding channel.
 *
 * @param[in] adChn channel number
 * @param[in] stream audio stream pointer
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks See the IMP_ADEC_SendFrame function description.
 *
 * @attention no.
 */
int IMP_ADEC_ReleaseStream(int adChn,IMPAudioStream *stream);

/**
 * @fn int IMP_ADEC_ClearChnBuf(int adChn)
 *
 * Clears the current audio data cache in the audio decode channel.
 *
 * @param[in] adChn channel number
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_ADEC_ClearChnBuf(int adChn);

/**
 * @fn int IMP_ADEC_RegisterDecoder(int *handle, IMPAudioDecDecoder *decoder)
 *
 * Register decoder
 *
 * @param[in] ps32handle register handle
 * @param[in] decoder Decoder attributes structure.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks sample code
 * @code
 * int handle_g711a = 0;
 * IMPAudioDecDecoder my_decoder;
 * sprintf(my_decoder.name, "%s", "MY_G711A");
 * my_decoder.openDecoder = NULL; // Decoder callback function
 * my_decoder.decodeFrm = MY_G711A_Decode_Frm; // Decoder callback function
 * my_decoder.getFrmInfo = NULL; // Decoder callback function
 * my_decoder.closeDecoder = NULL; // Decoder callback function
 *
 * // Register decoder
 * ret = IMP_ADEC_RegisterDecoder(&handle_g711a, &my_decoder);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "IMP_ADEC_RegisterDecoder failed\n");
 *		return ret;
 * }
 *
 * // use decoder
 * int adChn = 0;
 * IMPAudioDecChnAttr attr;
 * attr.type = handle_g711a; // The encoder type is equal to the value of the handle_g711a returned by the successfully registered.
 * attr.bufSize = 20;
 * attr.mode = ADEC_MODE_PACK;
 * // create decode channel
 * ret = IMP_ADEC_CreateChn(adChn, &attr);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "imp audio decoder create channel failed\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention After registration, the use of the method as well as the use of SDK comes with an decoder.
 * @attention After registration, the use of the method and the use of SDK in the decoder are all the same. (After registration, clients' methods and SDK prebuilt functions are considered as the SDK functions, that means the clients can change the content of the SDK as they wish)
 */
 int IMP_ADEC_RegisterDecoder(int *handle, IMPAudioDecDecoder *decoder);

/**
 * @fn int IMP_ADEC_UnRegisterDecoder(int *handle)
 *
 * Unregister Decoder
 *
 * @param[in] ps32handle Register handle (the handle obtained at the time of registration of the decoder).
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_ADEC_UnRegisterDecoder(int *handle);

/**
 * ACODEC configuration.
 */
/**
 * @fn int IMP_AI_SetVol(int audioDevId, int aiChn, int aiVol)
 *
 * Set audio input volume.
 *
 * @param[in] aiDevId audio input device number
 * @param[in] aiChn audio input channel number
 * @param[in] aiVol audio input volume
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks volume in the range of [-30 ~ 120]. - 30 represents mute, 120 is to amplify the sound of to 30dB, step 0.5dB.
 * @remarks 60 is to set the volume to a critical point. In this case, the software does not increase or decrease the volume, when the volume value is less than 60, for each drop of 1, the volume is decreased by 0.5dB; when the volume value is greater than 60, for each rise of 1, the volume is increased by 0.5dB.
 *
 * sample code
 * @code
 * int volume = 60;
 * ret = IMP_AI_SetVol(devID, chnID, volume);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
 *		return ret;
 * }
 * @endcode
 * @attention If the input of the aiVol exceeds the range of [-30 ~ 120]. when it is less than -30 it will take -30 as value, more than 120 it will be considered as 120.
 */
 int IMP_AI_SetVol(int audioDevId, int aiChn, int aiVol);

/**
 * @fn int IMP_AI_GetVol(int audioDevId, int aiChn, int *vol)
 *
 * Get the volume of the audio input.
 *
 * @param[in] aiDevId Audio input device number
 * @param[in] aiChn Audio input channel number
 * @param[out] vol audio input volume
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_AI_GetVol(int audioDevId, int aiChn, int *vol);

/**
 * @fn int IMP_AI_SetVolMute(int audioDevId, int aiChn, int mute)
 *
 * Set audio input mute.
 *
 * @param[in] aiDevId Audio input device number
 * @param[in] aiChn Audio input channel number
 * @param[out] mute Audio input mute flag, mute = 0: off mute, mute = 1: on mute.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks By calling the interface, it can be muted immediately.
 *
 * @attention no.
 */
 int IMP_AI_SetVolMute(int audioDevId, int aiChn, int mute);

/**
 * @fn int IMP_AO_SetVol(int audioDevId, int aoChn, int aoVol)
 *
 * Set audio output channel volume.
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 * @param[in] aoVol audio output volume
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks volume in the range of [-30 ~ 120]. - 30 represents mute, 120 is to amplify the sound of to 30dB, step 0.5dB.
 * @remarks 60 is to set the volume to a critical point. In this case, the software does not increase or decrease the volume, when the volume value is less than 60, for each drop of 1, the volume is decreased by 0.5dB; when the volume value is greater than 60, for each rise of 1, the volume is increased by 0.5dB.
 *
 * @attention If the input of the aiVol exceeds the range of [-30 ~ 120], when it is less than -30 it will take -30 as value , more than 120 it will be 120.
 */
 int IMP_AO_SetVol(int audioDevId, int aoChn, int aoVol);

/**
 * @fn int IMP_AO_GetVol(int audioDevId, int aoChn, int *vol)
 *
 * Get audio output channel volume.
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 * @param[out] aoVol audio output volume
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_AO_GetVol(int audioDevId, int aoChn, int *vol);

/**
 * @fn int IMP_AO_SetVolMute(int audioDevId, int aoChn, int mute)
 *
 * Set audio output mute
 *
 * @param[in] audioDevId audio device number
 * @param[in] aoChn audio output channel number
 * @param[out] mute Audio output mute flag, mute = 0: off mute, mute = 1: on mute.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks By calling the interface, it can be muted immediately.
 *
 * @attention no.
 */
 int IMP_AO_SetVolMute(int audioDevId, int aoChn, int mute);

/**
 * @fn int IMP_AI_SetGain(int audioDevId, int aiChn, int aiGain)
 *
 * Set audio input gain.
 *
 * @param[in] audioDevId Audio input device number
 * @param[in] aiChn Audio input channel number
 * @param[out] aiGain Audio input gain, range [0 ~ 31], correspond to [-18dB ~ 28.5dB], step is 1.5dB.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention aiGain range of [0 ~ 31], if the input value is less than 0, then the value of aiGain will be set to 0 if the value is greater than 31, the value of aiGain will be set to 31
 *
 */
 int IMP_AI_SetGain(int audioDevId, int aiChn, int aiGain);

/**
 * @fn int IMP_AI_GetGain(int audioDevId, int aiChn, int *aiGain)
 *
 * Get AI gain value
 *
 * @param[in] audioDevId Audio input device number
 * @param[in] aiChn Audio input channel number
 * @param[out] aiGain audio input gain
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_AI_GetGain(int audioDevId, int aiChn, int *aiGain);

/**
 * @fn int IMP_AO_SetGain(int audioDevId, int aoChn, int aoGain)
 *
 * Set audio output gain.
 *
 * @param[in] audioDevId Audio output device number
 * @param[in] aoChn audio output channel number
 * @param[out] aoGain Audio output gain, range [0 ~ 0x1f], correspond to [-39dB ~ 6dB], step is 1.5dB.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention aoGain range of [0 ~ 31], if the input value is less than 0, then the value of aoGain will be set to 0.\n
 * aoGain if the value is greater than 31, the value of aiGain will be set to 31
 *
 */
 int IMP_AO_SetGain(int audioDevId, int aoChn, int aoGain);

/**
 * @fn int IMP_AO_GetGain(int audioDevId, int aoChn, int *aoGain)
 *
 * Get audio output gain.
 *
 * @param[in] audioDevId audio output device number.
 * @param[in] aoChn audio output channel number.
 * @param[out] aoGain audio output gain.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 *
 * @attention no.
 */
 int IMP_AO_GetGain(int audioDevId, int aoChn, int *aoGain);

/**
 * @fn int IMP_AO_Soft_Mute(int audioDevId, int aoChn)
 *
 * Output soft mute control.
 *
 * @param[in] audioDevId Audio output device number.
 * @param[in] aoChn Audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks when this interface is called, it wil not be directly in silence mode from normal playback state. It will slow down gradually, until it reaches the silence mode.
 *
 * @attention no.
 */
 int IMP_AO_Soft_Mute(int audioDevId, int aoChn);

/**
 * @fn int IMP_AO_Soft_UNMute(int audioDevId, int aoChn)
 *
 * Output soft unmute control.
 *
 * @param[in] audioDevId Audio output device number.
 * @param[in] aoChn Audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks when this interface is called, it will not directly restore the current volume. It will gradually increase the volume from silence mode until the volume reaches a set of good volume.
 *
 * @attention no.
 */
 int IMP_AO_Soft_UNMute(int audioDevId, int aoChn);

/**
 * @fn int IMP_AI_GetFrameAndRef(int audioDevId, int aiChn, IMPAudioFrame *frm, IMPAudioFrame *ref, IMPBlock block)
 *
 * Get audio frame and output reference frame.
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aiChn audio input channel number.
 * @param[out] frm audio frame structure pointer.
 * @param[out] ref reference frame structure pointer.
 * @param[in] block block and non-block identitifier.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks Sample code
 * @code
 * IMPAudioFrame frm;
 * IMPAudioFrame ref;
 * // Get audio frame and output reference frame
 * ret = IMP_AI_GetFrameAndRef(devID, chnID, &frm, &ref, BLOCK);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
 *		return ret;
 * }
 *
 * fwrite(frm.virAddr, 1, frm.len, record_file); // Use audio frame data
 * fwrite(ref.virAddr, 1, ref.len, ref_file); // Use audio reference frame
 *
 * // Release audio frame
 * ret = IMP_AI_ReleaseFrame(devID, chnID, &frm);
 * if(ret != 0) {
 *		IMP_LOG_ERR(TAG, "Audio release frame data error\n");
 *		return ret;
 * }
 * @endcode
 *
 * @attention no.
 */
 int IMP_AI_GetFrameAndRef(int audioDevId, int aiChn, IMPAudioFrame *frm, IMPAudioFrame *ref, IMPBlock block);

/**
 * @fn int IMP_AI_EnableAecRefFrame(int audioDevId, int aiChn, int audioAoDevId, int aoChn)
 *
 * Open access reference frame.
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aiChn audio input channel number.
 * @param[in] audioAoDevId audio output device number.
 * @param[in] aoChn audio output channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks Use this current interface before using IMP_AI_GetFrameAndRef.
 * @attention no.
 */
 int IMP_AI_EnableAecRefFrame(int audioDevId, int aiChn, int audioAoDevId, int aoChn);

/**
 * @fn int IMP_AI_DisableAecRefFrame(int audioDevId, int aiChn, int audioAoDevId, int aoChn)
 *
 * Close access to the reference frame.
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aiChn audio input channel number.
 * @param[in] audioAoDevId audio output device number.
 * @param[in] aoChn audio output channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 * @attention no.
 */
 int IMP_AI_DisableAecRefFrame(int audioDevId, int aiChn, int audioAoDevId, int aoChn);

/**
 * @fn int IMP_AO_CacheSwitch(int audioDevId, int aoChn, int cache_en)
 * close audio play cache
 * @param[in] audioDevId audio device number.
 * @param[in] aoChn audio output channel number.
 * @param[in] cache_en switch of control cache.
 *
 * @retval 0 success
 * @retval non-0 failure.
 *
 * @remarks no
 * @attention no.
 */
 int IMP_AO_CacheSwitch(int audioDevId, int aoChn, int cache_en);

/**
 * @fn int IMP_AO_FlushChnBuf(int audioDevId, int aoChn);
 * Wait for last audio data to play.
 * @param[in] audioDevId audio device number.
 * @param[in] aoChn audio output channel number.
 *
 * @retval 0 success
 * @retval non-0 failure.
 *
 * @remarks no
 * @attention no.
 */
 int IMP_AO_FlushChnBuf(int audioDevId, int aoChn);

/**
 * @fn int IMP_AI_EnableAlgo(int audioDevId, int aiChn)
 *
 * Enable AI algorithm(NS/AGC/HPF).
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aiChn audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 * @remarks no.
 *
 * @attention none
 */
int IMP_AI_EnableAlgo(int audioDevId, int aiChn);

/**
 * @fn int IMP_AI_DisableAlgo(int audioDevId, int aiChn)
 *
 * Disable AI algorithm(NS/AGC/HPF).
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aiChn audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 * @remarks no.
 *
 * @attention none
 */
int IMP_AI_DisableAlgo(int audioDevId, int aiChn);

/**
 * @fn int IMP_AO_EnableAlgo(int audioDevId, int aoChn)
 *
 * Enable AO algorithm(NS/AGC/HPF).
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aoChn audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 * @remarks no.
 *
 * @attention none
 */
int IMP_AO_EnableAlgo(int audioDevId, int aoChn);
/**
 * @fn int IMP_AO_DisableAlgo(int audioDevId, int aoChn)
 *
 * Disable AO algorithm(NS/AGC/HPF).
 *
 * @param[in] audioDevId audio device number.
 * @param[in] aoChn audio input channel number.
 *
 * @retval 0 success.
 * @retval non-0 failure.
 *
 * @remarks no.
 * @remarks no.
 *
 * @attention none
 */
int IMP_AO_DisableAlgo(int audioDevId, int aoChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_AUDIO_H__ */
