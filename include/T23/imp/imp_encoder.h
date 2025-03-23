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
 * IMP video encoding header file
 */

/**
 * @defgroup IMP_Encoder
 * @ingroup imp
 * @brief video encoding （JPEG, H264）modules，including video encoding channel management and it's parameter setting and so on
 *
 * @section enc_struct 1 module structure
 * Encoder module internal structure as following:
 * @image html encoder_struct.jpg
 * In this mentioned picture, The coding module consists of a number of Group (the T15 supports two Group), each Group consists of encoding Channel.
 * each encoding Channel comes with an output stream buffer, which is composed of a number of buffer.
 *
 * @section enc_channel 2 encoder Channel
 * One coding channel can only deal with one coding protocol entity, each group can add 2 encoding channel;
 *
 * @section enc_rc 3 encoder bitrate control
 * @subsection enc_cbr 3.1 CBR
 * CBR, Constant Bit Rate, the code rate is constant in the rate statistical time.
 * Such as H264 encode, users can set maxQp,minQp,bitrate and so on.
 * maxQp, minQp is used to control the quality range.
 * bitrate is used to control the constant bitrate. (Average coding bit rate for a certain time)
 * when the coding rate is greater than the constant bit rate, the image maxQp will gradually adjust to the QP, when the coding rate is much smaller than the constant bit rate, the image QP will gradually adjust to the minQp.
 * when the image QP reaches maxQP, Qp is clamped to the maximum value, bitrate clamping effect fails, the coding rate might exceed bitrate.
 * when the image QP reaches minQp, QP is clamped to the minimum value, and when the coding rate has reaches the maximum value, the image quality will get better.
 *
 * @subsection enc_FixQP 3.2 FixQP
 * Fix Qp fixes the value of Qp. In the rate statistics time, all the Qp value of the coded image is the same, and the image Qp value is adopted by the user.
 * @{
 */

/**
 * Attention:
 * direct_mode=2 is Double Sensor IVDC. In this mode:
 * Framesource0:Main sensor Mian channel; Framesource3:Second sensor Main channel.
 * Encoder_Grouop0 band to Framesource0; Encoder_Grouop1 bind to Framesource3.
 * Enc_chn0(h264) register to Encoder_Group0, use for Main sensor Main channel's bitstream.
 * Enc_chn1(h264) register to Encoder_Group1, use for Second sensor Main channel's bitstream.
 *
 * If capture Main sensor Main channel's jpeg or Second sensor Main channel's jpeg,
 * encoder channels need to be set channel2 or channel3.
 * Main Sensor main channel:        |--enc_chn0 for Main Sensor main channel's H264
 * FrameSource0 -- Encoder_Group0->
 *                                  |--enc_chn2 for Main Sensor main channel's JPEG (If not captured, please ignore)
 *
 * Second Sensor main channel:      |--enc_chn1 for Second Sensor main channel's H264
 * FrameSource3 -- Encoder_Group1->
 *                                  |--enc_chn3 for Second Sensor main channel's JPEG (If not captured, please ignore)
 *
 * Framesource1,Framesource2,Framesource4,Framesource5 are No Double Sensor IVDC, Encoder channel and Group don't have special restrictions.
 */

/**
 * Define channel ratecontrol method
 */
typedef enum {
	ENC_RC_MODE_FIXQP               = 0,	/**< Fixqp method */
	ENC_RC_MODE_CBR                 = 1,	/**< CBR method */
	ENC_RC_MODE_VBR                 = 2,	/**< VBR method */
	ENC_RC_MODE_SMART               = 3,	/**< Smart method */
	ENC_RC_MODE_INV                 = 4,	/**< invalidate method */
} IMPEncoderRcMode;

/**
 * Define video encode channel framerate structure
 * frmRateNum and frmRateDen is LCM must be 1 and GCD must be not greater than 64
 * The least common multiple of the greatest common divisor divisibility between frmRateNum and frmRateDen  can not be more than 64, it is better to get the greatest common divisor number before the settings.
 */
typedef struct {
	uint32_t	frmRateNum;				/**< the tick num in one second, the numerator of framerate */
	uint32_t	frmRateDen;				/**< the tick num in one frame, the denominator of framerate */
} IMPEncoderFrmRate;

/**
 * Define the FixQp ratecontrol attribute of h264 channel
 */
typedef struct {
	uint32_t			qp;			/**< Frame level Qp parameter */
} IMPEncoderAttrH264FixQP;

/**
 * Define the CBR ratecontrol attribute of h264 channel
 */
typedef struct {
	uint32_t			maxQp;			/**< max Frame level Qp  parameter,range:[0-51] */
	uint32_t			minQp;			/**< min Frame level Qp parameter,range:[0-maxQp] */
	uint32_t			outBitRate;		/**< output bitstream rate, unit:kbps */
	int					iBiasLvl;		/**< I proportional frame support (-3~3), 7 levels */
	uint32_t			frmQPStep;		/**< Inter frames QP variation*/
	uint32_t			gopQPStep;		/**< QP variation among GOPs. */
	bool				adaptiveMode;	/**< adaptive mode */
	bool				gopRelation;	/**< whether GOP has relationship */
} IMPEncoderAttrH264CBR;

/**
 * Define the VBR ratecontrol attribute of h264 channel
 */
typedef struct {
	uint32_t			maxQp;			/**< max encoder Quantization parameter,range:[0-51] */
	uint32_t			minQp;			/**< min encoder Quantization parameter,range:[0-maxQp] */
	uint32_t			staticTime;		/**< bitstream statistic time, unit:second */
	uint32_t			maxBitRate;		/**< max bitrate in one second, unit:kbps */
    int32_t             iBiasLvl;       /**< I proportional frame support (-3~3), 7 levels */
	uint32_t			changePos;		/**< adjust qp position avriation when bitrate exceeds changepos*maxBitRate, range:[50, 100] */
    uint32_t            qualityLvl;     /**< Video quality lowest level, range:[0-7]；the lower the value, the higher the quality and the biger the bitstream, minBitRate = maxBitRate * quality[qualityLvl], quality[] = {0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1} */
	uint32_t			frmQPStep;		/**< Inter frames QP variation*/
	uint32_t			gopQPStep;		/**< QP variation among GOPs. */
	bool				gopRelation;	/**< gop relationship whether or not*/
} IMPEncoderAttrH264VBR;

/**
 * Define the Smart ratecontrol attribute of h264 channel
 */
typedef struct {
	uint32_t			maxQp;			/**< max encoder Quantization parameter,range:[0-51] */
	uint32_t			minQp;			/**< min encoder Quantization parameter,range:[0-maxQp] */
	uint32_t			staticTime;		/**< bitstream statistic time, unit:second */
	uint32_t			maxBitRate;		/**< max bitrate in one second, unit:kbps */
    int32_t             iBiasLvl;       /**< I proportional frame support (-3~3), 7 levels */
	uint32_t			changePos;		/**< adjust qp position avriation when bitrate exceeds changepos*maxBitRate, range:[50, 100] */
    uint32_t            qualityLvl;     /**< Video quality level, range:[0-6]；the higher the value, the higher the quality and the biger the bitstream, default:4 */
	uint32_t			frmQPStep;		/**< Inter frames QP variation */
	uint32_t			gopQPStep;		/**< QP variation among GOPs */
	bool				gopRelation;	/**< whether GOP has relationship */
} IMPEncoderAttrH264Smart;

typedef struct {
	uint32_t			qp;			/**< Frame level Qp parameter */
} IMPEncoderAttrH265FixQP;

typedef struct {
	uint32_t			maxQp;			/**< max Frame level Qp  parameter,range:[0-51] */
	uint32_t			minQp;			/**< min Frame level Qp parameter,range:[0-maxQp] */
	uint32_t			staticTime;		/**< bitstream statistic time, unit:second */
	uint32_t			outBitRate;		/**< output bitstream rate, unit:kbps */
	int					iBiasLvl;		/**< I proportional frame support (-3~3), 7 levels */
	uint32_t			frmQPStep;		/**< Inter frames QP variation*/
	uint32_t			gopQPStep;		/**< QP variation among GOPs. */
	uint32_t			flucLvl;		/**< max bitrate fluctuation grade relative to average bitrate, range [0-4] */
} IMPEncoderAttrH265CBR;

typedef struct {
	uint32_t			maxQp;			/**< max encoder Quantization parameter,range:[0-51] */
	uint32_t			minQp;			/**< min encoder Quantization parameter,range:[0-maxQp] */
	uint32_t			staticTime;		/**< bitstream statistic time, unit:second */
	uint32_t			maxBitRate;		/**< max bitrate in one second, unit:kbps */
    int32_t             iBiasLvl;       /**< I proportional frame support (-3~3), 7 levels */
	uint32_t			changePos;		/**< adjust qp position avriation when bitrate exceeds changepos*maxBitRate, range:[50, 100] */
    uint32_t            qualityLvl;     /**< Video quality lowest level, range:[0-7]；the lower the value, the higher the quality and the biger the bitstream, minBitRate = maxBitRate * quality[qualityLvl], quality[] = {0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1} */
	uint32_t			frmQPStep;		/**< Inter frames QP variation*/
	uint32_t			gopQPStep;		/**< QP variation among GOPs. */
	uint32_t			flucLvl;		/**< max bitrate fluctuation grade relative to average bitrate, range [0-4] */
} IMPEncoderAttrH265VBR;

typedef struct {
	uint32_t			maxQp;			/**< max encoder Quantization parameter,range:[0-51] */
	uint32_t			minQp;			/**< min encoder Quantization parameter,range:[0-maxQp] */
	uint32_t			staticTime;		/**< bitstream statistic time, unit:second */
	uint32_t			maxBitRate;		/**< max bitrate in one second, unit:kbps */
    int32_t             iBiasLvl;       /**< I proportional frame support (-3~3), 7 levels */
	uint32_t			changePos;		/**< adjust qp position avriation when bitrate exceeds changepos*maxBitRate, range:[50, 100] */
    uint32_t            qualityLvl;     /**< Video quality level, range:[0-6]；the higher the value, the higher the quality and the biger the bitstream, default:4 */
	uint32_t			frmQPStep;		/**< Inter frames QP variation */
	uint32_t			gopQPStep;		/**< QP variation among GOPs */
	uint32_t			flucLvl;		/**< max bitrate fluctuation grade relative to average bitrate, range [0-4] */
} IMPEncoderAttrH265Smart;

/**
 * Define the denoise attribute of h264 channel, the enable member can't be changed after create channel, buf other members can be changed any time after create channel
 */
typedef struct {
	bool			enable;			/**< whether denoise function is enabled/disabled , 0:disable, 1:enable */
	int				dnType;			/**< denoise type,0:NONE-no denoise, 1:IP-loss info most, 2:II-loss info less */
	int				dnIQp;			/**< I slice quantization parameter */
	int				dnPQp;			/**< P slice quantization parameter */
} IMPEncoderAttrDenoise;

/**
 * Define the input frame usage mode of h264 channel
 */
typedef enum {
	ENC_FRM_BYPASS	= 0,		/**< Sequential full use mode - default mode */
	ENC_FRM_REUSED	= 1,		/**< repeat use of a frame mode*/
	ENC_FRM_SKIP	= 2,		/**< Frame loss mode */
} EncFrmUsedMode;

/**
 * Define the input frame usage mode attribute of h264 channel
 */
typedef struct {
	bool				enable;			/**< whether enale/disable input frame usage mode */
	EncFrmUsedMode		frmUsedMode;	/**< input frame usage mode variable */
	uint32_t			frmUsedTimes;	/**< the frame interval (repeat or skip mode) */
} IMPEncoderAttrFrmUsed;

/**
 * Define the encode skip type of h264 channel
 */
typedef enum {
	IMP_Encoder_STYPE_N1X       = 0,	/**< 1 time (skip frame reference) */
	IMP_Encoder_STYPE_N2X       = 1,	/**< 2 times (skip frame reference) */
	IMP_Encoder_STYPE_N4X       = 2,	/**< 4 times (skip frame reference) */
	IMP_Encoder_STYPE_HN1_FALSE = 3,	/**< high skip frame reference mode:N1 open skip frame */
	IMP_Encoder_STYPE_HN1_TRUE	= 4,	/**< high skip frame reference mode:N1 close skip frame */
	IMP_Encoder_STYPE_H1M_FALSE = 5,	/**< high skip frame reference mode:1M open skip frame */
	IMP_Encoder_STYPE_H1M_TRUE  = 6,	/**< high skip frame reference mode:1M close skip frame */
} IMPSkipType;

/**
 * Define the encode ref type of h264 channel
 */
typedef enum {
	IMP_Encoder_FSTYPE_IDR		= 0,	/**< the idr frame in high skip frame reference mode */
	IMP_Encoder_FSTYPE_LBASE	= 1,	/**< the long base(p) frame in high skip frame reference mode */
	IMP_Encoder_FSTYPE_SBASE	= 2,	/**< the short base(p) frame in high skip frame reference mode */
	IMP_Encoder_FSTYPE_ENHANCE	= 3,	/**< the enhance(p) frame in high skip frame reference mode */
} IMPRefType;

/**
 * Define high skip frame type structure of h264 channel
 */
typedef struct {
	IMPSkipType	skipType;           /**< skip type */
	int			m;                  /**< enhance frame interval */
	int			n;                  /**< base frame interval */
	int			maxSameSceneCnt;    /**< one scene over gop num, only effect to H1M skip type, when set more than 1, m no effect */
	int			bEnableScenecut;    /**< is enable scenecut? 0: not, 1: yes, only effect to H1M Skip type */
	int			bBlackEnhance;      /**< be made to use black coded bitstream to enhance frame? */
} IMPEncoderAttrHSkip;

/**
 * Define high skip frame type init structure of h264 channel
 */
typedef struct {
	IMPEncoderAttrHSkip	hSkipAttr;	  /**< high skip frame type attr */
	IMPSkipType			maxHSkipType; /**< must used max skip type, effect rmem size, N1X to N2X need 2 rd frame space, N4X to H1M_TRUE need 3 rd frame space size, should set according to your hskip requirement*/
} IMPEncoderAttrInitHSkip;

/**
 * Define the h264 encoder channel ratecontroller mode attribute
 */
typedef struct {
	IMPEncoderRcMode rcMode;						/**< ratecontrol mode */
	union {
		IMPEncoderAttrH264FixQP	 attrH264FixQp;		/**< the H.264 protocol FixQp ratecontrol attribute  */
		IMPEncoderAttrH264CBR	 attrH264Cbr;		/**< the H.264 protocol CBR ratecontrol attribute */
		IMPEncoderAttrH264VBR	 attrH264Vbr;		/**< the H.264 protocol VBR ratecontrol attribute */
		IMPEncoderAttrH264Smart	 attrH264Smart;		/**< the H.264 protocol Smart ratecontrol attribute */
		IMPEncoderAttrH265FixQP	 attrH265FixQp;		/**< Unsupport H.265 protocol */
		IMPEncoderAttrH265CBR	 attrH265Cbr;		/**< Unsupport H.265 protocol */
		IMPEncoderAttrH265VBR	 attrH265Vbr;		/**< Unsupport H.265 protocol */
		IMPEncoderAttrH265Smart	 attrH265Smart;		/**< Unsupport H.265 protocol */
	};
} IMPEncoderAttrRcMode;

/**
 * Define the h264 encoder channel ratecontroller attribute
 */
typedef struct {
	IMPEncoderFrmRate           outFrmRate;		/**< output framerate, (frame output shouldn't be bigger than input framerate) */
	uint32_t			        maxGop;			/**< gop value，suggested to be an integer multiple of the framerate */
    IMPEncoderAttrRcMode        attrRcMode;     /**< ratecontrol mode attribute */
	IMPEncoderAttrFrmUsed	    attrFrmUsed;	/**< input frame usage mode attribute */
	IMPEncoderAttrDenoise	    attrDenoise;	/**< denoise attribute */
	IMPEncoderAttrInitHSkip	    attrHSkip;		/**< high skip frame type attribute */
} IMPEncoderRcAttr;

/**
 * H264 stream NAL unit type codes
 */
typedef enum {
	IMP_H264_NAL_UNKNOWN	= 0,	/**< Unspecified */
	IMP_H264_NAL_SLICE		= 1,	/**< Coded slice of a non-IDR picture  */
	IMP_H264_NAL_SLICE_DPA	= 2,	/**< Coded slice data partition A */
	IMP_H264_NAL_SLICE_DPB	= 3,	/**< Coded slice data partition B */
	IMP_H264_NAL_SLICE_DPC	= 4,	/**< Coded slice data partition C */
	IMP_H264_NAL_SLICE_IDR	= 5,	/**< Coded slice of an IDR picture */
	IMP_H264_NAL_SEI		= 6,	/**< Supplemental enhancement information(SEI) */
	IMP_H264_NAL_SPS		= 7,	/**< Sequence parameter set */
	IMP_H264_NAL_PPS		= 8,	/**< Picture parameter set */
	IMP_H264_NAL_AUD		= 9,	/**< Access unit delimiter */
	IMP_H264_NAL_FILLER		= 12,	/**< Filler data */
} IMPEncoderH264NaluType;

typedef enum {
    IMP_H265_NAL_SLICE_TRAIL_N      = 0,        /**< Coded slice segment of a non-TSA, non-STSA trailing picture, none reference */
    IMP_H265_NAL_SLICE_TRAIL_R      = 1,        /**< Coded slice segment of a non-TSA, non-STSA trailing picture, with reference */
    IMP_H265_NAL_SLICE_TSA_N        = 2,        /**< Coded slice segment of a temporal sub-layer access picture, none reference */
    IMP_H265_NAL_SLICE_TSA_R        = 3,        /**< Coded slice segment of a temporal sub-layer access picture, with reference */
    IMP_H265_NAL_SLICE_STSA_N       = 4,        /**< Coded slice segment of a step-wise temporal sub-layer access picture, none reference */
    IMP_H265_NAL_SLICE_STSA_R       = 5,        /**< Coded slice segment of a step-wise temporal sub-layer access picture, with reference */
    IMP_H265_NAL_SLICE_RADL_N       = 6,        /**< Coded slice segment of a random access decodable leading picture, none reference */
    IMP_H265_NAL_SLICE_RADL_R       = 7,        /**< Coded slice segment of a random access decodable leading picture, with reference */
    IMP_H265_NAL_SLICE_RASL_N       = 8,        /**< Coded slice segment of a random access skipped leading picture, none reference*/
    IMP_H265_NAL_SLICE_RASL_R       = 9,        /**< Coded slice segment of a random access skipped leading picture, with reference */
    IMP_H265_NAL_SLICE_BLA_W_LP     = 16,       /**< Coded slice segment of a broken link access picture, with leading picture */
    IMP_H265_NAL_SLICE_BLA_W_RADL   = 17,       /**< Coded slice segment of a broken link access picture, with leading RADL */
    IMP_H265_NAL_SLICE_BLA_N_LP     = 18,       /**< Coded slice segment of a broken link access picture, none leading picture */
    IMP_H265_NAL_SLICE_IDR_W_RADL   = 19,       /**< Coded slice segment of an instantaneous decoding refresh picture, with RADL */
    IMP_H265_NAL_SLICE_IDR_N_LP     = 20,       /**< Coded slice segment of an instantaneous decoding refresh picture, none leading picture */
    IMP_H265_NAL_SLICE_CRA          = 21,       /**< Coded slice segment of a clean random access picture, with leading picture */
    IMP_H265_NAL_VPS                = 32,       /**< video parameter set */
    IMP_H265_NAL_SPS                = 33,       /**< sequence parameter set */
    IMP_H265_NAL_PPS                = 34,       /**< picture parameter set */
    IMP_H265_NAL_AUD                = 35,       /**< access unit delimiter */
    IMP_H265_NAL_EOS                = 36,       /**< end of sequence */
    IMP_H265_NAL_EOB                = 37,       /**< end of bitstream */
    IMP_H265_NAL_FILLER_DATA        = 38,       /**< filler data */
    IMP_H265_NAL_PREFIX_SEI         = 39,       /**< prifix supplemental enhancement information */
    IMP_H265_NAL_SUFFIX_SEI         = 40,       /**< suffix supplemental enhancement information */
    IMP_H265_NAL_INVALID            = 64,       /**< invalid nal unit type */
} IMPEncoderH265NaluType;

/**
 * Define the h264 bitstream nal type
 */
typedef union {
	IMPEncoderH264NaluType		h264Type;		/**< H264 stream NAL unit type code */
	IMPEncoderH265NaluType		h265Type;		/**< H265 stream NAL unit type code, Unsupport H.265 protocol */
} IMPEncoderDataType;

/**
 * Define frame bitstream packet structure
 */
typedef struct {
	uint32_t	phyAddr;			/**< bitstream packet physical address */
	uint32_t	virAddr;			/**< bitstream packet virtual address */
	uint32_t	length;				/**< bitstram packet length */
	int64_t		timestamp;			/**< timestamp，unit:us */
	bool		frameEnd;			/**< frame end mark */
	IMPEncoderDataType	dataType;	/**< h264 bitstream nal unit type */
} IMPEncoderPack;

/**
 * Define the bitstream structure of one frame
 */
typedef struct {
	IMPEncoderPack  *pack;			/**< bitstream packet */
	uint32_t        packCount;		/**< packet count related to one frame */
	uint32_t        seq;			/**< bitstream sequence number */
    IMPRefType      refType;		/**< h264 reference type of encode frame */
} IMPEncoderStream;

/**
 * Define the encoder crop attribute(crop first, scaler second)
 */
typedef struct {
    bool	enable;			/**< whether enable/disable crop, false:disable, true: enable */
    uint32_t	x;			/**< the left-top x-coordinate of crop region */
    uint32_t	y;			/**< the left-top y-coordinate of crop region */
    uint32_t	w;			/**< the width of crop region */
    uint32_t	h;			/**< the height of crop region */
} IMPEncoderCropCfg;

/**
 * Define insert user data attribute of h264 channel
 */
typedef struct {
	uint32_t			maxUserDataCnt;		/**< User's Maximum number of insert data cache, range:0-2 */
	uint32_t			maxUserDataSize;	/**< User's Maximum size of insert data cache, range:16-1024 */
} IMPEncoderUserDataCfg;

/**
 * Define encoder attribute structure
 */
typedef struct {
	IMPPayloadType			enType;			/**< encode protocal type */
	uint32_t				bufSize;		/**< Configure buffer size，shouldn't be less than width*height*3/2. When setting the channel encoding property, set this parameter to 0, IMP will automatically calculate the size*/
	uint32_t				profile;		/**< encode profile, 0: baseline; 1:MP; 2:HP */
	uint32_t				picWidth;		/**< frame width, must be 16 aligned,shouldn't less than 256 */
	uint32_t				picHeight;		/**< frame height, shouldn't less than 16 */
	IMPEncoderCropCfg		crop;			/**< crop attributel */
	IMPEncoderUserDataCfg	userData;		/**< insert userdata attribute, only for h264*/
} IMPEncoderAttr;

/**
 * Define encoder channel attribute structure
 */
typedef struct {
	IMPEncoderAttr      encAttr;     /**< encoder attribute */
	IMPEncoderRcAttr    rcAttr;      /**< ratecontrol attribute, only for h264 */
	bool                bEnableIvdc; /**< Enable ISP VPU Direct Connect */
} IMPEncoderCHNAttr;

/**
 * Define encode channel state attribute structure
 */
typedef struct {
	bool		registered;			/**< registered to group variable, 0:no,1:yes */
	uint32_t	leftPics;			/**< Number of images to be encoded */
	uint32_t	leftStreamBytes;	/**< Number of Stream buffer remaining byte */
	uint32_t	leftStreamFrames;	/**< Number of Stream buffer remaining frames */
	uint32_t	curPacks;			/**< Number of packets in current frame */
	uint32_t	work_done;			/**< work done state，0：running，1，not running */
} IMPEncoderCHNStat;

/**
 * Define color to grey parameter
 */
typedef struct {
    bool	enable;			/**< variable to enable color to grey, 0: no, 1: yes */
} IMPEncoderColor2GreyCfg;

/**
 * Define the h264 channel enalbe IDR parameter
 */
typedef struct {
    bool	enable;			/**< Configuration of EnableIDR, 0:disable, 1:enable */
} IMPEncoderEnIDRCfg;

/**
 * Define the h264 channel gopsize parameter
 */
typedef struct {
	int		gopsize;		/**< IDR parameters */
} IMPEncoderGOPSizeCfg;

/**
 * Define h264 channel ROI parameters
 */
typedef struct {
	uint32_t	u32Index;	/**< ROI region index value，range:[0-7] */
	bool		bEnable;	/**< whether enable/disable ROI func to the region */
	bool		bRelatedQp;	/**< 0：absolute ROI，1：relative ROI */
	int		s32Qp;			/**< the absolute or relative qp in roi region */
	IMPRect		rect;		/**< region coordinate attribute(s) */
} IMPEncoderROICfg;

/**
 * Define h264 channel process strategy of ratecontrol super frame
 */
typedef enum {
    IMP_RC_SUPERFRM_NONE        = 0,    /**< no strategy (supported) */
    IMP_RC_SUPERFRM_DISCARD     = 1,    /**< dop super frame (unsupported), users should decide whether drop or not */
    IMP_RC_SUPERFRM_REENCODE    = 2,    /**< reencoder super frame (supported), default */
    IMP_RC_SUPERFRM_BUTT        = 3,
} IMPEncoderSuperFrmMode;

/**
 * Define h264 channel priority type of ratecontrol
 */
typedef enum {
    IMP_RC_PRIORITY_RDO                 = 0,    /**< rdo priority */
    IMP_RC_PRIORITY_BITRATE_FIRST       = 1,    /**< target frame rate first */
    IMP_RC_PRIORITY_FRAMEBITS_FIRST     = 2,    /**< super frame threshold first */
    IMP_RC_PRIORITY_BUTT                = 3,
} IMPEncoderRcPriority;

/**
 * Define h264 channel Paramete of super frame process strategy
 */
typedef struct {
    IMPEncoderSuperFrmMode      superFrmMode;       /**< super frame process strategy, default: SUPERFRM_REENCODE */
    uint32_t                    superIFrmBitsThr;   /**< super I frame threshold, default: w*h*3/2*8/ratio, ratio: mainres-6, subres-3 */
    uint32_t                    superPFrmBitsThr;   /**< super P frame threadhold, default:super I frame threshold divide by 1.4 */
    uint32_t                    superBFrmBitsThr;   /**< super B frame threadhold, default:super P frame threshold divide by 1.3 */
    IMPEncoderRcPriority        rcPriority;         /**< priority of ratecontrol, default: RDO */
} IMPEncoderSuperFrmCfg;

/**
 * Define H.264 encoder channel chroma quantization struct
 */
typedef struct {
    int         chroma_qp_index_offset;     /**< refer to  H.264 prococol, system default value is 0; range:[-12, 12] */
} IMPEncoderH264TransCfg;

/**
 * Define macroblock ratecontrol mode struct, H264 only support 1-4 mode
 */
typedef struct {
    int         chroma_cr_qp_offset;
    int         chroma_cb_qp_offset;    /** Unsupport H.265 protocol */
} IMPEncoderH265TransCfg;

typedef enum {
    ENC_QPG_CLOSE   = 0,    /**< close macroblock level ratecontrol */
    ENC_QPG_CRP     = 1,    /**< open CRP mode (default) */
    ENC_QPG_SAS     = 2,    /**< open SAS mode */
    ENC_QPG_SASM    = 3,    /**< open SAS mode and mult-thresh mode */
    ENC_QPG_MBRC    = 4,    /**< open macroblock level ratecontrol */
    ENC_QPG_CRP_TAB = 5,    /**< open CRP mode and QP_TAB */
    ENC_QPG_SAS_TAB = 6,    /**< open SAS mode and QP_TAB */
    ENC_QPG_SASM_TAB= 7,    /**< open SAS mode, mult-thresh mode and QP_TAB */
} IMPEncoderQpgMode;

/**
 * Define JPEG encoder channel quantization table set param struct
 */
typedef struct {
    bool user_ql_en;		/**< 0:use default quantization table; 1:use user's quantization table*/
    uint8_t qmem_table[128];/**< user's quantization table*/
} IMPEncoderJpegeQl;

/**
 * @fn int IMP_Encoder_CreateGroup(int encGroup)
 *
 * Create encode Group
 *
 * @param[in] encGroup Group number,range:[0, @ref NR_MAX_ENC_GROUPS - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark one Group only support one resolution, and can allow 2 encode channel to be registered into.
 *
 * @attention return failure if group has been created.
 */
int IMP_Encoder_CreateGroup(int encGroup);

/**
 * @fn int IMP_Encoder_DestroyGroup(int encGroup)
 *
 * destroy the encoding Group
 *
 * @param[in] encGroup Group number,range:[0, @ref NR_MAX_ENC_GROUPS - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark Destroy a Group, requires that the group must be empty, that is to say,no channel is registered into, otherwise it will return a failure value.
 *
 * @attention the group must have been created, otherwise it will return a failure value.
 */
int IMP_Encoder_DestroyGroup(int encGroup);

/**
 * @fn int IMP_Encoder_CreateChn(int encChn, const IMPEncoderCHNAttr *attr)
 *
 * create encoding channel
 *
 * @param[in] encChn encoder channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] attr encode channel attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark encoder channel contains encoder attribute and ratecontroller attribute
 * @remark encoder should choose encoder protocol, and then set the attribute to the encoder
 */
int IMP_Encoder_CreateChn(int encChn, const IMPEncoderCHNAttr *attr);

/**
 * @fn int IMP_Encoder_DestroyChn(int encChn)
 *
 * destroy encode Channel
 *
 * @param[in] encChn encoder channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @attention destroy uncreated channel will cause failure
 * @attention Before destroying a channel, make sure that it is not registered, otherwise it will return a failure value.
 */
int IMP_Encoder_DestroyChn(int encChn);

/**
 * @fn int IMP_Encoder_GetChnAttr(int encChn, IMPEncoderCHNAttr * const attr)
 *
 * get encode channel attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] attr encode channel attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 */
int IMP_Encoder_GetChnAttr(int encChn, IMPEncoderCHNAttr * const attr);

/**
 * @fn int IMP_Encoder_RegisterChn(int encGroup, int encChn)
 *
 * register encode channel to Group
 *
 * @param[in] encGroup encode group num, range:[0, @ref NR_MAX_ENC_GROUPS - 1]
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @attention registration of a channel to an uncreated Group will cause failure
 * @attention One channel can be registed to only one group, if it has already been registered to another Group, it will return failure;
 * @attention If one Group is already used by a Channel, so it can not be used again by another one unless this Group's Channel is first destroyed, otherwise it wil return failure.
 */

int IMP_Encoder_RegisterChn(int encGroup, int encChn);
/**
 * @fn int IMP_Encoder_UnRegisterChn(int encChn)
 *
 * unregister encode channel from group
 *
 * @param[in] encGroup encode group num, range:[0, @ref NR_MAX_ENC_GROUPS - 1]
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark After unregistering a Channel, the channel bitstream buffer will be flushed.
 * if the bitstream hasn't been released, the data in bitstream buffer might be invalid.
 * so, users can use the IMP_Encoder_Query interface to check the status of the buffer's Channel code in order to confirm Confirm code stream buffer in the code stream to take over and then we can make new registration.
 *
 * @attention To unregister uncreated channel will return failure.
 * @attention To unregister unregistered channel will return failure
 * @attention if the channel is running, then it will return failure
 */
int IMP_Encoder_UnRegisterChn(int encChn);

/**
 * @fn int IMP_Encoder_StartRecvPic(int encChn)
 *
 * start encode channel to receive frames
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark First open the encoded Channel (receive the image) then this function can be used.
 *
 * @attention uncreated channel will cause failure
 * @attention unregistered channel whill cause failure
 */
int IMP_Encoder_StartRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_StopRecvPic(int encChn)
 *
 * stop encode channel to receive frames
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the function can be successly used repeatedly
 * @remark Call this interface only to stop receiving the original data encoding, code stream buffer and will not be eliminated.
 *
 * @attention stop uncreated channel will cause failure
 * @attention stop unregistered channel whill cause failure
 */
int IMP_Encoder_StopRecvPic(int encChn);

/**
 * @fn int IMP_Encoder_Query(int encChn, IMPEncoderCHNStat *stat)
 *
 * query encode channel state
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] stat encode channel stat pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 */
int IMP_Encoder_Query(int encChn, IMPEncoderCHNStat *stat);

/**
 * @fn int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag)
 *
 * Get encode bitstream
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] stream the bitstream pointer
 * @param[in] blockFlag use block mode variable，0：no，1：yes
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark each time get one frame bitstream

 * @remark If the user does not get any stream for a long time, the stream buffer will be full. If a code Channel is full, The last received images will be lost until the user starts getting some streams, in order to have enough code stream buffer for encoding.
 *
 * It is suggested that the users access the code stream interface and the release of the code stream interface in pairs , and then release the stream as soon as possible, in order to prevent the user state to get the stream, when the release is not timely it might lead to the case where the stream
buffer is full, and then stop coding.

 * @remark For H264 type code stream, each time that it is called successfully, it is to get one frame stream, this frame stream may contain multiple packages.
 * @remark For JPEG type code stream,  each time that it is called successfully, it is to get one frame stream, this frame can contain only one package, this frame contains the complete information of the JPEG image file.
 *
 * examples：
 * @code
 * int ret;
 * ret = IMP_Encoder_PollingStream(ENC_H264_CHANNEL, 1000); //Polling the bitstream buffer
 * if (ret < 0) {
 *     printf("Polling stream timeout\n");
 *     return -1;
 * }
 *
 * IMPEncoderStream stream;
 * ret = IMP_Encoder_GetStream(ENC_H264_CHANNEL, &stream, 1); //get a frame stream; block mode
 * if (ret < 0) {
 *     printf("Get Stream failed\n");
 *     return -1;
 * }
 *
 * int i, nr_pack = stream.packCount;
 * for (i = 0; i < nr_pack; i++) {        //save each package of the frame stream
 *     ret = write(stream_fd, (void *)stream.pack[i].virAddr,
 *                stream.pack[i].length);
 *     if (ret != stream.pack[i].length) {
 *         printf("stream write error:%s\n", strerror(errno));
 *         return -1;
 *     }
 * }
 * @endcode
 *
 * @attention if pstStream is NULL,then return failure；
 * @attention if the channel uncreated，then return failure；
 */
int IMP_Encoder_GetStream(int encChn, IMPEncoderStream *stream, bool blockFlag);

/**
 * @fn int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream)
 *
 * release the bitstream buffer
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] stream the bitstream pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark This interface should be paired up with IMP_Encoder_GetStream
 * users after getting the stream must release in time the obtained stream caches, otherwise it might cause the stream buffer to be full, this fact will have an impact the encoder. \n
* and the user must use FIFO process for the releasing.
 * @remark After cancelling the registration of the encoding Channel, all of the non-released stream packages are invalid, they can no longer be used or released. this current function is invalid when the channel is unregistered
 *
 * @attention if pstStream is NULL,then return failure；
 * @attention if the channel uncreated，then return failure；
 * @attention invalid release bitstream will cause failure
 */
int IMP_Encoder_ReleaseStream(int encChn, IMPEncoderStream *stream);

/**
 * @fn int IMP_Encoder_PollingStream(int encChn, uint32_t timeoutMsec)
 *
 * Polling bitstream buffer
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] timeoutMsec polling timeout，unit:ms
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark call this interface before calling IMP_Encoder_GetStream
 *
 */
int IMP_Encoder_PollingStream(int encChn, uint32_t timeoutMsec);

/**
 * @fn int IMP_Encoder_GetFd(int encChn)
 *
 * Get encode channel's device file descriptor handler
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval >=0 success, return the file descriptor
 * @retval <0  failure
 *
 * @remark when getting mult channel's bitstream in one place, IMP_Encoder_PollingStream may be not suitable, \n
 * you can use the descriptor and select or poll and similar to poll the encode finish state.
 * @remark uncreated channel will cause failure
 */
int IMP_Encoder_GetFd(int encChn);

/**
 * @fn int IMP_Encoder_SetMaxStreamCnt(int encChn, int nrMaxStream)
 *
 * Set bitstream buffer number
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] nrMaxStream bitstream buffer num,range:[1, @ref NR_MAX_ENC_CHN_STREAM]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark Since the stream cache Buffer number has been fixed during the channel creation, so the API needs to call this current before creating the Channel.
 * @remark If you do not call this API before the creation of the channel to set the number of stream cache Buffer, then the H264 channel default number is 5, JPEG channel default number is 1.
 */
int IMP_Encoder_SetMaxStreamCnt(int encChn, int nrMaxStream);

/**
 * @fn int IMP_Encoder_GetMaxStreamCnt(int encChn, int *nrMaxStream)
 *
 * Get bitstream buffer number
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] nrMaxStream bitstream buffer num pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 */
int IMP_Encoder_GetMaxStreamCnt(int encChn, int *nrMaxStream);

/**
 * @fn int IMP_Encoder_RequestIDR(int encChn)
 *
 * Requst IDR frame
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark  After calling this current function there will be an application for IDR frame coding.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_RequestIDR(int encChn);

/**
 * @fn int IMP_Encoder_FlushStream(int encChn)
 *
 * flush old bitstream of encoder and start with idr frame to encode
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark  After calling this current function there will be an application for IDR frame coding.
 */
int IMP_Encoder_FlushStream(int encChn);

/**
 * @fn int IMP_Encoder_SetChnColor2Grey(int encChn, const IMPEncoderColor2GreyCfg *pstColor2Grey).
 *
 * set color to grey attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstColor2Grey color to grey encode parameter
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark After calling this interface, the grayscale function will be active until it is deactivated.
 * @remark the channel should be created before calling this function
 */
int IMP_Encoder_SetChnColor2Grey(int encChn, const IMPEncoderColor2GreyCfg *pstColor2Grey);

/**
 * @fn int IMP_Encoder_GetChnColor2Grey(int encChn, IMPEncoderColor2GreyCfg *pstColor2Grey).
 *
 * get color to grey attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstColor2Grey color to grey encode parameter
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this interface just get the grayscale function attribute(s). the channel should be created before calling this function
 * 
 * @attention it is meaningless for JPEG Encoder
 */
int IMP_Encoder_GetChnColor2Grey(int encChn, IMPEncoderColor2GreyCfg *pstColor2Grey);

/**
 * @fn int IMP_Encoder_SetChnAttrRcMode(int encChn, const IMPEncoderAttrRcMode *pstRcModeCfg).
 *
 * Set encoding channel rate controller mode attribute(s)
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstRcCfg encode channel ratecontrol mode attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark This function configures the current Channel rate controller attribute(s) until it is deactivated,  \n
 * the channel should be created before calling this function.
 *
 * @attention up to now, rate control supports ENC_RC_MODE_FIXQP, ENC_RC_MODE_CBR. ENC_RC_MODE_VBR, ENC_RC_MODE_SMART
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnAttrRcMode(int encChn, const IMPEncoderAttrRcMode *pstRcModeCfg);

/**
 * @fn int IMP_Encoder_GetChnAttrRcMode(int encChn, IMPEncoderAttrRcMode *pstRcModeCfg).
 *
 * Get encode channel rate controller mode attribute(s)
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstRcCfg encode channel rate control mode attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this function gets the mode attribute(s) of the current Channel rate controller. \n
 * The channel must have been created before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnAttrRcMode(int encChn, IMPEncoderAttrRcMode *pstRcModeCfg);

/**
 * @fn int IMP_Encoder_SetChnFrmRate(int encChn, const IMPEncoderFrmRate *pstFps)
 *
 * Set encode channel framerate controlled attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstFpsCfg framerate controlled attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this function can reset encode frame rate attribute(s), and it will take effect at the next GOP(delay to be effective:1s).
 * @remark if you want to use IMP_FrameSource_SetChnFPS() to set isp framerate, you need to first call this current function in order to set encoder framerate
 * @remark the channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnFrmRate(int encChn, const IMPEncoderFrmRate *pstFps);

/**
 * @fn int IMP_Encoder_GetChnFrmRate(int encChn, IMPEncoderFrmRate *pstFps)
 *
 * Get encode channel framerate controlled attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstFpsCfg framerate controlled attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this function gets the Channel 's frame rate control attribute(s). The channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnFrmRate(int encChn, IMPEncoderFrmRate *pstFps);

/**
 * @fn int IMP_Encoder_SetChnROI(int encChn, const IMPEncoderROICfg *pstVencRoiCfg)
 *
 * Set encoder channel ROI attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstFpsCfg ROI attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel's ROI attribute(s), and the channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnROI(int encChn, const IMPEncoderROICfg *pstVencRoiCfg);

/**
 * @fn int IMP_Encoder_GetChnROI(int encChn, IMPEncoderROICfg *pstVencRoiCfg)
 *
 * Get encoder channel roi attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstFpsCfg ROI attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It gets the Channel's ROI attribute(s), and the channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnROI(int encChn, IMPEncoderROICfg *pstVencRoiCfg);

/**
 * @fn int IMP_Encoder_GetGOPSize(int encChn, IMPEncoderGOPSizeCfg *pstGOPSizeCfg)
 *
 * Get encoder channel GOP attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstGOPSizeCfg GOPSize attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It gets the Channel's GOPSize attribute(s), and the channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetGOPSize(int encChn, IMPEncoderGOPSizeCfg *pstGOPSizeCfg);

/**
 * @fn int IMP_Encoder_SetGOPSize(int encChn, const IMPEncoderGOPSizeCfg *pstGOPSizeCfg)
 *
 * Set encoder channel gop attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstGOPSizeCfg GOPSize attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel's GOPSize attribute(s), and the channel must have been created before calling this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetGOPSize(int encChn, const IMPEncoderGOPSizeCfg *pstGOPSizeCfg);

/**
 * @fn int IMP_Encoder_SetChnFrmUsedMode(int encChn, const IMPEncoderAttrFrmUsed *pfrmUsedAttr)
 *
 * Set encoder channel input frame usage mode attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pfrmUsedAttr input frame usage mode attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel's input frame usage mode attribute(s), and the channel must have been created before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnFrmUsedMode(int encChn, const IMPEncoderAttrFrmUsed *pfrmUsedAttr);

/**
 * @fn int IMP_Encoder_GetChnFrmUsedMode(int encChn, IMPEncoderAttrFrmUsed *pfrmUsedAttr)
 *
 * Get encoder channel input frame usage mode attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pfrmUsedAttr input frame usage mode attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It gets the Channel's input frame usage mode attribute(s), and the channel must have been created before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnFrmUsedMode(int encChn, IMPEncoderAttrFrmUsed *pfrmUsedAttr);

/**
 * @fn int IMP_Encoder_SetChnDenoise(int encChn, const IMPEncoderAttrDenoise *pdenoiseAttr)
 *
 * Set encode channel denoise attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pdenoiseAttr denoise attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel's denoise attribute(s), and the channel must have been created before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnDenoise(int encChn, const IMPEncoderAttrDenoise *pdenoiseAttr);

/**
 * @fn int IMP_Encoder_GetChnDenoise(int encChn, IMPEncoderAttrDenoise *pdenoiseAttr)
 *
 * Get encode channel denoise attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pdenoiseAttr denoise attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It gets the Channel's denoise attribute(s), and the channel must have been created before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnDenoise(int encChn, IMPEncoderAttrDenoise *pdenoiseAttr);

/**
 * @fn int IMP_Encoder_SetChnHSkip(int encChn, const IMPEncoderAttrHSkip *phSkipAttr)
 *
 * Set encode channel high skip reference attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] phSkipAttr high skip reference attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel's high skip reference attribute(s), and the channel must have been created before calling this function.
 * @remark If the Channel's created high skip reference type is one of IMP_Encoder_STYPE_N1X or IMP_Encoder_STYPE_N2X,
 * this function only be permited to set to be IMP_Encoder_STYPE_N1X or IMP_Encoder_STYPE_N2X;
 * @remark If the Channel's created high skip reference type is one of the type from IMP_Encoder_STYPE_N4X to
 * IMP_Encoder_STYPE_H1M_TRUE, then this function can set any one enum skip type of IMPSkipType;
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnHSkip(int encChn, const IMPEncoderAttrHSkip *phSkipAttr);

/**
 * @fn int IMP_Encoder_GetChnHSkip(int encChn, IMPEncoderAttrHSkip *phSkipAttr)
 *
 * Get encode channel high skip reference attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] phSkipAttr high skip reference attribute pointer
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It gets the Channel's high skip reference attribute(s), and the channel must have been created
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChnHSkip(int encChn, IMPEncoderAttrHSkip *phSkipAttr);

/**
 * @fn int IMP_Encoder_SetChnHSkipBlackEnhance(int encChn, const int bBlackEnhance)
 *
 * Set encode channel'bBlackEnhance in high skip reference attribute
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] bBlackEnhance bool value, mean to bBlackEnhance in IMPEncoderAttrHSkip attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark It sets the Channel' bBlackEnhance in high skip reference attribute(s), and the channel must have been created
 * before calling this function.
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChnHSkipBlackEnhance(int encChn, const int bBlackEnhance);

/**
 * @fn int IMP_Encoder_InsertUserData(int encChn, void *userData, uint32_t userDataLen)
 *
 * insert userdata
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] userData userdata virtual address pointer
 * @param[in] userDataLen user data length, range:(0, 1024], unit:byte
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before call this function, otherwise it will return a failure value.
 * @remark if userData is null or userDataLen is 0, then return a failure value
 * @remark this function only supports h264 protocol
 * @remark H.264 protocol channel allocation is up to 2 blocks of memory space for caching user data, and each user data size does not exceed 1K byte.
 * This interface will return an error if the data inserted by the user is more than 2 blocks, or if the user data is more than byte 1K.
 * @remark Each user data in SEI package is inserted into the previous new image codestream package.  After a user data package is encoded and transmitted, the H.264 channel customer data in the cache memory space is cleared for new user data storage.
 * @remark each userdata is encapsulated to sei nal and placed at the front of the latest frame nal.
 * @remark when the userdata is encapsulated, the userdata buffer will be set to zero
 * @remark the channel must have been created before call this function
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_InsertUserData(int encChn, void *userData, uint32_t userDataLen);

/**
 * @fn int IMP_Encoder_SetFisheyeEnableStatus(int encChn, int enable)
 *
 * set enabled status of ingenic fisheye correction algorithm
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] enable 0: disable(default), 1: enable
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark Since the enabled status of ingenic fisheye correction algorithm has been fixed during the channel creation, so the API needs to call this current before creating the Channel.
 * @remark If you do not call this API before the creation of the channel to set enabled status of ingenic fisheye correction algorithm, then the status is disable, that is to say, you can't use ingenic fisheye correction algorithm to correct any h264 stream;
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetFisheyeEnableStatus(int encChn, int enable);

/**
 * @fn int IMP_Encoder_GetFisheyeEnableStatus(int encChn, int *enable)
 *
 * get enabled status of ingenic fisheye correction algorithm
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] enable 0: disabled, 1: enabled
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetFisheyeEnableStatus(int encChn, int *enable);

/**
 * @fn int IMP_Encoder_SetChangeRef(int encChn, int bEnable)
 *
 * set whether allow change base skip frame's reference or not
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] enable 0：not allowed to change, 1: allowed to change(default)
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this API should be used after create encoder channel, only available to smart ratecontrol
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetChangeRef(int encChn, int bEnable);

/**
 * @fn int IMP_Encoder_GetChangeRef(int encChn, int *bEnable)
 *
 * get whether allow change base skip frame's reference or not
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] enable return the status whether allow change base skip frame's reference or not, 0：not allowed, 1: allowed
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetChangeRef(int encChn, int *bEnable);

/**
 * @fn int IMP_Encoder_SetMbRC(int encChn, int bEnable)
 *
 * set whether open mb ratecontrol or not
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] bEnable 0:close(default), 1:open
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark this API should be used after create encoder channel, and effect next frame
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetMbRC(int encChn, int bEnable);

/**
 * @fn int IMP_Encoder_GetMbRC(int encChn, int *bEnable)
 *
 * get the status whether open mb ratecontrol or not
 *
 * @param[in] encChn encode channel num,range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] bEnable return the status whether open mb ratecontrol or not, 0:close, 1:open
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetMbRC(int encChn, int *bEnable);

/**
 * @fn int IMP_Encoder_SetSuperFrameCfg(int encChn, const IMPEncoderSuperFrmCfg *pstSuperFrmParam)
 *
 * Set video coded supper frame configuration
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstSuperFrmParam video coded supper frame configuration
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_SetSuperFrameCfg(int encChn, const IMPEncoderSuperFrmCfg *pstSuperFrmParam);

/**
 * @fn int IMP_Encoder_GetSuperFrameCfg(int encChn, IMPEncoderSuperFrmCfg *pstSuperFrmParam)
 *
 * Get video coded supper frame configuration
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstSuperFrmParam return video coded supper frame configuration
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @attention this API only used to H264 channel
 */
int IMP_Encoder_GetSuperFrameCfg(int encChn, IMPEncoderSuperFrmCfg *pstSuperFrmParam);

/**
 * @fn int IMP_Encoder_SetH264TransCfg(int encChn, const IMPEncoderH264TransCfg *pstH264TransCfg)
 *
 * Set H.264 encode channel chroma quantization attribute
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstH264TransCfg H.264 encode channel chroma quantization attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by h264;
 * @remark advice used after IMP_Encoder_CreateChn and before IMP_Encoder_StartRecvPic function, should GetH264TransCfg first, \n
 * and then use this API to set h264 transform configuration
 */
int IMP_Encoder_SetH264TransCfg(int encChn, const IMPEncoderH264TransCfg *pstH264TransCfg);

/**
 * @fn int IMP_Encoder_GetH264TransCfg(int encChn, IMPEncoderH264TransCfg *pstH264TransCfg)
 *
 * Get H.264 encode channel chroma quantization attribute
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstH264TransCfg returned H.264 encode channel chroma quantization attribute
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by h264;
 */
int IMP_Encoder_GetH264TransCfg(int encChn, IMPEncoderH264TransCfg *pstH264TransCfg);

/**
 * @fn int IMP_Encoder_SetQpgMode(int encChn, const IMPEncoderQpgMode *pstQpgMode)
 *
 * Set macroblock encode mode
 *
 * @param[in] encChn encode channel num,range: [0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstQpgMode macroblock encode mode
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by H264
 *
 */
int IMP_Encoder_SetQpgMode(int encChn, const IMPEncoderQpgMode *pstQpgMode);

/**
 * @fn int IMP_Encoder_GetQpgMode(int encChn, IMPEncoderQpgMode *pstQpgMode)
 *
 * Get macroblock encode mode
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstQpgMode returned macroblock encode mode
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by H264;
 */
int IMP_Encoder_GetQpgMode(int encChn, IMPEncoderQpgMode *pstQpgMode);

/**
 * @fn int IMP_Encoder_GetChnEncType(int encChn, IMPPayloadType *payLoadType)
 *
 * Get encode channel protocol type
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] payLoadType return encode channel protocol type
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 */
int IMP_Encoder_GetChnEncType(int encChn, IMPPayloadType *payLoadType);

/**
 * @fn int IMP_Encoder_SetJpegeQl(int encChn, const IMPEncoderJpegeQl *pstJpegeQl)
 *
 * Set JPEG encode channel quantization table set param
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[in] pstJpegeQl JPEG encode channel quantization table set param,Fill in the  128 bytes of the quantized table
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by JPEG;
 */
int IMP_Encoder_SetJpegeQl(int encChn, const IMPEncoderJpegeQl *pstJpegeQl);

/**
 * @fn int IMP_Encoder_GetJpegeQl(int encChn, IMPEncoderJpegeQl *pstJpegeQl)
 *
 * Get JPEG encode channel quantization table set param
 *
 * @param[in] encChn encode channel num, range:[0, @ref NR_MAX_ENC_CHN - 1]
 * @param[out] pstJpegeQl returned JPEG encode channel user's quantization table set param
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remark the channel must have been created before calling this function.
 * @remark this api only used by JPEG;
 */
int IMP_Encoder_GetJpegeQl(int encChn, IMPEncoderJpegeQl *pstJpegeQl);

/**
 * @brief IMP_Encoder_SetPool(int chnNum, int poolID);
 *
 * bind channel to mempool, let chnNum malloc from pool.
 *
 * @param[in] chnNum		Channnel ID.
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
 * @brief int IMP_Encoder_InputJpege(uint8_t *src, uint8_t *dst, int src_w, int src_h, int q,int *stream_length);
 *
 * IVPU external input NV12 encoded JPEG
 *
 * @param[in] *src Source data address pointer
 * @param[in] *dst Bitstream data address pointer
 * @param[in] src_w Image width
 * @param[in] src_h Image height
 * @param[in] q Image quality control<Not supported at this time>
 * @param[out] stream_length Bitstream data length
 *
 * @retval 0 success
 * @retval not 0 failure
 *
 * @remarks This API only works with NV12 input encoding JPEGs that are 32 wide aligned and 8 high aligned
 *
 */
int IMP_Encoder_InputJpege(uint8_t *src, uint8_t *dst, int src_w, int src_h, int q,int *stream_length);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_ENCODER_H__ */
