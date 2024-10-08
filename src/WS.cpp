#include "WS.hpp"
#include <random>
#include <set>
#include <fstream>
#include <memory>
#include <variant>
#include "Config.hpp"
#include "libwebsockets.h"
#include <imp/imp_osd.h>
#include <imp/imp_isp.h>
#include <imp/imp_audio.h>
#include "OSD.hpp"
#include "worker.hpp"
#include "globals.hpp"
#include <filesystem>
#include <sys/inotify.h>

#include <iomanip>

#define MODULE "WEBSOCKET"

#pragma region keys_and_enums

using namespace std::chrono;
using namespace std::filesystem;
/*
    ToDo's
    add new font scales
    add new font stroke
    add stream buffer sharing
*/

/* PNT_WS_MSG */
enum
{
    PNT_WS_MSG_OK,
    PNT_WS_MSG_NULL,
    PNT_WS_MSG_ERROR,
    PNT_WS_MSG_INITIATED,
    PNT_WS_MSG_DROPPED,
    PNT_WS_MSG_TRUE,
    PNT_WS_MSG_FALSE,
    PNT_WS_MSG_UNSUPPORTED
};

static const char *const pnt_ws_msg[] = {
    "ok",
    "null",
    "error",
    "initiated",
    "dropped",
    "true",
    "false",
    "not supported on this plattform"
};

/* u_ctx->flag */
enum
{
    PNT_FLAG_SEPARATOR = 1,

    PNT_FLAG_ROI_ARRAY = 2,
    PNT_FLAG_ROI_ENTRY = 4,

    PNT_FLAG_RESTART_RTSP = 32,
    PNT_FLAG_RESTART_VIDEO = 64,
    PNT_FLAG_RESTART_AUDIO = 128,

    PNT_FLAG_WS_REQUEST_PENDING = 256,
    PNT_FLAG_WS_PREVIEW_PENDING = 512,
    PNT_FLAG_WS_REQUEST_PREVIEW = 1024,
    PNT_FLAG_WS_SEND_PREVIEW = 2048,

    PNT_FLAG_HTTP_SEND_MESSAGE = 4096,
    PNT_FLAG_HTTP_RECEIVED_MESSAGE = 8192,
    PNT_FLAG_HTTP_SEND_PREVIEW = 16384,
    PNT_FLAG_HTTP_SEND_INVALID = 32768
};

/* ROOT */
enum
{
    PNT_GENERAL = 1,
    PNT_RTSP,
    PNT_SENSOR,
    PNT_IMAGE,
    PNT_AUDIO,
    PNT_STREAM0,
    PNT_STREAM1,
    PNT_STREAM2,
    PNT_MOTION,
    PNT_INFO,
    PNT_ACTION
};

static const char *const root_keys[] = {
    "general",
    "rtsp",
    "sensor",
    "image",
    "audio",
    "stream0",
    "stream1",
    "stream2",
    "motion",
    "info",
    "action"};

/* GENERAL */
enum
{
    PNT_GENERAL_LOGLEVEL = 1,
    PNT_GENERAL_OSD_POOL_SIZE,
    PNT_GENERAL_IMP_POLLING_TIMEOUT
};

static const char *const general_keys[] = {
    "loglevel",
    "osd_pool_size",
    "imp_polling_timeout"};

/* RTSP */
enum
{
    PNT_RTSP_PORT = 1,
    PNT_RTSP_EST_BITRATE,
    PNT_RTSP_OUT_BUFFER_SIZE,
    PNT_RTSP_SEND_BUFFER_SIZE,
    PNT_RTSP_AUTH_REQUIRED,
    PNT_RTSP_NAME,
    PNT_RTSP_USERNAME,
    PNT_RTSP_PASSWORD
};

static const char *const rtsp_keys[] = {
    "port",
    "est_bitrate",
    "out_buffer_size",
    "send_buffer_size",
    "auth_required",
    "name",
    "username",
    "password"};

/* SENSOR */
enum
{
    PNT_SENSOR_MODEL = 1,
    PNT_SENSOR_FPS,
    PNT_SENSOR_WIDTH,
    PNT_SENSOR_HEIGHT,
    PNT_SENSOR_I2C_ADDRESS
};

static const char *const sensor_keys[] = {
    "model",
    "fps",
    "width",
    "height",
    "i2c_address"};

/* IMAGE */
enum
{
    PNT_IMAGE_BRIGHTNESS = 1,
    PNT_IMAGE_CONTRAST,
    PNT_IMAGE_HUE,
    PNT_IMAGE_SHARPNESS,
    PNT_IMAGE_SATURATION,
    PNT_IMAGE_SINTER_STRENGTH,
    PNT_IMAGE_TEMPER_STRENGTH,
    PNT_IMAGE_VFLIP,
    PNT_IMAGE_HFLIP,
    PNT_IMAGE_ANTIFLICKER,
    PNT_IMAGE_RUNNING_MODE,
    PNT_IMAGE_AE_COMPENSATION,
    PNT_IMAGE_DPC_STRENGTH,
    PNT_IMAGE_DEFOG_STRENGTH,
    PNT_IMAGE_DRC_STRENGTH,
    PNT_IMAGE_HIGHLIGHT_DEPRESS,
    PNT_IMAGE_BACKLIGHT_COMPENSTATION,
    PNT_IMAGE_MAX_AGAIN,
    PNT_IMAGE_MAX_DGAIN,
    PNT_IMAGE_CORE_WB_MODE,
    PNT_IMAGE_WB_RGAIN,
    PNT_IMAGE_WB_BGAIN
};

static const char *const image_keys[] = {
    "brightness",
    "contrast",
    "hue",
    "sharpness",
    "saturation",
    "sinter_strength",
    "temper_strength",
    "vflip",
    "hflip",
    "anti_flicker",
    "running_mode",
    "ae_compensation",
    "dpc_strength",
    "defog_strength",
    "drc_strength",
    "highlight_depress",
    "backlight_compensation",
    "max_again",
    "max_dgain",
    "core_wb_mode",
    "wb_rgain",
    "wb_bgain"};

#if defined(AUDIO_SUPPORT)
/* AUDIO */
enum
{
    PNT_AUDIO_INPUT_ENABLED = 1,
    PNT_AUDIO_INPUT_AGC_ENABLED,
    PNT_AUDIO_INPUT_HIGH_PASS_FILTER,
    PNT_AUDIO_INPUT_VOL,
    PNT_AUDIO_INPUT_GAIN,
    PNT_AUDIO_INPUT_ALC_GAIN,
    PNT_AUDIO_INPUT_NOISE_SUPPRESSION,
    PNT_AUDIO_INPUT_AGC_TARGET_LEVEL_DBFS,
    PNT_AUDIO_INPUT_AGC_COMPRESSION_GAIN_DB,
    PNT_AUDIO_INPUT_BITRATE,
    PNT_AUDIO_INPUT_FORMAT,
    PNT_AUDIO_INPUT_SAMPLE_RATE
};

static const char *const audio_keys[] = {
    "input_enabled",
    "input_agc_enabled",
    "input_high_pass_filter",
    "input_vol",
    "input_gain",
    "input_alc_gain",
    "input_noise_suppression",
    "input_agc_target_level_dbfs",
    "input_agc_compression_gain_db",
    "input_bitrate",
    "input_format",
    "input_sample_rate"};
#endif

/* STREAM */
enum
{
    PNT_STREAM_ENABLED = 1,
    PNT_STREAM_AUDIO_ENABLED,
    PNT_STREAM_SCALE_ENABLED,
    PNT_STREAM_RTSP_ENDPOINT,
    PNT_STREAM_FORMAT,
    PNT_STREAM_MODE,
    PNT_STREAM_GOP,
    PNT_STREAM_MAX_GOP,
    PNT_STREAM_FPS,
    PNT_STREAM_BUFFERS,
    PNT_STREAM_WIDTH,
    PNT_STREAM_HEIGHT,
    PNT_STREAM_BITRATE,
    PNT_STREAM_ROTATION,
    PNT_STREAM_SCALE_WIDTH,
    PNT_STREAM_SCALE_HEIGHT,
    PNT_STREAM_PROFILE,
    PNT_STREAM_STATS,
    PNT_STREAM_OSD
};

static const char *const stream_keys[] = {
    "enabled",
    "audio_enabled",
    "scale_enabled",
    "rtsp_endpoint",
    "format",
    "mode",
    "gop",
    "max_gop",
    "fps",
    "buffers",
    "width",
    "height",
    "bitrate",
    "rotation",
    "scale_width",
    "scale_height",
    "profile",
    "stats",
    "osd"};

/* STREAM2 (JPEG) */
enum
{
    PNT_STREAM2_JPEG_ENABLED = 1,
    PNT_STREAM2_JPEG_PATH,
    PNT_STREAM2_JPEG_QUALITY,
    //PNT_STREAM2_JPEG_REFRESH,
    PNT_STREAM2_JPEG_CHANNEL,
    PNT_STREAM2_STATS,
    PNT_STREAM2_FPS
};

static const char *const stream2_keys[] = {
    "jpeg_enabled",
    "jpeg_path",
    "jpeg_quality",
    //"jpeg_refresh",
    "jpeg_channel",
    "stats",
    "fps"};

/* OSD */
enum
{
    PNT_OSD_TIME_TRANSPARENCY = 1,
    PNT_OSD_USER_TEXT_TRANSPARENCY,
    PNT_OSD_UPTIME_TRANSPARENCY,
    PNT_OSD_LOGO_TRANSPARENCY,

    PNT_OSD_FONT_SIZE,
    PNT_OSD_FONT_STROKE_SIZE,
    PNT_OSD_LOGO_HEIGHT,
    PNT_OSD_LOGO_WIDTH,
    PNT_OSD_POS_TIME_X,
    PNT_OSD_POS_TIME_Y,
    PNT_OSD_TIME_ROTATION,
    PNT_OSD_POS_USER_TEXT_X,
    PNT_OSD_POS_USER_TEXT_Y,
    PNT_OSD_USER_TEXT_ROTATION,
    PNT_OSD_POS_UPTIME_X,
    PNT_OSD_POS_UPTIME_Y,
    PNT_OSD_UPTIME_ROTATION,

    PNT_OSD_POS_LOGO_X,
    PNT_OSD_POS_LOGO_Y,
    PNT_OSD_LOGO_ROTATION,

    PNT_OSD_ENABLED,
    PNT_OSD_TIME_ENABLED,
    PNT_OSD_USER_TEXT_ENABLED,
    PNT_OSD_UPTIME_ENABLED,
    PNT_OSD_LOGO_ENABLED,
    PNT_OSD_FONT_STROKE_ENABLED,

    PNT_OSD_FONT_PATH,
    PNT_OSD_TIME_FORMAT,
    PNT_OSD_UPTIME_FORMAT,
    PNT_OSD_USER_TEXT_FORMAT,
    PNT_OSD_LOGO_PATH,
    PNT_OSD_FONT_COLOR,
    PNT_OSD_FONT_STROKE_COLOR,
};

static const char *const osd_keys[] = {
    "time_transparency",
    "user_text_transparency",
    "uptime_transparency",
    "logo_transparency",

    "font_size",
    "font_stroke_size",
    "logo_height",
    "logo_width",
    "pos_time_x",
    "pos_time_y",
    "time_rotation",
    "pos_user_text_x",
    "pos_user_text_y",
    "user_text_rotation",
    "pos_uptime_x",
    "pos_uptime_y",
    "uptime_rotation",

    "pos_logo_x",
    "pos_logo_y",
    "logo_rotation",

    "enabled",
    "time_enabled",
    "user_text_enabled",
    "uptime_enabled",
    "logo_enabled",
    "font_stroke_enabled",
    "font_path",
    "time_format",
    "uptime_format",
    "user_text_format",
    "logo_path",
    "font_color",
    "font_stroke_color",
};

/* MOTION */
enum
{
    PNT_MOTION_DEBOUNCE_TIME = 1,
    PNT_MOTION_POST_TIME,
    PNT_MOTION_COOLDOWN_TIME,
    PNT_MOTION_INIT_TIME,
    PNT_MOTION_MIN_TIME,
    PNT_MOTION_THREAD_WAIT,
    PNT_MOTION_SENSITIVITY,
    PNT_MOTION_SKIP_FRAME_COUNT,
    PNT_MOTION_FRAME_WIDTH,
    PNT_MOTION_FRAME_HEIGHT,
    PNT_MOTION_ROI_0_X,
    PNT_MOTION_ROI_0_Y,
    PNT_MOTION_ROI_1_X,
    PNT_MOTION_ROI_1_Y,
    PNT_MOTION_ROI_COUNT,
    PNT_MOTION_ENABLED,
    PNT_MOTION_SCRIPT_PATH,
    PNT_MOTION_ROIS,
};

static const char *const motion_keys[] = {
    "debounce_time",
    "post_time",
    "cooldown_time",
    "init_time",
    "min_time",
    "thread_wait",
    "sensitivity",
    "skip_frame_count",
    "frame_width",
    "frame_height",
    "roi_0_x",
    "roi_0_x",
    "roi_1_x",
    "roi_1_y",
    "roi_count",
    "enabled",
    "script_path",
    "rois"};

/* INFO */
enum
{
    PNT_INFO_IMP_SYSTEM_VERSION = 1
};

static const char *const info_keys[] = {
    "imp_system_version"};

/* ACTION */
enum
{
    PNT_RESTART_THREAD = 1,
    PNT_SAVE_CONFIG,
    PNT_CAPTURE
};

enum
{
    PNT_THREAD_RTSP = 1,
    PNT_THREAD_VIDEO = 2,
    PNT_THREAD_AUDIO = 4
};

static const char *const action_keys[] = {
    "restart_thread",
    "save_config",
    "capture"};

#pragma endregion keys_and_enums

char token[WEBSOCKET_TOKEN_LENGTH + 1]{0};

struct snapshot_info
{
    int r;             // current requests
    int rps;           // requests per second
    int throttle = 50; // throttle value to set a variable request delay time
    steady_clock::time_point last_snapshot_request;
};

struct user_ctx
{
    std::string id;   // session id
    struct lws *wsi;  // libwebsockets handle
    std::string root; // json root path
    std::string path; // json sub path
    int value;        // to use a number in the JSON parser e.g. encChn (encoder channel)
    int flag;         // bitmask info store e.g. JSON separator (“,”) or thread restart
    roi region;
    int midx;
    int vidx;
    size_t post_data_size;
    std::string rx_message;
    std::string tx_message;
    std::string message;
    lws_sorted_usec_list_t sul; // lws Soft Timer
    struct snapshot_info snapshot;

    user_ctx(const std::string& session_id, lws *wsi_handle)
        : id(session_id), wsi(wsi_handle), root(), path(), value(0), flag(0),
          region(), midx(0), vidx(0), post_data_size(0), rx_message(), tx_message(),
          message(), sul(), snapshot()
    {}  
};

std::string generateToken(int length)
{
    static const char characters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const int maxIndex = sizeof(characters) - 1;

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, maxIndex);

    std::string randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; i++)
    {
        randomString += characters[distribution(generator)];
    }

    return randomString;
}

int restart_threads_by_signal(int &flag)
{
    // inform main to restart threads
    std::unique_lock lck(mutex_main);
    if (!global_restart_rtsp && !global_restart_video && !global_restart_audio) 
    {
        if ((flag & PNT_FLAG_RESTART_RTSP) || (flag & PNT_FLAG_RESTART_VIDEO) || (flag & PNT_FLAG_RESTART_AUDIO))
        {
            if (flag & PNT_FLAG_RESTART_RTSP)
            {
                global_restart_rtsp = true;
                flag &= ~PNT_FLAG_RESTART_RTSP;
            }
            if (flag & PNT_FLAG_RESTART_VIDEO)
            {
                global_restart_video = true;
                flag &= ~PNT_FLAG_RESTART_VIDEO;
            }
            if (flag & PNT_FLAG_RESTART_AUDIO)
            {
                global_restart_audio = true;
                flag &= ~PNT_FLAG_RESTART_AUDIO;
            }
            global_cv_worker_restart.notify_one();
            return 1;
        }
    }
    else
    {
        return -1;
    }
    return 0;
}

bool get_snapshot(std::vector<unsigned char> &image)
{
    std::ifstream file(global_jpeg[0]->stream->jpeg_path, std::ios::binary);
    if (!file.is_open())
    {
        LOG_DDEBUG(strerror(errno));
        return false;
    }

    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    if (file_size)
    {
        image.resize(LWS_PRE + file_size);
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char *>(image.data() + LWS_PRE), file_size);
        file.close();
        return true;
    }

    return false;
}

template <typename... Args>
void append_session_msg(std::string &ws_send_msg, const char *t, Args &&...a)
{
    char message[256];
    std::memset(message, 0, sizeof(message));
    std::snprintf(message, sizeof(message), t, std::forward<Args>(a)...);
    ws_send_msg += message;
}

void add_json_null(std::string &message) {
    append_session_msg(
        message, "%s", pnt_ws_msg[PNT_WS_MSG_NULL]);    
}

void add_json_bool(std::string &message, bool bl) {
    append_session_msg(
        message, "%s", bl ? pnt_ws_msg[PNT_WS_MSG_TRUE] : pnt_ws_msg[PNT_WS_MSG_FALSE]);   
}

void add_json_str(std::string &message, const char *value) {
    append_session_msg(
        message, "\"%s\"", value);   
}

void add_json_num(std::string &message, int value) {
    append_session_msg(
        message, "%d", value);   
}

void add_json_uint(std::string &message, unsigned int value) {
    append_session_msg(
        message, "\"%#x\"", value);   
}

void add_json_key(std::string &message, bool separator, const char *key, const char * opener = "") {
    append_session_msg(
        message, "%s\"%s\":%s", separator ? "," : "", key, opener);
}

signed char WS::general_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if ((reason & LEJP_FLAG_CB_IS_VALUE) && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), general_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        if (ctx->path_match >= PNT_GENERAL_OSD_POOL_SIZE && ctx->path_match <= PNT_GENERAL_IMP_POLLING_TIMEOUT)
        { // integer values
            if (reason == LEJPCB_VAL_NUM_INT)
                cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        else 
        {
            switch (ctx->path_match)
            {
            case PNT_GENERAL_LOGLEVEL:
                if (reason == LEJPCB_VAL_STR_END)
                {
                    if (cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                    {
                        Logger::setLevel(ctx->buf);
                    }
                }
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));                
                break;                    
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;
            }
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::rtsp_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), rtsp_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        // int values
        if (ctx->path_match >= PNT_RTSP_PORT && ctx->path_match <= PNT_RTSP_SEND_BUFFER_SIZE)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    // better restart rtsp manually ?
                    // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART; // restart RTSP
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
            // const char * values
        }
        else if (ctx->path_match >= PNT_RTSP_NAME && ctx->path_match <= PNT_RTSP_PASSWORD)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                    // better restart rtsp manually ?
                    // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                }
            }
            add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_RTSP_AUTH_REQUIRED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        // better restart rtsp manually ?
                        // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        // better restart rtsp manually ?
                        // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                    }
                }
                add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
                break;
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;
            }
        }
        
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::sensor_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), sensor_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        // int values
        if (ctx->path_match >= PNT_SENSOR_FPS && ctx->path_match <= PNT_SENSOR_HEIGHT)
        {
            // normally this cannot be set and is read from proc
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_SENSOR_MODEL:
                // normally this cannot be set and is read from proc
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
                break;
            case PNT_SENSOR_I2C_ADDRESS:
                // normally this cannot be set and is read from proc
                add_json_uint(u_ctx->message, cfg->get<unsigned int>(u_ctx->path));
                break;
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;                
            }            
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::image_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), image_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        if (ctx->path_match == PNT_IMAGE_DEFOG_STRENGTH)
        {
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    uint8_t t = static_cast<uint8_t>(cfg->get<int>(u_ctx->path));
                    IMP_ISP_Tuning_SetDefog_Strength(reinterpret_cast<uint8_t *>(&t));
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
            add_json_null(u_ctx->message);
#endif
        }
        else if (ctx->path_match >= PNT_IMAGE_CORE_WB_MODE && ctx->path_match <= PNT_IMAGE_WB_BGAIN)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    IMPISPWB wb;
                    memset(&wb, 0, sizeof(IMPISPWB));
                    int ret = IMP_ISP_Tuning_GetWB(&wb);
                    if (ret == 0)
                    {
                        wb.mode = (isp_core_wb_mode)cfg->image.core_wb_mode;
                        wb.rgain = cfg->image.wb_rgain;
                        wb.bgain = cfg->image.wb_bgain;
                        ret = IMP_ISP_Tuning_SetWB(&wb);
                    }
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_IMAGE_BRIGHTNESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBrightness(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_CONTRAST:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetContrast(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_HUE:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBcshHue(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);
#endif
                break;
            case PNT_IMAGE_SATURATION:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSaturation(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_SHARPNESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSharpness(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;               
            case PNT_IMAGE_SINTER_STRENGTH:
#if !defined(PLATFORM_T21)             
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSinterStrength(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);                
#endif
                break;
            case PNT_IMAGE_TEMPER_STRENGTH:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetTemperStrength(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_VFLIP:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_ENABLE);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_DISABLE);
                    }
                }
                add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
                break;
            case PNT_IMAGE_HFLIP:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_ENABLE);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_DISABLE);
                    }
                }
                add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
                break;
            case PNT_IMAGE_ANTIFLICKER:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_RUNNING_MODE:
                {
                    if (reason == LEJPCB_VAL_NUM_INT)
                    {
                        if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                        {
                            IMP_ISP_Tuning_SetISPRunningMode((IMPISPRunningMode)cfg->get<int>(u_ctx->path));
                        }
                    }

                    IMPISPRunningMode running_mode;
                    IMP_ISP_Tuning_GetISPRunningMode(&running_mode);
                    add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                }
                break;
            case PNT_IMAGE_AE_COMPENSATION:
#if !defined(PLATFORM_T21)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetAeComp(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);
#endif
                break;
            case PNT_IMAGE_DPC_STRENGTH:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetDPC_Strength(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);
#endif
                break;
            case PNT_IMAGE_DRC_STRENGTH:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetDRC_Strength(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);
#endif
                break;
            case PNT_IMAGE_HIGHLIGHT_DEPRESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetHiLightDepress(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_BACKLIGHT_COMPENSTATION:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBacklightComp(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
                add_json_null(u_ctx->message);
#endif
                break;
            case PNT_IMAGE_MAX_AGAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetMaxAgain(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_MAX_DGAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetMaxDgain(cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;                 
            }
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

#if defined(AUDIO_SUPPORT)
signed char WS::audio_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), audio_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        if (ctx->path_match == PNT_AUDIO_INPUT_HIGH_PASS_FILTER)
        {
            IMPAudioIOAttr ioattr;
            int ret = IMP_AI_GetPubAttr(u_ctx->value, &ioattr);
            if (ret == 0)
            {
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        ret = IMP_AI_EnableHpf(&ioattr);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        ret = IMP_AI_DisableHpf();
                    }
                }
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
        }
        // integer values
        else if (ctx->path_match == PNT_AUDIO_INPUT_NOISE_SUPPRESSION || 
                 ctx->path_match == PNT_AUDIO_INPUT_SAMPLE_RATE ||
                 ctx->path_match == PNT_AUDIO_INPUT_BITRATE )
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    global_restart_audio = true;
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
#if defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) || defined(PLATFORM_T23) || defined(PLATFORM_T30) || defined(PLATFORM_T31)
        else if (ctx->path_match == PNT_AUDIO_INPUT_AGC_ENABLED)
        {
            IMPAudioIOAttr ioattr;
            int ret = IMP_AI_GetPubAttr(u_ctx->value, &ioattr);
            if (ret == 0)
            {
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        global_restart_audio = true;
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        global_restart_audio = true;
                    }
                }
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
        }
        else if (ctx->path_match == PNT_AUDIO_INPUT_AGC_TARGET_LEVEL_DBFS || ctx->path_match == PNT_AUDIO_INPUT_AGC_COMPRESSION_GAIN_DB)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    global_restart_audio = true;
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
#else
        add_json_null(u_ctx->message);
    }
#endif
        else
        {
            switch (ctx->path_match)
            {
            case PNT_AUDIO_INPUT_ENABLED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_AI_Enable(u_ctx->value);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_AI_Disable(u_ctx->value);
                    }
                }
                add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_VOL:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetVol(u_ctx->value, global_audio[u_ctx->value]->aiChn, cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_GAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetGain(u_ctx->value, global_audio[u_ctx->value]->aiChn, cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_ALC_GAIN:
#if defined(PLATFORM_T21) || defined(PLATFORM_T31)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetAlcGain(0, 0, cfg->get<int>(u_ctx->path));
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
#else
            add_json_str(u_ctx->message, pnt_ws_msg[PNT_WS_MSG_UNSUPPORTED]);
#endif
                break;
            case PNT_AUDIO_INPUT_FORMAT:
                if (reason == LEJPCB_VAL_STR_END)
                    cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
                break;                
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;                  
            }
        }
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}
#endif

signed char WS::stream_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
    u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), stream_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        if (ctx->path_match >= PNT_STREAM_GOP && ctx->path_match <= PNT_STREAM_PROFILE)
        { // integer values
            if (reason == LEJPCB_VAL_NUM_INT)
                cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        else if(ctx->path_match >= PNT_STREAM_ENABLED && ctx->path_match <= PNT_STREAM_SCALE_ENABLED)
        { // bool values
            if (reason == LEJPCB_VAL_TRUE)
            {
                cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                cfg->set<bool>(u_ctx->path, false);
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_STREAM_RTSP_ENDPOINT:
                if (reason == LEJPCB_VAL_STR_END)
                    cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
                break;
            case PNT_STREAM_SCALE_ENABLED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    cfg->set<bool>(u_ctx->path, true);
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    cfg->set<bool>(u_ctx->path, false);
                }
                add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
                break;
            case PNT_STREAM_FORMAT:
                if (reason == LEJPCB_VAL_STR_END)
                    cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
                break;
            case PNT_STREAM_MODE:
                if (reason == LEJPCB_VAL_STR_END)
                    cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
                break;                
            case PNT_STREAM_STATS:
                if (reason == LEJPCB_VAL_NULL)
                {
                    uint8_t fps = 0;
                    uint32_t bps = 0;
                    if (u_ctx->root == "stream0")
                    {
                        fps = cfg->stream0.stats.fps;
                        bps = cfg->stream0.stats.bps;
                    }
                    else if (u_ctx->root == "stream1")
                    {
                        fps = cfg->stream1.stats.fps;
                        bps = cfg->stream1.stats.bps;
                    }
                    append_session_msg(
                        u_ctx->message, "{\"fps\":%d,\"Bps\":%d}", fps, bps);
                }
                break;                
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;                
            };
        }
    }
    else if (reason == LECPCB_PAIR_NAME && ctx->path_match == PNT_STREAM_OSD)
    {
        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), stream_keys[ctx->path_match - 1], "{");

        // remove separator for sub section
        u_ctx->flag &= ~PNT_FLAG_SEPARATOR;

        lejp_parser_push(ctx, u_ctx,
                         osd_keys, LWS_ARRAY_SIZE(osd_keys), osd_callback);
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::stream2_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), stream2_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        switch (ctx->path_match)
        {
        case PNT_STREAM2_JPEG_ENABLED:
            if (reason == LEJPCB_VAL_TRUE)
            {
                cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                cfg->set<bool>(u_ctx->path, false);
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
            break;
        case PNT_STREAM2_JPEG_PATH:
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
            break;
        case PNT_STREAM2_JPEG_QUALITY:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
            break;
        case PNT_STREAM2_FPS:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
            break;       
        case PNT_STREAM2_JPEG_CHANNEL:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                }
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
            break;
        case PNT_STREAM2_STATS:
            if (reason == LEJPCB_VAL_NULL)
            {
                uint8_t fps = 0;
                uint32_t bps = 0;
                if (u_ctx->root == "stream2")
                {
                    fps = cfg->stream2.stats.fps;
                    bps = cfg->stream2.stats.bps;
                }
                append_session_msg(
                    u_ctx->message, "{\"fps\":%d,\"Bps\":%d}", fps, bps);
            }
            break;
        default:
            u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
            break;             
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::osd_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + ".osd." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), osd_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        if (ctx->path_match >= PNT_OSD_TIME_TRANSPARENCY &&
            ctx->path_match <= PNT_OSD_LOGO_TRANSPARENCY)
        {
            int hnd = -1;
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {

                    _regions regions{-1,-1,-1,-1};

                    if (u_ctx->value == 0)
                    {
                        regions = cfg->stream0.osd.regions;
                    }
                    else if (u_ctx->value == 1)
                    {
                        regions = cfg->stream1.osd.regions;
                    }

                    switch (ctx->path_match)
                    {
                    case PNT_OSD_TIME_TRANSPARENCY:
                        hnd = regions.time;
                        break;
                    case PNT_OSD_USER_TEXT_TRANSPARENCY:
                        hnd = regions.user;
                        break;
                    case PNT_OSD_UPTIME_TRANSPARENCY:
                        hnd = regions.uptime;
                        break;
                    case PNT_OSD_LOGO_TRANSPARENCY:
                        hnd = regions.logo;
                        break;
                    }

                    if(hnd >= 0) {
                        IMPOSDGrpRgnAttr grpRgnAttr;
                        int ret = IMP_OSD_GetGrpRgnAttr(hnd, u_ctx->value, &grpRgnAttr);
                        if (ret == 0)
                        {
                            memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
                            grpRgnAttr.show = 1;
                            grpRgnAttr.gAlphaEn = 1;
                            grpRgnAttr.fgAlhpa = cfg->get<int>(u_ctx->path);
                            IMP_OSD_SetGrpRgnAttr(hnd, u_ctx->value, &grpRgnAttr);
                        }
                    }
                };
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        // integer
        else if (ctx->path_match >= PNT_OSD_FONT_SIZE && ctx->path_match <= PNT_OSD_UPTIME_ROTATION)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
        }
        // bool
        else if (ctx->path_match >= PNT_OSD_ENABLED && ctx->path_match <= PNT_OSD_FONT_STROKE_ENABLED)
        {
            if (reason == LEJPCB_VAL_TRUE)
            {
                cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                cfg->set<bool>(u_ctx->path, false);
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
        }
        // const char *
        else if (ctx->path_match >= PNT_OSD_FONT_PATH && ctx->path_match <= PNT_OSD_LOGO_PATH)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
        }
        // unsigned int
        else if (ctx->path_match >= PNT_OSD_FONT_COLOR && ctx->path_match <= PNT_OSD_FONT_STROKE_COLOR)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (cfg->set<unsigned int>(u_ctx->path, (unsigned int)strtoll(ctx->buf, NULL, 16)))
                {
                }
            }
            add_json_uint(u_ctx->message, cfg->get<unsigned int>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_OSD_POS_LOGO_X:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                    IMPOSDRgnAttr rgnAttr;
                    memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));
                    if (IMP_OSD_GetRgnAttr(3, &rgnAttr) == 0)
                    {
                        if (u_ctx->value == 0)
                        {
                            OSD::set_pos(&rgnAttr, cfg->stream0.osd.pos_logo_x,
                                         cfg->stream0.osd.pos_logo_y, 0, 0, cfg->stream0.width, cfg->stream0.height);
                        }
                        else if (u_ctx->value == 1)
                        {
                            OSD::set_pos(&rgnAttr, cfg->stream1.osd.pos_logo_x,
                                         cfg->stream1.osd.pos_logo_y, 0, 0, cfg->stream1.width, cfg->stream1.height);
                        }
                        IMP_OSD_SetRgnAttr(3, &rgnAttr);
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_OSD_POS_LOGO_Y:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                    IMPOSDRgnAttr rgnAttr;
                    memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));
                    if (IMP_OSD_GetRgnAttr(3, &rgnAttr) == 0)
                    {
                        if (u_ctx->value == 0)
                        {
                            OSD::set_pos(&rgnAttr, cfg->stream0.osd.pos_logo_y,
                                         cfg->stream0.osd.pos_logo_y, 0, 0, cfg->stream0.width, cfg->stream0.height);
                        }
                        else if (u_ctx->value == 1)
                        {
                            OSD::set_pos(&rgnAttr, cfg->stream1.osd.pos_logo_y,
                                         cfg->stream1.osd.pos_logo_y, 0, 0, cfg->stream1.width, cfg->stream1.height);
                        }
                        IMP_OSD_SetRgnAttr(3, &rgnAttr);
                    }
                }
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            case PNT_OSD_LOGO_ROTATION:
                // encoder restart required
                if (reason == LEJPCB_VAL_NUM_INT)
                    cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
                break;
            default:
                u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
                break;                 
            };
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::motion_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
    u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), motion_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        // integer
        if (ctx->path_match >= PNT_MOTION_DEBOUNCE_TIME && ctx->path_match <= PNT_MOTION_ROI_COUNT)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            }
            add_json_num(u_ctx->message, cfg->get<int>(u_ctx->path));
            // bool
        }
        else if (ctx->path_match == PNT_MOTION_ENABLED)
        {
            if (reason == LEJPCB_VAL_TRUE)
            {
                cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                cfg->set<bool>(u_ctx->path, false);
            }
            add_json_bool(u_ctx->message, cfg->get<bool>(u_ctx->path));
            // std::string
        }
        else if (ctx->path_match == PNT_MOTION_SCRIPT_PATH)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            add_json_str(u_ctx->message, cfg->get<const char *>(u_ctx->path));
        }
        else if (ctx->path_match == PNT_MOTION_ROIS)
        {
            if (reason == LEJPCB_VAL_NULL)
            {
                for (int i = 0; i < cfg->motion.roi_count; i++)
                {
                }
            }
            add_json_str(u_ctx->message, cfg->get<std::string>(u_ctx->path).c_str());
        }
        else
        {
            u_ctx->flag &= ~PNT_FLAG_SEPARATOR;             
        }
    }
    else if (reason == LECPCB_PAIR_NAME && ctx->path_match == PNT_MOTION_ROIS)
    {
        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), motion_keys[ctx->path_match - 1]);

        // remove separator for sub section
        u_ctx->flag &= ~PNT_FLAG_SEPARATOR;

        lejp_parser_push(ctx, u_ctx,
                         motion_keys, LWS_ARRAY_SIZE(motion_keys), motion_roi_callback);
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::motion_roi_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
    u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

    if ((reason & LEJP_FLAG_CB_IS_VALUE) && (reason == LEJPCB_VAL_NULL))
    {
        u_ctx->message.append("[");
        for (int i = 0; i < cfg->motion.roi_count; i++)
        {
            if ((u_ctx->flag & PNT_FLAG_SEPARATOR))
                u_ctx->message.append(",");

            append_session_msg(
                u_ctx->message, "[%d,%d,%d,%d]", cfg->motion.rois[i].p0_x, cfg->motion.rois[i].p0_y,
                cfg->motion.rois[i].p1_x, cfg->motion.rois[i].p1_y);
            u_ctx->flag |= PNT_FLAG_SEPARATOR;
        }
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("]");
        lejp_parser_pop(ctx);
    }
    else
    {
        switch (reason)
        {
        case LEJPCB_ARRAY_START:
            // not first, we need a separator
            if (u_ctx->flag & PNT_FLAG_SEPARATOR)
            {
                u_ctx->message.append(",");
            }
            // is roi array open ? if so, open entry array
            if (u_ctx->flag & PNT_FLAG_ROI_ARRAY)
            {
                u_ctx->flag |= PNT_FLAG_ROI_ENTRY; // entry array
                u_ctx->vidx = 0;                   // entry array index
                u_ctx->message.append("[");
            }
            // roi array is not open ! open it
            else
            {
                u_ctx->flag |= PNT_FLAG_ROI_ARRAY; // roi array
                u_ctx->midx = 0;                   // roi array index
                u_ctx->message.append("[");
            }
            break;

        case LEJPCB_VAL_NUM_INT:
            // parse roi entry with 4 elements
            if (u_ctx->flag & PNT_FLAG_ROI_ENTRY)
            {
                u_ctx->vidx++;
                if (u_ctx->vidx == 1)
                {
                    u_ctx->region.p0_x = atoi(ctx->buf);
                }
                else if (u_ctx->vidx == 2)
                {
                    u_ctx->region.p0_y = atoi(ctx->buf);
                }
                else if (u_ctx->vidx == 3)
                {
                    u_ctx->region.p1_x = atoi(ctx->buf);
                }
                else if (u_ctx->vidx == 4)
                {
                    u_ctx->region.p1_y = atoi(ctx->buf);
                }
            }
            break;

        case LEJPCB_ARRAY_END:
            // roi entry closed
            if (u_ctx->flag & PNT_FLAG_ROI_ENTRY)
            {
                u_ctx->flag &= ~PNT_FLAG_ROI_ENTRY;

                // we read 4 roi values add to message
                if (u_ctx->vidx >= 4)
                {
                    append_session_msg(
                        u_ctx->message, "%d,%d,%d,%d", u_ctx->region.p0_x, u_ctx->region.p0_y, u_ctx->region.p1_x, u_ctx->region.p1_y);
                }
                u_ctx->message.append("]");

                // roi entry parsed
                u_ctx->flag |= PNT_FLAG_SEPARATOR;

                // read up to 52 roi entries into u_ctx->region
                if (u_ctx->midx <= 52)
                {
                    cfg->motion.rois[u_ctx->midx] =
                        {u_ctx->region.p0_x, u_ctx->region.p0_y, u_ctx->region.p1_x, u_ctx->region.p1_y};
                    u_ctx->midx++;
                }
            }
            // roi main array closed
            else if (u_ctx->flag & PNT_FLAG_ROI_ARRAY)
            {
                u_ctx->flag |= PNT_FLAG_SEPARATOR;
                cfg->motion.roi_count = u_ctx->midx;
                u_ctx->message.append("]");
                lejp_parser_pop(ctx);
            }
            break;
        }
    }
    return 0;
}

signed char WS::info_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);
        
        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), info_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        
        switch (ctx->path_match)
        {
        case PNT_INFO_IMP_SYSTEM_VERSION:
            {
                IMPVersion impVersion;
                int ret = IMP_System_GetVersion(&impVersion);
                if (ret)
                {
                    add_json_str(u_ctx->message, impVersion.aVersion);
                }
                else
                {
                    add_json_str(u_ctx->message, pnt_ws_msg[PNT_WS_MSG_ERROR]);
                }
            }
            break;
        default:
            u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
            break;               
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::action_callback(struct lejp_ctx *ctx, char reason)
{
    struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), action_keys[ctx->path_match - 1]);

        u_ctx->flag |= PNT_FLAG_SEPARATOR;

        switch (ctx->path_match)
        {
        case PNT_RESTART_THREAD:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                int msg_id = PNT_WS_MSG_INITIATED;
                int restart_flag = 0;
                int thread_restart = atoi(ctx->buf);

                if (thread_restart & PNT_THREAD_RTSP)
                {
                    restart_flag |= PNT_FLAG_RESTART_RTSP;
                }
                if (thread_restart & PNT_THREAD_VIDEO)
                {
                    restart_flag |= PNT_FLAG_RESTART_VIDEO;
                }
                if (thread_restart & PNT_THREAD_AUDIO)
                {
                    restart_flag |= PNT_FLAG_RESTART_AUDIO;
                }
                if (restart_flag) {
                    if(restart_threads_by_signal(restart_flag) < 0)
                        msg_id = PNT_WS_MSG_DROPPED;
                }
                else
                {
                    msg_id = PNT_WS_MSG_ERROR;
                }
                add_json_str(u_ctx->message, pnt_ws_msg[msg_id]);                                          
            }
            else
            {
                add_json_null(u_ctx->message);
            }
            break;
        case PNT_SAVE_CONFIG:
            cfg->updateConfig();
            add_json_str(u_ctx->message, pnt_ws_msg[PNT_WS_MSG_INITIATED]); 
            break;
        case PNT_CAPTURE:
            u_ctx->flag |= PNT_FLAG_WS_REQUEST_PREVIEW;
            add_json_str(u_ctx->message, pnt_ws_msg[PNT_WS_MSG_INITIATED]); 
            break;
        default:
            u_ctx->flag &= ~PNT_FLAG_SEPARATOR;
            break;               
        }
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        u_ctx->flag |= PNT_FLAG_SEPARATOR;
        u_ctx->message.append("}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::root_callback(struct lejp_ctx *ctx, char reason)
{
    if ((reason & LEJPCB_OBJECT_START) && ctx->path_match)
    {
        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path.clear();
        u_ctx->root = ctx->path;

        add_json_key(u_ctx->message, (u_ctx->flag & PNT_FLAG_SEPARATOR), root_keys[ctx->path_match - 1], "{");

        u_ctx->flag &= ~PNT_FLAG_SEPARATOR;

        switch (ctx->path_match)
        {
        case PNT_GENERAL:
            lejp_parser_push(ctx, u_ctx,
                             general_keys, LWS_ARRAY_SIZE(general_keys), general_callback);
            break;
        case PNT_RTSP:
            lejp_parser_push(ctx, u_ctx,
                             rtsp_keys, LWS_ARRAY_SIZE(rtsp_keys), rtsp_callback);
            break;
        case PNT_SENSOR:
            lejp_parser_push(ctx, u_ctx,
                             sensor_keys, LWS_ARRAY_SIZE(sensor_keys), sensor_callback);
            break;
        case PNT_IMAGE:
            lejp_parser_push(ctx, u_ctx,
                             image_keys, LWS_ARRAY_SIZE(image_keys), image_callback);
            break;

#if defined(AUDIO_SUPPORT)
        case PNT_AUDIO:
            u_ctx->value = global_audio[0]->aiChn;
            lejp_parser_push(ctx, u_ctx,
                             audio_keys, LWS_ARRAY_SIZE(audio_keys), audio_callback);
            break;
#endif

        case PNT_STREAM0:
            u_ctx->value = global_video[0]->encChn;
            lejp_parser_push(ctx, &u_ctx,
                             stream_keys, LWS_ARRAY_SIZE(stream_keys), stream_callback);
            break;
        case PNT_STREAM1:
            u_ctx->value = global_video[1]->encChn;
            lejp_parser_push(ctx, &u_ctx,
                             stream_keys, LWS_ARRAY_SIZE(stream_keys), stream_callback);
            break;
        case PNT_STREAM2:
            lejp_parser_push(ctx, u_ctx,
                             stream2_keys, LWS_ARRAY_SIZE(stream2_keys), stream2_callback);
            break;
        case PNT_MOTION:
            lejp_parser_push(ctx, u_ctx,
                             motion_keys, LWS_ARRAY_SIZE(motion_keys), motion_callback);
            break;
        case PNT_INFO:
            lejp_parser_push(ctx, u_ctx,
                             info_keys, LWS_ARRAY_SIZE(info_keys), info_callback);
            break;
        case PNT_ACTION:
            lejp_parser_push(ctx, u_ctx,
                             action_keys, LWS_ARRAY_SIZE(action_keys), action_callback);
            break;
        }
    }

    return 0;
}

std::string generateSessionID()
{
    std::random_device rd;
    std::mt19937_64 eng(rd());
    std::uniform_int_distribution<uint64_t> distr;
    uint64_t randomValue = distr(eng);
    auto timeStamp = std::time(nullptr);
    std::stringstream ss;
    ss << std::hex << randomValue << timeStamp;
    return ss.str();
}

static void
send_snapshot(lws_sorted_usec_list_t *sul)
{
    struct user_ctx *u_ctx = lws_container_of(sul, struct user_ctx, sul);
    LOG_DDEBUG("process shedule. id:" << u_ctx->id);
    u_ctx->flag |= PNT_FLAG_WS_SEND_PREVIEW;
    lws_callback_on_writable(u_ctx->wsi);
}

int WS::ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    struct lejp_ctx ctx;
    user_ctx *u_ctx = (struct user_ctx *)user;

    char client_ip[128];
    lws_get_peer_simple(wsi, client_ip, sizeof(client_ip));

    char *url_ptr = nullptr;
    int url_length = 0;
    int request_method = 0;

    // security token ?token=
    char url_token[128]{0};
    char content_type[128]{0};
    std::string json_data((char *)in, len);

    //LOG_DDEBUG(reason);

    // get url and method
    if (reason >= LWS_CALLBACK_HTTP && reason <= LWS_CALLBACK_HTTP_WRITEABLE)
    {
        request_method = lws_http_get_uri_and_method(wsi, &url_ptr, &url_length);
        lws_hdr_copy(wsi, content_type, sizeof(content_type), WSI_TOKEN_HTTP_CONTENT_TYPE);
    }

    switch (reason)
    {
    // ############################ WEBSOCKET ###############################
    case LWS_CALLBACK_ESTABLISHED:
        LOG_DDEBUG("LWS_CALLBACK_ESTABLISHED id:" << u_ctx->id << ", ip:" << client_ip);

        // check if security is required and validate token
        url_length = lws_get_urlarg_by_name_safe(wsi, "token", url_token, sizeof(url_token));
        if (strcmp(token, url_token) == 0 || (strcmp(cfg->websocket.usertoken, "") != 0 && strcmp(cfg->websocket.usertoken, url_token) == 0))
        {
            /* initialize new u_ctx session structure.
             * assign current wsi and a new sessionid
             */
            new (user) user_ctx(generateSessionID(), wsi);
        }
        else
        {
            LOG_DEBUG("Unauthenticated websocket connect from: " << client_ip);
            if (cfg->websocket.ws_secured)
            {
                LOG_DEBUG("Connection refused.");
                return -1;
            }
        }
        break;

    case LWS_CALLBACK_RECEIVE:
        LOG_DDEBUG("LWS_CALLBACK_RECEIVE " << 
            " id:" << u_ctx->id << 
            " ,flag:" << u_ctx->flag << 
            " ,ip:" << client_ip << 
            " ,len:" << len << 
            " ,last:" << lws_is_final_fragment(wsi));

        /* larger requests can be segmented into several requests, 
         * so we have to collect all the data until we reach the last segment.
         * On receiving the first segment we should clear the rx_message
         */
        if (lws_is_first_fragment(wsi))
            u_ctx->rx_message.clear();

        u_ctx->rx_message.append(json_data);

        if (!lws_is_final_fragment(wsi))
            return 0;    

        LOG_DDEBUG("u_ctx->rx_message: id:" << u_ctx->id << ", rx:" << u_ctx->rx_message);

        // set request pending
        //u_ctx->flag |= PNT_FLAG_WS_REQUEST_PENDING;

        // parse json and write response into u_ctx->message
        u_ctx->message = "{";               // open response json 
        lejp_construct(&ctx, root_callback, u_ctx, root_keys, LWS_ARRAY_SIZE(root_keys));
        lejp_parse(&ctx, (uint8_t *)u_ctx->rx_message.c_str(), u_ctx->rx_message.length());
        lejp_destruct(&ctx);
        u_ctx->message.append("}");         // close response json
        u_ctx->rx_message.clear();          // cleanup received data
        u_ctx->flag &= ~PNT_FLAG_SEPARATOR; // always reset separator after parsing

        // splitt overlapping responses with a ";"
        if(u_ctx->flag & PNT_FLAG_WS_REQUEST_PENDING) {
            u_ctx->tx_message.append(";");
        }

        u_ctx->flag |= PNT_FLAG_WS_REQUEST_PENDING;

        // incoming snapshot request via websocket
        if (u_ctx->flag & PNT_FLAG_WS_REQUEST_PREVIEW)
        {
            // clenaup request flag
            u_ctx->flag &= ~PNT_FLAG_WS_REQUEST_PREVIEW;

            // drop overlapping image requests
            if (u_ctx->flag & PNT_FLAG_WS_PREVIEW_PENDING) {
                LOG_DDEBUG("drop overlapping image request. id:" << u_ctx->id);
                return 0;
            };

            // set prview pending flag 
            u_ctx->flag |= PNT_FLAG_WS_PREVIEW_PENDING;

            /* 'first_request_delay'
             * first request after thread sleep should be a bit delayed, because the first
             * images after wakup can be incomplete or having osd missed
             * a default of 100 milliseconds should delay ~3 images
             */
            int first_request_delay = 0;
            u_ctx->snapshot.r++;

            {
                global_jpeg[0]->request();

                /* if the jpeg channel is inactive we need to start him
                * this can also cause that required video channel also
                * must been started
                */
                if (!global_jpeg[0]->active)
                {
                    first_request_delay = cfg->websocket.first_image_delay * 1000;
                    global_jpeg[0]->should_grab_frames.notify_all();
                    global_jpeg[0]->is_activated.acquire();
                }
            }

            auto now = steady_clock::now();
            auto dur = duration_cast<milliseconds>(now - u_ctx->snapshot.last_snapshot_request).count();

            /* Throttling to prevent images from being sent faster than they are created
             * 'u_ctx->snapshot.throttle' is calculated to delay sendings
             */
            if (dur > 1000)
            {
                u_ctx->snapshot.last_snapshot_request = now;
                u_ctx->snapshot.rps = u_ctx->snapshot.r;
                u_ctx->snapshot.r = 0;
                
                u_ctx->snapshot.throttle +=
                    global_jpeg[0]->stream->stats.fps - u_ctx->snapshot.rps;
                
                if (u_ctx->snapshot.throttle > 100)
                {
                    u_ctx->snapshot.throttle = 100;
                }
                else if (u_ctx->snapshot.throttle < 1)
                {
                    u_ctx->snapshot.throttle = 1;
                }

                LOG_DDEBUG("RPS: " << u_ctx->snapshot.rps << " " << u_ctx->snapshot.throttle << " " << dur);
            }

            int delay = (LWS_USEC_PER_SEC / (global_jpeg[0]->stream->stats.fps + u_ctx->snapshot.throttle)) + first_request_delay;
            LOG_DDEBUG("shedule preview image. id:" << u_ctx->id << " delay:" << delay);
            lws_sul_schedule(lws_get_context(wsi), 0, &u_ctx->sul, send_snapshot, delay);

            // send response for the image request 
            u_ctx->tx_message.append(u_ctx->message);
            lws_callback_on_writable(wsi);                             
        } else {

            // send response for all 'non image request' json requests
            u_ctx->tx_message.append(u_ctx->message);
            lws_callback_on_writable(wsi);
        }

        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        LOG_DDEBUG("LWS_CALLBACK_SERVER_WRITEABLE id:" << u_ctx->id << ", ip:" << client_ip);

        // send response message
        if (!u_ctx->tx_message.empty())
        {
            u_ctx->flag &= ~PNT_FLAG_WS_REQUEST_PENDING;

            /* send all outstanding messages
             * if messages faster received than an answer can be send, they will append to 
             * tx_message and separated with a ";". now we split them and send each 
             * segment separate
             */
            std::stringstream ss(u_ctx->tx_message);
            std::string item;

            while (std::getline(ss, item, ';')) {
                LOG_DDEBUG("u_ctx->tx_message id:" << u_ctx->id << ", tx:" << item);
                item = std::string(LWS_PRE, '\0') + item;
                lws_write(wsi, (unsigned char *)item.c_str() + LWS_PRE, item.length() - LWS_PRE, LWS_WRITE_TEXT);
            }

            u_ctx->tx_message.clear();            
        }

        // delayed snapshot request via websocket, sending the image
        if (u_ctx->flag & PNT_FLAG_WS_SEND_PREVIEW)
        {
            LOG_DDEBUG("send preview image. id:" << u_ctx->id);
            global_jpeg[0]->request();
            std::vector<unsigned char> jpeg_buf;
            if (get_snapshot(jpeg_buf))
            {
                lws_write(wsi, jpeg_buf.data() + LWS_PRE, jpeg_buf.size() - LWS_PRE, LWS_WRITE_BINARY);
            }
            u_ctx->flag &= ~(PNT_FLAG_WS_SEND_PREVIEW | PNT_FLAG_WS_PREVIEW_PENDING);
        }
        break;

    case LWS_CALLBACK_CLOSED:
        LOG_DDEBUG("LWS_CALLBACK_CLOSED id:" << u_ctx->id << ", ip:" << client_ip << ", flag:" << u_ctx->flag);

        // cleanup delete possibly existing shedules for this session    
        lws_sul_cancel(&u_ctx->sul);

        u_ctx->~user_ctx();
        break;


    // ############################ HTTP ###############################
    case LWS_CALLBACK_HTTP:
        LOG_DDEBUG("LWS_CALLBACK_HTTP ip:" << client_ip << " url:" << (char *)url_ptr << " method:" << request_method);

        // check if security is required and validate token
        url_length = lws_get_urlarg_by_name_safe(wsi, "token", url_token, sizeof(url_token));
        if (strcmp(token, url_token) == 0 || (strcmp(cfg->websocket.usertoken, "") != 0 && strcmp(cfg->websocket.usertoken, url_token) == 0))
        {
            /* initialize new u_ctx session structure.
            * assign current wsi and a new sessionid
            ' don't know if we need it for http
            */
            new (user) user_ctx(generateSessionID(), wsi);
        }
        else
        {
            LOG_DEBUG("Unauthenticated http connect from: " << client_ip);
            if (cfg->websocket.http_secured)
            {
                LOG_DEBUG("Connection refused.");
                if (lws_return_http_status(wsi, HTTP_STATUS_FORBIDDEN, NULL) ||
                    lws_http_transaction_completed(wsi)) {
                    return -1;
                }
            }
        }

        // http GET
        if (request_method == 0)
        {
            // Send preview image
            if (strcmp(url_ptr, "/preview.jpg") == 0)
            {
                u_ctx->flag |= PNT_FLAG_HTTP_SEND_PREVIEW;

                global_jpeg[0]->request();

                if (!global_jpeg[0]->active)
                {
                    global_jpeg[0]->should_grab_frames.notify_all();
                    global_jpeg[0]->is_activated.acquire();
                    /* we need this delay to grab a valid image when stream resume from sleep
                     * usleep is a bad choice, but lws_sul_schedule won't work as expected here
                     * hopfully we find a better solution later
                     */
                    usleep(cfg->websocket.first_image_delay * 1000);
                }

                lws_callback_on_writable(wsi);
                return 0;
            }
        }
        // http POST
        else if (request_method == 1)
        {
            // get content length
            if (strcmp(url_ptr, "/json") == 0 && strcmp(content_type, "application/json") == 0)
            {
                // Read content length header and store received data
                char length_str[16];
                if (lws_hdr_copy(wsi, length_str, sizeof(length_str), WSI_TOKEN_HTTP_CONTENT_LENGTH) > 0)
                {
                    if (atoi(length_str))
                    {
                        u_ctx->flag |= PNT_FLAG_HTTP_RECEIVED_MESSAGE;
                    }
                }
                return 0;
            }
        }

        // not implemented
        u_ctx->flag |= PNT_FLAG_HTTP_SEND_INVALID;
        lws_callback_on_writable(wsi);
        return 0;
        break;

    case LWS_CALLBACK_HTTP_BODY:
        LOG_DDEBUG("LWS_CALLBACK_HTTP_BODY ip:" << client_ip);
        u_ctx->rx_message.append(json_data);
        break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION: //LWS_CALLBACK_HTTP_BODY:
        LOG_DDEBUG("LWS_CALLBACK_HTTP_BODY ip:" << client_ip << ", data:" << u_ctx->rx_message);

        if (u_ctx->flag & PNT_FLAG_HTTP_RECEIVED_MESSAGE)
        {
            // parse json and write response into u_ctx->message
            u_ctx->message = "{";               // open response json
            lejp_construct(&ctx, root_callback, u_ctx, root_keys, LWS_ARRAY_SIZE(root_keys));
            lejp_parse(&ctx, (uint8_t *)u_ctx->rx_message.c_str(), u_ctx->rx_message.length());
            lejp_destruct(&ctx);
            u_ctx->message.append("}");         // close response json
            u_ctx->rx_message.clear();          // cleanup received data
            u_ctx->flag &= ~PNT_FLAG_SEPARATOR; // always reset separator after parsing
            u_ctx->flag |= PNT_FLAG_HTTP_SEND_MESSAGE;

            /* copy response into u_ctx->message into u_ctx->tx_message
             * can be helpfull to handle overlapping requests in future
             */ 
            u_ctx->tx_message = u_ctx->message;
            u_ctx->tx_message.clear();

            // send response
            lws_callback_on_writable(wsi);

            return 0;
        }
        break;

    case LWS_CALLBACK_HTTP_WRITEABLE:
        LOG_DDEBUG("LWS_CALLBACK_HTTP_WRITEABLE ip:" << client_ip << " " << (int)u_ctx->flag);

        {
            uint8_t header[LWS_PRE + 1024];
            memset(header, 0, sizeof(header));
            uint8_t *start = &header[LWS_PRE];
            uint8_t *p = &header[LWS_PRE];
            uint8_t *end = &header[sizeof(header) - 1];

            if (u_ctx->flag & PNT_FLAG_HTTP_SEND_PREVIEW)
            {
                u_ctx->flag &= ~PNT_FLAG_HTTP_SEND_PREVIEW;

                // Write image
                std::vector<unsigned char> jpeg_buf;
                if (get_snapshot(jpeg_buf))
                {
                    if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "image/jpeg", jpeg_buf.size() - LWS_PRE, &p, end) ||
                        lws_finalize_write_http_header(wsi, start, &p, end) ||
                        !lws_write(wsi, jpeg_buf.data() + LWS_PRE, jpeg_buf.size() - LWS_PRE, LWS_WRITE_BINARY) ||
                        lws_http_transaction_completed(wsi))
                    {

                        LOG_ERROR("lws error sending image");
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
            }

            if (u_ctx->flag & PNT_FLAG_HTTP_SEND_MESSAGE)
            {
                u_ctx->flag &= ~PNT_FLAG_HTTP_SEND_MESSAGE;
                LOG_DDEBUG("/json " << u_ctx->flag);
                if (!u_ctx->message.empty())
                {
                    LOG_DDEBUG("TO " << client_ip << ":  " << u_ctx->message);

                    // Prepare the HTTP headers
                    if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "application/json", u_ctx->message.length(), &p, end) ||
                        lws_finalize_write_http_header(wsi, start, &p, end) ||
                        !lws_write(wsi, (unsigned char *)u_ctx->message.c_str(), u_ctx->message.length(), LWS_WRITE_TEXT) ||
                        lws_http_transaction_completed(wsi))
                    {

                        LOG_ERROR("lws error sending response");
                        return -1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                return 0;
            }

            if (lws_add_http_common_headers(wsi, HTTP_STATUS_NOT_IMPLEMENTED, "text/plain", 0, &p, end) ||
                lws_finalize_write_http_header(wsi, start, &p, end) ||
                lws_http_transaction_completed(wsi))
            {

                LOG_ERROR("lws error sending not implemented");
            };
            return -1;
        }
        break;

    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
        LOG_DDEBUG("LWS_CALLBACK_HTTP_DROP_PROTOCOL ip:" << client_ip << ", id:" << u_ctx->id);
        u_ctx->~user_ctx();
        break;

    default:
        break;
    }

    return 0;
}

void WS::start()
{
    char *ip = NULL;

    // create websocket authentication token and write it into /run/
    // only websocket connect with token parameter accepted
    // ws://<ip>:<port>/?token=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    strncpy(token, strdup(generateToken(WEBSOCKET_TOKEN_LENGTH).c_str()), WEBSOCKET_TOKEN_LENGTH);
    std::ofstream outFile("/run/prudynt_websocket_token");
    outFile << token;
    outFile.close();

    LOG_DEBUG("WS TOKEN::" << token);

    lws_set_log_level(cfg->websocket.loglevel, lwsl_emit_stderr);

    protocols.name = cfg->websocket.name;
    protocols.callback = ws_callback;
    protocols.per_session_data_size = sizeof(user_ctx);
    protocols.rx_buffer_size = 65536;

    memset(&info, 0, sizeof(info));
    info.port = cfg->websocket.port;
    info.iface = ip;
    info.protocols = &protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);

    if (!context)
    {
        LOG_ERROR("lws init failed");
    }

    LOG_INFO("Server started on port " << cfg->websocket.port);

    while (true)
    {
        lws_service(context, 50);
    }

    LOG_INFO("Server stopped.");

    lws_context_destroy(context);
}

void *WS::run(void *arg)
{
    ((WS *)arg)->start();
    return nullptr;
}
