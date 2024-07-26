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

using namespace std::chrono;

std::mutex mutex_main;
std::condition_variable global_cv_lock_main;

bool global_restart_rtsp = false;
bool global_restart_video = false;

bool global_osd_thread_signal = false;
bool global_main_thread_signal = false;
char volatile global_rtsp_thread_signal{1};

std::shared_ptr<jpeg_stream> global_jpeg = {nullptr};
std::shared_ptr<audio_stream> audio[NUM_AUDIO_CHANNELS] = {nullptr};
std::shared_ptr<video_stream> video[NUM_VIDEO_CHANNELS] = {nullptr};

auto main_thread_signal = std::make_shared<std::atomic<int>>(0);
std::shared_ptr<CFG> cfg = std::make_shared<CFG>();

WS ws(cfg, main_thread_signal);
RTSP rtsp;
//Worker worker(cfg);
IMPSystem *impsystem = nullptr;

void stop_encoder()
{

    LOG_DEBUG("Stop worker." << cfg->worker_thread_signal);

    milliseconds duration;
    auto t0 = high_resolution_clock::now();

    cfg->worker_thread_signal.fetch_xor(3);
    cfg->worker_thread_signal.fetch_or(4);
    cfg->worker_thread_signal.notify_all();

    while ((cfg->worker_thread_signal.load() & 8) != 8)
    {
        usleep(100);
        duration = duration_cast<milliseconds>(high_resolution_clock::now() - t0);
        if (duration.count() > 1000 * 1000 * 10)
        {
            LOG_ERROR("Unable to stop encoder, no response.");
            return;
        }
    }
    LOG_DEBUG("Worker stopped in " << duration.count() << "ms");
}

void start_encoder()
{

    LOG_DEBUG("Start worker.");

    milliseconds duration;
    auto t0 = high_resolution_clock::now();

    cfg->worker_thread_signal.fetch_xor(8);
    cfg->worker_thread_signal.fetch_or(1);

    while ((cfg->worker_thread_signal.load() & 2) != 2)
    {

        usleep(100);
        duration = duration_cast<milliseconds>(high_resolution_clock::now() - t0);
        if (duration.count() > 1000 * 1000 * 10)
        {
            LOG_ERROR("Unable to start worker, no response.");
            return;
        }
    }
    LOG_DEBUG("Worker started in " << duration.count() << "ms");
}

void stop_rtsp()
{

    LOG_DEBUG("Stop RTSP Server.");

    milliseconds duration;
    auto t0 = high_resolution_clock::now();

    cfg->rtsp_thread_signal = 1;

    while (cfg->rtsp_thread_signal != 2)
    {

        usleep(100);
        duration = duration_cast<milliseconds>(high_resolution_clock::now() - t0);
        if (duration.count() > 1000 * 1000 * 10)
        {
            LOG_ERROR("Unable to stop RTSP Server, no response.");
            return;
        }
    }
    LOG_DEBUG("RTSP Server stopped in " << duration.count() << "ms");
}

void start_rtsp()
{

    LOG_DEBUG("Start RTSP Server.");

    cfg->rtsp_thread_signal = 0;
}

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
    int *encChnPtr = &encChn;
    pthread_create(&video[encChn]->thread, nullptr, Worker::stream_grabber, static_cast<void *>(encChnPtr));

    // wait for initialization done
    std::unique_lock lck(mutex_main);
    global_cv_lock_main.wait(lck);
}

int main(int argc, const char *argv[])
{
    LOG_INFO("PRUDYNT Video Daemon: " << VERSION);

    pthread_t osd_thread;
    pthread_t rtsp_thread;

    std::thread ws_thread;
    //std::thread rtsp_thread;
    //std::thread worker_thread;

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

    if (!impsystem)
    {
        impsystem = IMPSystem::createNew(cfg);
    }

    video[0] = std::make_shared<video_stream>(video_stream{0, &cfg->stream0, "stream0"});
    video[1] = std::make_shared<video_stream>(video_stream{1, &cfg->stream1, "stream1"});
    audio[0] = std::make_shared<audio_stream>(audio_stream{0, 0});
    global_jpeg = std::make_shared<jpeg_stream>(jpeg_stream{2, &cfg->stream2});

    ws_thread = std::thread(&WS::run, ws);
    // rtsp_thread = std::thread(&RTSP::start, rtsp);
    // worker_thread = std::thread(&Worker::run, &worker);

    while (true)
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
            int *encChnPtr = new int(2);
            pthread_create(&global_jpeg->thread, nullptr, Worker::jpeg_grabber, static_cast<void *>(encChnPtr));

            // wait for initialization done
            std::unique_lock lck(mutex_main);
            global_cv_lock_main.wait(lck);
        }

        if (cfg->stream0.osd.enabled || cfg->stream1.osd.enabled)
        {
            pthread_create(&osd_thread, nullptr, Worker::update_osd, NULL);
        }

        //start rtsp server
        if(global_rtsp_thread_signal != 0) {
            pthread_create(&rtsp_thread, nullptr, RTSP::run, &rtsp);

            //wait for initialization done
            //std::unique_lock lck(mutex_main);
            //global_cv_lock_main.wait(lck);       
        }
        
        LOG_DEBUG("main thread is going to sleep");
        std::unique_lock lck(mutex_main);
        global_cv_lock_main.wait(lck);
        LOG_DEBUG("wakup main thread");

        if (global_restart_rtsp)
        {
            int ret;
            global_restart_rtsp = false;
            
            // stop rtsp thread
            if (global_rtsp_thread_signal == 0)
            {
                global_rtsp_thread_signal = 1;
                ret = pthread_join(rtsp_thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join rtsp thread");
            }           
        }

        if (global_restart_video)
        {
            int ret;
            global_restart_video = false;

            // stop osd thread
            if (global_osd_thread_signal)
            {
                global_osd_thread_signal = false;
                ret = pthread_join(osd_thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join osd thread");
            }

            // stop jpeg
            if (global_jpeg->imp_encoder)
            {
                global_jpeg->running = false;
                ret = pthread_join(global_jpeg->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join jpeg thread");
            }

            // stop stream1
            if (video[1]->imp_encoder)
            {
                video[1]->running = false;
                ret = pthread_join(video[1]->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join stream1 thread");
            }

            // stop stream0
            if (video[0]->imp_encoder)
            {
                video[0]->running = false;
                ret = pthread_join(video[0]->thread, NULL);
                LOG_DEBUG_OR_ERROR(ret, "join stream0 thread");
            }
        }
    }
    /*
    while (true)
    {

        LOG_DEBUG("main thread is going to sleep.");

        cfg->main_thread_signal.wait(1);

        LOG_DEBUG("main thread wakeup");

        if (cfg->main_thread_signal & 8)
        { // 8 = stop action
            cfg->main_thread_signal.fetch_xor(8);
            if (cfg->main_thread_signal & 1)
            { // 1 = rtsp thread
                stop_rtsp();
            }
            if (cfg->main_thread_signal & 2)
            { // 2 = encoder thread
                stop_encoder();
            }
        }

        if (cfg->main_thread_signal & 16)
        { // 16 = start action
            cfg->main_thread_signal.fetch_xor(16);
            if (cfg->main_thread_signal & 1)
            { // 1 = rtsp thread
                start_rtsp();
            }
            if (cfg->main_thread_signal & 2)
            { // 2 = encoder thread
                start_encoder();
            }
        }

        cfg->main_thread_signal.store(1);
    }
    */

    return 0;
}
