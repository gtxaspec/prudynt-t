#pragma once

#include <map>
#include <tuple>
#include <string>
#include <atomic>
#include <variant>
#include <iostream>
#include <functional>
#include <libconfig.h++>

struct intEntry {
	int& value;
    int defaultValue;
    std::function<bool(const int&)> isValid;
	std::string message;
	std::string procPath;
};

struct bolEntry {
	bool& value;
    bool defaultValue;
    std::function<bool(const bool&)> isValid;
	std::string message;
	std::string procPath;
};

struct strEntry {
	std::string& value;
    std::string defaultValue;
    std::function<bool(const std::string&)> isValid;
	std::string message;	
	std::string procPath;
};

struct uintEntry {
	unsigned int& value;
    unsigned int defaultValue;
    std::function<bool(const unsigned int&)> isValid;
	std::string message;	
	std::string procPath;
};

class CFG {
	private:
		struct _general {
			std::string loglevel;
		};
		struct _rtsp {
			int port;
			int est_bitrate;
			int out_buffer_size;
			int send_buffer_size;		
			bool auth_required;
			std::string username;
			std::string password;
			std::string name;
		};
		struct _sensor {
			int fps;
			int width;
			int height;
			std::string model;
			unsigned int i2c_address;
		};
		struct _stream0 {
			int gop;
			int max_gop;
			int fps;
			int buffers;
			int width;
			int height;
			int bitrate;
			int osd_pos_time_x;
			int osd_pos_time_y;
			int osd_time_transparency;
            int osd_time_rotation;
			int osd_pos_user_text_x;
			int osd_pos_user_text_y;
			int osd_user_text_transparency;
            int osd_user_text_rotation;
			int osd_pos_uptime_x;
			int osd_pos_uptime_y;
			int osd_uptime_transparency;
            int osd_uptime_rotation;
			int osd_pos_logo_x;
			int osd_pos_logo_y;
			int osd_logo_transparency;
            int osd_logo_rotation;
			int rotation;
			int scale_width;
			int scale_height;
			int jpeg_quality;
			int jpeg_refresh;
			bool scale_enabled;
			std::string rtsp_endpoint;
			std::string format;
		};	
		struct _stream1 {
			int jpeg_quality;
			int jpeg_refresh;
			bool jpeg_enabled;
			std::string jpeg_path;

		};
		struct _osd {
			int font_size;
			int font_stroke_size;
			int logo_height;
			int logo_width;
			bool enabled;
			bool logo_enabled;
			bool time_enabled;
			bool user_text_enabled;
			bool font_stroke_enabled;
			bool uptime_enabled;
			std::string font_path;
			std::string time_format;
			std::string uptime_format;
			std::string user_text_format;
			std::string logo_path;
			unsigned int font_color;
			unsigned int font_stroke_color;
		};
		struct _motion {
			int debounce_time;
			int post_time;
			int cooldown_time;
			int init_time;
			int sensitivity;
			int skip_frame_count;
			int frame_width;
			int frame_height;
			int roi_0_x;
			int roi_0_y;
			int roi_1_x;
			int roi_1_y;
			int roi_count;
			bool enabled;
			std::string script_path;
		};

        bool config_loaded = false;
        libconfig::Config lc;

    public:

		CFG();
        bool readConfig();
        bool updateConfig();

		_general general;
		_rtsp rtsp;
		_sensor sensor;
		_stream0 stream0;
		_stream1 stream1;
		_osd osd;
		_motion motion;

        std::atomic<int> main_thread_signal{1};

        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> encoder_thread_signal{1};
        std::atomic<int> jpg_thread_signal{1};
        char volatile rtsp_thread_signal{0};

        template <typename T>
        T get(const std::string& key) {
            auto it = settings.find(key);
            if (it == settings.end()) {
                std::cout << "[ERROR:config.hpp] Key not found: " << key << std::endl;
                T r; return r;
            } else {
                if constexpr (std::is_same_v<T, int>) {
                    return std::get<intEntry>(it->second).value;
                } else if constexpr (std::is_same_v<T, bool>) {
                    return std::get<bolEntry>(it->second).value;
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return std::get<strEntry>(it->second).value;
                } else if constexpr (std::is_same_v<T, unsigned int>) {
                    return std::get<uintEntry>(it->second).value;
                } else {
                    std::cout << "[ERROR:config.hpp] Unsupported type for key: " << key << std::endl;
                }
            }
        }

        template <typename T>
        void set(const std::string& key, const T& value) {
            auto it = settings.find(key);
            if (it == settings.end()) {
                std::cout << "[ERROR:config.hpp] Key not found: " << key << std::endl;
                return;
            }

            if constexpr (std::is_same_v<T, int>) {
                auto& entry = std::get<intEntry>(it->second);
                if (entry.isValid(value)) {
                    entry.value = value;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid int value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, bool>) {
                auto& entry = std::get<bolEntry>(it->second);
                if (entry.isValid(value)) {
                    entry.value = value;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid bool value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                auto& entry = std::get<strEntry>(it->second);
                if (entry.isValid(value)) {
                    entry.value = value;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid str value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                auto& entry = std::get<uintEntry>(it->second);
                if (entry.isValid(value)) {
                    entry.value = value;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid uint value for key: " << key << std::endl;
                }
            } else {
                std::cout << "[ERROR:config.hpp] Unsupported type for key: " << key << std::endl;
            }
        }
        
        std::map<std::string, std::variant<intEntry, bolEntry, strEntry, uintEntry>> settings = {
			{"general.loglevel",		strEntry{general.loglevel, "INFO", [](const std::string &v) { return !v.empty(); }, ""}},
			
			{"rtsp.port", 				intEntry{rtsp.port, 554, [](const int &v) { return v > 0 && v <= 65535; }, "RTSP port must be between 1 and 65535", ""}},
            {"rtsp.auth_required",      bolEntry{rtsp.auth_required, true, [](const bool &v) { return true; }, "RTSP authentication required flag. Must be either true or false.", ""}},
            {"rtsp.username",           strEntry{rtsp.username, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP username cannot be empty.", ""}},
            {"rtsp.password",           strEntry{rtsp.password, "thingino", [](const std::string &v) { return !v.empty(); }, "RTSP password cannot be empty.", ""}},
			{"rtsp.name", 				strEntry{rtsp.name, "thingino prudynt", [](const std::string &v) { return !v.empty(); }, "RTSP realm name cannot be empty.", ""}},
			{"rtsp.est_bitrate", 		intEntry{rtsp.est_bitrate, 5000, [](const int &v) { return v > 0; }, "Estimated bitrate for RTSP streaming, should be greater than 0.", ""}},
			{"rtsp.out_buffer_size", 	intEntry{rtsp.out_buffer_size, 500000, [](const int &v) { return v > 0; }, "Buffer size for outgoing RTSP data, should be greater than 0.", ""}},
			{"rtsp.send_buffer_size", 	intEntry{rtsp.send_buffer_size, 307200, [](const int &v) { return v > 0; }, "Buffer size for sending RTSP data, should be greater than 0.", ""}},

            {"sensor.model",			strEntry{sensor.model, "gc2053", [](const std::string &v) { return !v.empty(); }, "Sensor model identifier cannot be empty.", "/proc/jz/sensor/name"}},
			{"sensor.fps", 				intEntry{sensor.fps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Sensor FPS must be between 1 and 60", "/proc/jz/sensor/max_fps"}},
			{"sensor.width", 			intEntry{sensor.width, 1920, [](const int &v) { return v > 0; }, "Width of the sensor's image in pixels", "/proc/jz/sensor/width"}},
			{"sensor.height", 			intEntry{sensor.height, 1080, [](const int &v) { return v > 0; }, "Height of the sensor's image in pixels", "/proc/jz/sensor/height"}},
			{"sensor.i2c_address", 		uintEntry{sensor.i2c_address, 0x37, [](const unsigned int &v) { return v <= 0x7F; }, "I2C address of the sensor. Must be between 0x00 and 0x7F.", "/proc/jz/sensor/i2c_addr"}},

			{"stream0.rtsp_endpoint",   strEntry{stream0.rtsp_endpoint, "ch0", [](const std::string &v) { return !v.empty(); }, "RTSP endpoint cannot be empty.", ""}},
            {"stream0.scale_enabled",   bolEntry{stream0.scale_enabled, false, [](const bool &v) { return true; }, "Scaling for Stream0 enabled flag. Must be either true or false.", ""}},
            {"stream0.format",          strEntry{stream0.format, "H264", [](const std::string &v) { return v == "H264" || v == "H265"; }, "Stream format must be either 'H264' or 'H265'.", ""}},
			{"stream0.gop", 			intEntry{stream0.gop, 0, [](const int &v) { return v > 0; }, "Group of pictures (GOP) size for stream 0", ""}},
			{"stream0.max_gop", 		intEntry{stream0.max_gop, 60, [](const int &v) { return v > 0; }, "Maximum GOP size for stream 0, should not be less than 'stream0.gop'", ""}},
			{"stream0.fps", 			intEntry{stream0.fps, 24, [](const int &v) { return v > 0 && v <= 60; }, "Stream 0 FPS must be between 1 and 60", ""}},
			{"stream0.buffers", 		intEntry{stream0.buffers, 2, [](const int &v) { return v > 0 && v <= 32; }, "Number of buffers for stream 0, must be between 1 and 32", ""}},
			{"stream0.width", 			intEntry{stream0.width, 1920, [](const int &v) { return v > 0; }, "Width of stream 0 in pixels", "/proc/jz/sensor/width"}},
			{"stream0.height", 			intEntry{stream0.height, 1080, [](const int &v) { return v > 0; }, "Height of stream 0 in pixels", "/proc/jz/sensor/height"}},
			{"stream0.bitrate", 		intEntry{stream0.bitrate, 1000, [](const int &v) { return v > 0; }, "Bitrate for stream 0, must be greater than 0", ""}},
			{"stream0.osd_pos_time_x", 	intEntry{stream0.osd_pos_time_x, 15, [](const int &v) { return v >= -15360 && v <= 15360; }, "X position for OSD in stream 0", ""}},
			{"stream0.osd_pos_time_y", 	intEntry{stream0.osd_pos_time_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; }, "Y position for OSD in stream 0", ""}},
			{"stream0.osd_time_transparency", intEntry{stream0.osd_time_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, "Transparency for time OSD in stream 0", ""}},
            {"stream0.osd_time_rotation", intEntry{stream0.osd_time_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; }, "Angle for time OSD rotation in stream 0", ""}},
			{"stream0.osd_pos_user_text_x", intEntry{stream0.osd_pos_user_text_x, 0, [](const int &v) { return v >= -15360 && v <= 15360; }, "X position for user text OSD in stream 0", ""}},
			{"stream0.osd_pos_user_text_y", intEntry{stream0.osd_pos_user_text_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; }, "Y position for user text OSD in stream 0", ""}},
			{"stream0.osd_user_text_transparency", intEntry{stream0.osd_user_text_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, "Transparency for user text OSD in stream 0", ""}},
            {"stream0.osd_user_text_rotation", intEntry{stream0.osd_user_text_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; }, "Angle for user text OSD rotation in stream 0", ""}},            
			{"stream0.osd_pos_uptime_x", intEntry{stream0.osd_pos_uptime_x, -15, [](const int &v) { return v >= -15360 && v <= 15360; }, "X position for uptime OSD in stream 0", ""}},
			{"stream0.osd_pos_uptime_y", intEntry{stream0.osd_pos_uptime_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; }, "Y position for uptime OSD in stream 0", ""}},
			{"stream0.osd_uptime_transparency", intEntry{stream0.osd_uptime_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, "Transparency for uptime OSD in stream 0", ""}},
            {"stream0.osd_uptime_rotation", intEntry{stream0.osd_uptime_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; }, "Angle for uptime OSD rotation in stream 0", ""}},            
			{"stream0.osd_pos_logo_x", 	intEntry{stream0.osd_pos_logo_x, -15, [](const int &v) { return v >= -15360 && v <= 15360; }, "X position for logo OSD in stream 0", ""}},
			{"stream0.osd_pos_logo_y", 	intEntry{stream0.osd_pos_logo_y, -10, [](const int &v) { return v >= -15360 && v <= 15360; }, "Y position for logo OSD in stream 0", ""}},
			{"stream0.osd_logo_transparency", intEntry{stream0.osd_logo_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, "Transparency for logo OSD in stream 0", ""}},
            {"stream0.osd_logo_rotation", intEntry{stream0.osd_logo_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; }, "Angle for logo OSD rotation in stream 0", ""}},            
			{"stream0.rotation", 		intEntry{stream0.rotation, 0, [](const int &v) { return v >= 0 && v <= 2; }, "Stream 0 rotation must be 0, 1, or 2", ""}},
			{"stream0.scale_width", 	intEntry{stream0.scale_width, 640, [](const int &v) { return v > 0; }, "Stream 0 scale width should be greater than 0", ""}},
			{"stream0.scale_height", 	intEntry{stream0.scale_height, 360, [](const int &v) { return v > 0; }, "Stream 0 scale height should be greater than 0", ""}},

			{"stream1.jpeg_enabled",    bolEntry{stream1.jpeg_enabled, true, [](const bool &v) { return true; }, "JPEG stream for Stream0 enabled flag. Must be either true or false.", ""}},
            {"stream1.jpeg_path",		strEntry{stream1.jpeg_path, "/tmp/snapshot.jpg", [](const std::string &v) { return !v.empty(); }, "Path for JPEG snapshots must not be empty.", ""}},
			{"stream1.jpeg_quality", 	intEntry{stream1.jpeg_quality, 75, [](const int &v) { return v > 0 && v <= 100; }, "Stream 0 jpeg quality must be between 1 and 100", ""}},
			{"stream1.jpeg_refresh", 	intEntry{stream1.jpeg_refresh, 1000, [](const int &v) { return v > 0; }, "Stream 0 jpeg refresh rate should be greater than 0", ""}},

			{"osd.enabled",             bolEntry{osd.enabled, true, [](const bool &v) { return true; }, "OSD (On-Screen Display) enabled flag. Must be either true or false.", ""}},
            {"osd.logo_enabled",        bolEntry{osd.logo_enabled, true, [](const bool &v) { return true; }, "OSD logo display enabled flag. Must be either true or false.", ""}},
            {"osd.time_enabled",        bolEntry{osd.time_enabled, true, [](const bool &v) { return true; }, "OSD time display enabled flag. Must be either true or false.", ""}},
            {"osd.user_text_enabled",   bolEntry{osd.user_text_enabled, true, [](const bool &v) { return true; }, "OSD user text display enabled flag. Must be either true or false.", ""}},
            {"osd.font_stroke_enabled", bolEntry{osd.font_stroke_enabled, true, [](const bool &v) { return true; }, "OSD font stroke (outline) display enabled flag. Must be either true or false.", ""}},
            {"osd.uptime_enabled",      bolEntry{osd.uptime_enabled, true, [](const bool &v) { return true; }, "OSD uptime display enabled flag. Must be either true or false.", ""}},
            {"osd.font_path",           strEntry{osd.font_path, "/usr/share/fonts/UbuntuMono-Regular2.ttf", [](const std::string &v) { return !v.empty(); }, "Font path for OSD cannot be empty.", ""}},
            {"osd.time_format",         strEntry{osd.time_format, "%I:%M:%S%p %m/%d/%Y", [](const std::string &v) { return !v.empty(); }, "OSD time format string cannot be empty.", ""}},
            {"osd.uptime_format",       strEntry{osd.uptime_format, "Uptime: %02lu:%02lu:%02lu", [](const std::string &v) { return !v.empty(); }, "OSD uptime format string cannot be empty.", ""}},
            {"osd.user_text_format",    strEntry{osd.user_text_format, "thingino", [](const std::string &v) { return true; }, "Custom user text for OSD; can be empty.", ""}},
            {"osd.logo_path",           strEntry{osd.logo_path, "/usr/share/thingino_logo_1.bgra", [](const std::string &v) { return !v.empty(); }, "OSD Logo path cannot be empty.", ""}},
			{"osd.font_size", 			intEntry{osd.font_size, 64, [](const int &v) { return v > 0; }, "OSD font size should be greater than 0", ""}},
			{"osd.font_stroke_size", 	intEntry{osd.font_stroke_size, 64, [](const int &v) { return v >= 0; }, "OSD font stroke size should be non-negative", ""}},
			{"osd.logo_height", 		intEntry{osd.logo_height, 30, [](const int &v) { return v > 0; }, "OSD logo height should be greater than 0", ""}},
			{"osd.logo_width", 			intEntry{osd.logo_width, 100, [](const int &v) { return v > 0; }, "OSD logo width should be greater than 0", ""}},
			{"osd.font_color", 			uintEntry{osd.font_color, 0xFFFFFFFF, [](const unsigned int &v) { return true; }, "OSD font color in ARGB format. Default is white.", ""}},
			{"osd.font_stroke_color", 	uintEntry{osd.font_stroke_color, 0xFF000000, [](const unsigned int &v) { return true; }, "OSD font stroke (outline) color in ARGB format. Default is black.", ""}},

            {"motion.enabled",          bolEntry{motion.enabled, false, [](const bool &v) { return true; }, "Motion detection enabled flag. Must be either true or false.", ""}},
            {"motion.script_path",      strEntry{motion.script_path, "/usr/sbin/motion", [](const std::string &v) { return !v.empty(); }, "Motion detection script path cannot be empty.", ""}},
			{"motion.debounce_time", 	intEntry{motion.debounce_time, 0, [](const int &v) { return v >= 0; }, "Motion debounce time should be non-negative", ""}},
			{"motion.post_time", 		intEntry{motion.post_time, 0, [](const int &v) { return v >= 0; }, "Motion post time should be non-negative", ""}},
			{"motion.cooldown_time", 	intEntry{motion.cooldown_time, 5, [](const int &v) { return v >= 0; }, "Motion cooldown time should be non-negative", ""}},
			{"motion.init_time", 		intEntry{motion.init_time, 5, [](const int &v) { return v >= 0; }, "Motion initialization time should be non-negative", ""}},
			{"motion.sensitivity", 		intEntry{motion.sensitivity, 1, [](const int &v) { return v >= 0; }, "Motion sensitivity should be non-negative"}},
			{"motion.skip_frame_count", intEntry{motion.skip_frame_count, 5, [](const int &v) { return v >= 0; }, "Motion skip frame count should be non-negative", ""}},
			{"motion.frame_width", 		intEntry{motion.frame_width, 1920, [](const int &v) { return v > 0; }, "Motion frame width should be greater than 0", ""}},
			{"motion.frame_height", 	intEntry{motion.frame_width, 1080, [](const int &v) { return v > 0; }, "Motion frame height should be greater than 0", ""}},
			{"motion.roi_0_x", 			intEntry{motion.roi_0_x, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 X position should be non-negative", ""}},
			{"motion.roi_0_y", 			intEntry{motion.roi_0_y, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 Y position should be non-negative", ""}},
			{"motion.roi_1_x", 			intEntry{motion.roi_1_x, 1920, [](const int &v) { return v >= 0; }, "Motion ROI 1 X position should be non-negative", ""}},
			{"motion.roi_1_y", 			intEntry{motion.roi_1_y, 1080, [](const int &v) { return v >= 0; }, "Motion ROI 1 Y position should be non-negative", ""}},
			{"motion.roi_count", 		intEntry{motion.roi_count, 1, [](const int &v) { return v >= 0 && v <= 4; }, "Motion ROI count should be between 0 and 4", ""}},						
		};
};

class Config {
public:
    // We only need one instance of Config
    static Config* singleton();

private:
    // Private constructor to prevent instantiation
    Config();
public:
    // Public configuration settings
    std::string sensorModel;
    unsigned int sensorI2Caddress;
    int sensorFps;
    int sensorHeight;
    int sensorWidth;

    std::string stream0format;
    int stream0fps;
    int stream0gop;
    int stream0maxGop;
    int stream0buffers;
    int stream0height;
    int stream0width;
    int stream0bitrate;
    int stream0osdPosTimeX;
    int stream0osdPosTimeY;
    int stream0osdTimeAlpha;
    int stream0osdUserTextPosX;
    int stream0osdUserTextPosY;
    int stream0osdUserTextAlpha;
    int stream0osdUptimeStampPosX;
    int stream0osdUptimeStampPosY;
    int stream0osdUptimeAlpha;
    int stream0osdLogoPosX;
    int stream0osdLogoPosY;
    int stream0osdLogoAlpha;
    std::string stream1jpegPath;
    bool stream1jpegEnable;
    bool stream0scaleEnable;
    std::string stream0endpoint;
    int stream1jpegQuality;
    int stream1jpegRefresh;
    int stream0rotation;
    int stream0scaleWidth;
    int stream0scaleHeight;
    int motionDebounce;
    int motionPostTime;
    int motionCooldownTime;
    std::string motionScriptPath;
    int motionInitTime;
    int motionSensitivity;
    int motionSkipFrameCnt;
    int motionFrameWidth;
    int motionFrameHeight;
    int motionRoi0X;
    int motionRoi0Y;
    int motionRoi1X;
    int motionRoi1Y;
    int roiCnt;
    bool motionEnable;

    std::string rtspUsername;
    std::string rtspPassword;
    std::string rtspName;
    int rtspPort;
    bool rtspAuthRequired;
    int rtspEstBitRate;
    int rtspOutBufferSize;
    int rtspSendBufferSize;

    std::string OSDFontPath;
    std::string OSDFormat;
    std::string OSDUptimeFormat;
    std::string OSDLogoPath;
    int OSDFontSize;
    int OSDFontStrokeSize;
    int OSDLogoWidth;
    int OSDLogoHeight;
    bool OSDEnable;
    unsigned int OSDFontColor;
    unsigned int OSDFontStrokeColor;
    bool OSDFontStrokeEnable;
    bool OSDUserTextEnable;
    std::string OSDUserTextString;
    bool OSDUptimeEnable;
    std::string logLevel;
    bool OSDLogoEnable;
    bool OSDTimeEnable;
private:
    // Holds the singleton instance
    static Config* instance;
};
