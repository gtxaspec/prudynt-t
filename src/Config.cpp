#include <filesystem>
#include <fstream>
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
    std::string procPath;
};

// Utility function to handle configuration lookup, default assignment, and validation
template<typename T>
void handleConfigItem(libconfig::Config &lc, ConfigItem<T> &item, std::vector<std::string> &missingConfigs) {
    bool readFromConfig = lc.lookupValue(item.path, item.value);
    bool readFromProc = false;

    if (!readFromConfig && !item.procPath.empty()) { // If not read from config and procPath is set
        // Attempt to read from the proc filesystem
        std::ifstream procFile(item.procPath);
        if (procFile) {
            T value;
            std::string line;
            if (std::getline(procFile, line)) {
                if constexpr (std::is_same_v<T, std::string>) { // Check if T is std::string
                    value = line;
                    readFromProc = true;
                } else if constexpr (std::is_same_v<T, unsigned int>) { // Check if T is unsigned int
                    std::istringstream iss(line);
                    if (line.find("0x") == 0) { // Check if the line starts with "0x"
                        iss >> std::hex >> value; // Read as hexadecimal
                    } else {
                        iss >> value; // Read as decimal
                    }
                    readFromProc = true;
                } else { // For other types, just read directly if possible
                    std::istringstream iss(line);
                    if (iss >> value) {
                        readFromProc = true;
                    }
                }
            }
            if (readFromProc) {
                item.value = value; // Assign the value read from proc
            }
        }
    }

    if (!readFromConfig && !readFromProc) {
        item.value = item.defaultValue; // Assign default value if not found anywhere
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
        {"rtsp.auth_required", rtspAuthRequired, true, [](const bool &v) { return true; }, "RTSP authentication required flag. Must be either true or false."},
        {"osd.enabled", OSDEnable, true, [](const bool &v) { return true; }, "OSD (On-Screen Display) enabled flag. Must be either true or false."},
        {"osd.logo_enabled", OSDLogoEnable, true, [](const bool &v) { return true; }, "OSD logo display enabled flag. Must be either true or false."},
        {"osd.time_enabled", OSDTimeEnable, true, [](const bool &v) { return true; }, "OSD time display enabled flag. Must be either true or false."},
        {"osd.user_text_enabled", OSDUserTextEnable, true, [](const bool &v) { return true; }, "OSD user text display enabled flag. Must be either true or false."},
        {"osd.font_stroke_enabled", OSDFontStrokeEnable, true, [](const bool &v) { return true; }, "OSD font stroke (outline) display enabled flag. Must be either true or false."},
        {"osd.uptime_display_enabled", OSDUptimeEnable, true, [](const bool &v) { return true; }, "OSD uptime display enabled flag. Must be either true or false."},
        {"stream0.jpeg_enabled", stream0jpegEnable, true, [](const bool &v) { return true; }, "JPEG stream for Stream0 enabled flag. Must be either true or false."},
        {"motion.enabled", motionEnable, false, [](const bool &v) { return true; }, "Motion detection enabled flag. Must be either true or false."},
        {"stream0.scale_enabled", stream0scaleEnable, false, [](const bool &v) { return true; }, "Scaling for Stream0 enabled flag. Must be either true or false."},
    };

    std::vector<ConfigItem<std::string>> stringItems = {
        {"rtsp.username", rtspUsername, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP username cannot be empty."},
        {"rtsp.password", rtspPassword, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP password cannot be empty."},
        {"rtsp.name", rtspName, "thingino prudynt", [](const std::string &v) { return !v.empty(); }, "RTSP realm name cannot be empty."},
        {"sensor.model", sensorModel, "gc2053", [](const std::string &v) { return !v.empty(); }, "Sensor model identifier cannot be empty.", "/proc/jz/sensor/name"},
        {"stream0.format", stream0format, "H264", [](const std::string &v) { return v == "H264" || v == "H265"; }, "Stream format must be either 'H264' or 'H265'."},
        {"stream0.rtsp_endpoint", stream0endpoint, "ch0", [](const std::string &v) { return !v.empty(); }, "RTSP endpoint cannot be empty."},
        {"osd.font_path", OSDFontPath, "/usr/share/fonts/UbuntuMono-Regular2.ttf", [](const std::string &v) { return !v.empty(); }, "Font path for OSD cannot be empty."},
        {"osd.time_format", OSDFormat, "%I:%M:%S%p %m/%d/%Y", [](const std::string &v) { return !v.empty(); }, "OSD time format string cannot be empty."},
        {"osd.uptime_display_format", OSDUptimeFormat, "Uptime: %02lu:%02lu:%02lu", [](const std::string &v) { return !v.empty(); }, "OSD uptime format string cannot be empty."},
        {"osd.user_text_string", OSDUserTextString, "thingino", [](const std::string &v) { return true; }, "Custom user text for OSD; can be empty."},
        {"general.loglevel", logLevel, "INFO", [](const std::string &v) { return !v.empty(); }, "Log level must not be empty."},
        {"stream0.jpeg_path", stream0jpegPath, "/tmp/snapshot.jpg", [](const std::string &v) { return !v.empty(); }, "Path for JPEG snapshots must not be empty."},
        {"osd.logo_path", OSDLogoPath, "/usr/share/thingino_logo_1.bgra", [](const std::string &v) { return !v.empty(); }, "OSD Logo path cannot be empty."},
        {"motion.script_path", motionScriptPath, "/usr/sbin/motion", [](const std::string &v) { return !v.empty(); }, "Motion detection script path cannot be empty."},
    };

    std::vector<ConfigItem<int>> intItems = {
        {"rtsp.port", rtspPort, 554, [](const int &v) { return v > 0 && v <= 65535; }, "RTSP port must be between 1 and 65535"},
        {"rtsp.est_bitrate", rtspEstBitRate, 5000, [](const int &v) { return v > 0; }, "Estimated bitrate for RTSP streaming, should be greater than 0."},
        {"rtsp.out_buffer_size", rtspOutBufferSize, 500000, [](const int &v) { return v > 0; }, "Buffer size for outgoing RTSP data, should be greater than 0."},
        {"rtsp.send_buffer_size", rtspSendBufferSize, 307200, [](const int &v) { return v > 0; }, "Buffer size for sending RTSP data, should be greater than 0."},
        {"sensor.fps", sensorFps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Sensor FPS must be between 1 and 60", "/proc/jz/sensor/fps"},
        {"sensor.width", sensorWidth, 1920, [](const int &v) { return v > 0; }, "Width of the sensor's image in pixels", "/proc/jz/sensor/width"},
        {"sensor.height", sensorHeight, 1080, [](const int &v) { return v > 0; }, "Height of the sensor's image in pixels", "/proc/jz/sensor/height"},
        {"stream0.gop", stream0gop, 30, [](const int &v) { return v > 0; }, "Group of pictures (GOP) size for stream 0"},
        {"stream0.max_gop", stream0maxGop, 60, [](const int &v) { return v > 0; }, "Maximum GOP size for stream 0, should not be less than 'stream0.gop'"},
        {"stream0.fps", stream0fps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Stream 0 FPS must be between 1 and 60"},
        {"stream0.buffers", stream0buffers, 2, [](const int &v) { return v > 0 && v <= 32; }, "Number of buffers for stream 0, must be between 1 and 32"},
        {"stream0.width", stream0width, 1920, [](const int &v) { return v > 0; }, "Width of stream 0 in pixels", "/proc/jz/sensor/width"},
        {"stream0.height", stream0height, 1080, [](const int &v) { return v > 0; }, "Height of stream 0 in pixels", "/proc/jz/sensor/height"},
        {"stream0.bitrate", stream0bitrate, 1000, [](const int &v) { return v > 0; }, "Bitrate for stream 0, must be greater than 0"},
        {"stream0.osd_pos_x", stream0osdPosX, 10, [](const int &v) { return v >= 0; }, "X position for OSD in stream 0"},
        {"stream0.osd_pos_y", stream0osdPosY, 10, [](const int &v) { return v >= 0; }, "Y position for OSD in stream 0"},
        {"stream0.osd_pos_user_text_x", stream0osdUserTextPosX, 0, [](const int &v) { return v >= 0; }, "X position for user text OSD in stream 0"},
        {"stream0.osd_pos_user_text_y", stream0osdUserTextPosY, 10, [](const int &v) { return v >= 0; }, "Y position for user text OSD in stream 0"},
        {"stream0.osd_pos_uptime_x", stream0osdUptimeStampPosX, 0, [](const int &v) { return v >= 0; }, "X position for uptime OSD in stream 0"},
        {"stream0.osd_pos_uptime_y", stream0osdUptimeStampPosY, 10, [](const int &v) { return v >= 0; }, "Y position for uptime OSD in stream 0"},
        {"stream0.osd_pos_logo_x", stream0osdLogoPosX, 0, [](const int &v) { return v >= 0; }, "X position for logo OSD in stream 0"},
        {"stream0.osd_pos_logo_y", stream0osdLogoPosY, 0, [](const int &v) { return v >= 0; }, "Y position for logo OSD in stream 0"},
        {"stream0.rotation", stream0rotation, 0, [](const int &v) { return v >= 0 && v <= 2; }, "Stream 0 rotation must be 0, 1, or 2"},
        {"stream0.scale_width", stream0scaleWidth, 640, [](const int &v) { return v > 0; }, "Stream 0 scale width should be greater than 0"},
        {"stream0.scale_height", stream0scaleHeight, 360, [](const int &v) { return v > 0; }, "Stream 0 scale height should be greater than 0"},
        {"stream1.jpeg_quality", stream0jpegQuality, 75, [](const int &v) { return v > 0 && v <= 100; }, "Stream 0 jpeg quality must be between 1 and 100"},
        {"stream1.jpeg_refresh", stream0jpegRefresh, 1000, [](const int &v) { return v > 0; }, "Stream 0 jpeg refresh rate should be greater than 0"},
        {"osd.font_size", OSDFontSize, 64, [](const int &v) { return v > 0; }, "OSD font size should be greater than 0"},
        {"osd.font_stroke_size", OSDFontStrokeSize, 64, [](const int &v) { return v >= 0; }, "OSD font stroke size should be non-negative"},
        {"osd.logo_height", OSDLogoHeight, 30, [](const int &v) { return v > 0; }, "OSD logo height should be greater than 0"},
        {"osd.logo_width", OSDLogoWidth, 100, [](const int &v) { return v > 0; }, "OSD logo width should be greater than 0"},
        {"motion.debounce_time", motionDebounce, 0, [](const int &v) { return v >= 0; }, "Motion debounce time should be non-negative"},
        {"motion.post_time", motionPostTime, 0, [](const int &v) { return v >= 0; }, "Motion post time should be non-negative"},
        {"motion.cooldown_time", motionCooldownTime, 5, [](const int &v) { return v >= 0; }, "Motion cooldown time should be non-negative"},
        {"motion.init_time", motionInitTime, 5, [](const int &v) { return v >= 0; }, "Motion initialization time should be non-negative"},
        {"motion.sensitivity", motionSensitivity, 1, [](const int &v) { return v >= 0; }, "Motion sensitivity should be non-negative"},
        {"motion.skip_frame_count", motionSkipFrameCnt, 5, [](const int &v) { return v >= 0; }, "Motion skip frame count should be non-negative"},
        {"motion.frame_width", motionFrameWidth, 1920, [](const int &v) { return v > 0; }, "Motion frame width should be greater than 0"},
        {"motion.frame_height", motionFrameHeight, 1080, [](const int &v) { return v > 0; }, "Motion frame height should be greater than 0"},
        {"motion.roi_0_x", motionRoi0X, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 X position should be non-negative"},
        {"motion.roi_0_y", motionRoi0Y, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 Y position should be non-negative"},
        {"motion.roi_1_x", motionRoi1X, 1920, [](const int &v) { return v >= 0; }, "Motion ROI 1 X position should be non-negative"},
        {"motion.roi_1_y", motionRoi1Y, 1080, [](const int &v) { return v >= 0; }, "Motion ROI 1 Y position should be non-negative"},
        {"motion.roi_count", roiCnt, 1, [](const int &v) { return v >= 0 && v <= 4; }, "Motion ROI count should be between 0 and 4"},
    };

    std::vector<ConfigItem<unsigned int>> uintItems = {
        {"osd.font_color", OSDFontColor, 0xFFFFFFFF, [](const unsigned int &v) { return true; }, "OSD font color in ARGB format. Default is white."},
        {"osd.font_stroke_color", OSDFontStrokeColor, 0xFF000000, [](const unsigned int &v) { return true; }, "OSD font stroke (outline) color in ARGB format. Default is black."},
        {"sensor.i2c_address", sensorI2Caddress, 0x37, [](const unsigned int &v) { return v <= 0x7F; }, "I2C address of the sensor. Must be between 0x00 and 0x7F.", "/proc/jz/sensor/i2c_addr"},
    };

    // Process all configuration items
    for (auto &item : boolItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : stringItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : intItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : uintItems) handleConfigItem(lc, item, missingConfigs);
}

Config* Config::singleton() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return instance;
}
