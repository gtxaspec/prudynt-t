#pragma once

#include <set>
#include <atomic>
#include <chrono>
#include <iostream>
#include <functional>
#include <libconfig.h++>
#include <sys/time.h>
#include <any>

//~65k
#define ENABLE_LOG_DEBUG

//~10k
//#define AUDIO_SUPPORT

//disable tunings (debugging)
//#define NO_TUNINGS

#define IMP_AUTO_VALUE 16384
#define OSD_AUTO_VALUE 16384
#define THREAD_SLEEP 100000
#define STREAM_POLLING_TIMEOUT 100
#define GET_STREAM_BLOCKING false
//#define NO_FIFO

//Some more debug output not usefull for users
#define DDEBUG

#if defined(PLATFORM_T31)
#define DEFAULT_ENC_MODE_0 "FIXQP"
#define DEFAULT_ENC_MODE_1 "CAPPED_QUALITY"
#define DEFAULT_BUFFERS_0 4
#define DEFAULT_BUFFERS_1 2
#define DEFAULT_SINTER 128
#define DEFAULT_TEMPER 128
#define DEFAULT_SINTER_VALIDATE validateInt255
#define DEFAULT_TEMPER_VALIDATE validateInt255
#else
#define DEFAULT_ENC_MODE_0 "SMART"
#define DEFAULT_ENC_MODE_1 "SMART"
#define DEFAULT_BUFFERS_0 2
#define DEFAULT_BUFFERS_1 2
#define DEFAULT_SINTER 50
#define DEFAULT_TEMPER 50
#define DEFAULT_SINTER_VALIDATE validateInt50_150
#define DEFAULT_TEMPER_VALIDATE validateInt50_150
#endif

struct roi{
    int p0_x;
    int p0_y;
    int p1_x;
    int p1_y;
};

template<typename T>
struct ConfigItem {
    const char *path;
    T& value;
    T defaultValue;
    std::function<bool(const T&)> validate;
    bool noSave;
    const char *procPath;
};

struct _stream_stats {
    uint32_t bps;
	uint8_t fps;
	struct timeval ts;
};

struct _regions {
    int time;
    int user;
    int uptime;
    int logo;
};
struct _general {
    const char *loglevel;
    int osd_pool_size;
};
struct _rtsp {
    int port;
    int est_bitrate;
    int out_buffer_size;
    int send_buffer_size;		
    bool auth_required;
    const char *username;
    const char *password;
    const char *name;
};
struct _sensor {
    int fps;
    int width;
    int height;
    const char *model;
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
struct _osd {            
    int font_size;
    int font_stroke_size;
    int font_xscale;
    int font_yscale;
    int font_stroke;
    int font_yoffset;
    int logo_height;
    int logo_width;
    int pos_time_x;
    int pos_time_y;
    int time_transparency;
    int time_rotation;
    int pos_user_text_x;
    int pos_user_text_y;
    int user_text_transparency;
    int user_text_rotation;
    int pos_uptime_x;
    int pos_uptime_y;
    int uptime_transparency;
    int uptime_rotation;
    int pos_logo_x;
    int pos_logo_y;
    int logo_transparency;
    int logo_rotation;            
    bool enabled;            
    bool time_enabled;
    bool user_text_enabled;
    bool uptime_enabled;
    bool logo_enabled;
    bool font_stroke_enabled;            
    const char *font_path;
    const char *time_format;
    const char *uptime_format;
    const char *user_text_format;
    const char *logo_path;
    unsigned int font_color;
    unsigned int font_stroke_color;
    _regions regions;
    _stream_stats stats;
    std::atomic<int> thread_signal;
};  
struct _stream {
    int gop;
    int max_gop;
    int fps;
    int buffers;
    int width;
    int height;
    int profile;
    int bitrate;
    int rotation;
    int scale_width;
    int scale_height;
    bool enabled;
    bool scale_enabled;
    const char *mode;
    const char *rtsp_endpoint;
    const char *format;
    /* JPEG stream*/
    int jpeg_quality;
    int jpeg_refresh;
    int jpeg_channel;
    const char *jpeg_path;
    _osd osd;
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
    const char *script_path;
    std::array<roi, 52> rois;
};
struct _websocket {
    bool enabled;
    bool secured;
    int port;
    int loglevel;
    const char *name;
};

class CFG {
	public:

        bool config_loaded = false;
        libconfig::Config lc{};
        std::string filePath{};

		CFG();
        static CFG *createNew();
        bool readConfig();
        bool updateConfig();

		_general general{};
		_rtsp rtsp{};
		_sensor sensor{};
        _image image{};
#if defined(AUDIO_SUPPORT)           
        _audio audio{};
#endif
		_stream stream0{};
        _stream stream1{};
		_stream stream2{};
		_motion motion{};
        _websocket websocket{};

        std::atomic<int> main_thread_signal{1};

        //initialized stopped so that the worker can start it initially
        char volatile rtsp_thread_signal{2};

        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> motion_thread_signal{1};

        // bit 1 = init, 2 = running, 4 = stop, 8 stopped, 256 = exit
        std::atomic<int> worker_thread_signal{1};
        
    template <typename T>
    T get(const std::string &name) {
        T result;
        std::vector<ConfigItem<T>> *items = nullptr;
        if constexpr (std::is_same_v<T, bool>) {
            items = &boolItems;
        } else if constexpr (std::is_same_v<T, const char*>) {
            items = &charItems;
        } else if constexpr (std::is_same_v<T, int>) {
            items = &intItems;
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            items = &uintItems;
        } else {
            return result;
        }
        for (auto &item : *items) {
            if (item.path == name) {
                return item.value;
            }
        }
        return result;
    }

    template <typename T>
    bool set(const std::string &name, T value, bool noSave = false) {
        //std::cout << name << "=" << value << std::endl;
        std::vector<ConfigItem<T>> *items = nullptr;
        if constexpr (std::is_same_v<T, bool>) {
            items = &boolItems;
        } else if constexpr (std::is_same_v<T, const char*>) {
            items = &charItems;
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
                    item.noSave = noSave;
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

    bool initImageValue(const char *name, const unsigned char value) {
        for (auto &item : intItems) {
            if (item.path == name) {
                item.defaultValue = (int)value;
                if (!item.validate(item.value)) {
                    item.value = item.defaultValue;
                    return false;
                } else {
                    if(item.value == (int)value) {
                        return false;
                    } else {
                        //value is valid and not default. So we want set it.
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private:

        std::vector<ConfigItem<bool>> boolItems{};
        std::vector<ConfigItem<const char *>> charItems{};
        std::vector<ConfigItem<int>> intItems{};
        std::vector<ConfigItem<unsigned int>> uintItems{};

        std::vector<ConfigItem<bool>> getBoolItems();
        std::vector<ConfigItem<const char *>> getCharItems() ;
        std::vector<ConfigItem<int>> getIntItems();
        std::vector<ConfigItem<unsigned int>> getUintItems();
};
