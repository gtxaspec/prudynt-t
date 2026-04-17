#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <json-c/json.h>
#include "Config.hpp"
#include "Logger.hpp"

#define WEBSOCKET_TOKEN_LENGTH 32

#define MODULE "CONFIG"

namespace fs = std::filesystem;

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

// Utility function to validate hexadecimal color format (#RRGGBBAA)
bool isValidHexColor(const char* str) {
    if (!str || strlen(str) != 9) {
        return false;
    }

    if (str[0] != '#') {
        return false;
    }

    for (int i = 1; i < 9; i++) {
        char c = str[i];
        if (!((c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'F') ||
              (c >= 'a' && c <= 'f'))) {
            return false;
        }
    }

    return true;
}

// Utility function to convert RGBA hexadecimal color string to unsigned int
unsigned int hexColorToUint(const char* str) {
    if (!isValidHexColor(str)) {
        return 0;
    }

    // Parse RGBA format: #RRGGBBAA
    // Extract each component separately to handle RGBA format correctly
    char rStr[3] = {str[1], str[2], '\0'};  // RR
    char gStr[3] = {str[3], str[4], '\0'};  // GG
    char bStr[3] = {str[5], str[6], '\0'};  // BB
    char aStr[3] = {str[7], str[8], '\0'};  // AA

    unsigned int r = strtoul(rStr, nullptr, 16);
    unsigned int g = strtoul(gStr, nullptr, 16);
    unsigned int b = strtoul(bStr, nullptr, 16);
    unsigned int a = strtoul(aStr, nullptr, 16);

    // Pack into ARGB format for internal use (A in bits 24-31, R in 16-23, G in 8-15, B in 0-7)
    // This matches the bit extraction logic used in OSD::drawText()
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// Validation function for OSD color fields that accepts both integer and hex string formats
bool validateOSDColor(const unsigned int &v) {
    // For unsigned int values, any value is valid (same as validateUint)
    return true;
}

// Validation function for OSD color fields when parsing from JSON string
bool validateOSDColorString(const char* str) {
    if (!str) {
        return false;
    }

    // Check if it's a valid hexadecimal color format
    return isValidHexColor(str);
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
        {"audio.output_enabled", audio.output_enabled, true, validateBool},
        {"audio.force_stereo", audio.force_stereo, false, validateBool},
#if defined(LIB_AUDIO_PROCESSING)
        {"audio.input_high_pass_filter", audio.input_high_pass_filter, false, validateBool},
        {"audio.input_agc_enabled", audio.input_agc_enabled, false, validateBool},
#endif
#endif
        {"image.isp_bypass", image.isp_bypass, true, validateBool},
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
        {"sensor.model", sensor.model, "unknown", validateCharNotEmpty, false, "/proc/jz/sensor/name"},
        {"sensor.chip_id", sensor.chip_id, "unknown", validateCharNotEmpty, false, "/proc/jz/sensor/chip_id"},
        {"sensor.version", sensor.version, "unknown", validateCharNotEmpty, false, "/proc/jz/sensor/version"},
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
        {"websocket.token", websocket.token, "auto", [](const char *v) {
            std::string token(v);
            return token == "auto" || token.empty() || token.length() == WEBSOCKET_TOKEN_LENGTH;
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
        {"motion.roi_0_x", motion.roi_0_x, 0, validateIntGe0},
        {"motion.roi_0_y", motion.roi_0_y, 0, validateIntGe0},
        {"motion.roi_1_x", motion.roi_1_x, IVS_AUTO_VALUE, validateIntGe0},
        {"motion.roi_1_y", motion.roi_1_y, IVS_AUTO_VALUE, validateIntGe0},
        {"motion.roi_count", motion.roi_count, 1, [](const int &v) { return v >= 1 && v <= 52; }},
        {"rtsp.est_bitrate", rtsp.est_bitrate, 5000, validateIntGe0},
        {"rtsp.out_buffer_size", rtsp.out_buffer_size, 500000, validateIntGe0},
        {"rtsp.port", rtsp.port, 554, validateInt65535},
        {"rtsp.send_buffer_size", rtsp.send_buffer_size, 307200, validateIntGe0},
        {"rtsp.session_reclaim", rtsp.session_reclaim, 65, validateIntGe0},
        {"sensor.i2c_bus", sensor.i2c_bus, 0, validateIntGe0, false, "/proc/jz/sensor/i2c_bus"},
        {"sensor.fps", sensor.fps, 25, validateInt120, false, "/proc/jz/sensor/max_fps"},
        {"sensor.min_fps", sensor.min_fps, 5, validateInt120, false, "/proc/jz/sensor/min_fps"},
        {"sensor.height", sensor.height, 1080, validateIntGe0, false, "/proc/jz/sensor/height"},
        {"sensor.width", sensor.width, 1920, validateIntGe0, false, "/proc/jz/sensor/width"},
        {"sensor.boot", sensor.boot, 0, validateIntGe0, false, "/proc/jz/sensor/boot"},
        {"sensor.mclk", sensor.mclk, 1, validateIntGe0, false, "/proc/jz/sensor/mclk"},
        {"sensor.video_interface", sensor.video_interface, 0, validateIntGe0, false, "/proc/jz/sensor/video_interface"},
        {"sensor.gpio_reset", sensor.gpio_reset, -1, [](const int &v) { return v >= -1; }, false, "/proc/jz/sensor/reset_gpio"},
        {"stream0.bitrate", stream0.bitrate, 3000, validateIntGe0},
        {"stream0.buffers", stream0.buffers, DEFAULT_BUFFERS_0, [](const int &v) { return v >= 1 && v <= 8; }},
        {"stream0.fps", stream0.fps, 25, validateInt120},
        {"stream0.gop", stream0.gop, 20, validateIntGe0},
        {"stream0.height", stream0.height, 1080, validateIntGe0},
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
        {"stream0.osd.start_delay", stream0.osd.start_delay, 0, [](const int &v) { return v >= 0 && v <= 5000; }},
        {"stream0.osd.time_rotation", stream0.osd.time_rotation, 0, validateInt360},
        {"stream0.osd.uptime_rotation", stream0.osd.uptime_rotation, 0, validateInt360},
        {"stream0.osd.user_text_rotation", stream0.osd.user_text_rotation, 0, validateInt360},
        {"stream0.rotation", stream0.rotation, 0, validateInt2},
        {"stream0.width", stream0.width, 1920, validateIntGe0},
        {"stream0.profile", stream0.profile, 2, validateInt2},
        {"stream1.bitrate", stream1.bitrate, 1000, validateIntGe0},
        {"stream1.buffers", stream1.buffers, DEFAULT_BUFFERS_1, [](const int &v) { return v >= 1 && v <= 8; }},
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
        {"stream1.osd.start_delay", stream1.osd.start_delay, 0, [](const int &v) { return v >= 0 && v <= 5000; }},
        {"stream1.osd.time_rotation", stream1.osd.time_rotation, 0, validateInt360},
        {"stream1.osd.uptime_rotation", stream1.osd.uptime_rotation, 0, validateInt360},
        {"stream1.osd.user_text_rotation", stream1.osd.user_text_rotation, 0, validateInt360},
        {"stream1.rotation", stream1.rotation, 0, validateInt2},
        {"stream1.width", stream1.width, 640, validateIntGe0},
        {"stream1.profile", stream1.profile, 2, validateInt2},
        {"stream2.jpeg_channel", stream2.jpeg_channel, 0, validateIntGe0},
        {"stream2.jpeg_quality", stream2.jpeg_quality, 75, [](const int &v) { return v > 0 && v <= 100; }},
        {"stream2.jpeg_idle_fps", stream2.jpeg_idle_fps, 1, [](const int &v) { return v >= 0 && v <= 30; }},
        {"stream2.fps", stream2.fps, 25, [](const int &v) { return v > 1 && v <= 30; }},
        {"websocket.port", websocket.port, 8089, validateInt65535},
        {"websocket.first_image_delay", websocket.first_image_delay, 100, validateInt65535},
    };
};

std::vector<ConfigItem<unsigned int>> CFG::getUintItems()
{
    return {
        {"sensor.i2c_address", sensor.i2c_address, 0x37, [](const unsigned int &v) { return v <= 0x7F; }, false, "/proc/jz/sensor/i2c_addr"},
        // Individual color settings for stream0 text elements
        {"stream0.osd.time_font_color", stream0.osd.time_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream0.osd.time_font_stroke_color", stream0.osd.time_font_stroke_color, 0xFF000000, validateOSDColor},
        {"stream0.osd.uptime_font_color", stream0.osd.uptime_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream0.osd.uptime_font_stroke_color", stream0.osd.uptime_font_stroke_color, 0xFF000000, validateOSDColor},
        {"stream0.osd.user_text_font_color", stream0.osd.user_text_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream0.osd.user_text_font_stroke_color", stream0.osd.user_text_font_stroke_color, 0xFF000000, validateOSDColor},
        // Individual color settings for stream1 text elements
        {"stream1.osd.time_font_color", stream1.osd.time_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream1.osd.time_font_stroke_color", stream1.osd.time_font_stroke_color, 0xFF000000, validateOSDColor},
        {"stream1.osd.uptime_font_color", stream1.osd.uptime_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream1.osd.uptime_font_stroke_color", stream1.osd.uptime_font_stroke_color, 0xFF000000, validateOSDColor},
        {"stream1.osd.user_text_font_color", stream1.osd.user_text_font_color, 0xFFFFFFFF, validateOSDColor},
        {"stream1.osd.user_text_font_stroke_color", stream1.osd.user_text_font_stroke_color, 0xFF000000, validateOSDColor},
    };
};

bool CFG::readConfig()
{
    // Clean up any existing JSON object
    if (jsonConfig) {
        json_object_put(jsonConfig);
        jsonConfig = nullptr;
    }

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.json";
    filePath = cfgFilePath;

    // Try to load the configuration file from the specified paths
    std::string configPath;

    // First try the binary directory
    if (fs::exists(cfgFilePath)) {
        configPath = cfgFilePath.string();
        LOG_INFO("Loaded configuration from " + configPath);
    } else {
        // Try /etc/prudynt.json
        fs::path etcPath = "/etc/prudynt.json";
        filePath = etcPath;

        if (fs::exists(etcPath)) {
            configPath = etcPath.string();
            LOG_INFO("Loaded configuration from " + configPath);
        } else {
            LOG_WARN("Failed to load prudynt configuration file from both locations.");
            return false; // Exit if configuration file is missing
        }
    }

    // Parse JSON file using json-c
    jsonConfig = json_object_from_file(configPath.c_str());
    if (!jsonConfig) {
        LOG_WARN("JSON parse error: Failed to parse " + configPath);
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

// Helper function to check if this is a sensor parameter with proc path
template <typename T>
bool isSensorProcParameter(const ConfigItem<T> &item) {
    return item.procPath != nullptr &&
           item.procPath[0] != '\0' &&
           std::string(item.procPath).find("/proc/jz/sensor/") == 0;
}

template <typename T>
void handleConfigItem(json_object *jsonConfig, ConfigItem<T> &item)
{
    bool readFromProc = false;
    bool readFromConfig = false;

    if (!jsonConfig) return;

    // For sensor parameters with proc paths, prioritize proc files over JSON
    if (isSensorProcParameter(item)) {
        // Try proc file first for sensor parameters
        std::ifstream procFile(item.procPath);
        if (procFile) {
            T value;
            std::string line;
            if (std::getline(procFile, line)) {
                if (processLine(line, value)) {
                    if constexpr (std::is_same_v<T, const char *>) {
                        item.value = strdup(value);
                    } else {
                        item.value = value;
                    }
                    readFromProc = true;
                }
            }
        }
    }

    // Only read from JSON if proc file failed or this is not a sensor proc parameter
    if (!readFromProc) {
        // Parse the path to navigate through nested JSON objects
        std::vector<std::string> pathParts;
        std::string path = item.path;
        size_t pos = 0;
        while ((pos = path.find('.')) != std::string::npos) {
            pathParts.push_back(path.substr(0, pos));
            path.erase(0, pos + 1);
        }
        pathParts.push_back(path);

        // Navigate through the JSON structure
        json_object *currentJson = jsonConfig;
        for (size_t i = 0; i < pathParts.size() - 1; ++i) {
            json_object *nextObj = nullptr;
            if (json_object_object_get_ex(currentJson, pathParts[i].c_str(), &nextObj)) {
                if (json_object_is_type(nextObj, json_type_object)) {
                    currentJson = nextObj;
                } else {
                    return; // Path doesn't exist or not an object
                }
            } else {
                return; // Path doesn't exist
            }
        }

        // Try to read the value from JSON
        const std::string& finalKey = pathParts.back();
        json_object *valueObj = nullptr;
        if (json_object_object_get_ex(currentJson, finalKey.c_str(), &valueObj)) {
            if constexpr (std::is_same_v<T, const char *>) {
                if (json_object_is_type(valueObj, json_type_string)) {
                    const char *str = json_object_get_string(valueObj);
                    item.value = strdup(str);
                    readFromConfig = true;
                }
            } else if constexpr (std::is_same_v<T, bool>) {
                if (json_object_is_type(valueObj, json_type_boolean)) {
                    item.value = json_object_get_boolean(valueObj);
                    readFromConfig = true;
                }
            } else if constexpr (std::is_same_v<T, int>) {
                if (json_object_is_type(valueObj, json_type_int)) {
                    item.value = json_object_get_int(valueObj);
                    readFromConfig = true;
                }
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                if (json_object_is_type(valueObj, json_type_int)) {
                    int64_t val = json_object_get_int64(valueObj);
                    if (val >= 0) {
                        item.value = static_cast<unsigned int>(val);
                        readFromConfig = true;
                    }
                } else if (json_object_is_type(valueObj, json_type_string)) {
                    // Check if this is an OSD color field that might be in hex format
                    std::string path = item.path;
                    if (path.find("font_color") != std::string::npos ||
                        path.find("font_stroke_color") != std::string::npos) {
                        const char *str = json_object_get_string(valueObj);
                        if (isValidHexColor(str)) {
                            item.value = hexColorToUint(str);
                            readFromConfig = true;
                        }
                    }
                }
            } else if constexpr (std::is_same_v<T, float>) {
                if (json_object_is_type(valueObj, json_type_double)) {
                    item.value = static_cast<float>(json_object_get_double(valueObj));
                    readFromConfig = true;
                } else if (json_object_is_type(valueObj, json_type_int)) {
                    item.value = static_cast<float>(json_object_get_int(valueObj));
                    readFromConfig = true;
                }
            }
        }
    }

    // For non-sensor parameters, try proc file as fallback if JSON failed
    if (!readFromConfig && !readFromProc && !isSensorProcParameter(item) &&
        item.procPath != nullptr && item.procPath[0] != '\0') {
        // Attempt to read from the proc filesystem
        std::ifstream procFile(item.procPath);
        if (procFile) {
            T value;
            std::string line;
            if (std::getline(procFile, line)) {
                if (processLine(line, value)) {
                    if constexpr (std::is_same_v<T, const char *>) {
                        item.value = strdup(value);
                    } else {
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
void handleConfigItem2(json_object *jsonConfig, ConfigItem<T> &item)
{
    if (!jsonConfig) return;

    // Parse the path to navigate through nested JSON objects
    std::vector<std::string> pathParts;
    std::string path = item.path;
    size_t pos = 0;
    while ((pos = path.find('.')) != std::string::npos) {
        pathParts.push_back(path.substr(0, pos));
        path.erase(0, pos + 1);
    }
    pathParts.push_back(path);

    // Navigate through the JSON structure, creating objects as needed
    json_object *currentJson = jsonConfig;
    for (size_t i = 0; i < pathParts.size() - 1; ++i) {
        json_object *nextObj = nullptr;
        if (!json_object_object_get_ex(currentJson, pathParts[i].c_str(), &nextObj)) {
            // Create new object if it doesn't exist
            nextObj = json_object_new_object();
            json_object_object_add(currentJson, pathParts[i].c_str(), nextObj);
        }
        currentJson = nextObj;
    }

    // Set the final value
    const std::string& finalKey = pathParts.back();
    json_object *valueObj = nullptr;

    if constexpr (std::is_same_v<T, const char *>) {
        valueObj = json_object_new_string(item.value);
    } else if constexpr (std::is_same_v<T, bool>) {
        valueObj = json_object_new_boolean(item.value);
    } else if constexpr (std::is_same_v<T, int>) {
        valueObj = json_object_new_int(item.value);
    } else if constexpr (std::is_same_v<T, unsigned int>) {
        valueObj = json_object_new_int64(item.value);
    } else if constexpr (std::is_same_v<T, float>) {
        valueObj = json_object_new_double(item.value);
    }

    if (valueObj) {
        json_object_object_add(currentJson, finalKey.c_str(), valueObj);
    }
}

bool CFG::updateConfig()
{
    config_loaded = readConfig();

    if (!jsonConfig) return false;

    for (auto &item : boolItems)
        handleConfigItem2(jsonConfig, item);
    for (auto &item : charItems)
        handleConfigItem2(jsonConfig, item);
    for (auto &item : intItems)
        handleConfigItem2(jsonConfig, item);
    for (auto &item : uintItems)
        handleConfigItem2(jsonConfig, item);
    for (auto &item : floatItems)
        handleConfigItem2(jsonConfig, item);

    // Handle ROIs
    json_object *roisObj = nullptr;
    if (json_object_object_get_ex(jsonConfig, "rois", &roisObj)) {
        json_object_object_del(jsonConfig, "rois");
    }

    roisObj = json_object_new_object();
    json_object_object_add(jsonConfig, "rois", roisObj);

    for (int i = 0; i < motion.roi_count; i++)
    {
        std::string roiKey = "roi_" + std::to_string(i);
        json_object *roiArray = json_object_new_array();

        json_object_array_add(roiArray, json_object_new_int(motion.rois[i].p0_x));
        json_object_array_add(roiArray, json_object_new_int(motion.rois[i].p0_y));
        json_object_array_add(roiArray, json_object_new_int(motion.rois[i].p1_x));
        json_object_array_add(roiArray, json_object_new_int(motion.rois[i].p1_y));

        json_object_object_add(roisObj, roiKey.c_str(), roiArray);
    }

    // Write JSON to file
    const char *jsonString = json_object_to_json_string_ext(jsonConfig, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_NOSLASHESCAPE);
    if (jsonString) {
        std::ofstream configFile(filePath);
        if (configFile.is_open()) {
            configFile << jsonString;
            configFile.close();
            LOG_DEBUG("Config is written to " << filePath);
            return true;
        } else {
            LOG_ERROR("Failed to write config to " << filePath);
            return false;
        }
    } else {
        LOG_ERROR("Failed to serialize JSON config");
        return false;
    }
};

std::vector<ConfigItem<float>> CFG::getFloatItems()
{
    return {
        {"rtsp.packet_loss_threshold", rtsp.packet_loss_threshold, 0.05f, [](const float &v) { return v >= 0.0f && v <= 1.0f; }},
        {"rtsp.bandwidth_margin", rtsp.bandwidth_margin, 1.2f, [](const float &v) { return v >= 1.0f && v <= 3.0f; }},
    };
};

void CFG::migrateOldColorSettings()
{
    // Helper function to combine RGB color with alpha transparency
    // Creates ARGB format for internal use (matches OSD::drawText() bit extraction)
    auto combineColorWithAlpha = [](unsigned int rgb_color, int transparency) -> unsigned int {
        // Extract RGB components from the input color
        unsigned int r = (rgb_color >> 16) & 0xFF;
        unsigned int g = (rgb_color >> 8) & 0xFF;
        unsigned int b = rgb_color & 0xFF;

        // Combine with alpha (transparency is 0-255, where 255 = opaque)
        unsigned int alpha = (unsigned int)transparency & 0xFF;

        // Pack into ARGB format for internal use (A in bits 24-31)
        return (alpha << 24) | (r << 16) | (g << 8) | b;
    };

    // Check for old configuration format and migrate
    json_object *stream0Obj = nullptr;
    json_object *stream1Obj = nullptr;

    if (json_object_object_get_ex(jsonConfig, "stream0", &stream0Obj)) {
        json_object *osdObj = nullptr;
        if (json_object_object_get_ex(stream0Obj, "osd", &osdObj)) {
            // Check if old format exists (font_color + transparency fields)
            json_object *fontColorObj = nullptr;
            json_object *fontStrokeColorObj = nullptr;
            json_object *timeTransparencyObj = nullptr;
            json_object *uptimeTransparencyObj = nullptr;
            json_object *userTextTransparencyObj = nullptr;

            bool hasOldFormat = json_object_object_get_ex(osdObj, "font_color", &fontColorObj) &&
                               json_object_object_get_ex(osdObj, "font_stroke_color", &fontStrokeColorObj) &&
                               (json_object_object_get_ex(osdObj, "time_transparency", &timeTransparencyObj) ||
                                json_object_object_get_ex(osdObj, "uptime_transparency", &uptimeTransparencyObj) ||
                                json_object_object_get_ex(osdObj, "user_text_transparency", &userTextTransparencyObj));

            if (hasOldFormat) {
                unsigned int fontColor = 0xFFFFFFFF;  // Default white
                unsigned int fontStrokeColor = 0xFF000000;  // Default black
                int timeTransparency = 255;
                int uptimeTransparency = 255;
                int userTextTransparency = 255;

                // Read old values
                if (json_object_is_type(fontColorObj, json_type_int)) {
                    fontColor = json_object_get_int64(fontColorObj);
                } else if (json_object_is_type(fontColorObj, json_type_string)) {
                    const char *str = json_object_get_string(fontColorObj);
                    if (isValidHexColor(str)) {
                        fontColor = hexColorToUint(str);
                    }
                }

                if (json_object_is_type(fontStrokeColorObj, json_type_int)) {
                    fontStrokeColor = json_object_get_int64(fontStrokeColorObj);
                } else if (json_object_is_type(fontStrokeColorObj, json_type_string)) {
                    const char *str = json_object_get_string(fontStrokeColorObj);
                    if (isValidHexColor(str)) {
                        fontStrokeColor = hexColorToUint(str);
                    }
                }

                if (timeTransparencyObj && json_object_is_type(timeTransparencyObj, json_type_int)) {
                    timeTransparency = json_object_get_int(timeTransparencyObj);
                }
                if (uptimeTransparencyObj && json_object_is_type(uptimeTransparencyObj, json_type_int)) {
                    uptimeTransparency = json_object_get_int(uptimeTransparencyObj);
                }
                if (userTextTransparencyObj && json_object_is_type(userTextTransparencyObj, json_type_int)) {
                    userTextTransparency = json_object_get_int(userTextTransparencyObj);
                }

                // Create new individual color settings
                json_object_object_add(osdObj, "time_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, timeTransparency)));
                json_object_object_add(osdObj, "time_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, timeTransparency)));
                json_object_object_add(osdObj, "uptime_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, uptimeTransparency)));
                json_object_object_add(osdObj, "uptime_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, uptimeTransparency)));
                json_object_object_add(osdObj, "user_text_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, userTextTransparency)));
                json_object_object_add(osdObj, "user_text_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, userTextTransparency)));

                // Remove old settings
                json_object_object_del(osdObj, "font_color");
                json_object_object_del(osdObj, "font_stroke_color");
                json_object_object_del(osdObj, "time_transparency");
                json_object_object_del(osdObj, "uptime_transparency");
                json_object_object_del(osdObj, "user_text_transparency");
            }
        }
    }

    // Repeat for stream1
    if (json_object_object_get_ex(jsonConfig, "stream1", &stream1Obj)) {
        json_object *osdObj = nullptr;
        if (json_object_object_get_ex(stream1Obj, "osd", &osdObj)) {
            // Similar migration logic for stream1
            json_object *fontColorObj = nullptr;
            json_object *fontStrokeColorObj = nullptr;
            json_object *timeTransparencyObj = nullptr;
            json_object *uptimeTransparencyObj = nullptr;
            json_object *userTextTransparencyObj = nullptr;

            bool hasOldFormat = json_object_object_get_ex(osdObj, "font_color", &fontColorObj) &&
                               json_object_object_get_ex(osdObj, "font_stroke_color", &fontStrokeColorObj) &&
                               (json_object_object_get_ex(osdObj, "time_transparency", &timeTransparencyObj) ||
                                json_object_object_get_ex(osdObj, "uptime_transparency", &uptimeTransparencyObj) ||
                                json_object_object_get_ex(osdObj, "user_text_transparency", &userTextTransparencyObj));

            if (hasOldFormat) {
                unsigned int fontColor = 0xFFFFFFFF;
                unsigned int fontStrokeColor = 0xFF000000;
                int timeTransparency = 255;
                int uptimeTransparency = 255;
                int userTextTransparency = 255;

                // Read old values (similar to stream0)
                if (json_object_is_type(fontColorObj, json_type_int)) {
                    fontColor = json_object_get_int64(fontColorObj);
                } else if (json_object_is_type(fontColorObj, json_type_string)) {
                    const char *str = json_object_get_string(fontColorObj);
                    if (isValidHexColor(str)) {
                        fontColor = hexColorToUint(str);
                    }
                }

                if (json_object_is_type(fontStrokeColorObj, json_type_int)) {
                    fontStrokeColor = json_object_get_int64(fontStrokeColorObj);
                } else if (json_object_is_type(fontStrokeColorObj, json_type_string)) {
                    const char *str = json_object_get_string(fontStrokeColorObj);
                    if (isValidHexColor(str)) {
                        fontStrokeColor = hexColorToUint(str);
                    }
                }

                if (timeTransparencyObj && json_object_is_type(timeTransparencyObj, json_type_int)) {
                    timeTransparency = json_object_get_int(timeTransparencyObj);
                }
                if (uptimeTransparencyObj && json_object_is_type(uptimeTransparencyObj, json_type_int)) {
                    uptimeTransparency = json_object_get_int(uptimeTransparencyObj);
                }
                if (userTextTransparencyObj && json_object_is_type(userTextTransparencyObj, json_type_int)) {
                    userTextTransparency = json_object_get_int(userTextTransparencyObj);
                }

                // Create new individual color settings
                json_object_object_add(osdObj, "time_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, timeTransparency)));
                json_object_object_add(osdObj, "time_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, timeTransparency)));
                json_object_object_add(osdObj, "uptime_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, uptimeTransparency)));
                json_object_object_add(osdObj, "uptime_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, uptimeTransparency)));
                json_object_object_add(osdObj, "user_text_font_color",
                    json_object_new_int64(combineColorWithAlpha(fontColor, userTextTransparency)));
                json_object_object_add(osdObj, "user_text_font_stroke_color",
                    json_object_new_int64(combineColorWithAlpha(fontStrokeColor, userTextTransparency)));

                // Remove old settings
                json_object_object_del(osdObj, "font_color");
                json_object_object_del(osdObj, "font_stroke_color");
                json_object_object_del(osdObj, "time_transparency");
                json_object_object_del(osdObj, "uptime_transparency");
                json_object_object_del(osdObj, "user_text_transparency");
            }
        }
    }
}

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
    floatItems = getFloatItems();

    config_loaded = readConfig();

    if (jsonConfig) {
        // Handle backward compatibility migration first
        migrateOldColorSettings();

        for (auto &item : boolItems)
            handleConfigItem(jsonConfig, item);
        for (auto &item : charItems)
            handleConfigItem(jsonConfig, item);
        for (auto &item : intItems)
            handleConfigItem(jsonConfig, item);
        for (auto &item : uintItems)
            handleConfigItem(jsonConfig, item);
        for (auto &item : floatItems)
            handleConfigItem(jsonConfig, item);
    }

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

    // Handle ROIs from JSON
    if (jsonConfig) {
        json_object *roisObj = nullptr;
        if (json_object_object_get_ex(jsonConfig, "rois", &roisObj) &&
            json_object_is_type(roisObj, json_type_object))
        {
            for (int i = 0; i < motion.roi_count; i++)
            {
                std::string roiKey = "roi_" + std::to_string(i);
                json_object *roiArray = nullptr;
                if (json_object_object_get_ex(roisObj, roiKey.c_str(), &roiArray) &&
                    json_object_is_type(roiArray, json_type_array))
                {
                    int arrayLen = json_object_array_length(roiArray);
                    if (arrayLen == 4)
                    {
                        motion.rois[i].p0_x = json_object_get_int(json_object_array_get_idx(roiArray, 0));
                        motion.rois[i].p0_y = json_object_get_int(json_object_array_get_idx(roiArray, 1));
                        motion.rois[i].p1_x = json_object_get_int(json_object_array_get_idx(roiArray, 2));
                        motion.rois[i].p1_y = json_object_get_int(json_object_array_get_idx(roiArray, 3));
                    }
                }
            }
        }
    }
}
