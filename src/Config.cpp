#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <libconfig.h++>
#include "Config.hpp"
#include "Logger.hpp"
#include "imp/imp_ivs_move.h"

#define MODULE "CONFIG"

namespace fs = std::filesystem;
using namespace libconfig;

bool validateIntGe0(const int &v)
{
    return v >= 0;
}

bool validateInt1(const int &v)
{
    return v >= 0 && v <= 2;
}

bool validateInt2(const int &v)
{
    return v >= 0 && v <= 2;
}

bool validateInt32(const int &v)
{
    return v >= 0 && v <= 32;
}

bool validateInt50_150(const int &v)
{
    return v >= 50 && v <= 150;
}

bool validateInt120(const int &v)
{
    return (v >= 0 && v <= 120) || v == IMP_AUTO_VALUE;
}

bool validateInt255(const int &v)
{
    return v >= 0 && v <= 255;
}

bool validateInt360(const int &v)
{
    return v >= 0 && v <= 360;
}

bool validateInt15360(const int &v)
{
    return v >= -15360 && v <= 15360;
}

bool validateInt65535(const int &v)
{
    return v >= 0 && v <= 65535;
}

bool validateCharDummy(const char *v)
{
    return true;
}

bool validateCharNotEmpty(const char *v)
{
    return std::strlen(v) > 0;
}

bool validateBool(const bool &v)
{
    return true;
}

bool validateUint(const unsigned int &v)
{
    return true;
}

bool validateSampleRate(const int &v)
{
    std::set<int> allowed_rates = {8000, 16000, 24000, 44100, 48000};
    return allowed_rates.count(v) == 1;
}

std::vector<ConfigItem<bool>> CFG::getBoolItems()
{
    return {
#if defined(AUDIO_SUPPORT)
        {"audio.input_enabled", audio.input_enabled, true, validateBool},
        {"audio.output_enabled", audio.output_enabled, false, validateBool},
        {"audio.force_stereo", audio.force_stereo, false, validateBool},
#if defined(LIB_AUDIO_PROCESSING)
        {"audio.input_high_pass_filter", audio.input_high_pass_filter, false, validateBool},
        {"audio.input_agc_enabled", audio.input_agc_enabled, false, validateBool},
#endif
#endif
        {"image.vflip", image.vflip, false, validateBool},
        {"image.hflip", image.hflip, false, validateBool},
        {"motion.enabled", motion.enabled, false, validateBool},
        {"rtsp.auth_required", rtsp.auth_required, true, validateBool},
#if defined(AUDIO_SUPPORT)
        {"stream0.audio_enabled", stream0.audio_enabled, true, validateBool},
#endif
        {"stream0.enabled", stream0.enabled, true, validateBool},
        {"stream0.allow_shared", stream0.allow_shared, true, validateBool},
        {"stream0.osd.enabled", stream0.osd.enabled, true, validateBool},
        {"stream0.osd.logo_enabled", stream0.osd.logo_enabled, true, validateBool},
        {"stream0.osd.time_enabled", stream0.osd.time_enabled, true, validateBool},
        {"stream0.osd.uptime_enabled", stream0.osd.uptime_enabled, true, validateBool},
        {"stream0.osd.user_text_enabled", stream0.osd.user_text_enabled, true, validateBool},
#if defined(AUDIO_SUPPORT)
        {"stream1.audio_enabled", stream1.audio_enabled, true, validateBool},
#endif
        {"stream1.enabled", stream1.enabled, true, validateBool},
        {"stream1.allow_shared", stream1.allow_shared, true, validateBool},
        {"stream1.osd.enabled", stream1.osd.enabled, true, validateBool},
        {"stream1.osd.logo_enabled", stream1.osd.logo_enabled, true, validateBool},
        {"stream1.osd.time_enabled", stream1.osd.time_enabled, true, validateBool},
        {"stream1.osd.uptime_enabled", stream1.osd.uptime_enabled, true, validateBool},
        {"stream1.osd.user_text_enabled", stream1.osd.user_text_enabled, true, validateBool},
        {"stream2.enabled", stream2.enabled, true, validateBool},
        {"websocket.enabled", websocket.enabled, true, validateBool},
        {"websocket.ws_secured", websocket.ws_secured, true, validateBool},
        {"websocket.http_secured", websocket.http_secured, true, validateBool},
    };
};

std::vector<ConfigItem<const char *>> CFG::getCharItems()
{
    return {
#if defined(AUDIO_SUPPORT)
        {"audio.input_format", audio.input_format, "OPUS", [](const char *v) {
            std::set<std::string> a = {"OPUS", "AAC", "PCM", "G711A", "G711U", "G726"};
            return a.count(std::string(v)) == 1;
        }},
#endif
        {"general.loglevel", general.loglevel, "INFO", [](const char *v) {
            std::set<std::string> a = {"EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"};
            return a.count(std::string(v)) == 1;
        }},
        {"motion.script_path", motion.script_path, "/usr/sbin/motion", validateCharNotEmpty},
        {"rtsp.name", rtsp.name, "thingino prudynt", validateCharNotEmpty},
        {"rtsp.password", rtsp.password, "thingino", validateCharNotEmpty},
        {"rtsp.username", rtsp.username, "thingino", validateCharNotEmpty},
        {"sensor.model", sensor.model, "gc2053", validateCharNotEmpty, false, "/proc/jz/sensor/name"},
        {"stream0.format", stream0.format, "H264", [](const char *v) { return strcmp(v, "H264") == 0 || strcmp(v, "H265") == 0; }},
        {"stream0.osd.font_path", stream0.osd.font_path, "/usr/share/fonts/UbuntuMono-Regular2.ttf", validateCharNotEmpty},
        {"stream0.osd.logo_path", stream0.osd.logo_path, "/usr/share/images/thingino_logo_1.bgra", validateCharNotEmpty},
        {"stream0.osd.time_format", stream0.osd.time_format, "%F %T", validateCharNotEmpty},
        {"stream0.osd.uptime_format", stream0.osd.uptime_format, "Up: %02lud %02lu:%02lu", validateCharNotEmpty},
        {"stream0.osd.user_text_format", stream0.osd.user_text_format, "%hostname", validateCharNotEmpty},
        {"stream0.mode", stream0.mode, DEFAULT_ENC_MODE_0, [](const char *v) {
            std::set<std::string> a = {"CBR", "VBR", "SMART", "FIXQP", "CAPPED_VBR", "CAPPED_QUALITY"};
            return a.count(std::string(v)) == 1;
        }},
        {"stream0.rtsp_endpoint", stream0.rtsp_endpoint, "ch0", validateCharNotEmpty},
        {"stream0.rtsp_info", stream0.rtsp_info, "stream0", validateCharNotEmpty},
        {"stream1.format", stream1.format, "H264", [](const char *v) { return strcmp(v, "H264") == 0 || strcmp(v, "H265") == 0; }},
        {"stream1.osd.font_path", stream1.osd.font_path, "/usr/share/fonts/NotoSansDisplay-Condensed2.ttf", validateCharNotEmpty},
        {"stream1.osd.logo_path", stream1.osd.logo_path, "/usr/share/images/thingino_logo_1.bgra", validateCharNotEmpty},
        {"stream1.osd.time_format", stream1.osd.time_format, "%F %T", validateCharNotEmpty},
        {"stream1.osd.uptime_format", stream1.osd.uptime_format, "Up: %02lud %02lu:%02lu", validateCharNotEmpty},
        {"stream1.osd.user_text_format", stream1.osd.user_text_format, "%hostname", validateCharNotEmpty},
        {"stream1.mode", stream1.mode, DEFAULT_ENC_MODE_1, [](const char *v) {
            std::set<std::string> a = {"CBR", "VBR", "SMART", "FIXQP", "CAPPED_VBR", "CAPPED_QUALITY"};
            return a.count(std::string(v)) == 1;
        }},
        {"stream1.rtsp_endpoint", stream1.rtsp_endpoint, "ch1", validateCharNotEmpty},
        {"stream1.rtsp_info", stream1.rtsp_info, "stream1", validateCharNotEmpty},
       {"stream2.jpeg_path", stream2.jpeg_path, "/tmp/snapshot.jpg", validateCharNotEmpty},
        {"websocket.name", websocket.name, "wss prudynt", validateCharNotEmpty},
        {"websocket.usertoken", websocket.usertoken, "", [](const char *v) {
            return std::string(v).length() < 32;
        }},
    };
};

std::vector<ConfigItem<int>> CFG::getIntItems()
{
    return {
#if defined(AUDIO_SUPPORT)
        {"audio.input_bitrate", audio.input_bitrate, 40, [](const int &v) { return v >= 6 && v <= 256; }},
        {"audio.input_sample_rate", audio.input_sample_rate, 16000, validateSampleRate},
        {"audio.output_sample_rate", audio.output_sample_rate, 16000, validateSampleRate},
        {"audio.input_vol", audio.input_vol, 80, [](const int &v) { return v >= -30 && v <= 120; }},
        {"audio.input_gain", audio.input_gain, 25, [](const int &v) { return v >= -1 && v <= 31; }},
#if defined(LIB_AUDIO_PROCESSING)
        {"audio.input_alc_gain", audio.input_alc_gain, 0, [](const int &v) { return v >= -1 && v <= 7; }},
        {"audio.input_agc_target_level_dbfs", audio.input_agc_target_level_dbfs, 10, [](const int &v) { return v >= 0 && v <= 31; }},
        {"audio.input_agc_compression_gain_db", audio.input_agc_compression_gain_db, 0, [](const int &v) { return v >= 0 && v <= 90; }},
        {"audio.input_noise_suppression", audio.input_noise_suppression, 0, [](const int &v) { return v >= 0 && v <= 3; }},
#endif
#endif
        {"general.imp_polling_timeout", general.imp_polling_timeout, 500, [](const int &v) { return v >= 1 && v <= 5000; }},
        {"general.osd_pool_size", general.osd_pool_size, 1024, [](const int &v) { return v >= 0 && v <= 65535; }},
        {"image.ae_compensation", image.ae_compensation, 128, validateInt255},
        {"image.anti_flicker", image.anti_flicker, 2, validateInt2},
        {"image.backlight_compensation", image.backlight_compensation, 0, [](const int &v) { return v >= 0 && v <= 10; }},
        {"image.brightness", image.brightness, 128, validateInt255},
        {"image.contrast", image.contrast, 128, validateInt255},
        {"image.core_wb_mode", image.core_wb_mode, 0, [](const int &v) { return v >= 0 && v <= 9; }},
        {"image.defog_strength", image.defog_strength, 128, validateInt255},
        {"image.dpc_strength", image.dpc_strength, 128, validateInt255},
        {"image.drc_strength", image.drc_strength, 128, validateInt255},
        {"image.highlight_depress", image.highlight_depress, 0, validateInt255},
        {"image.hue", image.hue, 128, validateInt255},
        {"image.max_again", image.max_again, 160, [](const int &v) { return v >= 0 && v <= 160; }},
        {"image.max_dgain", image.max_dgain, 80, [](const int &v) { return v >= 0 && v <= 160; }},
        {"image.running_mode", image.running_mode, 0, validateInt1},
        {"image.saturation", image.saturation, 128, validateInt255},
        {"image.sharpness", image.sharpness, 128, validateInt255},
        {"image.sinter_strength", image.sinter_strength, DEFAULT_SINTER, DEFAULT_SINTER_VALIDATE},
        {"image.temper_strength", image.temper_strength, DEFAULT_TEMPER, DEFAULT_TEMPER_VALIDATE},
        {"image.wb_bgain", image.wb_bgain, 0, [](const int &v) { return v >= 0 && v <= 34464; }},
        {"image.wb_rgain", image.wb_rgain, 0, [](const int &v) { return v >= 0 && v <= 34464; }},
        {"motion.debounce_time", motion.debounce_time, 0, validateIntGe0},
        {"motion.post_time", motion.post_time, 0, validateIntGe0},
        {"motion.ivs_polling_timeout", motion.ivs_polling_timeout, 1000, [](const int &v) { return v >= 100 && v <= 10000; }},
        {"motion.cooldown_time", motion.cooldown_time, 5, validateIntGe0},
        {"motion.init_time", motion.init_time, 5, validateIntGe0},
        {"motion.min_time", motion.min_time, 1, validateIntGe0},
        {"motion.sensitivity", motion.sensitivity, 1, validateIntGe0},
        {"motion.skip_frame_count", motion.skip_frame_count, 5, validateIntGe0},
        {"motion.frame_width", motion.frame_width, IVS_AUTO_VALUE, validateIntGe0},
        {"motion.frame_height", motion.frame_height, IVS_AUTO_VALUE, validateIntGe0},
        {"motion.monitor_stream", motion.monitor_stream, 1, validateInt1},
        {"motion.roi_count", motion.roi_count, IVS_AUTO_VALUE, [](const int &v) { return v >= 1 && v <= IMP_IVS_MOVE_MAX_ROI_CNT; }},
        {"rtsp.est_bitrate", rtsp.est_bitrate, 5000, validateIntGe0},
        {"rtsp.out_buffer_size", rtsp.out_buffer_size, 500000, validateIntGe0},
        {"rtsp.port", rtsp.port, 554, validateInt65535},
        {"rtsp.send_buffer_size", rtsp.send_buffer_size, 307200, validateIntGe0},
        {"rtsp.session_reclaim", rtsp.session_reclaim, 65, validateIntGe0},
        {"sensor.i2c_bus", sensor.i2c_bus, 0, validateIntGe0, false, "/proc/jz/sensor/i2c_bus"},
        {"sensor.fps", sensor.fps, 25, validateInt120, false, "/proc/jz/sensor/max_fps"},
        {"sensor.height", sensor.height, 1080, validateIntGe0, false, "/proc/jz/sensor/height"},
        {"sensor.width", sensor.width, 1920, validateIntGe0, false, "/proc/jz/sensor/width"},
        {"sensor.boot", sensor.boot, 0, validateIntGe0, false, "/proc/jz/sensor/boot"},
        {"sensor.mclk", sensor.mclk, 1, validateIntGe0, false, "/proc/jz/sensor/mclk"},
        {"sensor.video_interface", sensor.video_interface, 0, validateIntGe0, false, "/proc/jz/sensor/video_interface"},
        {"sensor.gpio_reset", sensor.gpio_reset, 91, validateIntGe0, false, "/proc/jz/sensor/reset_gpio"},
        {"stream0.bitrate", stream0.bitrate, 3000, validateIntGe0},
        {"stream0.buffers", stream0.buffers, DEFAULT_BUFFERS_0, validateInt32},
        {"stream0.fps", stream0.fps, 25, validateInt120},
        {"stream0.gop", stream0.gop, 20, validateIntGe0},
        {"stream0.height", stream0.height, 1080, validateIntGe0, false, "/proc/jz/sensor/height"},
        {"stream0.max_gop", stream0.max_gop, 60, validateIntGe0},
        {"stream0.osd.font_size", stream0.osd.font_size, OSD_AUTO_VALUE, validateIntGe0},
        {"stream0.osd.font_stroke", stream0.osd.font_stroke, 1, validateIntGe0},
        {"stream0.osd.font_xscale", stream0.osd.font_xscale, 100, validateInt50_150},
        {"stream0.osd.font_yscale", stream0.osd.font_yscale, 100, validateInt50_150},
        {"stream0.osd.font_yoffset", stream0.osd.font_yoffset, 3, validateIntGe0},
        {"stream0.osd.logo_height", stream0.osd.logo_height, 30, validateIntGe0},
        {"stream0.osd.logo_rotation", stream0.osd.logo_rotation, 0, validateInt360},
        {"stream0.osd.logo_transparency", stream0.osd.logo_transparency, 255, validateInt255},
        {"stream0.osd.logo_width", stream0.osd.logo_width, 100, validateIntGe0},
        {"stream0.osd.pos_logo_x", stream0.osd.pos_logo_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_logo_y", stream0.osd.pos_logo_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_time_x", stream0.osd.pos_time_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_time_y", stream0.osd.pos_time_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_uptime_x", stream0.osd.pos_uptime_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_uptime_y", stream0.osd.pos_uptime_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_user_text_x", stream0.osd.pos_user_text_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.pos_user_text_y", stream0.osd.pos_user_text_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream0.osd.start_delay", stream0.osd.start_delay, 1, [](const int &v) { return v >= 1 && v <= 5000; }},
        {"stream0.osd.time_rotation", stream0.osd.time_rotation, 0, validateInt360},
        {"stream0.osd.time_transparency", stream0.osd.time_transparency, 255, validateInt255},
        {"stream0.osd.uptime_rotation", stream0.osd.uptime_rotation, 0, validateInt360},
        {"stream0.osd.uptime_transparency", stream0.osd.uptime_transparency, 255, validateInt255},
        {"stream0.osd.user_text_rotation", stream0.osd.user_text_rotation, 0, validateInt360},
        {"stream0.osd.user_text_transparency", stream0.osd.user_text_transparency, 255, validateInt255},
        {"stream0.rotation", stream0.rotation, 0, validateInt2},
        {"stream0.width", stream0.width, 1920, validateIntGe0, false, "/proc/jz/sensor/width"},
        {"stream0.profile", stream0.profile, 2, validateInt2},
        {"stream1.bitrate", stream1.bitrate, 1000, validateIntGe0},
        {"stream1.buffers", stream1.buffers, DEFAULT_BUFFERS_1, validateInt32},
        {"stream1.fps", stream1.fps, 25, validateInt120},
        {"stream1.gop", stream1.gop, 20, validateIntGe0},
        {"stream1.height", stream1.height, 360, validateIntGe0},
        {"stream1.max_gop", stream1.max_gop, 60, validateIntGe0},
        {"stream1.osd.font_size", stream1.osd.font_size, OSD_AUTO_VALUE, validateIntGe0},
        {"stream1.osd.font_stroke", stream1.osd.font_stroke, 1, validateIntGe0},
        {"stream1.osd.font_xscale", stream1.osd.font_xscale, 100, validateInt50_150},
        {"stream1.osd.font_yscale", stream1.osd.font_yscale, 100, validateInt50_150},
        {"stream1.osd.font_yoffset", stream1.osd.font_yoffset, 3, validateIntGe0},
        {"stream1.osd.logo_height", stream1.osd.logo_height, 30, validateIntGe0},
        {"stream1.osd.logo_rotation", stream1.osd.logo_rotation, 0, validateInt360},
        {"stream1.osd.logo_transparency", stream1.osd.logo_transparency, 255, validateInt255},
        {"stream1.osd.logo_width", stream1.osd.logo_width, 100, validateIntGe0},
        {"stream1.osd.pos_logo_x", stream1.osd.pos_logo_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_logo_y", stream1.osd.pos_logo_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_time_x", stream1.osd.pos_time_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_time_y", stream1.osd.pos_time_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_uptime_x", stream1.osd.pos_uptime_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_uptime_y", stream1.osd.pos_uptime_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_user_text_x", stream1.osd.pos_user_text_x, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.pos_user_text_y", stream1.osd.pos_user_text_y, OSD_AUTO_VALUE, validateInt15360},
        {"stream1.osd.start_delay", stream1.osd.start_delay, 1, [](const int &v) { return v >= 1 && v <= 5000; }},
        {"stream1.osd.time_rotation", stream1.osd.time_rotation, 0, validateInt360},
        {"stream1.osd.time_transparency", stream1.osd.time_transparency, 255, validateInt255},
        {"stream1.osd.uptime_rotation", stream1.osd.uptime_rotation, 0, validateInt360},
        {"stream1.osd.uptime_transparency", stream1.osd.uptime_transparency, 255, validateInt255},
        {"stream1.osd.user_text_transparency", stream1.osd.user_text_transparency, 255, validateInt255},
        {"stream1.osd.user_text_rotation", stream1.osd.user_text_rotation, 0, validateInt360},
        {"stream1.rotation", stream1.rotation, 0, validateInt2},
        {"stream1.width", stream1.width, 640, validateIntGe0},
        {"stream1.profile", stream1.profile, 2, validateInt2},
        {"stream2.jpeg_channel", stream2.jpeg_channel, 0, validateIntGe0},
        {"stream2.jpeg_quality", stream2.jpeg_quality, 75, [](const int &v) { return v > 0 && v <= 100; }},
        {"stream2.jpeg_idle_fps", stream2.jpeg_idle_fps, 1, [](const int &v) { return v >= 0 && v <= 30; }},
        {"stream2.fps", stream2.fps, 25, [](const int &v) { return v > 1 && v <= 30; }},
        {"websocket.loglevel", websocket.loglevel, 4096, [](const int &v) { return v > 0 && v <= 4096; }},
        {"websocket.port", websocket.port, 8089, validateInt65535},
        {"websocket.first_image_delay", websocket.first_image_delay, 100, validateInt65535},
        // Next section contains old options, only necessary for your migration.
        {"motion.roi_0_x", migration.motion_roi_0_x, 0, validateIntGe0},
        {"motion.roi_0_y", migration.motion_roi_0_y, 0, validateIntGe0},
        {"motion.roi_1_x", migration.motion_roi_1_x, 0, validateIntGe0},
        {"motion.roi_1_y", migration.motion_roi_1_y, 0, validateIntGe0},
    };
};

std::vector<ConfigItem<unsigned int>> CFG::getUintItems()
{
    return {
        {"sensor.i2c_address", sensor.i2c_address, 0x37, [](const unsigned int &v) { return v <= 0x7F; }, false, "/proc/jz/sensor/i2c_addr"},
        {"stream0.osd.font_stroke_color", stream0.osd.font_stroke_color, 0xFF000000, validateUint},
        {"stream0.osd.font_color", stream0.osd.font_color, 0xFFFFFFFF, validateUint},
        {"stream1.osd.font_color", stream1.osd.font_color, 0xFFFFFFFF, validateUint},
        {"stream1.osd.font_stroke_color", stream1.osd.font_stroke_color, 0xFF000000, validateUint},
    };
};

void ensurePathExists(Setting &root, const std::string &path)
{
    std::stringstream ss(path);
    std::string segment;
    Setting *current = &root;

    while (std::getline(ss, segment, '.'))
    {
        if (!current->exists(segment))
        {
            current = &current->add(segment, Setting::TypeGroup);
        }
        else
        {
            current = &current->lookup(segment);
        }
    }
}

bool findSetting(const std::string &path, const Setting *&foundSetting, Setting *root)
{

    std::string::size_type pos = path.find_first_of('.');
    std::string currentPath = path.substr(0, pos);

    if (root->exists(currentPath))
    {
        if (pos == std::string::npos)
        {
            foundSetting = &root->lookup(currentPath);
            return true;
        }
        else
        {
            return findSetting(path.substr(pos + 1), foundSetting, &root->lookup(currentPath));
        }
    }
    else
    {
        foundSetting = nullptr;
        return false;
    }
}

bool CFG::readConfig()
{

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.cfg";
    filePath = cfgFilePath;

    // Try to load the configuration file from the specified paths
    try
    {
        lc.readFile(cfgFilePath.c_str());
        LOG_INFO("Loaded configuration from " + cfgFilePath.string());
    }
    catch (const FileIOException &)
    {
        fs::path etcPath = "/etc/prudynt.cfg";
        filePath = etcPath;
        try
        {
            lc.readFile(etcPath.c_str());
            LOG_INFO("Loaded configuration from " + etcPath.string());
        }
        catch (...)
        {
            LOG_WARN("Failed to load prudynt configuration file from /etc/config.");
            return false; // Exit if configuration file is missing
        }
    }
    catch (const ParseException &pex)
    {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return false; // Exit on parsing error
    }

    return true;
}

template <typename T>
bool processLine(const std::string &line, T &value)
{
    if constexpr (std::is_same_v<T, std::string>)
    {
        value = line;
        return true;
    }
    else if constexpr (std::is_same_v<T, const char *>)
    {
        value = line.c_str();
        return true;
    }
    else if constexpr (std::is_same_v<T, unsigned int>)
    {
        std::istringstream iss(line);
        if (line.find("0x") == 0)
        {
            iss >> std::hex >> value;
        }
        else
        {
            iss >> value;
        }
        return !iss.fail();
    }
    else
    {
        std::istringstream iss(line);
        iss >> value;
        return !iss.fail();
    }
}

template <typename T>
void handleConfigItem(Config &lc, ConfigItem<T> &item)
{
    bool readFromProc = false;
    bool readFromConfig = false;
    T configValue{};

    if constexpr (std::is_same_v<T, const char *>)
    {
        std::string temp;
        readFromConfig = lc.lookupValue(item.path, temp);
        if (readFromConfig)
        {
            item.value = strdup(temp.c_str());
        }
    }
    else
    {
        readFromConfig = lc.lookupValue(item.path, configValue);
        if (readFromConfig)
        {
            item.value = configValue;
        }
    }

    if (!readFromConfig && item.procPath != nullptr && item.procPath[0] != '\0')
    { // If not read from config and procPath is set
        // Attempt to read from the proc filesystem
        std::ifstream procFile(item.procPath);
        if (procFile)
        {
            T value;
            std::string line;
            if (std::getline(procFile, line))
            {
                if (processLine(line, value))
                {
                    if constexpr (std::is_same_v<T, const char *>)
                    {
                        item.value = strdup(value);
                    }
                    else
                    {
                        item.value = value;
                    }
                    readFromProc = true;
                }
            }
        }
    }

    if (!readFromConfig && !readFromProc)
    {
        item.value = item.defaultValue; // Assign default value if not found anywhere
    }
    else if (!item.validate(item.value))
    {
        LOG_ERROR("invalid config value. " << item.path << " = " << item.value);
        item.value = item.defaultValue; // Revert to default if validation fails
    }

    if constexpr (std::is_same_v<T, const char *>)
    {
        if (!readFromConfig && !readFromProc)
        {
            item.value = strdup(item.defaultValue);
        }
        else if (!item.validate(item.value))
        {
            LOG_ERROR("invalid config value. " << item.path << " = " << item.value);
            item.value = strdup(item.defaultValue);
        }
    }
}

template <typename T>
void handleConfigItem2(Config &lc, ConfigItem<T> &item)
{
    std::string path(item.path);
    size_t pos = path.find_last_of('.');
    std::string sect = path.substr(0, pos);
    std::string entr = path.substr(pos + 1);

    ensurePathExists(lc.getRoot(), item.path);

    Setting &section = lc.lookup(sect);
    if (section.exists(entr))
    {
        section.remove(entr);
    }

    Setting::Type type;
    if constexpr (std::is_same_v<T, bool>)
    {
        type = Setting::TypeBoolean;
    }
    else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char *>)
    {
        type = Setting::TypeString;
    }
    else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, unsigned int>)
    {
        type = Setting::TypeInt;
    }

    Setting &newSetting = section.add(entr, type);
    if constexpr (std::is_same_v<T, unsigned int>)
    {
        newSetting = static_cast<long>(item.value);
        newSetting.setFormat(Setting::FormatHex);
    }
    else if constexpr (std::is_same_v<T, const char *>)
    {
        newSetting = std::string(item.value);
    }
    else
    {
        newSetting = item.value;
    }
}

bool CFG::updateConfig()
{
    config_loaded = readConfig();

    for (auto &item : boolItems)
        handleConfigItem2(lc, item);
    for (auto &item : charItems)
        handleConfigItem2(lc, item);
    for (auto &item : intItems)
        handleConfigItem2(lc, item);
    for (auto &item : uintItems)
        handleConfigItem2(lc, item);

    Setting &root = lc.getRoot();

    if (root.exists("rois"))
        root.remove("rois");

    Setting &rois = root.add("rois", Setting::TypeGroup);

    for (int i = 0; i < motion.roi_count; i++)
    {
        Setting &entry = rois.add("roi_" + std::to_string(i), Setting::TypeArray);
        entry.add(Setting::TypeInt) = motion.rois[i].p0_x;
        entry.add(Setting::TypeInt) = motion.rois[i].p0_y;
        entry.add(Setting::TypeInt) = motion.rois[i].p1_x;
        entry.add(Setting::TypeInt) = motion.rois[i].p1_y;
    }

    if (root.exists("motion.roi_0_x"))
        root.remove("motion.roi_0_x");
    if (root.exists("motion.roi_0_y"))
        root.remove("motion.roi_0_y");
    if (root.exists("motion.roi_1_x"))
        root.remove("motion.roi_1_x");
    if (root.exists("motion.roi_1_y"))
        root.remove("motion.roi_1_y");

    lc.writeFile(filePath);
    LOG_DEBUG("Config is written to " << filePath);

    return true;
};

CFG::CFG()
{
    load();
}

void CFG::load()
{
    boolItems = getBoolItems();
    charItems = getCharItems();
    intItems = getIntItems();
    uintItems = getUintItems();

    config_loaded = readConfig();

    for (auto &item : boolItems)
        handleConfigItem(lc, item);
    for (auto &item : charItems)
        handleConfigItem(lc, item);
    for (auto &item : intItems)
        handleConfigItem(lc, item);
    for (auto &item : uintItems)
        handleConfigItem(lc, item);

    if (stream2.jpeg_channel == 0)
    {
        stream2.width = stream0.width;
        stream2.height = stream0.height;
    } 
    else
    {
        stream2.width = stream1.width;
        stream2.height = stream1.height;        
    }

    Setting &root = lc.getRoot();

    if (root.exists("rois"))
    {
        Setting &rois = root.lookup("rois");
        for (int i = 0; i < motion.roi_count; i++)
        {
            if (rois.exists("roi_" + std::to_string(i)))
            {
                if (rois[i].getLength() == 4)
                {
                    motion.rois[i].p0_x = rois[i][0];
                    motion.rois[i].p0_y = rois[i][1];
                    motion.rois[i].p1_x = rois[i][2];
                    motion.rois[i].p1_y = rois[i][3];
                }
            }
        }
    }

    if (motion.roi_count == 1 && migration.motion_roi_0_x > 0)
        motion.rois[0].p0_x = migration.motion_roi_0_x;
    if (motion.roi_count == 1 && migration.motion_roi_0_y > 0)
        motion.rois[0].p0_y = migration.motion_roi_0_y;
    if (motion.roi_count == 1 && migration.motion_roi_1_x > 0)
        motion.rois[0].p1_x = migration.motion_roi_1_x;
    if (motion.roi_count == 1 && migration.motion_roi_1_y > 0)
        motion.rois[0].p1_y = migration.motion_roi_1_y;
}
