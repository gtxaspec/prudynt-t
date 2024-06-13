#pragma once

#include <map>
#include <set>
#include <string>
#include <atomic>
#include <variant>
#include <iostream>
#include <functional>
#include <libconfig.h++>

//~65k
#define ENABLE_LOG_DEBUG

//~10k
//#define AUDIO_SUPPORT

//~13k
#define OLD_CONFIG

//testing second stream as lowres
#define LOW_STREAM

struct roi{
    int p0_x;
    int p0_y;
    int p1_x;
    int p1_y;
};

#if defined(OLD_CONFIG)
template<typename T>
struct ConfigItem {
    std::string path;
    T& value;
    T defaultValue;
    std::function<bool(const T&)> validate;
    std::string procPath;
};
#else
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
#endif

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
		struct _image {
            int contrast;
            int sharpness;
            int saturation;
            int brightness;
            int hue;
            int sinter_strength;
            int temper_strength;
            bool vflip;
            bool hflip;
            int running_mode;
            int anti_flicker;
            int ae_compensation;
            int dpc_strength;
            int defog_strength;
            int drc_strength;
            int highlight_depress;
            int backlight_compensation;
            int max_again;
            int max_dgain;
            int core_wb_mode;
            int wb_rgain;
            int wb_bgain;

        };
#if defined(AUDIO_SUPPORT)        
        struct _audio {
            bool input_enabled;
            int input_vol;
            int input_gain;
            int input_alc_gain;
            bool output_enabled;            
            int output_vol;
            int output_gain;
            int input_noise_suppression;            
            bool input_echo_cancellation;
            bool input_high_pass_filter;
            bool output_high_pass_filter;
        };
#endif        
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
			bool time_enabled;
			bool user_text_enabled;
			bool uptime_enabled;
            bool logo_enabled;
			bool font_stroke_enabled;            
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
            int thread_wait;
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
            std::array<roi, 52> rois;
		};
        struct _websocket {
            bool enabled;
            bool secured;
            int port;
            int loglevel;
            std::string name;
        };
        bool config_loaded = false;
        libconfig::Config lc;
        std::string filePath;
        
    public:

		CFG();
        bool readConfig();
        bool updateConfig();

		_general general;
		_rtsp rtsp;
		_sensor sensor;
        _image image;
#if defined(AUDIO_SUPPORT)           
        _audio audio;
#endif
		_stream0 stream0;
		_stream1 stream1;
		_osd osd;
		_motion motion;
        _websocket websocket;

        std::atomic<int> main_thread_signal{1};

        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> encoder_thread_signal{1};
        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> jpg_thread_signal{1};
        // bit 0 = start, 1 = stop, 2 = stopped, 256 = exit
        char volatile rtsp_thread_signal{0};
        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> motion_thread_signal{1};

#if defined(OLD_CONFIG)

    template <typename T>
    T get(const std::string &name) {
        T result;
        std::vector<ConfigItem<T>> *items = nullptr;
        if constexpr (std::is_same_v<T, bool>) {
            items = &boolItems;
        } else if constexpr (std::is_same_v<T, std::string>) {
            items = &stringItems;
        } else if constexpr (std::is_same_v<T, int>) {
            items = &intItems;
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            items = &uintItems;
        } else {
            return result; // Unsupported type
        }
        for (auto &item : *items) {
            if (item.path == name) {
                return item.value;
            }
        }
        return result; // Wenn nicht gefunden
    }

    template <typename T>
    bool set(const std::string &name, T value) {
        std::vector<ConfigItem<T>> *items = nullptr;
        if constexpr (std::is_same_v<T, bool>) {
            items = &boolItems;
        } else if constexpr (std::is_same_v<T, std::string>) {
            items = &stringItems;
        } else if constexpr (std::is_same_v<T, int>) {
            items = &intItems;
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            items = &uintItems;
        } else {
            return false;
        }
        for (auto &item : *items) {
            if (item.path == name) {
                if (item.validate(value)) {
                    item.value = value;
                    return true; 
                } else {
                    return false;
                }
            }
        }
        return false;
    }

        std::vector<std::string> missingConfigs;

        std::vector<ConfigItem<bool>> boolItems = {
            {"rtsp.auth_required",      rtsp.auth_required, true, [](const bool &v) { return true; }, ""},
            {"image.vflip", 	        image.vflip, false, [](const bool &v) { return true; }, ""},
            {"image.hflip", 	        image.hflip, false, [](const bool &v) { return true; }, ""},
#if defined(WITH_AUDIO)            
            {"audio.input_enabled",	    audio.input_enabled, false, [](const bool &v) { return true; }, ""},
            {"audio.output_enabled",	audio.output_enabled, false, [](const bool &v) { return true; }, ""},
			{"audio.input_echo_cancellation", audio.input_echo_cancellation, false, [](const bool &v) { return true; }, ""},
            {"audio.output_high_pass_filter", audio.input_high_pass_filter, false, [](const bool &v) { return true; }, ""},
            {"audio.output_high_pass_filter", audio.output_high_pass_filter, false, [](const bool &v) { return true; }, +""},
#endif
            {"stream0.scale_enabled",   stream0.scale_enabled, false, [](const bool &v) { return true; }, ""},
			{"stream1.jpeg_enabled",    stream1.jpeg_enabled, true, [](const bool &v) { return true; }, ""},
			{"osd.enabled",             osd.enabled, true, [](const bool &v) { return true; }, ""},
            {"osd.logo_enabled",        osd.logo_enabled, true, [](const bool &v) { return true; }, ""},
            {"osd.time_enabled",        osd.time_enabled, true, [](const bool &v) { return true; }, ""},
            {"osd.user_text_enabled",   osd.user_text_enabled, true, [](const bool &v) { return true; }, ""},
            {"osd.font_stroke_enabled", osd.font_stroke_enabled, true, [](const bool &v) { return true; }, ""},
            {"osd.uptime_enabled",      osd.uptime_enabled, true, [](const bool &v) { return true; }, ""},
            {"motion.enabled",          motion.enabled, false, [](const bool &v) { return true; }, ""},
			{"websocket.enabled",       websocket.enabled, true, [](const bool &v) { return true; }, ""},
            {"websocket.secured",       websocket.secured, false, [](const bool &v) { return true; }, ""},
        };
        
        std::vector<ConfigItem<std::string>> stringItems = {
			{"general.loglevel",		general.loglevel, "INFO", [](const std::string &v) { std::set<std::string> a = {"EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"}; return a.count(v)==1; }, ""},
            {"rtsp.username",           rtsp.username, "thingino", [](const std::string &v) { return !v.empty(); },""},
            {"rtsp.password",           rtsp.password, "thingino", [](const std::string &v) { return !v.empty(); }, ""},
			{"rtsp.name", 				rtsp.name, "thingino prudynt", [](const std::string &v) { return !v.empty(); },""},
            {"sensor.model",			sensor.model, "gc2053", [](const std::string &v) { return !v.empty(); }, "/proc/jz/sensor/name"},
			{"stream0.rtsp_endpoint",   stream0.rtsp_endpoint, "ch0", [](const std::string &v) { return !v.empty(); }, ""},
            {"stream0.format",          stream0.format, "H264", [](const std::string &v) { return v == "H264" || v == "H265"; },""},
            {"stream1.jpeg_path",		stream1.jpeg_path, "/tmp/snapshot.jpg", [](const std::string &v) { return !v.empty(); }, ""},
            {"osd.font_path",           osd.font_path, "/usr/share/fonts/UbuntuMono-Regular2.ttf", [](const std::string &v) { return !v.empty(); }, ""},
            {"osd.time_format",         osd.time_format, "%I:%M:%S%p %m/%d/%Y", [](const std::string &v) { return !v.empty(); }, ""},
            {"osd.uptime_format",       osd.uptime_format, "Uptime: %02lu:%02lu:%02lu", [](const std::string &v) { return !v.empty(); }, ""},
            {"osd.user_text_format",    osd.user_text_format, "thingino", [](const std::string &v) { return true; },  ""},
            {"osd.logo_path",           osd.logo_path, "/usr/share/thingino_logo_1.bgra", [](const std::string &v) { return !v.empty(); }, ""},
            {"motion.script_path",      motion.script_path, "/usr/sbin/motion", [](const std::string &v) { return !v.empty(); },""},
			{"websocket.name", 	        websocket.name, "wss prudynt", [](const std::string &v) { return !v.empty(); },  ""},
        };

        std::vector<ConfigItem<int>> intItems = {
			{"rtsp.port", 				rtsp.port, 554, [](const int &v) { return v > 0 && v <= 65535; }, ""},
			{"rtsp.est_bitrate", 		rtsp.est_bitrate, 5000, [](const int &v) { return v > 0; }, ""},
			{"rtsp.out_buffer_size", 	rtsp.out_buffer_size, 500000, [](const int &v) { return v > 0; },  ""},
			{"rtsp.send_buffer_size", 	rtsp.send_buffer_size, 307200, [](const int &v) { return v > 0; },  ""}, 
			{"sensor.fps", 				sensor.fps, 24, [](const int &v) { return v > 0 && v <= 60; },"/proc/jz/sensor/max_fps"},
			{"sensor.width", 			sensor.width, 1920, [](const int &v) { return v > 0; },  "/proc/jz/sensor/width"},
			{"sensor.height", 			sensor.height, 1080, [](const int &v) { return v > 0; },"/proc/jz/sensor/height"},
            {"image.brightness",		image.brightness, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
			{"image.contrast", 			image.contrast, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
			{"image.hue",   			image.hue, 128, [](const int &v) { return v >= 0 && v <= 255; },  ""},
			{"image.sharpness", 		image.sharpness, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
			{"image.saturation", 		image.saturation, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
			{"image.sinter_strength", 	image.sinter_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
			{"image.temper_strength", 	image.temper_strength, 128, [](const int &v) { return v >= 0 && v <= 255; },""},
            {"image.running_mode",		image.running_mode, 0, [](const int &v) { return v >= 0 && v <= 1; },  ""},
            {"image.anti_flicker",   	image.anti_flicker, 2, [](const int &v) { return v >= 0 && v <= 2; }, ""},
            {"image.ae_compensation",	image.ae_compensation, 128, [](const int &v) { return v >= 0 && v <= 255; }, ""},
            {"image.dpc_strength",   	image.dpc_strength, 128, [](const int &v) { return v >= 0 && v <= 255; },  ""},
            {"image.defog_strength",    image.defog_strength, 128, [](const int &v) { return v >= 0 && v <= 255; },""},
            {"image.drc_strength",      image.drc_strength, 128, [](const int &v) { return v >= 0 && v <= 255; },  ""},
            {"image.highlight_depress", image.highlight_depress, 0, [](const int &v) { return v >= 0 && v <= 255; },  ""},
            {"image.backlight_compensation", image.backlight_compensation, 0, [](const int &v) { return v >= 0 && v <= 10; },  ""},
            {"image.max_again",         image.max_again, 160, [](const int &v) { return v >= 0 && v <= 160; }, ""},
            {"image.max_dgain",         image.max_dgain, 80, [](const int &v) { return v >= 0 && v <= 160; },""},
            {"image.core_wb_mode",      image.core_wb_mode, 0, [](const int &v) { return v >= 0 && v <= 9; },  ""},
            {"image.wb_rgain",          image.wb_rgain, 0, [](const int &v) { return v >= 0 && v <= 34464; },""},
            {"image.wb_bgain",          image.wb_bgain, 0, [](const int &v) { return v >= 0 && v <= 34464; },""},
#if defined(WITH_AUDIO)            
            {"audio.input_vol",	        audio.input_vol, 0, [](const int &v) { return v >= -30 && v <= 120; },""},
			{"audio.input_gain", 		audio.input_gain, 0, [](const int &v) { return v >= 0 && v <= 31; }, ""},
			{"audio.input_alc_gain",   	audio.input_alc_gain, 0, [](const int &v) { return v >= 0 && v <= 7; }, ""},
			{"audio.output_vol", 		audio.output_vol, 0, [](const int &v) { return v >= -30 && v <= 120; },  ""},
			{"audio.output_gain", 		audio.output_gain, 0, [](const int &v) { return v >= 0 && v <= 31; },  ""},
			{"audio.input_noise_suppression", audio.input_noise_suppression, 0, [](const int &v) { return v >= 0 && v <= 3; }, ""},
#endif            
			{"stream0.gop", 			stream0.gop, 20, [](const int &v) { return v > 0; }, ""},
			{"stream0.max_gop", 		stream0.max_gop, 60, [](const int &v) { return v > 0; }, ""},
			{"stream0.fps", 			stream0.fps, 24, [](const int &v) { return v > 0 && v <= 60; }, ""},
			{"stream0.buffers", 		stream0.buffers, 1, [](const int &v) { return v > 0 && v <= 32; }, ""},
			{"stream0.width", 			stream0.width, 1920, [](const int &v) { return v > 0; },  "/proc/jz/sensor/width"},
			{"stream0.height", 			stream0.height, 1080, [](const int &v) { return v > 0; },"/proc/jz/sensor/height"},
			{"stream0.bitrate", 		stream0.bitrate, 1000, [](const int &v) { return v > 0; }, ""},
			{"stream0.osd_pos_time_x", 	stream0.osd_pos_time_x, 15, [](const int &v) { return v >= -15360 && v <= 15360; },""},
			{"stream0.osd_pos_time_y", 	stream0.osd_pos_time_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; }, ""},
			{"stream0.osd_time_transparency", stream0.osd_time_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, ""},
            {"stream0.osd_time_rotation", stream0.osd_time_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; },  ""},
			{"stream0.osd_pos_user_text_x", stream0.osd_pos_user_text_x, 0, [](const int &v) { return v >= -15360 && v <= 15360; }, ""},
			{"stream0.osd_pos_user_text_y", stream0.osd_pos_user_text_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; },  ""},
			{"stream0.osd_user_text_transparency", stream0.osd_user_text_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; },  ""},
            {"stream0.osd_user_text_rotation", stream0.osd_user_text_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; },  ""},            
			{"stream0.osd_pos_uptime_x", stream0.osd_pos_uptime_x, -15, [](const int &v) { return v >= -15360 && v <= 15360; },  ""},
			{"stream0.osd_pos_uptime_y", stream0.osd_pos_uptime_y, 10, [](const int &v) { return v >= -15360 && v <= 15360; },  ""},
			{"stream0.osd_uptime_transparency", stream0.osd_uptime_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, ""},
            {"stream0.osd_uptime_rotation", stream0.osd_uptime_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; },  ""},            
			{"stream0.osd_pos_logo_x", 	stream0.osd_pos_logo_x, -15, [](const int &v) { return v >= -15360 && v <= 15360; },""},
			{"stream0.osd_pos_logo_y", 	stream0.osd_pos_logo_y, -10, [](const int &v) { return v >= -15360 && v <= 15360; },  ""},
			{"stream0.osd_logo_transparency", stream0.osd_logo_transparency, 255, [](const int &v) { return v >= 0 && v <= 255; }, ""},
            {"stream0.osd_logo_rotation", stream0.osd_logo_rotation, 0, [](const int &v) { return v >= 0 && v <= 360; }, ""},            
			{"stream0.rotation", 		stream0.rotation, 0, [](const int &v) { return v >= 0 && v <= 2; }, ""},
			{"stream0.scale_width", 	stream0.scale_width, 640, [](const int &v) { return v > 0; }, ""},
			{"stream0.scale_height", 	stream0.scale_height, 360, [](const int &v) { return v > 0; },""},
			{"stream1.jpeg_quality", 	stream1.jpeg_quality, 75, [](const int &v) { return v > 0 && v <= 100; }, ""},
			{"stream1.jpeg_refresh", 	stream1.jpeg_refresh, 1000, [](const int &v) { return v > 0; }, ""},
			{"osd.font_size", 			osd.font_size, 32, [](const int &v) { return v > 0; }, ""},
			{"osd.font_stroke_size", 	osd.font_stroke_size, 32, [](const int &v) { return v >= 0; },""},
			{"osd.logo_height", 		osd.logo_height, 30, [](const int &v) { return v > 0; },  ""},
			{"osd.logo_width", 			osd.logo_width, 100, [](const int &v) { return v > 0; },""},
			{"motion.debounce_time", 	motion.debounce_time, 0, [](const int &v) { return v >= 0; },""},
			{"motion.post_time", 		motion.post_time, 0, [](const int &v) { return v >= 0; },""},
            {"motion.thread_wait", 		motion.thread_wait, 5000, [](const int &v) { return v >= 1000 && v <= 10000; }, ""},
			{"motion.cooldown_time", 	motion.cooldown_time, 5, [](const int &v) { return v >= 0; },  ""},
			{"motion.init_time", 		motion.init_time, 5, [](const int &v) { return v >= 0; },  ""},
			{"motion.sensitivity", 		motion.sensitivity, 1, [](const int &v) { return v >= 0; }, ""},
			{"motion.skip_frame_count", motion.skip_frame_count, 5, [](const int &v) { return v >= 0; }, ""},
			{"motion.frame_width", 		motion.frame_width, 1920, [](const int &v) { return v > 0; },  ""},
			{"motion.frame_height", 	motion.frame_height, 1080, [](const int &v) { return v > 0; },  ""},
			{"motion.roi_0_x", 			motion.roi_0_x, 0, [](const int &v) { return v >= 0; }, ""},
			{"motion.roi_0_y", 			motion.roi_0_y, 0, [](const int &v) { return v >= 0; },""},
			{"motion.roi_1_x", 			motion.roi_1_x, 1920, [](const int &v) { return v >= 0; },""},
			{"motion.roi_1_y", 			motion.roi_1_y, 1080, [](const int &v) { return v >= 0; },  ""},
			{"motion.roi_count", 		motion.roi_count, 1, [](const int &v) { return v >= 1 && v <= 52; },""},						
            {"websocket.port", 			websocket.port, 8089, [](const int &v) { return v > 0 && v <= 65535; },""},
            {"websocket.loglevel", 		websocket.loglevel, 4096, [](const int &v) { return v > 0 && v <= 1024; }, ""},
        };

        std::vector<ConfigItem<unsigned int>> uintItems = {
			{"sensor.i2c_address", 		sensor.i2c_address, 0x37, [](const unsigned int &v) { return v <= 0x7F; },  "/proc/jz/sensor/i2c_addr"},
			{"osd.font_color", 			osd.font_color, 0xFFFFFFFF, [](const unsigned int &v) { return true; }, ""},
			{"osd.font_stroke_color", 	osd.font_stroke_color, 0xFF000000, [](const unsigned int &v) { return true; },""},
        };
#else
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
        bool set(const std::string& key, const T& value) {
            auto it = settings.find(key);
            if (it == settings.end()) {
                std::cout << "[ERROR:config.hpp] Key not found: " << key << std::endl;
            }

            if constexpr (std::is_same_v<T, int>) {
                auto& entry = std::get<intEntry>(it->second);
                if (entry.isValid(value) && entry.value != value) {
                    entry.value = value; 
                    return true;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid int value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, bool>) {
                auto& entry = std::get<bolEntry>(it->second);
                if (entry.isValid(value) && entry.value != value) {
                    entry.value = value;
                    return true;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid bool value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, std::string>) {
                auto& entry = std::get<strEntry>(it->second);
                if (entry.isValid(value) && entry.value != value) {
                    entry.value = value;
                    return true;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid str value for key: " << key << std::endl;
                }
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                auto& entry = std::get<uintEntry>(it->second);
                if (entry.isValid(value) && entry.value != value) {
                    entry.value = value;
                    return true;
                } else {
                    std::cout << "[ERROR:config.hpp] Invalid uint value for key: " << key << std::endl;
                }
            } else {
                std::cout << "[ERROR:config.hpp] Unsupported type for key: " << key << std::endl;
            }
            return false;
        }
        
        std::map<std::string, std::variant<intEntry, bolEntry, strEntry, uintEntry>> settings = {
			{"general.loglevel",		strEntry{general.loglevel, "INFO", [](const std::string &v) { std::set<std::string> a = {"EMERGENCY", "ALERT", "CRITICAL", "ERROR", "WARN", "NOTICE", "INFO", "DEBUG"}; return a.count(v)==1; }, ""}},
			
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

            {"image.brightness",		intEntry{image.brightness, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Brightness must between 0 and 255", ""}},
			{"image.contrast", 			intEntry{image.contrast, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Contrast must between 0 and 255", ""}},
			{"image.hue",   			intEntry{image.hue, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Hue must between 0 and 255", ""}},
			{"image.sharpness", 		intEntry{image.sharpness, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Sharpness must between 0 and 255", "/"}},
			{"image.saturation", 		intEntry{image.saturation, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Saturation must between 0 and 255", ""}},
			{"image.sinter_strength", 	intEntry{image.sinter_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Sinter strength must between 0 and 255", ""}},
			{"image.temper_strength", 	intEntry{image.temper_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, "Temper strength must between 0 and 255", ""}},
            {"image.vflip", 	        bolEntry{image.vflip, false, [](const bool &v) { return true; }, "vflip value can only be true or false", ""}},
            {"image.hflip", 	        bolEntry{image.hflip, false, [](const bool &v) { return true; }, "hflip value can only be true or false", ""}},
            {"image.running_mode",		intEntry{image.running_mode, 0, [](const int &v) { return v >= 0 && v <= 1; }, "Running mode must between 0 = day and 1 = night", ""}},
            {"image.anti_flicker",   	intEntry{image.anti_flicker, 2, [](const int &v) { return v >= 0 && v <= 2; }, "Anti flicker must be 0 = disabled, 1 = 50Hz or 2 = 60Hz", ""}},
            {"image.ae_compensation",	intEntry{image.ae_compensation, 128, [](const int &v) { return v >= 0 && v <= 255; }, "AE Compensation must between 0 and 255", ""}},
            {"image.dpc_strength",   	intEntry{image.dpc_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, "DPC Strength must between 0 and 255", ""}},
            {"image.defog_strength",    intEntry{image.defog_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, "defog_strength must between 0 and 255", ""}},
            {"image.drc_strength",      intEntry{image.drc_strength, 128, [](const int &v) { return v >= 0 && v <= 255; }, "DRC strength must between 0 and 255", ""}},
            {"image.highlight_depress", intEntry{image.highlight_depress, 0, [](const int &v) { return v >= 0 && v <= 255; }, "Highlight depress must between 0 and 10", ""}},
            {"image.backlight_compensation", intEntry{image.backlight_compensation, 0, [](const int &v) { return v >= 0 && v <= 10; }, "Backlight compenstation must between 0 and 10", ""}},
            {"image.max_again",         intEntry{image.max_again, 160, [](const int &v) { return v >= 0 && v <= 160; }, "AGAIN must between 0 and 160", ""}},
            {"image.max_dgain",         intEntry{image.max_dgain, 80, [](const int &v) { return v >= 0 && v <= 160; }, "DGAIN must between 0 and 160", ""}},
            {"image.core_wb_mode",      intEntry{image.core_wb_mode, 0, [](const int &v) { return v >= 0 && v <= 9; }, "WB (White Balance) must between 0 and 255", ""}},
            {"image.wb_rgain",          intEntry{image.wb_rgain, 0, [](const int &v) { return v >= 0 && v <= 34464; }, "defog_strength must between 0 and 34464", ""}},
            {"image.wb_bgain",          intEntry{image.wb_bgain, 0, [](const int &v) { return v >= 0 && v <= 34464; }, "defog_strength must between 0 and 34464", ""}},
#if defined(AUDIO_SUPPORT)   
            {"audio.input_enabled",	    bolEntry{audio.input_enabled, false, [](const bool &v) { return true; }, "Audio input enabled must be true or false", ""}},
            {"audio.input_vol",	        intEntry{audio.input_vol, 0, [](const int &v) { return v >= -30 && v <= 120; }, "Audio input volume must between -30 and 120", ""}},
			{"audio.input_gain", 		intEntry{audio.input_gain, 0, [](const int &v) { return v >= 0 && v <= 31; }, "Audio input gain must between 0 and 30", ""}},
			{"audio.input_alc_gain",   	intEntry{audio.input_alc_gain, 0, [](const int &v) { return v >= 0 && v <= 7; }, "Audio input alc gain between 0 and 7", ""}},
            {"audio.output_enabled",	bolEntry{audio.output_enabled, false, [](const bool &v) { return true; }, "Audio output enabled must be true or false", ""}},
			{"audio.output_vol", 		intEntry{audio.output_vol, 0, [](const int &v) { return v >= -30 && v <= 120; }, "Audio output volume must between -30 and 120", "/"}},
			{"audio.output_gain", 		intEntry{audio.output_gain, 0, [](const int &v) { return v >= 0 && v <= 31; }, "Audio output gain must between 0 and 31", ""}},
			{"audio.input_echo_cancellation", bolEntry{audio.input_echo_cancellation, false, [](const bool &v) { return true; }, "Audio echo cancellation can only be true or false", ""}},
			{"audio.input_noise_suppression", intEntry{audio.input_noise_suppression, 0, [](const int &v) { return v >= 0 && v <= 3; }, "Audio noise suppression must between 0 and 3", ""}},
            {"audio.output_high_pass_filter", bolEntry{audio.input_high_pass_filter, false, [](const bool &v) { return true; }, "Audio input high pass filter can only be true or false", ""}},
            {"audio.output_high_pass_filter", bolEntry{audio.output_high_pass_filter, false, [](const bool &v) { return true; }, "Audio output high pass filter can only be true or false", ""}},
#endif
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
            {"motion.thread_wait", 		intEntry{motion.thread_wait, 5000, [](const int &v) { return v >= 1000 && v <= 10000; }, "Motion detection wait time.", ""}},
			{"motion.cooldown_time", 	intEntry{motion.cooldown_time, 5, [](const int &v) { return v >= 0; }, "Motion cooldown time should be non-negative", ""}},
			{"motion.init_time", 		intEntry{motion.init_time, 5, [](const int &v) { return v >= 0; }, "Motion initialization time should be non-negative", ""}},
			{"motion.sensitivity", 		intEntry{motion.sensitivity, 1, [](const int &v) { return v >= 0; }, "Motion sensitivity should be non-negative"}},
			{"motion.skip_frame_count", intEntry{motion.skip_frame_count, 5, [](const int &v) { return v >= 0; }, "Motion skip frame count should be non-negative", ""}},
			{"motion.frame_width", 		intEntry{motion.frame_width, 1920, [](const int &v) { return v > 0; }, "Motion frame width should be greater than 0", ""}},
			{"motion.frame_height", 	intEntry{motion.frame_height, 1080, [](const int &v) { return v > 0; }, "Motion frame height should be greater than 0", ""}},
			{"motion.roi_0_x", 			intEntry{motion.roi_0_x, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 X position should be non-negative", ""}},
			{"motion.roi_0_y", 			intEntry{motion.roi_0_y, 0, [](const int &v) { return v >= 0; }, "Motion ROI 0 Y position should be non-negative", ""}},
			{"motion.roi_1_x", 			intEntry{motion.roi_1_x, 1920, [](const int &v) { return v >= 0; }, "Motion ROI 1 X position should be non-negative", ""}},
			{"motion.roi_1_y", 			intEntry{motion.roi_1_y, 1080, [](const int &v) { return v >= 0; }, "Motion ROI 1 Y position should be non-negative", ""}},
			{"motion.roi_count", 		intEntry{motion.roi_count, 1, [](const int &v) { return v >= 1 && v <= 52; }, "Motion ROI count should be between 1 and 52", ""}},						

			{"websocket.enabled",       bolEntry{websocket.enabled, true, [](const bool &v) { return true; }, "Websocket enabled flag. Must be either true or false.", ""}},
            {"websocket.secured",       bolEntry{websocket.secured, false, [](const bool &v) { return true; }, "Websocket security flag. Must be either true or false.", ""}},
            {"websocket.port", 			intEntry{websocket.port, 8089, [](const int &v) { return v > 0 && v <= 65535; }, "Websocket port must be between 1 and 65535", ""}},
            {"websocket.loglevel", 		intEntry{websocket.loglevel, 4096, [](const int &v) { return v > 0 && v <= 1024; }, "Websocket loglevel must be between 1 and 12", ""}},
			{"websocket.name", 	        strEntry{websocket.name, "wss prudynt", [](const std::string &v) { return !v.empty(); }, "Websocket name cannot be empty.", ""}},

		};
#endif
};
