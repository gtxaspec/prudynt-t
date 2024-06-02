#ifndef Encoder_hpp
#define Encoder_hpp

#include <array>
#include <memory>
#include <ctime>
#include <map>

#include <sys/file.h>

#include <imp/imp_framesource.h>
#include <imp/imp_system.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include "MsgChannel.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "OSD.hpp"
#include "Motion.hpp"

#include <../sysutils/su_base.h>

#if defined(PLATFORM_T31)
	#define IMPEncoderCHNAttr IMPEncoderChnAttr
	#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

static const std::array<int, 64> jpeg_chroma_quantizer = {{
	17, 18, 24, 47, 99, 99, 99, 99,
	18, 21, 26, 66, 99, 99, 99, 99,
	24, 26, 56, 99, 99, 99, 99, 99,
	47, 66, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99
}};

static const std::array<int, 64> jpeg_luma_quantizer = {{
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77,
	24, 35, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103, 99
}};

struct H264NALUnit {
	std::vector<uint8_t> data;
	struct timeval time;
	int64_t imp_ts;
	int64_t duration;
};

struct EncoderSink {
	std::shared_ptr<MsgChannel<H264NALUnit>> chn;
	bool IDR;
	std::string name;
};

class Encoder {
	public:
		Encoder(std::shared_ptr<CFG> _cfg) : cfg(_cfg) {};

		void run();

		static void flush() {
			IMP_Encoder_RequestIDR(0);
			IMP_Encoder_FlushStream(0);
		}

		template <class T> static uint32_t connect_sink(T *c, std::string name = "Unnamed") {
			LOG_DEBUG("Create Sink: " << Encoder::sink_id);
			std::shared_ptr<MsgChannel<H264NALUnit>> chn = std::make_shared<MsgChannel<H264NALUnit>>(20);
			std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
			Encoder::sinks.insert(std::pair<uint32_t,EncoderSink>(Encoder::sink_id, {chn, false, name}));
			c->set_framesource(chn);
			Encoder::flush();
			return Encoder::sink_id++;
		}

		static void remove_sink(uint32_t sinkid) {
			LOG_DEBUG("Destroy Sink: " << sinkid);
			std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
			Encoder::sinks.erase(sinkid);
		}
		static const int FRAME_RATE;

	private:
		std::shared_ptr<CFG> cfg;
		OSD osd;
		Motion motion;

		bool init();
		void exit();

		int system_init();
		int framesource_init();
		int encoder_init();
		int channel_init(int chn_nr, int grp_nr, IMPEncoderCHNAttr *chn_attr);
		int channel_deinit(int chn_nr);

		static std::mutex sinks_lock;
		static uint32_t sink_id;
		static std::map<uint32_t, EncoderSink> sinks;
		struct timeval imp_time_base;
		IMPFSChnAttr create_fs_attr();
		IMPSensorInfo create_sensor_info(std::string sensor);
		
		IMPCell fs = { DEV_ID_FS, 0, 0 };
		IMPCell osd_cell = { DEV_ID_OSD, 0, 0 };
		IMPCell enc = { DEV_ID_ENC, 0, 0 };    
		IMPSensorInfo sinfo;
		
		std::thread jpeg_thread;
		void jpeg_snap(std::shared_ptr<CFG>& cfg);

		bool osdInitialized{0};
		bool motionInitialized{0};
};

#endif
