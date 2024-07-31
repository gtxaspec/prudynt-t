#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <memory>
#include <functional>
#include "MsgChannel.hpp"
#include "IMPAudio.hpp"
#include "IMPEncoder.hpp"
#include "IMPFramesource.hpp"

#define MSG_CHANNEL_SIZE 20
#define NUM_AUDIO_CHANNELS 1
#define NUM_VIDEO_CHANNELS 2

/*
    ToDo's
    simplify msgChannel only vector required
*/

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

struct jpeg_stream {
    int encChn;
    _stream* stream;
    bool running;
    pthread_t thread;
    IMPEncoder * imp_encoder;
    jpeg_stream(int encChn, _stream* stream)
        : encChn(encChn), stream(stream), running(false) {}  
};

struct audio_stream {
    int devId;
    int aiChn;
    bool running;
    pthread_t thread;
    IMPAudio *imp_audio;
    std::shared_ptr<MsgChannel<AudioFrame>> msgChannel;
    //std::shared_ptr<MsgChannel<std::vector<uint8_t>>> msgChannel;
    std::function<void(void)> onDataCallback;
    std::mutex lock;    // protects mutable fields: imp_framesource, onDataCallback, etc.
    std::condition_variable should_grab_frames;

    audio_stream(int devId, int aiChn)
        : devId(devId), aiChn(aiChn), running(false), 
          msgChannel(std::make_shared<MsgChannel<AudioFrame>>(30)), onDataCallback(nullptr) {}

    /* Check whether onDataCallback is not null in a data race free manner.
     * Returns a momentary value that may be stale by the time it is returned.
     * Use only for optimizations, i.e., to skip work if no data callback
     * is registered right now.
     */
    bool hasDataCallback() {
        std::unique_lock<std::mutex> lock_stream {lock};
        return onDataCallback != nullptr;
    }
};

struct video_stream {
    int encChn;
    _stream* stream;
    const char *name;
    bool running;
    pthread_t thread;
    bool idr;
    int idr_fix;
    IMPEncoder *imp_encoder;
    IMPFramesource *imp_framesource;
    std::shared_ptr<MsgChannel<H264NALUnit>> msgChannel;
    //std::shared_ptr<MsgChannel<std::vector<uint8_t>>> msgChannel;
    std::function<void(void)> onDataCallback;
    std::mutex lock;    // protects mutable fields: imp_framesource, onDataCallback, etc.
    std::condition_variable should_grab_frames;

    video_stream(int encChn, _stream* stream, const char *name)
        : encChn(encChn), stream(stream), running(false), name(name), idr(false), imp_encoder(nullptr), imp_framesource(nullptr),
          msgChannel(std::make_shared<MsgChannel<H264NALUnit>>(MSG_CHANNEL_SIZE)), onDataCallback(nullptr), idr_fix(0) {}
    bool hasDataCallback() {
        std::unique_lock<std::mutex> lock_stream {lock};
        return onDataCallback != nullptr;
    }
};

extern std::condition_variable global_cv_worker_restart;

extern std::mutex mutex_main;   // protects global_restart_rtsp and global_restart_video
extern bool global_restart_rtsp;
extern bool global_restart_video;
extern bool global_restart_audio;

extern bool global_osd_thread_signal; 
extern bool global_main_thread_signal; 
extern char volatile global_rtsp_thread_signal; 

extern std::shared_ptr<jpeg_stream> global_jpeg;
extern std::shared_ptr<audio_stream> global_audio[NUM_AUDIO_CHANNELS];
extern std::shared_ptr<video_stream> global_video[NUM_VIDEO_CHANNELS];

#endif // GLOBALS_HPP
