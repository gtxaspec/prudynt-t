#ifndef Worker_hpp
#define Worker_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include "IMPSystem.hpp"
#include "IMPEncoder.hpp"
#include "IMPFramesource.hpp"
#include "Motion.hpp"
#include <memory>
#include <thread>
#include <pthread.h>

struct H264NALUnit
{
	std::vector<uint8_t> data;
	struct timeval time;
	int64_t imp_ts;
	int64_t duration;
};

struct EncoderSink
{
	std::string name;
	int encChn;
	bool IDR;
	std::function<int(const H264NALUnit &)> data_available_callback;
};

struct Channel
{
	const int encChn;
	_stream *stream;
	EncoderSink *sink;
	pthread_mutex_t lock;
	std::function<void()> updateOsd = nullptr;
	std::atomic<bool> thread_signal;
};

class Worker
{
public:
	static Worker *createNew(std::shared_ptr<CFG> cfg);

	Worker(std::shared_ptr<CFG> cfg) : cfg(cfg)
	{
		// init();
	}

	~Worker()
	{
		deinit();
	};

	static void flush(int encChn)
	{
		IMP_Encoder_RequestIDR(encChn);
		IMP_Encoder_FlushStream(encChn);
	};

	template <class T>
	static void connect_sink(T *c, const char *name, int encChn)
	{
		LOG_DEBUG("Create Sink: " << encChn);

		if (encChn == 0)
		{
			pthread_mutex_lock(&sink_lock0);
			stream0_sink->data_available_callback =
				[c](const H264NALUnit &nalu)
			{ return c->on_data_available(nalu); };
			pthread_mutex_unlock(&sink_lock0);
		}
		else if (encChn == 1)
		{
			pthread_mutex_lock(&sink_lock1);
			stream1_sink->data_available_callback =
				[c](const H264NALUnit &nalu)
			{ return c->on_data_available(nalu); };
			pthread_mutex_unlock(&sink_lock1);
		}
		Worker::flush(encChn);
	};

	static void remove_sink(int encChn)
	{
		LOG_DEBUG("Remove Sink: " << encChn);

		if (encChn == 0)
		{
			pthread_mutex_lock(&sink_lock0);
			stream0_sink->data_available_callback = nullptr;
			pthread_mutex_unlock(&sink_lock0);
		}
		else if (encChn == 1)
		{
			pthread_mutex_lock(&sink_lock1);
			stream1_sink->data_available_callback = nullptr;
			pthread_mutex_unlock(&sink_lock1);
		}
	}

	int init();
	void run();
	int deinit();
	int destroy();

	static pthread_mutex_t sink_lock0;
	static pthread_mutex_t sink_lock1;

	static EncoderSink *stream0_sink;
	static EncoderSink *stream1_sink;

private:
	Motion motion;

	std::shared_ptr<CFG> cfg;
	static void *jpeg_grabber(void *arg);
	static void *stream_grabber(void *arg);

	Channel *channels[3] = {nullptr, nullptr, nullptr};
	IMPSystem *impsystem = nullptr;
	IMPEncoder *encoder[3] = {nullptr, nullptr, nullptr};
	IMPFramesource *framesources[2] = {nullptr, nullptr};

	pthread_t worker_threads[3];
};

#endif