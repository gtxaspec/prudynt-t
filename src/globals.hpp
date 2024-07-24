#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <memory>
#include <functional>
#include "MsgChannel.hpp"

constexpr size_t NUM_AUDIO_CHANNELS = 1;
constexpr size_t NUM_VIDEO_CHANNELS = 2;

struct AudioFrame
{
	std::vector<uint8_t> data;
	struct timeval time;
};

struct H264NALUnit
{
	std::vector<uint8_t> data;
	struct timeval time;
	int64_t imp_ts;
};

struct audio_stream {
    int devId;
    int encChn;
    bool thread_loop;
    std::shared_ptr<MsgChannel<AudioFrame>> msgChannel;
    std::function<void(void)> onDataCallback;

    audio_stream(int devId, int encChn, bool thread_loop)
        : devId(devId), encChn(encChn), thread_loop(thread_loop), 
          msgChannel(std::make_shared<MsgChannel<AudioFrame>>(20)), onDataCallback(nullptr) {}
};

struct video_stream {
    int encChn;
    _stream* stream;
    bool thread_loop;
    bool IDR;
    std::shared_ptr<MsgChannel<H264NALUnit>> msgChannel;
    std::function<void(void)> onDataCallback;

    video_stream(int encChn, _stream* stream, bool thread_loop)
        : encChn(encChn), stream(stream), thread_loop(true), IDR(false),
          msgChannel(std::make_shared<MsgChannel<H264NALUnit>>(20)), onDataCallback(nullptr) {}
};

extern int polling_timeout; 
extern std::shared_ptr<audio_stream> audio[NUM_AUDIO_CHANNELS];
extern std::shared_ptr<video_stream> video[NUM_VIDEO_CHANNELS];

#endif // GLOBALS_HPP