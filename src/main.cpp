#include <chrono>
#include <thread>
#include <atomic>
#include "RTSP.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "WS.hpp"
#include "version.hpp"
#include "worker.hpp"

using namespace std::chrono;

auto main_thread_signal = std::make_shared<std::atomic<int>>(0);
std::shared_ptr<CFG> cfg = std::make_shared<CFG>();

WS ws(cfg, main_thread_signal);
RTSP rtsp(cfg);
Worker worker(cfg);

void stop_encoder()
{

    LOG_DEBUG("Stop worker." << cfg->worker_thread_signal);

    milliseconds duration;
    auto t0 = high_resolution_clock::now();

    cfg->worker_thread_signal.fetch_xor(3);
    cfg->worker_thread_signal.fetch_or(4);
    //cfg->worker_thread_signal.notify_one();

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

int main(int argc, const char *argv[])
{

    LOG_INFO("PRUDYNT Video Daemon: " << VERSION);

    std::thread ws_thread;
    std::thread rtsp_thread;
    std::thread worker_thread;

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

    ws_thread = std::thread(&WS::run, ws);
    rtsp_thread = std::thread(&RTSP::run, rtsp);
    worker_thread = std::thread(&Worker::run, &worker);

    while (1)
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

    return 0;
}
