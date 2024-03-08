#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <libconfig.h++>
#include "Config.hpp"
#include "Logger.hpp"

#define MODULE "CONFIG"

namespace fs = std::filesystem;
Config* Config::instance = nullptr;

// Define a structure for configuration items
template<typename T>
struct ConfigItem {
    std::string path;
    T& value;
    T defaultValue;
    std::function<bool(const T&)> validate;
    std::string errorMessage;
};

// Utility function to handle configuration lookup, default assignment, and validation
template<typename T>
void handleConfigItem(libconfig::Config &lc, ConfigItem<T> &item, std::vector<std::string> &missingConfigs) {
    if (!lc.lookupValue(item.path, item.value)) {
        item.value = item.defaultValue; // Assign default value if not found
        missingConfigs.push_back(item.path); // Record missing config
        LOG_WARN("Missing configuration for " + item.path + ", using default.");
    } else if (!item.validate(item.value)) {
        LOG_ERROR(item.errorMessage); // Log validation error
        item.value = item.defaultValue; // Revert to default if validation fails
    }
}

Config::Config() {
    libconfig::Config lc;

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.cfg";

    // Try to load the configuration file from the specified paths
    try {
        lc.readFile(cfgFilePath.c_str());
        LOG_INFO("Loaded configuration from " + cfgFilePath.string());
    } catch (const libconfig::FileIOException &) {
        LOG_DEBUG("Failed to load prudynt configuration file from binary directory. Trying /etc...");
        fs::path etcPath = "/etc/prudynt.cfg";
        try {
            lc.readFile(etcPath.c_str());
            LOG_INFO("Loaded configuration from " + etcPath.string());
        } catch (...) {
            LOG_WARN("Failed to load prudynt configuration file from /etc.");
            return; // Exit if configuration file is missing
        }
    } catch (const libconfig::ParseException &pex) {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return; // Exit on parsing error
    }

    std::vector<std::string> missingConfigs; // Track missing configuration entries

    // Configuration items
    std::vector<ConfigItem<bool>> boolItems = {
        {"rtsp.auth_required", rtspAuthRequired, false, [](const bool &v) { return true; }, "rtsp.auth_required must be true or false"},
        {"osd.enabled", OSDEnabled, true, [](const bool &v) { return true; }, "osd.enabled must be true or false"},
        {"osd.user_text_enabled", OSDUserTextEnable, true, [](const bool &v) { return true; }, "osd.enabled must be true or false"},
        {"osd.font_stroke_enabled", OSDFontStrokeEnable, true, [](const bool &v) { return true; }, "osd.font_stroke_enabled must be true or false"},
        {"osd.user_text_enabled", OSDUserTextEnable, true, [](const bool &v) { return true; }, "osd.user_text_enabled must be true or false"},
        {"stream0.jpeg_enabled", stream0jpegEnable, true, [](const bool &v) { return true; }, "osd.user_text_enabled must be true or false"},
    };

    std::vector<ConfigItem<std::string>> stringItems = {
        {"rtsp.username", rtspUsername, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP username cannot be empty"},
        {"rtsp.password", rtspPassword, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP password cannot be empty"},
        {"rtsp.name", rtspName, "thingino prudynt", [](const std::string &v) { return !v.empty(); }, "RTSP realm name cannot be empty"},
        {"sensor.model", sensorModel, "gc2053", [](const std::string &v) { return !v.empty(); }, "Sensor model cannot be empty"},
        {"stream0.format", stream0format, "H264", [](const std::string &v) { return !v.empty(); }, "Stream format must be H264 or H265"},
        {"osd.font_path", OSDFontPath, "/usr/share/fonts/UbuntuMono-Regular2.ttf", [](const std::string &v) { return !v.empty(); }, "Must specify valid font path"},
        {"osd.format", OSDFormat, "%I:%M:%S%p %m/%d/%Y", [](const std::string &v) { return !v.empty(); }, "OSD format string must not be empty"},
        {"osd.user_text_string", OSDUserTextString, "thingino", [](const std::string &v) { return true; }, ""},
        {"general.loglevel", logLevel, "INFO", [](const std::string &v) { return true; }, ""},
        {"stream0.jpeg_path", stream0jpegPath, "/tmp/snapshot.jpg", [](const std::string &v) { return true; }, "JPEG snapshot path string must not be empty"},
    };

    std::vector<ConfigItem<int>> intItems = {
        {"rtsp.port", rtspPort, 554, [](const int &v) { return v > 0 && v <= 65535; }, "RTSP port must be between 1 and 65535"},
        {"rtsp.est_bitrate", rtspEstBitRate, 5000, [](const int &v) { return true; }, ""},
        {"rtsp.out_buffer_size", rtspOutBufferSize, 500000, [](const int &v) { return true; }, ""},
        {"rtsp.send_buffer_size", rtspSendBufferSize, 307200, [](const int &v) { return true; }, ""},
        {"sensor.fps", sensorFps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Sensor FPS must be between 1 and 240"},
        {"stream0.gop", stream0gop, 30, [](const int &v) { return true; }, ""},
        {"stream0.fps", stream0fps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Stream 0 FPS must be between 1 and 240"},
        {"stream0.buffers", stream0buffers, 2, [](const int &v) { return v > 0 && v <= 32; }, "Stream 0 buffers must be between 1 and 32"},
        {"stream0.height", stream0height, 1080, [](const int &v) { return true; }, ""},
        {"stream0.width", stream0width, 1920, [](const int &v) { return true; }, ""},
        {"stream0.bitrate", stream0bitrate, 1000, [](const int &v) { return true; }, ""},
        {"stream0.osd_pos_width", stream0osdPosWidth, 5, [](const int &v) { return true; }, ""},
        {"stream0.osd_pos_height", stream0osdPosHeight, 5, [](const int &v) { return true; }, ""},
        {"stream0.osd_user_text_pos_width", stream0osdUserTextPosWidth, 5, [](const int &v) { return true; }, ""},
        {"stream0.osd_user_text_pos_height", stream0osdUserTextPosHeight, 5, [](const int &v) { return true; }, ""},
        {"osd.font_size", OSDFontStrokeSize, 64, [](const int &v) { return true; }, ""},
        {"osd.font_stroke_size", OSDFontSize, 64, [](const int &v) { return true; }, ""},
        {"stream0.jpeg_quality", stream0jpegQuality, 75, [](const int &v) { return v > 0 && v <= 100; }, "Stream 0 jpeg quality must be between 1 and 100"},
        {"stream0.jpeg_refresh", stream0jpegRefresh, 1000, [](const int &v) { return true; }, ""},
    };

    std::vector<ConfigItem<unsigned int>> uintItems = {
        {"osd.font_color", OSDFontColor, 0xFFFFFFFF, [](const unsigned int &v) { return true; }, ""},
        {"osd.font_stroke_color:", OSDFontStrokeColor, 0xFF000000, [](const unsigned int &v) { return true; }, ""},
        {"sensor.i2c_address", sensorI2Caddress, 0x37, [](const unsigned int &v) { return true; }, ""},
    };

    // Process all configuration items
    for (auto &item : boolItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : stringItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : intItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : uintItems) handleConfigItem(lc, item, missingConfigs);

    // Log missing configurations if any
    if (!missingConfigs.empty()) {
        for (const auto &missing : missingConfigs) {
            LOG_ERROR("Missing configuration: " + missing);
        }
    }
}

Config* Config::singleton() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return instance;
}
