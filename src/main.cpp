#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "RTSP.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "WS.hpp"
#include "version.hpp"
#include "worker.hpp"
#include "globals.hpp"
#include "IMPSystem.hpp"
#include "Motion.hpp"
using namespace std::chrono;

std::mutex mutex_main;
std::condition_variable global_cv_worker_restart;

bool startup = true;

bool global_restart_rtsp = false;
bool global_restart_video = false;
bool global_restart_audio = false;

bool global_osd_thread_signal = false;
bool global_main_thread_signal = false;
bool global_motion_thread_signal = false;
std::atomic<char> global_rtsp_thread_signal{1};

std::shared_ptr<jpeg_stream> global_jpeg[NUM_VIDEO_CHANNELS] = {nullptr};
std::shared_ptr<video_stream> global_video[NUM_VIDEO_CHANNELS] = {nullptr};
#if defined(AUDIO_SUPPORT)
std::shared_ptr<audio_stream> global_audio[NUM_AUDIO_CHANNELS] = {nullptr};
#endif

std::shared_ptr<CFG> cfg = std::make_shared<CFG>();

WS ws;
RTSP rtsp;
Motion motion;
IMPSystem *imp_system = nullptr;

bool timesync_wait()
{
    // I don't really have a better way to do this than
    // a no-earlier-than time. The most common sync failure
    // is time() == 0
    int timeout = 0;
    while (time(NULL) < 1647489843)
    {
        std::this_thread::sleep_for(seconds(1));
        ++timeout;
        if (timeout == 60)
            return false;
    }
    return true;
}

void start_video(int encChn)
{
    StartHelper sh{encChn};
    int ret = pthread_create(&global_video[encChn]->thread, nullptr, Worker::stream_grabber, static_cast<void *>(&sh));
    LOG_DEBUG_OR_ERROR(ret, "create video["<< encChn << "] thread");

    // wait for initialization done
    sh.has_started.acquire();
}

int main(int argc, const char *argv[])
{
    LOG_INFO("PRUDYNT-T Next-Gen Video Daemon: " << VERSION);

    pthread_t cf_thread;      // monitor config changes
    pthread_t ws_thread;
    pthread_t osd_thread;
    pthread_t rtsp_thread;
    pthread_t motion_thread;

    if (Logger::init(cfg->general.loglevel))
    {
        LOG_ERROR("Logger initialization failed.");
        return 1;
    }
    LOG_INFO("Starting Prudynt Video Server.");

    if (!timesync_wait())
    {
        LOG_ERROR("Time is not synchronized.");
        return 1;
    }

    if (!imp_system)
    {
        imp_system = IMPSystem::createNew();
    }

    global_video[0] = std::make_shared<video_stream>(0, &cfg->stream0, "stream0");
    global_video[1] = std::make_shared<video_stream>(1, &cfg->stream1, "stream1");
    global_jpeg[0] = std::make_shared<jpeg_stream>(2, &cfg->stream2);
    //global_jpeg[1] = std::make_shared<jpeg_stream>(jpeg_stream{3, &cfg->stream3});

#if defined(AUDIO_SUPPORT)
    global_audio[0] = std::make_shared<audio_stream>(1, 0, 0);
#endif

    pthread_create(&cf_thread, nullptr, Worker::watch_config_poll, nullptr);
    pthread_create(&ws_thread, nullptr, WS::run, &ws);

    while (true)
    {
        if (global_restart_video || startup)
        {
            if (cfg->stream0.enabled)
            {
                start_video(0);
            }

            if (cfg->stream1.enabled)
            {
                start_video(1);
            }

            if (cfg->stream2.enabled)
            {
                StartHelper sh{2};
                int ret = pthread_create(&global_jpeg[0]->thread, nullptr, Worker::jpeg_grabber, static_cast<void *>(&sh));
                LOG_DEBUG_OR_ERROR(ret, "create jpeg thread");
                // wait for initialization done
                sh.has_started.acquire();
            }

            if (cfg->stream0.osd.enabled || cfg->stream1.osd.enabled)
            {
                int ret = pthread_create(&osd_thread, nullptr, Worker::update_osd, NULL);
                LOG_DEBUG_OR_ERROR(ret, "create osd thread");
            }

            if (cfg->motion.enabled)
            {
                int ret = pthread_create(&motion_thread, nullptr, Motion::run, &motion);
                LOG_DEBUG_OR_ERROR(ret, "create motion thread");
            }            
        }

#if defined(AUDIO_SUPPORT)
        if (cfg->audio.input_enabled && (global_restart_audio || startup))
        {
            StartHelper sh{0};
            int ret = pthread_create(&global_audio[0]->thread, nullptr, Worker::audio_grabber, static_cast<void *>(&sh));
            LOG_DEBUG_OR_ERROR(ret, "create audio thread");
            // wait for initialization done
            sh.has_started.acquire();
        }
#endif

        // start rtsp server
        if (global_rtsp_thread_signal != 0 && (global_restart_rtsp || startup))
        {
            int ret = pthread_create(&rtsp_thread, nullptr, RTSP::run, &rtsp);
            LOG_DEBUG_OR_ERROR(ret, "create rtsp thread");
        }

        /* we should wait a short period to ensure all services are up 
         * and running, additionally we add the timespan which is configured as 
         * OSD startup delay.
         */
        usleep(250000 + (cfg->stream0.osd.start_delay * 1000) + cfg->stream1.osd.start_delay * 1000);
        
        LOG_DEBUG("main thread is going to sleep");
        std::unique_lock lck(mutex_main);
        
        startup = false;
        global_restart_video = false;
        global_restart_audio = false;
        global_restart_rtsp = false;        
        
        while (!global_restart_rtsp && !global_restart_video && !global_restart_audio)
            global_cv_worker_restart.wait(lck);
        lck.unlock();
        LOG_DEBUG("wakup main thread");

        if (global_restart_rtsp)
        {
            // stop rtsp thread
            if (global_rtsp_thread_signal == 0)
            {
                global_rtsp_thread_signal = 1;
                int ret = pthread_join(rtsp_thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join rtsp thread");
            }
        }

        if (global_restart_video)
        {
            // stop motion thread
            if (global_motion_thread_signal)
            {
                global_motion_thread_signal = false;
                int ret = pthread_join(motion_thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join motion thread");
            }

            // stop osd thread
            if (global_osd_thread_signal)
            {
                global_osd_thread_signal = false;
                int ret = pthread_join(osd_thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join osd thread");
            }

            // stop jpeg
            if (global_jpeg[0]->imp_encoder)
            {
                global_jpeg[0]->running = false;
                global_jpeg[0]->should_grab_frames.notify_one();
                int ret = pthread_join(global_jpeg[0]->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join jpeg thread");
            }

            // stop stream1
            if (global_video[1]->imp_encoder)
            {
                global_video[1]->running = false;
                global_video[1]->should_grab_frames.notify_one();
                int ret = pthread_join(global_video[1]->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join stream1 thread");
            }

            // stop stream0
            if (global_video[0]->imp_encoder)
            {
                global_video[0]->running = false;
                global_video[0]->should_grab_frames.notify_one();
                int ret = pthread_join(global_video[0]->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join stream0 thread");
            }
        }

        // stop audio
        if (global_audio[0]->imp_audio && global_restart_audio)
        {
            global_audio[0]->running = false;
            global_audio[0]->should_grab_frames.notify_one();
            int ret = pthread_join(global_audio[0]->thread, NULL);
            LOG_DEBUG_OR_ERROR(ret, "join audio thread");
        }
    }

    return 0;
}
