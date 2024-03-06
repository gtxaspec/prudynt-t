/*
 * IMP Encoder func header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_ENCODER_H__
#define __IMP_ENCODER_H__

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
 * IMP Video Encoder header file
 */

/**
 * @defgroup IMP_Encoder
 * @ingroup imp
 * @brief Video Encoder Module(JPEG, H264, H265), it contains encoder channel management, encoder parameter setting and other functions.
 *
 * @section enc_struct 1 Encoder Module Structure
 * The internal structure of the Encoder is as follows:
 * @image html encoder_struct.jpg
 * As shown in the figure above, The Encoder Module consists of several groups(Two groups are supported on T15), Each group consists of Encoder channels.
 * Each Encoder channel is attached with an output stream area, This area consists of multiple buffers.
 *
 * @section enc_channel 2 Encoder Channel
 * A Encoder Channel can realize the encoding of a protocol, Each group can add two encoding channels.
 *
 * @section enc_rc 3 Rate Control
 * @subsection enc_cbr 3.1 CBR
 *		CBR(Constent Bit Rate), Keep the bitrate constant in the encoding statistics time.
 *		Take H264 code as an example, Users can set maxQp, minQp, bitrate an so on.
 *		maxQp, minQp: it is used to control the quality range of an image.
 *		bitrate: It is used for the average encoding rate in the statistical time of clamp bit rate.
 *		When the encoding bitrate is greater than the constant bitrate, the image QP will gradually adjust to maxqp.
 *		When the encoding bitrate is much less than the constant bitrate, the image QP will gradually adjust to minqp.
 *		When the image QP reaches maxqp, QP is clamped to the maximum, bitrate clamping effect is invalid, and the encoding bitrate may exceed bitrate.
 *		When the image QP reaches minqp, QP is clamped to the minimum value, and the bitrate of encoding has reached the maximum value and the image quality is the best.
 * @subsection enc_FixQP 3.2 FixQP
 *		Fix Qp, Fixed QP value.
 *		In the bitrate statistics time, the QP values of all macroblocks in the coded image are the same, and the image QP values set by user.
 * @{
 */

/**
 * Defining the Nalu type of H.264
 */
typedef enum {
	IMP_H264_NAL_UNKNOWN		= 0,	/**< Undefined */
	IMP_H264_NAL_SLICE		    = 1,	/**< A encoding strip for non IDR image */
	IMP_H264_NAL_SLICE_DPA	    = 2,	/**< Encoding stripe data partition block A */
	IMP_H264_NAL_SLICE_DPB	    = 3,	/**< Encoding stripe data partition block B */
	IMP_H264_NAL_SLICE_DPC	    = 4,	/**< Encoding stripe data partition block C */
	IMP_H264_NAL_SLICE_IDR	    = 5,	/**< Encoding strip of IDR image */
	IMP_H264_NAL_SEI			= 6,	/**< SEI(Supplemental Enhancement Information) */
	IMP_H264_NAL_SPS			= 7,	/**< SPS(Sequence Paramater Set) */
	IMP_H264_NAL_PPS			= 8,	/**< PPS(Picture Paramater Set) */
	IMP_H264_NAL_AUD			= 9,	/**< Access unit separator */
	IMP_H264_NAL_FILLER		    = 12,	/**< Fill in data */
} IMPEncoderH264NaluType;

/**
 * Defining the Nalu type of H.265
 */
typedef enum {
    IMP_H265_NAL_SLICE_TRAIL_N      = 0,        /**< Trailing image without reference information */
    IMP_H265_NAL_SLICE_TRAIL_R      = 1,        /**< Trailing image with reference information */
    IMP_H265_NAL_SLICE_TSA_N        = 2,        /**< Sub layer access point image in time domain without reference information */
    IMP_H265_NAL_SLICE_TSA_R        = 3,        /**< Sub layer access point image in time domain with reference information */
    IMP_H265_NAL_SLICE_STSA_N       = 4,        /**< Step by step time domain sub layer access point image without reference information */
    IMP_H265_NAL_SLICE_STSA_R       = 5,        /**< Step by step time domain sub layer access point image with reference information */
    IMP_H265_NAL_SLICE_RADL_N       = 6,        /**< The front image can be decoded randomly without reference information */
    IMP_H265_NAL_SLICE_RADL_R       = 7,        /**< The front image can be decoded randomly with reference information */
    IMP_H265_NAL_SLICE_RASL_N       = 8,        /**< Skip the front image of random access, without reference information */
    IMP_H265_NAL_SLICE_RASL_R       = 9,        /**< Skip the front image of random access, with reference information */
    IMP_H265_NAL_SLICE_BLA_W_LP     = 16,       /**< Breakpoint connection access with front image */
    IMP_H265_NAL_SLICE_BLA_W_RADL   = 17,       /**< Breakpoint connection access with front image RADL */
    IMP_H265_NAL_SLICE_BLA_N_LP     = 18,       /**< Breakpoint connection access without front image */
    IMP_H265_NAL_SLICE_IDR_W_RADL   = 19,       /**< Instant decoding refresh with front image RADL */
    IMP_H265_NAL_SLICE_IDR_N_LP     = 20,       /**< Instant decoding refresh without front image */
    IMP_H265_NAL_SLICE_CRA          = 21,       /**< Pure random access with front image	*/
    IMP_H265_NAL_VPS                = 32,       /**< Video Parameter Set */
    IMP_H265_NAL_SPS                = 33,       /**< SPS(Sequence Paramater Set) */
    IMP_H265_NAL_PPS                = 34,       /**< PPS(Picture Paramater Set) */
    IMP_H265_NAL_AUD                = 35,       /**< Access unit separator */
    IMP_H265_NAL_EOS                = 36,       /**< End of sequence */
    IMP_H265_NAL_EOB                = 37,       /**< End of bitstream */
    IMP_H265_NAL_FILLER_DATA        = 38,       /**< Fill in data */
    IMP_H265_NAL_PREFIX_SEI         = 39,       /**< SEI(Supplemental Enhancement Information) */
    IMP_H265_NAL_SUFFIX_SEI         = 40,       /**< SEI(Supplemental Enhancement Information) */
    IMP_H265_NAL_INVALID            = 64,       /**< Invalid nal type */
} IMPEncoderH265NaluType;

/**
 * Define the nal type of H.264 and h.265 encoding channel stream
 */
typedef union {
	IMPEncoderH264NaluType		h264NalType;		/**< H264E NALU */
	IMPEncoderH265NaluType		h265NalType;		/**< H265E NALU */
} IMPEncoderNalType;

typedef enum {
  IMP_ENC_SLICE_SI = 4,         /**< AVC SI Slice */
  IMP_ENC_SLICE_SP = 3,         /**< AVC SP Slice */
  IMP_ENC_SLICE_GOLDEN = 3,     /**< Golden Slice */
  IMP_ENC_SLICE_I = 2,          /**< I Slice (can contain I blocks) */
  IMP_ENC_SLICE_P = 1,          /**< P Slice (can contain I and P blocks) */
  IMP_ENC_SLICE_B = 0,          /**< B Slice (can contain I, P and B blocks) */
  IMP_ENC_SLICE_CONCEAL = 6,    /**< Conceal Slice (slice was concealed) */
  IMP_ENC_SLICE_SKIP = 7,       /**< Skip Slice */
  IMP_ENC_SLICE_REPEAT = 8,     /**< Repeat Slice (repeats the content of its reference) */
  IMP_ENC_SLICE_MAX_ENUM,       /**< sentinel */
} IMPEncoderSliceType;

/**
 * Define the packet structure of coded frame and stream
 */
typedef struct {
	uint32_t	offset;						/**< Stream packet offset */
	uint32_t	length;						/**< Stream packet length */
	int64_t	timestamp;						/**< Time stamp, unit us */
	bool	frameEnd;						/**< End of frame flag */
	IMPEncoderNalType   nalType;			/**< Nal type of encoder channel */
	IMPEncoderSliceType sliceType;          /**< Nal type of encoder slice */
} IMPEncoderPack;

typedef struct {
  int32_t       iNumBytes;       /**< Number of bytes in the stream */
  uint32_t      uNumIntra;       /**< Number of 8x8 blocks coded with intra mode */
  uint32_t      uNumSkip;        /**< Number of 8x8 blocks coded with skip mode */
  uint32_t      uNumCU8x8;       /**< Number of 8x8 CUs */
  uint32_t      uNumCU16x16;     /**< Number of 16x16 CUs */
  uint32_t      uNumCU32x32;     /**< Number of 32x32 CUs */
  uint32_t      uNumCU64x64;     /**< Number of 64x64 CUs */
  int16_t       iSliceQP;        /**< Slice QP value */
  int16_t       iMinQP;          /**< Minimum QP value */
  int16_t       iMaxQP;          /**< Maximum QP value */
} IMPEncoderStreamInfo;

typedef struct {
  int32_t       iNumBytes;       /**< Number of bytes in the stream */
  int16_t       iQPfactor;       /**< JPEG QP param */
} IMPEncoderJpegInfo;

/**
 * Define the structure of coded frame stream type
 */
typedef struct {
	uint32_t        phyAddr;            /**< Physical address of frame */
	uint32_t        virAddr;            /**< Virtual address of frame */
	uint32_t        streamSize;         /**< Size of the allocated virtual address */
	IMPEncoderPack  *pack;				/**< Frame stream packet */
	uint32_t        packCount;			/**< The number of all packets in a frame stream */
	uint32_t        seq;				/**< Sequence number of coded frame */
	bool            isVI;
	union
	{
		IMPEncoderStreamInfo streamInfo;
		IMPEncoderJpegInfo jpegInfo;
	};
} IMPEncoderStream;

typedef enum {
  IMP_ENC_TYPE_AVC      = 0,
  IMP_ENC_TYPE_HEVC     = 1,
  IMP_ENC_TYPE_JPEG     = 4,
} IMPEncoderEncType;

#define IMP_ENC_AVC_PROFILE_IDC_BASELINE      66
#define IMP_ENC_AVC_PROFILE_IDC_MAIN          77
#define IMP_ENC_AVC_PROFILE_IDC_HIGH          100
#define IMP_ENC_HEVC_PROFILE_IDC_MAIN         1

typedef enum {
  IMP_ENC_PROFILE_AVC_BASELINE  = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_BASELINE)),
  IMP_ENC_PROFILE_AVC_MAIN      = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_MAIN)),
  IMP_ENC_PROFILE_AVC_HIGH      = ((IMP_ENC_TYPE_AVC << 24) | (IMP_ENC_AVC_PROFILE_IDC_HIGH)),
  IMP_ENC_PROFILE_HEVC_MAIN     = ((IMP_ENC_TYPE_HEVC << 24) | (IMP_ENC_HEVC_PROFILE_IDC_MAIN)),
  IMP_ENC_PROFILE_JPEG          = (IMP_ENC_TYPE_JPEG << 24),
} IMPEncoderProfile;

typedef enum {
  IMP_ENC_PIC_FORMAT_400_8BITS = 0x0088,
  IMP_ENC_PIC_FORMAT_420_8BITS = 0x0188,
  IMP_ENC_PIC_FORMAT_422_8BITS = 0x0288,
} IMPEncoderPicFormat;

typedef enum {
  IMP_ENC_OPT_QP_TAB_RELATIVE   = 0x00000001,
  IMP_ENC_OPT_FIX_PREDICTOR     = 0x00000002,
  IMP_ENC_OPT_CUSTOM_LDA        = 0x00000004,
  IMP_ENC_OPT_ENABLE_AUTO_QP    = 0x00000008,
  IMP_ENC_OPT_ADAPT_AUTO_QP     = 0x00000010,
  IMP_ENC_OPT_COMPRESS          = 0x00000020,
  IMP_ENC_OPT_FORCE_REC         = 0x00000040,
  IMP_ENC_OPT_FORCE_MV_OUT      = 0x00000080,
  IMP_ENC_OPT_HIGH_FREQ         = 0x00002000,
  IMP_ENC_OPT_SRD               = 0x00008000,
  IMP_ENC_OPT_FORCE_MV_CLIP     = 0x00020000,
  IMP_ENC_OPT_RDO_COST_MODE     = 0x00040000,
} IMPEncoderEncOptions;

typedef enum {
  IMP_ENC_TOOL_WPP              = 0x00000001,
  IMP_ENC_TOOL_TILE             = 0x00000002,
  IMP_ENC_TOOL_LF               = 0x00000004,
  IMP_ENC_TOOL_LF_X_SLICE       = 0x00000008,
  IMP_ENC_TOOL_LF_X_TILE        = 0x00000010,
  IMP_ENC_TOOL_SCL_LST          = 0x00000020,
  IMP_ENC_TOOL_CONST_INTRA_PRED = 0x00000040,
  IMP_ENC_TOOL_TRANSFO_SKIP     = 0x00000080,
  IMP_ENC_TOOL_PCM              = 0x00000800,
} IMPEncoderEncTools;

/**
 * Define encoder croping attributes, image of the input encoder is cropped first, then compared with the size of the encoding channel, and then scaled
 */
typedef struct {
    bool		enable;		/**< Crop or not, range: [FALSE, TRUE], TRUE: crop, FALSE: not crop */
    uint32_t	x;			/**< The x-coordinate of the upper left corner of the crop region */
    uint32_t	y;			/**< The y-coordinate of the upper left corner of the crop region */
    uint32_t	w;			/**< Width of Crop Region */
    uint32_t	h;			/**< Height of Crop Region */
} IMPEncoderCropCfg;

/**
 * Define encoder entropy mode
 */
typedef enum {
  IMP_ENC_ENTROPY_MODE_CAVLC = 0x1,     /* AVC only */
  IMP_ENC_ENTROPY_MODE_CABAC = 0x2,
} IMPEncoderEntropyMode;

/**
 * Define encoder attribute structure
 */
typedef struct {
  IMPEncoderProfile       eProfile;
  uint8_t                 uLevel;
  uint8_t                 uTier;
  uint16_t                uWidth;
  uint16_t                uHeight;
  IMPEncoderPicFormat     ePicFormat;
  uint32_t                eEncOptions;
  uint32_t                eEncTools;
  IMPEncoderCropCfg       crop;           /**< Encoder croping properties */
} IMPEncoderEncAttr;

typedef enum {
  IMP_ENC_GOP_CTRL_MODE_DEFAULT       = 0x02,
  IMP_ENC_GOP_CTRL_MODE_PYRAMIDAL     = 0x04,
  IMP_ENC_GOP_CTRL_MODE_SMARTP		  = 0xfe,
  IMP_ENC_GOP_CTRL_MAX_ENUM           = 0xff,
} IMPEncoderGopCtrlMode;

typedef struct {
  IMPEncoderGopCtrlMode   uGopCtrlMode;
  uint16_t                uGopLength;
  //uint8_t                 uNumB;
  uint16_t				  uNotifyUserLTInter;
  uint32_t                uMaxSameSenceCnt;
  bool                    bEnableLT;
  uint32_t                uFreqLT;
  bool                    bLTRC;
} IMPEncoderGopAttr;

typedef enum {
  IMP_ENC_RC_MODE_FIXQP             = 0x0,
  IMP_ENC_RC_MODE_CBR               = 0x1,
  IMP_ENC_RC_MODE_VBR               = 0x2,
  IMP_ENC_RC_MODE_CAPPED_VBR        = 0x4,
  IMP_ENC_RC_MODE_CAPPED_QUALITY    = 0x8,
  IMP_ENC_RC_MODE_INVALID           = 0xff,
} IMPEncoderRcMode;

typedef enum IMPEncoderRcOptions {
  IMP_ENC_RC_OPT_NONE           = 0x00000000,
  IMP_ENC_RC_SCN_CHG_RES        = 0x00000001,
  IMP_ENC_RC_DELAYED            = 0x00000002,
  IMP_ENC_RC_STATIC_SCENE       = 0x00000004,
  IMP_ENC_RC_ENABLE_SKIP        = 0x00000008,
  IMP_ENC_RC_OPT_SC_PREVENTION  = 0x00000010,
  IMP_ENC_RC_MAX_ENUM,
} IMPEncoderRcOptions;

typedef struct {
  int16_t       iInitialQP;
} IMPEncoderAttrFixQP;

typedef struct {
  uint32_t              uTargetBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
} IMPEncoderAttrCbr;

typedef struct {
  uint32_t              uTargetBitRate;
  uint32_t              uMaxBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
} IMPEncoderAttrVbr;

typedef struct {
  uint32_t              uTargetBitRate;
  uint32_t              uMaxBitRate;
  int16_t               iInitialQP;
  int16_t               iMinQP;
  int16_t               iMaxQP;
  int16_t               iIPDelta;
  int16_t               iPBDelta;
  uint32_t              eRcOptions;
  uint32_t              uMaxPictureSize;
  uint16_t              uMaxPSNR;
} IMPEncoderAttrCappedVbr;

typedef IMPEncoderAttrCappedVbr IMPEncoderAttrCappedQuality;

typedef struct {
  IMPEncoderRcMode              rcMode;
  union {
    IMPEncoderAttrFixQP         attrFixQp;
    IMPEncoderAttrCbr           attrCbr;
    IMPEncoderAttrVbr           attrVbr;
    IMPEncoderAttrCappedVbr     attrCappedVbr;
    IMPEncoderAttrCappedQuality attrCappedQuality;
  };
} IMPEncoderAttrRcMode;

/**
 * Define the frame rate structure of the encoded channel
 * The minimum common multiple between frmratenum and frmrateden after the maximum common divisor division cannot exceed 64, preferably by the maximum common divisor before setting
 */
typedef struct {
	uint32_t	frmRateNum;				/**< The number of time units in a second, in units of time. Frame rate molecules */
	uint32_t	frmRateDen;				/**< The number of time units in a second, in units of time. Frame rate denominator */
} IMPEncoderFrmRate;


typedef struct {
  IMPEncoderAttrRcMode      attrRcMode;
  IMPEncoderFrmRate         outFrmRate;
} IMPEncoderRcAttr;

/**
 * Define encoded channel attribute structure
 */
typedef struct {
	IMPEncoderEncAttr     encAttr;	/**< Encoder attribute structure */
	IMPEncoderRcAttr      rcAttr;	/**< Rate controller attribute structure, only for H264 and H265 */
	IMPEncoderGopAttr     gopAttr;	/**< Encoder attribute structure */
} IMPEncoderChnAttr;

/**
 * Define the state structure of encoded channel
 */
typedef struct {
	bool		registered;			/**< Flag registered to the group, range: [TRUE, FALSE], TRUE: registered, FALSE: no registered */
	uint32_t	leftPics;			/**< Number of images to be encoded */
	uint32_t	leftStreamBytes;	/**< The number of bytes remaining in the bitstream buffer */
	uint32_t	leftStreamFrames;	/**< The number of frames remaining in the bitstream buffer */
	uint32_t	curPacks;			/**< The number of stream packets in the current frame */
	uint32_t	work_done;			/**< Channel program running status, 0: running, 1: no running */
} IMPEncoderChnStat;

/**
 * @fn int IMP_Encoder_CreateGroup(int encGroup)
 *
 * Create Encoding Group.
 *
 * @param[in] encGroup: Group ID, range: [0, @ref NR_MAX_ENC_GROUPS - 1]
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks One way group only supports one way resolution, and a new group needs to be started for different resolutions
 * One way group allows up to 2 coding channels to be registered.
 *
 * @attention If the group already exists, then return fails.
 */
int IMP_Encoder_CreateGroup(int encGroup);

/**
 * @fn int IMP_Encoder_DestroyGroup(int encGroup)
 *
 * Destroy Encoding Group.
 *
 * @param[in] encGroup: Group ID, range: [0, @ref NR_MAX_ENC_GROUPS - 1]
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks When destroying a group, it must be ensured that the group is empty, that is, no channel is registered in the group.
 *
 * @attention If destroy a nonexistent group, then return fails.
 */
int IMP_Encoder_DestroyGroup(int encGroup);

/**
* @fn int IMP_Encoder_SetDefaultParam(IMPEncoderChnAttr *chnAttr, IMPEncoderProfile profile, IMPEncoderRcMode rcMode, uint16_t uWidth, uint16_t uHeight, uint32_t frmRateNum, uint32_t frmRat
*
* Set encoding default properties.
*
* @param[out] chnAttr: Struct for encoding.
* @param[in]  profile: Encoding profile.
* @param[in]  rcMode:  Rate control mode.
* @param[in]  uWidth:  Encoding width.
* @param[in]  uHeight: Encoding height.
* @param[in]  frmRateNum: Encoding fps num.
* @param[in]  frmRateDen: Encoding fps den.
* @param[in]  uGopLength: GOP length.
* @param[in]  uMaxSameSenceCnt: GOPLength = uGopLength * uMaxSameSenceCnt, Default is 2.
* @param[in]  iInitialQP: Initialize QP, Default is -1.
* @param[in]  uTargetBitRate: Target bitrate.
*
* @retval 0				success.
* @retval ~0			failed.
*
* @remarks If you are not familiar with coding related parameters, please use the default values.
*
* @attention none.
*/
int IMP_Encoder_SetDefaultParam(IMPEncoderChnAttr *chnAttr, IMPEncoderProfile profile, IMPEncoderRcMode rcMode, uint16_t uWidth, uint16_t uHeight, uint32_t frmRateNum, uint32_t frmRateDen, uint32_t uGopLength, int uMaxSameSenceCnt, int iInitialQP, uint32_t uTargetBitRate);

/**
 * @fn int IMP_Encoder_CreateChn(int encChn, const IMPEncoderChnAttr *attr)
 *
 * Create Encoder Channel.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] attr:   Encode channel property pointer.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks The coding channel attribute consists of two parts: encoder attribute and rate control attribute.
 * @remarks Encoder attributes need to select the coding protocol first, and then assign the corresponding attributes to each protocol.
 */
int IMP_Encoder_CreateChn(int encChn, const IMPEncoderChnAttr *attr);

/**
 * @fn int IMP_Encoder_DestroyChn(int encChn)
 *
 * Destroy Encoder Channel.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @attention Destroy channels that do not exist, then return fails.
 * @attention Before destroying, you must ensure that the channel has been de registered from the group, otherwise it will return fails.
 */
int IMP_Encoder_DestroyChn(int encChn);

/**
 * @fn int IMP_Encoder_GetChnAttr(int encChn, IMPEncoderChnAttr * const attr)
 *
 * Gets the properties of the encoding channel.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] attr:   Encoding channel properties.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 */
int IMP_Encoder_GetChnAttr(int encChn, IMPEncoderChnAttr * const attr);

/**
 * @fn int IMP_Encoder_RegisterChn(int encGroup, int encChn)
 *
 * Register encoding channel to group.
 *
 * @param[in] encGroup: Group ID, range: [0, @ref NR_MAX_ENC_GROUPS - 1].
 * @param[in] encChn:   Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @attention If you register a channel that does not exist, then return fails.
 * @attention If the channel is registered to a nonexistent group, then return fails.
 * @attention The same coding channel can only be registered to one group. If the channel has been registered to a group, then return fails.
 * @attention If a group has been registered, it cannot be registered by other channels.
 */

int IMP_Encoder_RegisterChn(int encGroup, int encChn);
/**
 * @fn int IMP_Encoder_UnRegisterChn(int encChn)
 *
 * Unregister encoding channel to group.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks After the channel is logged off, the encoding channel will be reset and the code stream buffers in the encoding channel will be cleared,
 * If the user is still using the bitstream buffer that has not been released in time, the correctness of the buffer data cannot be guaranteed,
 * Users can use IMP_Encoder_Query interface to query the status of the code channel buffer, After the register of the bitstream is completed,
 * the buffer is retrieved.
 *
 * @attention Unregister the uncreated channel, then return fails.
 * @attention Unregister the unregistered channel, then return fails.
 * @attention If the encoding channel does not stop receiving image encoding, then return fails.
 */
int IMP_Encoder_UnRegisterChn(int encChn);

/**
 * @fn int IMP_Encoder_StartRecvPic(int encChn)
 *
 * Start receiving coded channel image.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval non-0			failed.
 *
 * @remarks The encoding can only be started after the encoding channel receives the image.
 *
 * @attention If the channel is not created, then return fails.
 * @attention If channel is not registered with group, then return fails.
 */
int IMP_Encoder_StartRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_StopRecvPic(int encChn)
 *
 * End receiving coded channel image.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks This interface does not determine whether the reception is stopped or not, it allows repeated stop reception without returning errors.
 * @remarks Calling this interface only stops receiving the original data encoding, and the stream buffer will not be eliminated.
 *
 * @attention If the channel is not created, then return fails.
 * @attention If channel is not registered with group, then return fails.
 */
int IMP_Encoder_StopRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_Query(int encChn, IMPEncoderChnStat *stat)
 *
 * Query encoded channel status.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] stat:  Encoding channel status.
 *
 * @retval 0			success.
 * @retval non-0			failed.
 *
 * @remarks none.
 *
 * @attention none.
 */
int IMP_Encoder_Query(int encChn, IMPEncoderChnStat *stat);

/**
 * @fn int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag)
 *
 * Get the encoded stream.
 *
 * @param[in] encChn:    Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] stream:    stream structure pointer.
 * @param[in] blockFlag: Whether to use blocking method to obtain, 0: no blocking, 1: blocking.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Get the data of one frame at a time.
 * @remarks If the user does not get the stream for a long time, the code stream buffer will be full. If a code stream buffer is full,
 * a coded channel will lose the image received later, Only when the user obtains the code stream and has enough buffer for coding, can the coding continue.
 * It is suggested that the user interface call to acquire the code stream and the interface call to release the code stream appear in pairs,
 * and the code stream should be released as soon as possible to prevent the user state from acquiring the code stream,
 * If the bit stream buffer is full due to untimely release, the encoding is stopped.
 * @remarks For H264 and h265 code streams, call success to get a frame code stream, which may contain multiple packets.
 * @remarks For JPEG type code stream, call success to get one frame code stream at a time. This frame code stream only contains one package,
 * and this frame contains the complete information of JPEG image file.
 *
 * example:
 * @code
 * int ret;
 * ret = IMP_Encoder_PollingStream(ENC_H264_CHANNEL, 1000);   //Polling Stream Buffer.
 * if (ret < 0) {
 *     printf("Polling stream timeout\n");
 *     return -1;
 * }
 *
 * IMPEncoderStream stream;
 * ret = IMP_Encoder_GetStream(ENC_H264_CHANNEL, &stream, 1); //Get a frame stream by blocking.
 * if (ret < 0) {
 *     printf("Get Stream failed\n");
 *     return -1;
 * }
 *
 * int i, nr_pack = stream.packCount;
 * for (i = 0; i < nr_pack; i++) {                            //Save each packet of this frame stream.
 *     ret = write(stream_fd, (void *)stream.pack[i].virAddr,
 *                stream.pack[i].length);
 *     if (ret != stream.pack[i].length) {
 *         printf("stream write error:%s\n", strerror(errno));
 *         return -1;
 *     }
 * }
 * @endcode
 *
 * @attention If pststream is null, then return fails.
 * @attention If the channel is not created, then return fails.
 */
int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag);

/**
 * @fn int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream)
 *
 * Release the encoded stream.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] stream: stream structure pointer.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks This interface should be with imp_ Encoder_ Getstream pairs to use,user must release the obtained bit stream buffer in time after acquiring the bit stream,
 * otherwise the bit stream buffer may be full and the encoder coding will be affected.
 * And the user must release the obtained bitstream cache in the order of acquire first and release first.
 * @remarks After the de registration of the coding channel, all the unreleased stream packets are invalid and can no longer be used or released.
 *
 * @attention If pststream is null, then return fails.
 * @attention If the channel is not created, then return fails.
 * @attention If the invalid stream is released, then return fails.
 */
int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream);

/**
 * @fn int IMP_Encoder_PollingStream(int encChn, uint32_t timeoutMsec)
 *
 * Polling encoded stream.
 *
 * @param[in] encChn:      Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] timeoutMsec: overtimes, unit: ms.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Before obtaining the code stream, you can use this API to poll. When the code stream cache is not empty or the time is over, the function returns.
 *
 * @attention none.
 */
int IMP_Encoder_PollingStream(int encChn, uint32_t timeoutMsec);

/**
 * @fn int IMP_Encoder_PollingModuleStream(uint32_t *encChnBitmap, uint32_t timeoutMsec)
 *
 * Poling the encoded stream of each channel of the whole encoding module.
 *
 * @param[out] encChnBitmap: Each bits represents the corresponding channel number, If there is a coded stream, the corresponding position 1, otherwise set to 0.
 * @param[in] timeoutMsec:   overtimes, unit: ms.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Before obtaining the code stream, you can use this API to poll. When the code stream cache is not empty or the time is over, the function returns.
 * @remarks *encChnBitmap The bit corresponding to 1 is only used after calling IMP_Encoder_ReleaseStream,
 * if it is detected that the channel stream cache corresponding to this bit is not empty, it will be set to zero.
 *
 * @attention none.
 */
int IMP_Encoder_PollingModuleStream(uint32_t *encChnBitmap, uint32_t timeoutMsec);

/**
 * @fn int IMP_Encoder_GetFd(int encChn)
 *
 * Get the device file handle corresponding to the encoding channel.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval >=0			success, return the device file descriptor.
 * @retval < 0			failed.
 *
 * @remarks If using IMP_Encoder_PollingStream is not suitable, For example, When the coding of multiple coding channels is completed in the same place,
 * you can use this file handle to call select, poll and other similar functions to block waiting for encoding to complete.
 * @remarks If the channel is not created, then return fails.
 *
 * @attention none.
 */
int IMP_Encoder_GetFd(int encChn);

/**
 * @fn int IMP_Encoder_SetbufshareChn(int encChn, int shareChn)
 *
 * Set JPEG channel to share H265/H264 encoding channel.
 *
 * @param[in] encChn:   Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] sharechn: Shared H264/H265 channel number, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Shared encoding channel created before calling this API.
 * @remarks This API needs to be called before the channel is created.
 *
 * @attention none.
 */
int IMP_Encoder_SetbufshareChn(int encChn, int shareChn);

/**
 * @fn int IMP_Encoder_SetChnResizeMode(int encChn, int en);
 *
 * Set whether additional rmem memory is required for encoding scaling.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] en:     alloc rmem or not, 1: no alloc rmem, 0: alloc rmem.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks This API is only called when the resolution of encoding scaling is less than the original resolution,
 * there is no need to call when the resolution of encoding scaling is greater than the original resolution.
 *
 * @attention none.
 */
int IMP_Encoder_SetChnResizeMode(int encChn, int en);

/**
 * @fn int IMP_Encoder_SetMaxStreamCnt(int encChn, int nrMaxStream)
 *
 * Set the number of stream buffers.
 *
 * @param[in] encChn:      Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] nrMaxStream: number of stream buffers, range: [1, @ref NR_MAX_ENC_CHN_STREAM].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Because the number of Buffer buffers is fixed at the time the channel is created, the secondary API needs to be called before the channel is created.
 * @remarks If this API is not called to set the number of buffers in the bitstream cache before the channel is created, the default number of buffers in the SDK is used.
 *
 * @attention none.
 */
int IMP_Encoder_SetMaxStreamCnt(int encChn, int nrMaxStream);

/**
 * @fn int IMP_Encoder_GetMaxStreamCnt(int encChn, int *nrMaxStream)
 *
 * Get the number of stream buffers.
 *
 * @param[in] encChn:       Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] nrMaxStream: stream buffer number variable pointer.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks none.
 *
 * @attention none.
 */
int IMP_Encoder_GetMaxStreamCnt(int encChn, int *nrMaxStream);

/**
 * @fn int IMP_Encoder_RequestIDR(int encChn)
 *
 * Request IDR frame.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks After calling this API, IDR frame coding will be applied in the latest coding frame.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_RequestIDR(int encChn);

/**
 * @fn int IMP_Encoder_FlushStream(int encChn)
 *
 * Wipe out old stream in the encoder, and the encoding starts with the IDR frame.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks After calling this API, IDR frame coding will be applied in the latest coding frame.
 *
 * @attention none.
 */
int IMP_Encoder_FlushStream(int encChn);

/**
 * @fn int IMP_Encoder_GetChnFrmRate(int encChn, IMPEncoderFrmRate *pstFps)
 *
 * Get frame rate control properties.
 *
 * @param[in] encChn:     Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] pstFpsCfg: Frame rate control attribute parameters.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will get the frame rate control property of the channel. Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_GetChnFrmRate(int encChn, IMPEncoderFrmRate *pstFps);

/**
 * @fn int IMP_Encoder_SetChnFrmRate(int encChn, const IMPEncoderFrmRate *pstFps)
 *
 * Set frame rate control properties dynamically.
 *
 * @param[in] encChn:     Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] pstFpsCfg: Frame rate control attribute parameters.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will reset the encoder frame rate attribute. The frame rate attribute takes effect in the next GOP with a maximum delay of 1 second.
 * Calling this API requires that the channel already exists.
 * @remarks If IMP_FRAMESOURCE_SETCHNFPS is called to change frame rate of the system dynamically, then the function is needed to modify the frame rate of the encoder
 * and complete the correct parameter configuration.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnFrmRate(int encChn, const IMPEncoderFrmRate *pstFps);

/**
 * @fn int IMP_Encoder_SetChnBitRate(int encChn, int iTargetBitRate, int iMaxBitRate)
 *
 * Set bitrate attribute dynamically.
 *
 * @param[in] encChn:         Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] iTargetBitRate: Target bitrate, iMaxBitRate Max bitrate unit: "bit/s".
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will reset the code rate attribute of the encoder. The code rate attribute takes effect in the next GOP with a maximum delay of 1 second.
 * Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnBitRate(int encChn, int iTargetBitRate, int iMaxBitRate);

/**
* @fn int IMP_Encoder_SetChnGopLength(int encChn, int iGopLength);
*
* Set the gopplength property of the encoding channel.
*
* @param[in] encChn Channel ID,range: [0, @ref NR_MAX_ENC_CHN - 1].
* @param[in] iGopLength GopLength attr.
*
* @retval 0				success.
* @retval ~0			failed.
*
* @remarks none.
*
* @attention none.
*/
int IMP_Encoder_SetChnGopLength(int encChn, int iGopLength);

/**
 * @fn int IMP_Encoder_GetChnAttrRcMode(int encChn, IMPEncoderAttrRcMode *pstRcModeCfg).
 *
 * Get the rate control mode attribute.
 *
 * @param[in] encChn:    Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] pstRcCfg: Rate control mode attribute parameters.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will get the bit rate control mode property of the channel. Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_GetChnAttrRcMode(int encChn, IMPEncoderAttrRcMode *pstRcModeCfg);

/**
 * @fn int IMP_Encoder_SetChnAttrRcMode(int encChn, const IMPEncoderAttrRcMode *pstRcModeCfg).
 *
 * Set rate control mode properties.
 *
 * @param[in] encChn:   Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] pstRcCfg: Rate control mode attribute parameters.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will set the bit rate control mode property of the channel, and the next IDR will take effect.
 * Calling this API requires that the channel already exists.
 *
 * @attention Currently, the code rate control mode supports ENC_RC_MODE_FIXQP, ENC_RC_MODE_CBR, ENC_RC_MODE_VBR, ENC_RC_MODE_CAPPED_QUALITY.
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnAttrRcMode(int encChn, const IMPEncoderAttrRcMode *pstRcModeCfg);

/**
* @fn int IMP_Encoder_GetChnGopAttr(int encChn, IMPEncoderGopAttr *pGopAttr);
*
* Obtain encoding channel GOP properties.
*
* @param[in] encChn: Channel ID,range: [0, @ref NR_MAX_ENC_CHN - 1]
* @param[out] pGopAttr: GOP attr
*
* @retval 0				success
* @retval ~0			failed
*
* @remarks none.
*
* @attention none
*/
int IMP_Encoder_GetChnGopAttr(int encChn, IMPEncoderGopAttr *pGopAttr);

/**
* @fn int IMP_Encoder_SetChnGopAttr(int encChn, const IMPEncoderGopAttr *pGopAttr);
*
* Set encoding channel GOP properties.
*
* @param[in] encChn: Channel ID,range: [0, @ref NR_MAX_ENC_CHN - 1]
* @param[in] pGopAttr: GOP attr
*
* @retval 0				success.
* @retval ~0			failed.
*
* @remarks none.
*
* @attention none
*/
int IMP_Encoder_SetChnGopAttr(int encChn, const IMPEncoderGopAttr *pGopAttr);

/**
 * @fn int IMP_Encoder_SetChnQp(int encChn, int iQP)
 *
 * Setting the rate control property QP dynamically.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] iQP:    QP, range: [iMinQP, iMaxQP]
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will reset the QP of the next frame, and the set QP will take effect in the next frame.
 *			Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnQp(int encChn, int iQP);

/**
 * @fn int IMP_Encoder_SetChnQpBounds(int encChn, int iMinQP, int iMaxQP);
 *
 * Dynamically set the bit rate control attributes maxqp and minqp.
 *
 * @param[in] encChn: Channel ID,range: [0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] iMinQP: iMinQP value,range: [0, 51]
 * @param[in] iMaxQP: iMaxQP value,range: [0, 51]
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Calling this API will reset the QP range of the next frame.
 *			Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnQpBounds(int encChn, int iMinQP, int iMaxQP);

/**
 * @fn int IMP_Encoder_SetChnQpIPDelta(int encChn, int uIPDelta)
 *
 * Setting the rate control property IPDelta dynamically.
 *
 * @param[in] encChn:   Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] IPDelta   QP difference between I frame and the first P frame after it
 *
 * @retval 0			success
 * @retval ~0			failed
 *
 * @remarks	Calling this API requires that the channel already exists.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetChnQpIPDelta(int encChn, int uIPDelta);

/**
 * @fn int IMP_Encoder_SetFisheyeEnableStatus(int encChn, int enable)
 *
 * Set the enabling state of the fish eye correction algorithm provided by ingenic.
 *
 * @param[in] encChn: Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[in] enable: 0: unable(default), 1: enable.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @remarks Because the enabling state of the fisheye correction algorithm is fixed at the time of the creation of the channel,
 * the secondary API needs to be called before the channel is created.
 * @remarks If this API is not called to set the enable state of the fish eye correction algorithm provided by ingenic before the channel is created,
 * it is not enabled by default, the fish eye correction algorithm provided by Junzheng cannot be used.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_SetFisheyeEnableStatus(int encChn, int enable);

/**
 * @fn int IMP_Encoder_GetFisheyeEnableStatus(int encChn, int *enable)
 *
 * Get the enabling state of the fish eye correction algorithm provided by ingenic.
 *
 * @param[in] encChn:  Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] enable: return Enable state, 0: unable, 1: enable.
 *
 * @retval 0			success.
 * @retval ~0			failed.
 *
 * @attention This API is only applicable to H264 and H265 encoding channels.
 */
int IMP_Encoder_GetFisheyeEnableStatus(int encChn, int *enable);

/**
 * @fn int IMP_Encoder_GetChnEncType(int encChn, IMPEncoderEncType *encType)
 *
 * Get image encoding protocol type.
 *
 * @param[in] encChn:   Channel ID, range: [0, @ref NR_MAX_ENC_CHN - 1].
 * @param[out] encType: return encoding protocol type.
 *
 * @retval 0				success.
 * @retval ~0				failed.
 *
 * @remarks If the channel is not created, then return fails.
 *
 * @attention none.
 */
int IMP_Encoder_GetChnEncType(int encChn, IMPEncoderEncType *encType);

/**
 * @brief IMP_Encoder_SetPool(int chnNum, int poolID);
 *
 * bind channel to mempool, let chnNum malloc from pool.
 *
 * @param[in] chnNum		Channel ID.
 * @param[in] poolID		Pool ID.
 *
 * @retval 0				success.
 * @retval other values		failed.
 *
 * @remarks In order to solve the fragmentation of rmem, the channel encoder is bound to
 * the corresponding MemPool. The encoder applies for MEM in the MemPool. If it is not
 * called, the encoder will apply in rmem. At this time, there is the possibility of
 * fragmentation for rmem.
 *
 * @attention: chnNum is greater than or equal to 0 and less than 32.
 */
int IMP_Encoder_SetPool(int chnNum, int poolID);

/**
 * @brief IMP_Encoder_GetPool(int chnNum);
 *
 * Get Pool ID by chnannel ID.
 *
 * @param[in] chnNum		Channel ID.
 *
 * @retval  >=0 && < 32     success.
 * @retval other values		failed.
 *
 * @remarks obtains poolid through channelid, which cannot be used by customers temporarily.
 *
 * @attention	none.
 */
int IMP_Encoder_GetPool(int chnNum);

/**
 * @brief IMP_Encoder_SetStreamBufSize(int encChn, uint32_t nrStreamSize);
 *
 * Set size of Stream buffer.
 *
 * @param[in] encChn:       Channel ID.
 * @param[in] nrStreamSize: size of Stream buffer.
 *
 * @retval  >=0 && < 32     success.
 * @retval  <0			    failed.
 *
 * @remarks This API must be called before the channel is created.
 *
 * @attention none.
 */
int IMP_Encoder_SetStreamBufSize(int encChn, uint32_t nrStreamSize);

/**
 * @brief IMP_Encoder_GetStreamBufSize(int encChn, uint32_t *nrStreamSize);
 *
 * Get size of Stream buffer.
 *
 * @param[in] encChn:        Channel ID.
 * @param[out] nrStreamSize: size of Stream buffer.
 *
 * @retval  >=0 && < 32    success.
 * @retval  <0			   failed.
 *
 * @remarks This API must be called after the channel is created.
 *
 * @attention none.
 */
int IMP_Encoder_GetStreamBufSize(int encChn, uint32_t *nrStreamSize);

/**
 * @brief IMP_Encoder_GetChnAveBitrate(int encChn, IMPEncoderStream *stream, int frames, double *br);
 *
 * Gets the average bit rate of the specified number of frames.
 *
 * @param[in] encChn       Channel ID.
 * @param[in] stream       Stream.
 * @param[in] frame        Count the number of frames(Integral multiple of GOP length).
 * @param[out] br          Average bitrate.
 *
 * @retval  >=0 && < 32    success.
 * @retval  <0			   failed.
 *
 * @remarks   This API must be called after using IMP_Encoder_GetStream.
 *
 * @attention none.
 */
int IMP_Encoder_GetChnAveBitrate(int encChn, IMPEncoderStream *stream, int frames, double *br);

/**
 * @brief IMP_Encoder_GetChnEvalInfo(int encChn, void *info);
 *
 * Obtain the evaluation parameters of each frame in the channel.
 *
 * @param[in] encChn       Channel ID.
 * @param[out] info        Eval info.
 *
 * @retval  >=0 && < 32    success.
 * @retval  <0			   failed.
 *
 * @remarks   This API must be called after using IMP_Encoder_GetStream.
 *
 * @attention none.
 */
int IMP_Encoder_GetChnEvalInfo(int encChn, void *info);

/**
 * @fn int IMP_Encoder_SetChnEntropyMode(int encChn,IMPEncoderEntropyMode eEntropyMode)
 *
 * Set entropy mode in the channel
 *
 * @param[in] encChn          Channel ID.
 * @param[in] eEntropyMode    Entropy mode.
 *
 * @retval 0                  success.
 * @retval <0                 failed.
 *
 * @remarks This API is only applicable to IMP_ENC_PROFILE_AVC_MAIN or IMP_ENC_PROFILE_AVC_HIGH
 * @remarks This API needs to be called before the channel is created.
 *
 * @attention This API is only applicable to H264 encoding channels.
 */
int IMP_Encoder_SetChnEntropyMode(int encChn,IMPEncoderEntropyMode eEntropyMode);

/**
 * @brief IMP_Encoder_SetFrameRelease(int encChn, int num, int den);
 *
 * Set release frame num and den.
 *
 * @param[in] encChn:        Channnel ID.
 * @param[in] num:           release num.
 * @param[in] den:           release den.
 *
 * @retval  >=0            success.
 * @retval  <0			   failed.
 *
 * @remarks This API must be called after the channel is created.
 *
 * @attention none.
 */
int IMP_Encoder_SetFrameRelease(int encChn, int num, int den);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_ENCODER_H__ */
