#include <chrono>
#include <iostream>
#include <thread>
#include "Encoder.hpp"
#include "RTSP.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "Motion.hpp"
#include "WS.hpp"
#include "version.hpp"

auto main_thread_signal = std::make_shared<std::atomic<int>>(0);
std::shared_ptr<CFG> cfg = std::make_shared<CFG>();

WS ws(cfg, main_thread_signal);
Encoder enc(cfg);
Encoder jpg(cfg);
RTSP rtsp(cfg);

void stop_encoder() {

    LOG_DEBUG("Stop encoder.");

    std::chrono::milliseconds duration;
    auto t0 = std::chrono::high_resolution_clock::now();

    cfg->encoder_thread_signal.fetch_xor(3);
    cfg->encoder_thread_signal.fetch_or(4);

    while((cfg->encoder_thread_signal.load() & 8) != 8) {

        usleep(1000);
        duration = duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0);
        if( duration.count() > 1000 * 1000 * 10 ) {
            LOG_ERROR("Unable to stop encoder, no response.");
            return;
        }
    }
    LOG_DEBUG("Encoder stopped in " << duration.count() << "ms");
}

void start_encoder() {

    LOG_DEBUG("Start encoder.");

    std::chrono::milliseconds duration;
    auto t0 = std::chrono::high_resolution_clock::now();

    cfg->encoder_thread_signal.fetch_xor(8);
    cfg->encoder_thread_signal.fetch_or(1);

    while((cfg->encoder_thread_signal.load() & 2) != 2) {

        usleep(1000);
        duration = duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0);
        if( duration.count() > 1000 * 1000 * 10 ) {
            LOG_ERROR("Unable to start encoder, no response.");
            return;
        }
    }
    LOG_DEBUG("Encoder started in " << duration.count() << "ms");
}

void stop_rtsp() {

    LOG_DEBUG("Stop RTSP Server.");

    std::chrono::milliseconds duration;
    auto t0 = std::chrono::high_resolution_clock::now();

    cfg->rtsp_thread_signal = 1;

    while(cfg->rtsp_thread_signal != 2) {

        usleep(1000);
        duration = duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t0);
        if( duration.count() > 1000 * 1000 * 10 ) {
            LOG_ERROR("Unable to stop RTSP Server, no response.");
            return;
        }
    }
    LOG_DEBUG("RTSP Server stopped in " << duration.count() << "ms");
}

void start_rtsp() {

    LOG_DEBUG("Start RTSP Server.");

    cfg->rtsp_thread_signal = 0;
}

bool timesync_wait() {
    // I don't really have a better way to do this than
    // a no-earlier-than time. The most common sync failure
    // is time() == 0
    int timeout = 0;
    while (time(NULL) < 1647489843) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ++timeout;
        if (timeout == 60)
            return false;
    }
    return true;
}

int main(int argc, const char *argv[]) {

    LOG_INFO("PRUDYNT Video Daemon: " << VERSION);

    std::thread ws_thread;
    std::thread enc_thread;
    std::thread rtsp_thread;

    if (Logger::init(cfg->general.loglevel)) {
        LOG_ERROR("Logger initialization failed.");
        return 1;
    }
    LOG_INFO("Starting Prudynt Video Server.");

    if (!timesync_wait()) {
        LOG_ERROR("Time is not synchronized.");
        return 1;
    }
    
    cfg->rtsp_thread_signal = 2;

    ws_thread = std::thread(&WS::run, ws);
    enc_thread = std::thread(&Encoder::run, &enc);
    rtsp_thread = std::thread(&RTSP::run, rtsp);

    while(1) {

        usleep(1000*1000);

        cfg->main_thread_signal.wait(1);

        std::cout << "MAIN SIGNAL" << std::endl;

        if(cfg->main_thread_signal & 8) { // 8 = stop action
            cfg->main_thread_signal.fetch_xor(8);
            if(cfg->main_thread_signal & 1) {  // 1 = rtsp thread
                stop_rtsp();
            }
            if(cfg->main_thread_signal & 2) { // 2 = encoder thread
                stop_encoder();
            }                              
        }

        if(cfg->main_thread_signal & 16) { // 16 = start action
            cfg->main_thread_signal.fetch_xor(16);
            if(cfg->main_thread_signal & 1) {  // 1 = rtsp thread
                start_rtsp();
            }
            if(cfg->main_thread_signal & 2) { // 2 = encoder thread
                start_encoder();
            }                              
        }

        cfg->main_thread_signal.store(1);
    }

    return 0;
}
