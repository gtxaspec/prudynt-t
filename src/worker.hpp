#ifndef Worker_hpp
#define Worker_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include "imp/imp_audio.h"
#include "imp/imp_system.h"
#include "imp/imp_encoder.h"
#include "globals.hpp"
#include <sys/file.h>
#include <sys/inotify.h>
#include <sys/stat.h>

struct StartHelper
{
	int encChn;
	std::binary_semaphore has_started{0};
};

class Worker
{
public:
	static void flush(int encChn)
	{
		IMP_Encoder_FlushStream(encChn);
		IMP_Encoder_RequestIDR(encChn);
	};

	void run();

	static void *jpeg_grabber(void *arg);
	static void *audio_grabber(void *arg);
	static void *stream_grabber(void *arg);
	static void *update_osd(void *arg);
	static void *watch_config_notify(void *arg);
	static void *watch_config_poll(void *arg);
	static std::vector<uint8_t> capture_jpeg_image(int encChn);
	static void *backchannel_processor(void *arg);
};

#endif
