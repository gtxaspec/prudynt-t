#include <iostream>
#include <cstring>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include <imp/imp_osd.h>
#include <ctime>

#define MODULE "ENCODER"

#include "IMP.hpp"
#include "Encoder.hpp"
#include "Config.hpp"

std::mutex Encoder::sinks_lock;
std::map<uint32_t,EncoderSink> Encoder::sinks;
uint32_t Encoder::sink_id = 0;

Encoder::Encoder() {}

bool Encoder::init() {
    int ret = 0;
    ret = IMP_Encoder_CreateGroup(0);
    if (ret < 0) {
        LOG_ERROR("IMP_Encoder_CreateGroup() == " << ret);
        return true;
    }

    ret = system_init();
    if (ret < 0) {
        LOG_ERROR("system_init failed ");
    }

    ret = encoder_init();
    if (ret < 0) {
        LOG_ERROR("Encoder Init Failed");
        return true;
    }

    if (Config::singleton()->OSDEnabled == 0) {
        // If OSD is not enabled, initialize without OSD and bind FrameSource directly to Encoder
        ret = osd.init();
        if (ret) {
            LOG_ERROR("OSD Init Failed");
            return true;
        }

        IMPCell fs = { DEV_ID_FS, 0, 0 };
        IMPCell enc = { DEV_ID_ENC, 0, 0 };
        // Framesource -> ENC
        ret = IMP_System_Bind(&fs, &enc);
        if (ret < 0) {
            LOG_ERROR("IMP_System_Bind(FS, ENC) == " << ret);
            return true;
        }

    } else {
        // If OSD is enabled, initialize OSD and bind FrameSource to OSD, then OSD to Encoder
        ret = osd.init();
        if (ret) {
            LOG_ERROR("OSD Init Failed");
            return true;
        }

        IMPCell fs = { DEV_ID_FS, 0, 0 };
        IMPCell osd_cell = { DEV_ID_OSD, 0, 0 };
        IMPCell enc = { DEV_ID_ENC, 0, 0 };
        // Framesource -> OSD
        ret = IMP_System_Bind(&fs, &osd_cell);
        if (ret < 0) {
            LOG_ERROR("IMP_System_Bind(FS, OSD) == " << ret);
            return true;
        }
        // OSD -> Encoder
        ret = IMP_System_Bind(&osd_cell, &enc);
        if (ret < 0) {
            LOG_ERROR("IMP_System_Bind(OSD, ENC) == " << ret);
            return true;
        }

    }

    ret = IMP_FrameSource_EnableChn(0);
    if (ret < 0) {
        LOG_ERROR("IMP_FrameSource_EnableChn() == " << ret);
        return true;
    }

    return false;
}

int Encoder::system_init() {
    int ret = 0;

    IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_60HZ);

    return ret;
}

int Encoder::encoder_init() {
    int ret = 0;

    IMPEncoderRcAttr *rc_attr;
    IMPEncoderChnAttr channel_attr;
    memset(&channel_attr, 0, sizeof(IMPEncoderChnAttr));
    rc_attr = &channel_attr.rcAttr;
    IMPEncoderProfile encoderProfile;

    // Allow user to specify the profile directly in the future with fallback defaults
    if (Config::singleton()->stream0format == "H265") {
        encoderProfile = IMP_ENC_PROFILE_HEVC_MAIN;
    } else {
        encoderProfile = IMP_ENC_PROFILE_AVC_HIGH;
    }

    IMP_Encoder_SetDefaultParam(
        &channel_attr, encoderProfile, IMP_ENC_RC_MODE_CAPPED_QUALITY, Config::singleton()->stream0width, Config::singleton()->stream0height,
        Config::singleton()->stream0fps, 1, Config::singleton()->stream0gop, 2,
        -1, Config::singleton()->stream0bitrate
    );

    switch (rc_attr->attrRcMode.rcMode) {
        case IMP_ENC_RC_MODE_CAPPED_QUALITY:
        rc_attr->attrRcMode.attrVbr.uTargetBitRate = Config::singleton()->stream0bitrate;
        rc_attr->attrRcMode.attrVbr.uMaxBitRate = Config::singleton()->stream0bitrate * 1.333;
        rc_attr->attrRcMode.attrVbr.iInitialQP = -1;
        rc_attr->attrRcMode.attrVbr.iMinQP = 20;
        rc_attr->attrRcMode.attrVbr.iMaxQP = 45;
        rc_attr->attrRcMode.attrVbr.iIPDelta = 3;
        rc_attr->attrRcMode.attrVbr.iPBDelta = 3;
        //rc_attr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
        rc_attr->attrRcMode.attrVbr.uMaxPictureSize = Config::singleton()->stream0width;
        rc_attr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
        break;
    }

    ret = IMP_Encoder_CreateChn(0, &channel_attr);
    if (ret < 0) {
        LOG_ERROR("IMP_Encoder_CreateChn() == " << ret);
        return ret;
    }

    ret = IMP_Encoder_RegisterChn(0, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_Encoder_RegisterChn() == " << ret);
        return ret;
    }

    return ret;
}

void Encoder::run() {
    LOG_INFO("Encoder Start.");

    //The encoder thread is very important, but we
    //want sink threads to have higher priority.
    nice(-19);

    int64_t last_nal_ts = 0;

    //The encoder tracks NAL timestamps with an int64_t.
    //INT64_MAX = 9,223,372,036,854,775,807
    //That means the encoder won't overflow its timestamp unless
    //this program is left running for more than 106,751,991 days,
    //or nearly 300,000 years. I think it's okay if we don't
    //handle timestamp overflows. :)
    IMP_System_RebaseTimeStamp(0);
    gettimeofday(&imp_time_base, NULL);
    IMP_Encoder_StartRecvPic(0);
    while (true) {
        IMPEncoderStream stream;

        if (IMP_Encoder_GetStream(0, &stream, true) != 0) {
            LOG_ERROR("IMP_Encoder_GetStream() failed");
            break;
        }

        //The I/P NAL is always last, but it doesn't
        //really matter which NAL we select here as they
        //all have identical timestamps.
        int64_t nal_ts = stream.pack[stream.packCount - 1].timestamp;
        if (nal_ts - last_nal_ts > 1.5*(1000000/IMP::FRAME_RATE)) {
            // Silence for now until further tests / THINGINO
            //LOG_WARN("The encoder dropped a frame.");
        }
        struct timeval encode_time;
        encode_time.tv_sec  = nal_ts / 1000000;
        encode_time.tv_usec = nal_ts % 1000000;

        for (unsigned int i = 0; i < stream.packCount; ++i) {
            uint8_t* start = (uint8_t*)stream.virAddr + stream.pack[i].offset;
            uint8_t* end = start + stream.pack[i].length;

            H264NALUnit nalu;
            nalu.imp_ts = stream.pack[i].timestamp;
            timeradd(&imp_time_base, &encode_time, &nalu.time);
            nalu.duration = 0;
            if (stream.pack[i].nalType.h264NalType == 5 || stream.pack[i].nalType.h264NalType == 1) {
                nalu.duration = last_nal_ts - nal_ts;
            } else if (stream.pack[i].nalType.h265NalType == 19 ||
                    stream.pack[i].nalType.h265NalType == 20 ||
                    stream.pack[i].nalType.h265NalType == 1) {
                nalu.duration = last_nal_ts - nal_ts;
            }
            //We use start+4 because the encoder inserts 4-byte MPEG
            //'startcodes' at the beginning of each NAL. Live555 complains
            //if those are present.
            nalu.data.insert(nalu.data.end(), start+4, end);

            std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
            for (std::map<uint32_t,EncoderSink>::iterator it=Encoder::sinks.begin();
                 it != Encoder::sinks.end(); ++it) {
                if (stream.pack[i].nalType.h264NalType == 7 ||
                    stream.pack[i].nalType.h264NalType == 8 ||
                    stream.pack[i].nalType.h264NalType == 5) {
                    it->second.IDR = true;
                } else if (stream.pack[i].nalType.h265NalType == 32) {
                    it->second.IDR = true;
                }
                if (it->second.IDR) {
                    if (!it->second.chn->write(nalu)) {
                        //Discard old NALUs if our sinks aren't keeping up.
                        //This prevents the MsgChannels from clogging up with
                        //old data.
                        LOG_WARN("Sink " << it->second.name << " clogged! Discarding NAL.");
                        H264NALUnit old_nal;
                        it->second.chn->read(&old_nal);
                    }
                }
            }
        }
        if (Config::singleton()->OSDEnabled == 1) {
            osd.update();
        }
        IMP_Encoder_ReleaseStream(0, &stream);
        last_nal_ts = nal_ts;
        std::this_thread::yield();
    }
    IMP_Encoder_StopRecvPic(0);
}
