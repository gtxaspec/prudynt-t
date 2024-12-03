#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <memory>
#include <functional>
#include <atomic>
#include "MsgChannel.hpp"
#include "IMPAudio.hpp"
#include "IMPEncoder.hpp"
#include "IMPFramesource.hpp"

#define MSG_CHANNEL_SIZE 20
#define NUM_AUDIO_CHANNELS 1
#define NUM_VIDEO_CHANNELS 2

using namespace std::chrono;

extern std::mutex mutex_main; // protects global_restart_rtsp and global_restart_video

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

struct jpeg_stream
{
    int encChn;
    _stream *stream;
    std::atomic<bool> running; // set to false to make jpeg_grabber thread exit
    std::atomic<bool> active{false};
    pthread_t thread;
    IMPEncoder *imp_encoder;
    std::condition_variable should_grab_frames;
    std::binary_semaphore is_activated{0};

    steady_clock::time_point last_image;
    steady_clock::time_point last_subscriber;

    void request()
    {
        auto now = steady_clock::now();
        std::unique_lock lck(mutex_main);
        last_subscriber = now;
    }

    bool request_or_overrun() {
        return duration_cast<milliseconds>(steady_clock::now() - last_subscriber).count() < 1000;
    }

    jpeg_stream(int encChn, _stream *stream)
        : encChn(encChn), stream(stream), running(false), imp_encoder(nullptr) {}
};

struct audio_stream
{
    int devId;
    int aiChn;
    int aeChn;
    bool running;
    bool active{false};
    pthread_t thread;
    IMPAudio *imp_audio;
    std::shared_ptr<MsgChannel<AudioFrame>> msgChannel;
    std::function<void(void)> onDataCallback;
    /* Check whether onDataCallback is not null in a data race free manner.
     * Returns a momentary value that may be stale by the time it is returned.
     * Use only for optimizations, i.e., to skip work if no data callback
     * is registered right now.
     */
    std::atomic<bool> hasDataCallback;
    std::mutex onDataCallbackLock; // protects onDataCallback from deallocation
    std::condition_variable should_grab_frames;
    std::binary_semaphore is_activated{0};

    audio_stream(int devId, int aiChn, int aeChn)
        : devId(devId), aiChn(aiChn), aeChn(aeChn), running(false), imp_audio(nullptr),
          msgChannel(std::make_shared<MsgChannel<AudioFrame>>(30)),
          onDataCallback{nullptr}, hasDataCallback{false} {}
};

struct video_stream
{
    int encChn;
    _stream *stream;
    const char *name;
    bool running;
    pthread_t thread;
    bool idr;
    int idr_fix;
    bool active{false};
    IMPEncoder *imp_encoder;
    IMPFramesource *imp_framesource;
    std::shared_ptr<MsgChannel<H264NALUnit>> msgChannel;
    std::function<void(void)> onDataCallback;
    bool run_for_jpeg;                 // see comment in audio_stream
    std::atomic<bool> hasDataCallback; // see comment in audio_stream
    std::mutex onDataCallbackLock;     // protects onDataCallback from deallocation
    std::condition_variable should_grab_frames;
    std::binary_semaphore is_activated{0};

    video_stream(int encChn, _stream *stream, const char *name)
        : encChn(encChn), stream(stream), name(name), running(false), idr(false), idr_fix(0), imp_encoder(nullptr), imp_framesource(nullptr),
          msgChannel(std::make_shared<MsgChannel<H264NALUnit>>(MSG_CHANNEL_SIZE)), onDataCallback(nullptr),  run_for_jpeg{false},
          hasDataCallback{false} {}
};

extern std::condition_variable global_cv_worker_restart;

extern bool global_restart_rtsp;
extern bool global_restart_video;
extern bool global_restart_audio;

extern bool global_osd_thread_signal;
extern bool global_main_thread_signal;
extern bool global_motion_thread_signal;
extern std::atomic<char> global_rtsp_thread_signal;

extern std::shared_ptr<jpeg_stream> global_jpeg[NUM_VIDEO_CHANNELS];
extern std::shared_ptr<audio_stream> global_audio[NUM_AUDIO_CHANNELS];
extern std::shared_ptr<video_stream> global_video[NUM_VIDEO_CHANNELS];

#endif // GLOBALS_HPP
