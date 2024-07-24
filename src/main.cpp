#include <iostream>
#include <thread>

#include "MsgChannel.hpp"
#include "Encoder.hpp"
#include "RTSP.hpp"
#include "Logger.hpp"
#include "IMP.hpp"
#include "Config.hpp"
#include "Motion.hpp"

#include "version.hpp"
#include "globals.hpp"

template <class T> void start_component(T &c) {
    c.run();
}

Encoder enc;
Encoder jpg;
Motion motion;
RTSP rtsp;
std::shared_ptr<MsgChannel<H264NALUnit>> msgChannel;

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

    std::thread enc_thread;
    std::thread rtsp_thread;
    std::thread motion_thread;

    if (Logger::init()) {
        LOG_ERROR("Logger initialization failed.");
        return 1;
    }
    LOG_INFO("Starting Prudynt Video Server.");

    if (!timesync_wait()) {
        LOG_ERROR("Time is not synchronized.");
        return 1;
    }
    if (IMP::init()) {
        LOG_ERROR("IMP initialization failed.");
        return 1;
    }
    if (Config::singleton()->motionEnable) {
        if (motion.init()) {
            std::cout << "Motion initialization failed." << std::endl;
            return 1;
        }
    }
    if (enc.init()) {
        LOG_ERROR("Encoder initialization failed.");
        return 1;
    }
    msgChannel = std::make_shared<MsgChannel<H264NALUnit>>(20);
    enc.set_output_channel(msgChannel);
    rtsp.set_input_channel(msgChannel);

    enc_thread = std::thread(start_component<Encoder>, std::ref(enc));
    rtsp_thread = std::thread(start_component<RTSP>, std::ref(rtsp));

    if (Config::singleton()->motionEnable) {
        LOG_DEBUG("Motion detection enabled");
        motion_thread = std::thread(start_component<Motion>, std::ref(motion));
    }

    if (Config::singleton()->stream1jpegEnable) {
        LOG_DEBUG("JPEG support enabled");
        std::thread jpegThread(&Encoder::jpeg_snap, &jpg);
        jpegThread.detach();
    }

    enc_thread.join();
    rtsp_thread.join();
    motion_thread.join();

    return 0;
}
