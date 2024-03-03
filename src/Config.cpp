#include "Config.hpp"
#include "Logger.hpp"
#include <libconfig.h++>

#define MODULE "CONFIG"

Config* Config::instance = nullptr;

Config::Config() {
    libconfig::Config lc;
    try {
        lc.readFile("/etc/prudynt.cfg");
    }
    catch (...) {
        LOG_WARN("Failed to load prudynt configuration file");
        loadDefaults();
        return;
    }
    loadDefaults();


    lc.lookupValue("rtsp.auth_required", rtspAuthRequired);
    lc.lookupValue("rtsp.username", rtspUsername);
    lc.lookupValue("rtsp.password", rtspPassword);
    lc.lookupValue("rtsp.name", rtspName);
    lc.lookupValue("rtsp.port", rtspPort);

    lc.lookupValue("sensor.model", sensorModel);
    lc.lookupValue("sensor.i2c_address", sensorI2Caddress);
    lc.lookupValue("sensor.fps", sensorFps);

    lc.lookupValue("stream0.fps", stream0fps);
    lc.lookupValue("stream0.buffers", stream0buffers);
    lc.lookupValue("stream0.bitrate", stream0bitrate);
    lc.lookupValue("stream0.osd_pos_width", stream0osdPosWidth);
    lc.lookupValue("stream0.osd_pos_height", stream0osdPosHeight);

    lc.lookupValue("osd.font_size", OSDFontSize);
    lc.lookupValue("osd.font_path", OSDFontPath);
    lc.lookupValue("osd.font_stroke_size", OSDFontStrokeSize);
    lc.lookupValue("osd.enabled", OSDEnabled);
    lc.lookupValue("osd.format", OSDFormat);


    if (!validateConfig()) {
        LOG_ERROR("Configuration is invalid, using defaults.");
        loadDefaults();
        return;
    }
}

void Config::loadDefaults() {
    rtspAuthRequired = false;
    rtspUsername = "";
    rtspPassword = "";
    rtspName = "thingino";
    rtspPort = 554;
    sensorModel = "gc2053";
    sensorI2Caddress = 0x37;
    sensorFps = 24;
    stream0buffers = 2;
    stream0width = 1920;
    stream0height = 1080;
    stream0bitrate = 1000;
    stream0fps = 24;
    stream0gop = 30;
    stream0osdPosWidth = 5;
    stream0osdPosHeight = 5;
    OSDFontPath = "/usr/share/fonts/NotoSansMono-Regular.ttf";
    OSDFontSize = 96;
    OSDFontStrokeSize = 96;
    OSDEnabled = 1;
    OSDFormat = "%I:%M:%S %p %m/%d/%Y";
}

bool Config::validateConfig() {
    /*if (nightModeString.compare("sun_track") != 0) {
        LOG_ERROR("The only supported night mode is sun_track.");
        return false;
    }*/
    if (sensorFps > 30) {
        LOG_ERROR("FPS out of range.");
        return false;
    }
    return true;
}

Config* Config::singleton() {
    if (Config::instance == nullptr) {
        Config::instance = new Config();
    }
    return Config::instance;
}
