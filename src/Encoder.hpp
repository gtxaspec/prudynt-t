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
#include <imp/imp_audio.h>

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
	std::string name;
	int encChn;
	bool IDR;
	std::function<int(const H264NALUnit&)> data_available_callback;
};

struct CHN {
	bool enabled;
	int encChn = -1;
	int64_t lastNalTs = 0;
	struct timeval impTimeBase;
	std::function<void()> updateOsd = nullptr;
};

class Encoder {
	public:
		Encoder(std::shared_ptr<CFG> cfg) : cfg(cfg) {};

		void run();
		
		static void flush(int encChn) {
			IMP_Encoder_RequestIDR(encChn);
			IMP_Encoder_FlushStream(encChn);
		}

		template <class T>
		static uint32_t connect_sink(T* c, std::string name, int encChn) {
			LOG_DEBUG("Create Sink: " << Encoder::sink_id << ", encChn:" << encChn);
			std::unique_lock<std::mutex> lck(Encoder::sinks_lock);
			Encoder::sinks[Encoder::sink_id] = EncoderSink{name, encChn, false, [c](const H264NALUnit& nalu) { return c->on_data_available(nalu); }};
			Encoder::flush(encChn);
			return Encoder::sink_id++;
		}

		static void remove_sink(uint32_t sink_id) {
			LOG_DEBUG("Remove Sink: " << sink_id);
			std::unique_lock<std::mutex> lck(sinks_lock);
			sinks.erase(sink_id);
		}

	private:
		std::shared_ptr<CFG> cfg;
		OSD *stream0_osd;
		OSD *stream1_osd;
		Motion motion;

		bool init();
		void exit();

		int system_init();
		int framesource_init();
		int encoder_init();
		int channel_init(int chn_nr, int grp_nr, IMPEncoderCHNAttr *chn_attr);
		int channel_deinit(int chn_nr);

		static std::mutex cv_mtx;
		static std::mutex sinks_lock;
		static uint32_t sink_id;

	public:
		static std::map<uint32_t, EncoderSink> sinks;

	private:
		struct timeval high_imp_time_base;
		struct timeval low_imp_time_base;

		IMPFSChnAttr create_fs_attr();
		IMPSensorInfo create_sensor_info(std::string sensor);
		
		IMPCell high_fs = { DEV_ID_FS, 0, 0 };
		IMPCell high_osd_cell = { DEV_ID_OSD, 0, 0 };
		IMPCell high_enc = { DEV_ID_ENC, 0, 0 }; 

		IMPCell low_fs = { DEV_ID_FS, 1, 0};
		IMPCell low_osd_cell = { DEV_ID_OSD, 1, 0 };
		IMPCell low_enc = { DEV_ID_ENC, 1, 0 };  	

		IMPSensorInfo sinfo;
		
		std::thread jpeg_thread;
		void jpeg_snap(std::shared_ptr<CFG>& cfg);

		bool osdStream0 = false;
		bool osdStream1 = false;
		bool motionInitialized{0};
		
		int stream0Status = 0;
		int stream1Status = 0;

		int errorCount = 0;
		CHN channels[2] = {{false, 0, 0},{false, 1, 0}};
};

#endif
