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

#define MODULE "WEBSOCKET"

#pragma region keys_and_enums

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
    PNT_TEST,
    PNT_TEST2
};

static const char *const general_keys[] = {
    "loglevel",
    "test",
    "xxx"};

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
    "name"
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
    PNT_AUDIO_INPUT_VOL,
    PNT_AUDIO_INPUT_GAIN,
    PNT_AUDIO_INPUT_ALC_GAIN,
    PNT_AUDIO_OUTPUT_ENABLED,
    PNT_AUDIO_OUTPUT_VOL,
    PNT_AUDIO_OUTPUT_GAIN,
    PNT_AUDIO_INPUT_ECHO_CANCELLATION,
    PNT_AUDIO_INPUT_NOISE_SUPPRESSION,
    PNT_AUDIO_INPUT_HIGH_PASS_FILTER,
    PNT_AUDIO_OUTPUT_HIGH_PASS_FILTER
};

static const char *const audio_keys[] = {
    "input_enabled",
    "input_vol",
    "input_gain",
    "input_alc_gain",
    "output_enabled",
    "output_vol",
    "output_gain",
    "input_echo_cancellation",
    "input_noise_suppression",
    "input_high_pass_filter",
    "output_high_pass_filter"};
#endif

/* STREAM */
enum
{
    PNT_STREAM_RTSP_ENDPOINT = 1,
    PNT_STREAM_SCALE_ENABLED,
    PNT_STREAM_FORMAT,
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
    PNT_STREAM_OSD
};

static const char *const stream_keys[] = {
    "rtsp_endpoint",
    "scale_enabled",
    "format",
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
    "osd"};

/* STREAM2 (JPEG) */
enum
{
    PNT_STREAM2_JPEG_ENABLED = 1,
    PNT_STREAM2_JPEG_PATH,
    PNT_STREAM2_JPEG_QUALITY,
    PNT_STREAM2_JPEG_REFRESH
};

static const char *const stream2_keys[] = {
    "jpeg_enabled",
    "jpeg_path",
    "jpeg_quality",
    "jpeg_refresh"};

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
    PNT_STOP_THREAD = 1,
    PNT_START_THREAD,
    PNT_RESTART_THREAD,
    PNT_SAVE_CONFIG
};

enum
{
    PNT_THREAD_RTSP = 1,
    PNT_THREAD_ENCODER = 2,
    PNT_THREAD_ACTION_STOP = 8,
    PNT_THREAD_ACTION_START = 16,
    PNT_THREAD_ACTION_RESTART = PNT_THREAD_ACTION_STOP | PNT_THREAD_ACTION_START
};

static const char *const action_keys[] = {
    "stop_thread",
    "start_thread",
    "restart_thread",
    "save_config"};

#pragma endregion keys_and_enums

char token[WEBSOCKET_TOKEN_LENGTH + 1]{0};
char ws_send_msg[2048];

struct user_ctx
{
    WS *ws;
    bool s;
    std::string root;
    std::string path;
    int signal;
    roi region;
    int flag;
    int midx;
    int vidx;
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

template <typename... Args>
void append_message(const char *t, Args &&...a)
{

    char message[128];
    memset(message, 0, sizeof(message));
    std::cout << message << std::endl;
    snprintf(message, sizeof(message), t, std::forward<Args>(a)...);
    std::strcat(ws_send_msg, message);
}

signed char WS::general_callback(struct lejp_ctx *ctx, char reason)
{
    if ((reason & LEJP_FLAG_CB_IS_VALUE) && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", general_keys[ctx->path_match - 1]);

        switch (ctx->path_match)
        {
        case PNT_GENERAL_LOGLEVEL:
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                    Logger::setLevel(ctx->buf);
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
            break;
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::rtsp_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", rtsp_keys[ctx->path_match - 1]);

        // int values
        if (ctx->path_match >= PNT_RTSP_PORT && ctx->path_match <= PNT_RTSP_SEND_BUFFER_SIZE)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    // better restart rtsp manually ?
                    // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART; // restart RTSP
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
            // const char * values
        }
        else if (ctx->path_match >= PNT_RTSP_NAME && ctx->path_match <= PNT_RTSP_PASSWORD)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                    // better restart rtsp manually ?
                    // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_RTSP_AUTH_REQUIRED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        // better restart rtsp manually ?
                        // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        // better restart rtsp manually ?
                        // u_ctx->signal = PNT_THREAD_RTSP | PNT_THREAD_ACTION_RESTART;
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            }
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::sensor_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", sensor_keys[ctx->path_match - 1]);

        // int values
        if (ctx->path_match >= PNT_SENSOR_FPS && ctx->path_match <= PNT_SENSOR_HEIGHT)
        {
            /* normally this cannot be set and is read from proc
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    u_ctx->signal = PNT_THREAD_ENCODER | PNT_THREAD_ACTION_RESTART; //restart encoder
                }
            }
            */
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        else
        {

            switch (ctx->path_match)
            {
            case PNT_SENSOR_MODEL:
                /* normally this cannot be set and is read from proc
                if (reason == LEJPCB_VAL_STR_END)
                {
                    if (u_ctx->ws->cfg->set<std::string>(u_ctx->path, ctx->buf))
                    {
                        u_ctx->signal = PNT_THREAD_ENCODER | PNT_THREAD_ACTION_RESTART; //restart encoder
                    }
                }
                */
                append_message(
                    "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
                break;
            case PNT_SENSOR_I2C_ADDRESS:
                /* normally this cannot be set and is read from proc
                if (reason == LEJPCB_VAL_STR_END)
                {
                    if (u_ctx->ws->cfg->set<unsigned int>(u_ctx->path, (unsigned int)strtol(ctx->buf, NULL, 16)))
                    {
                        u_ctx->signal = PNT_THREAD_ENCODER | PNT_THREAD_ACTION_RESTART; //restart encoder
                    }
                }
                */
                append_message(
                    "\"%#x\"", u_ctx->ws->cfg->get<unsigned int>(u_ctx->path));
                break;
            }
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::image_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", image_keys[ctx->path_match - 1]);

        if (ctx->path_match == PNT_IMAGE_DEFOG_STRENGTH)
        {
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    uint8_t t = static_cast<uint8_t>(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    IMP_ISP_Tuning_SetDefog_Strength(reinterpret_cast<uint8_t *>(&t));
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
            append_message(
                "%s", "null");
#endif
        }
        else if (ctx->path_match >= PNT_IMAGE_CORE_WB_MODE && ctx->path_match <= PNT_IMAGE_WB_BGAIN)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                    IMPISPWB wb;
                    memset(&wb, 0, sizeof(IMPISPWB));
                    int ret = IMP_ISP_Tuning_GetWB(&wb);
                    if (ret == 0)
                    {
                        wb.mode = (isp_core_wb_mode)u_ctx->ws->cfg->image.core_wb_mode;
                        wb.rgain = u_ctx->ws->cfg->image.wb_rgain;
                        wb.bgain = u_ctx->ws->cfg->image.wb_bgain;
                        ret = IMP_ISP_Tuning_SetWB(&wb);
                    }
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        else
        {

            switch (ctx->path_match)
            {
            case PNT_IMAGE_BRIGHTNESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBrightness(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_CONTRAST:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetContrast(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_HUE:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBcshHue(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
                append_message(
                    "%s", "null");
#endif
                break;
            case PNT_IMAGE_SATURATION:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSaturation(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_SHARPNESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSharpness(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_SINTER_STRENGTH:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetSinterStrength(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_TEMPER_STRENGTH:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetTemperStrength(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_VFLIP:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_ENABLE);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_ISP_Tuning_SetISPVflip(IMPISP_TUNING_OPS_MODE_DISABLE);
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            case PNT_IMAGE_HFLIP:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_ENABLE);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_ISP_Tuning_SetISPHflip(IMPISP_TUNING_OPS_MODE_DISABLE);
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            case PNT_IMAGE_ANTIFLICKER:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetAntiFlickerAttr((IMPISPAntiflickerAttr)u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_RUNNING_MODE:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetISPRunningMode((IMPISPRunningMode)u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_AE_COMPENSATION:
#if !defined(PLATFORM_T21)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetAeComp(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
#else
                append_message(
                    "%s", "null");
#endif
            case PNT_IMAGE_DPC_STRENGTH:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetDPC_Strength(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
                append_message(
                    "%s", "null");
#endif
                break;
            case PNT_IMAGE_DRC_STRENGTH:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetDRC_Strength(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
                append_message(
                    "%s", "null");
#endif
                break;
            case PNT_IMAGE_HIGHLIGHT_DEPRESS:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetHiLightDepress(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_BACKLIGHT_COMPENSTATION:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetBacklightComp(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
                append_message(
                    "%s", "null");
#endif
                break;
            case PNT_IMAGE_MAX_AGAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetMaxAgain(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_IMAGE_MAX_DGAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_ISP_Tuning_SetMaxDgain(u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            }
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

#if defined(AUDIO_SUPPORT)
signed char WS::audio_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", audio_keys[ctx->path_match - 1]);

        if (ctx->path_match == PNT_AUDIO_OUTPUT_HIGH_PASS_FILTER)
        {
            IMPAudioIOAttr ioattr;
            int ret = IMP_AO_GetPubAttr(0, &ioattr);
            if (ret == 0)
            {
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        ret = IMP_AO_EnableHpf(&ioattr);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        ret = IMP_AO_DisableHpf();
                    }
                }
            }
            append_message(
                "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
        }
        else if (ctx->path_match == PNT_AUDIO_INPUT_HIGH_PASS_FILTER)
        {
            IMPAudioIOAttr ioattr;
            int ret = IMP_AI_GetPubAttr(0, &ioattr);
            if (ret == 0)
            {
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        ret = IMP_AI_EnableHpf(&ioattr);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        ret = IMP_AI_DisableHpf();
                    }
                }
            }
            append_message(
                "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
        }
        else if (ctx->path_match == PNT_AUDIO_INPUT_NOISE_SUPPRESSION)
        {
            IMPAudioIOAttr ioattr;
            int ret = IMP_AI_GetPubAttr(0, &ioattr);
            if (ret == 0)
            {
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    int ns = atoi(ctx->buf);
                    if (ns)
                    {
                        if (u_ctx->ws->cfg->set<int>(u_ctx->path, ns))
                        {
                            ret = IMP_AI_EnableNs(&ioattr, u_ctx->ws->cfg->get<int>(u_ctx->path));
                        }
                    }
                    else
                    {
                        if (u_ctx->ws->cfg->set<int>(u_ctx->path, ns))
                        {
                            ret = IMP_AI_DisableNs();
                        }
                    }
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        else
        {

            switch (ctx->path_match)
            {
            case PNT_AUDIO_INPUT_ENABLED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_AI_Enable(0);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_AI_Disable(0);
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            case PNT_AUDIO_INPUT_VOL:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetVol(0, 0, u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_GAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetGain(0, 0, u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_ALC_GAIN:
#if !defined(PLATFORM_T10) && !defined(PLATFORM_T20) && !defined(PLATFORM_T21) && !defined(PLATFORM_T23) && !defined(PLATFORM_T30)
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AI_SetAlcGain(0, 0, u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
#else
                append_message(
                    "%s", "null");
#endif
                break;
            case PNT_AUDIO_OUTPUT_ENABLED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_AO_Enable(0);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_AO_Disable(0);
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            case PNT_AUDIO_OUTPUT_VOL:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AO_SetVol(0, 0, u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_OUTPUT_GAIN:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                    {
                        IMP_AO_SetGain(0, 0, u_ctx->ws->cfg->get<int>(u_ctx->path));
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_AUDIO_INPUT_ECHO_CANCELLATION:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, true))
                    {
                        IMP_AI_EnableAec(0, 0, 0, 0);
                    }
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    if (u_ctx->ws->cfg->set<bool>(u_ctx->path, false))
                    {
                        IMP_AI_DisableAec(0, 0);
                    }
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            }
        }
        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
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
        // LOG_DEBUG("stream_callback: " << u_ctx->path << " = " << (char *)ctx->buf);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", stream_keys[ctx->path_match - 1]);

        if ((ctx->path_match >= PNT_STREAM_GOP && ctx->path_match <= PNT_STREAM_OSD))
        { // integer values
            if (reason == LEJPCB_VAL_NUM_INT)
                u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_STREAM_RTSP_ENDPOINT:
                if (reason == LEJPCB_VAL_STR_END)
                    u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                append_message(
                    "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
                break;
            case PNT_STREAM_SCALE_ENABLED:
                if (reason == LEJPCB_VAL_TRUE)
                {
                    u_ctx->ws->cfg->set<bool>(u_ctx->path, true);
                }
                else if (reason == LEJPCB_VAL_FALSE)
                {
                    u_ctx->ws->cfg->set<bool>(u_ctx->path, false);
                }
                append_message(
                    "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
                break;
            case PNT_STREAM_FORMAT:
                if (reason == LEJPCB_VAL_STR_END)
                    u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf));
                append_message(
                    "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
                break;
            };
        }

        u_ctx->s = 1;
    }
    else if (reason == LECPCB_PAIR_NAME && ctx->path_match == PNT_STREAM_OSD)
    {

        append_message(
            "%s\"%s\":{", u_ctx->s ? "," : "", stream_keys[ctx->path_match - 1]);

        lejp_parser_push(ctx, u_ctx,
                         osd_keys, LWS_ARRAY_SIZE(osd_keys), osd_callback);
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::stream2_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", stream2_keys[ctx->path_match - 1]);

        switch (ctx->path_match)
        {
        case PNT_STREAM2_JPEG_ENABLED:
            if (reason == LEJPCB_VAL_TRUE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, false);
            }
            append_message(
                "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
            break;
        case PNT_STREAM2_JPEG_PATH:
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
            break;
        case PNT_STREAM2_JPEG_QUALITY:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
            break;
        case PNT_STREAM2_JPEG_REFRESH:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {
                }
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
            break;
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::osd_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {
        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->path + "." + std::string(ctx->path);

        // LOG_DEBUG("osd_callback: " << u_ctx->path << " = " << (char *)ctx->buf << ", " << ctx->path_match);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", osd_keys[ctx->path_match - 1]);

        if (ctx->path_match >= PNT_OSD_TIME_TRANSPARENCY &&
            ctx->path_match <= PNT_OSD_LOGO_TRANSPARENCY)
        {
            int hnd;
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                if (u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf)))
                {

                    _regions regions;

                    if (u_ctx->flag == 0)
                    {
                        regions = u_ctx->ws->cfg->stream0.osd.regions;
                    }
                    else if (u_ctx->flag == 1)
                    {
                        regions = u_ctx->ws->cfg->stream1.osd.regions;
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
                        ;
                        break;
                    case PNT_OSD_LOGO_TRANSPARENCY:
                        hnd = regions.logo;
                        ;
                        break;
                    }

                    IMPOSDGrpRgnAttr grpRgnAttr;
                    int ret = IMP_OSD_GetGrpRgnAttr(hnd, u_ctx->flag, &grpRgnAttr);

                    if (ret == 0)
                    {
                        memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
                        grpRgnAttr.show = 1;
                        grpRgnAttr.gAlphaEn = 1;
                        grpRgnAttr.fgAlhpa = u_ctx->ws->cfg->get<int>(u_ctx->path);
                        IMP_OSD_SetGrpRgnAttr(hnd, u_ctx->flag, &grpRgnAttr);
                    }
                };
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        // integer
        else if (ctx->path_match >= PNT_OSD_FONT_SIZE && ctx->path_match <= PNT_OSD_UPTIME_ROTATION)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
        }
        // bool
        else if (ctx->path_match >= PNT_OSD_ENABLED && ctx->path_match <= PNT_OSD_FONT_STROKE_ENABLED)
        {
            if (reason == LEJPCB_VAL_TRUE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, false);
            }
            append_message(
                "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
        }
        // const char *
        else if (ctx->path_match >= PNT_OSD_FONT_PATH && ctx->path_match <= PNT_OSD_LOGO_PATH)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
        }
        // unsigned int
        else if (ctx->path_match >= PNT_OSD_FONT_COLOR && ctx->path_match <= PNT_OSD_FONT_STROKE_COLOR)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<unsigned int>(u_ctx->path, (unsigned int)strtoll(ctx->buf, NULL, 16)))
                {
                }
            }
            append_message(
                "\"%#x\"", u_ctx->ws->cfg->get<unsigned int>(u_ctx->path));
        }
        else
        {
            switch (ctx->path_match)
            {
            case PNT_OSD_POS_LOGO_X:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                    IMPOSDRgnAttr rgnAttr;
                    memset(&rgnAttr, u_ctx->flag, sizeof(IMPOSDRgnAttr));
                    if (IMP_OSD_GetRgnAttr(3, &rgnAttr) == 0)
                    {
                        if (u_ctx->flag == 0)
                        {
                            OSD::set_pos(&rgnAttr, u_ctx->ws->cfg->stream0.osd.pos_logo_x, u_ctx->ws->cfg->stream0.osd.pos_logo_y);
                        }
                        else if (u_ctx->flag == 1)
                        {
                            OSD::set_pos(&rgnAttr, u_ctx->ws->cfg->stream1.osd.pos_logo_x, u_ctx->ws->cfg->stream1.osd.pos_logo_y);
                        }
                        IMP_OSD_SetRgnAttr(3, &rgnAttr);
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_OSD_POS_LOGO_Y:
                if (reason == LEJPCB_VAL_NUM_INT)
                {
                    u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                    IMPOSDRgnAttr rgnAttr;
                    memset(&rgnAttr, u_ctx->flag, sizeof(IMPOSDRgnAttr));
                    if (IMP_OSD_GetRgnAttr(3, &rgnAttr) == 0)
                    {
                        if (u_ctx->flag == 0)
                        {
                            OSD::set_pos(&rgnAttr, u_ctx->ws->cfg->stream0.osd.pos_logo_y, u_ctx->ws->cfg->stream0.osd.pos_logo_y);
                        }
                        else if (u_ctx->flag == 1)
                        {
                            OSD::set_pos(&rgnAttr, u_ctx->ws->cfg->stream1.osd.pos_logo_y, u_ctx->ws->cfg->stream1.osd.pos_logo_y);
                        }
                        IMP_OSD_SetRgnAttr(3, &rgnAttr);
                    }
                }
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            case PNT_OSD_LOGO_ROTATION:
                // encoder restart required
                if (reason == LEJPCB_VAL_NUM_INT)
                    u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
                append_message(
                    "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
                break;
            };
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
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

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", motion_keys[ctx->path_match - 1]);

        // integer
        if (ctx->path_match >= PNT_MOTION_DEBOUNCE_TIME && ctx->path_match <= PNT_MOTION_ROI_COUNT)
        {
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                u_ctx->ws->cfg->set<int>(u_ctx->path, atoi(ctx->buf));
            }
            append_message(
                "%d", u_ctx->ws->cfg->get<int>(u_ctx->path));
            // bool
        }
        else if (ctx->path_match == PNT_MOTION_ENABLED)
        {
            if (reason == LEJPCB_VAL_TRUE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, true);
            }
            else if (reason == LEJPCB_VAL_FALSE)
            {
                u_ctx->ws->cfg->set<bool>(u_ctx->path, false);
            }
            append_message(
                "%s", u_ctx->ws->cfg->get<bool>(u_ctx->path) ? "true" : "false");
            // std::string
        }
        else if (ctx->path_match == PNT_MOTION_SCRIPT_PATH)
        {
            if (reason == LEJPCB_VAL_STR_END)
            {
                if (u_ctx->ws->cfg->set<const char *>(u_ctx->path, strdup(ctx->buf)))
                {
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<const char *>(u_ctx->path));
        }
        else if (ctx->path_match == PNT_MOTION_ROIS)
        {
            if (reason == LEJPCB_VAL_NULL)
            {
                for (int i = 0; i < u_ctx->ws->cfg->motion.roi_count; i++)
                {
                }
            }
            append_message(
                "\"%s\"", u_ctx->ws->cfg->get<std::string>(u_ctx->path).c_str());
        }

        u_ctx->s = 1;
    }
    else if (reason == LECPCB_PAIR_NAME && ctx->path_match == PNT_MOTION_ROIS)
    {
        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", motion_keys[ctx->path_match - 1]);

        u_ctx->flag = 0;
        lejp_parser_push(ctx, u_ctx,
                         motion_keys, LWS_ARRAY_SIZE(motion_keys), motion_roi_callback);

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
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
        std::strcat(ws_send_msg, "[");
        for (int i = 0; i < u_ctx->ws->cfg->motion.roi_count; i++)
        {
            if ((u_ctx->flag & 4))
                std::strcat(ws_send_msg, ",");
            append_message(
                "[%d,%d,%d,%d]", u_ctx->ws->cfg->motion.rois[i].p0_x, u_ctx->ws->cfg->motion.rois[i].p0_x, u_ctx->region.p0_y,
                u_ctx->ws->cfg->motion.rois[i].p1_x, u_ctx->ws->cfg->motion.rois[i].p1_y);
            u_ctx->flag |= 4;
        }
        std::strcat(ws_send_msg, "]");
        lejp_parser_pop(ctx);
    }
    else
    {
        switch (reason)
        {
        case LEJPCB_ARRAY_START:
            if ((u_ctx->flag & 4))
            {
                std::strcat(ws_send_msg, ",");
            }
            if ((u_ctx->flag & 1) != 1)
            {
                u_ctx->flag |= 1; // main array
                u_ctx->midx = 0;  // main array index
                std::strcat(ws_send_msg, "[");
            }
            else
            {
                u_ctx->flag |= 2; // entry array
                u_ctx->vidx = 0;  // entry array index
                std::strcat(ws_send_msg, "[");
            }
            break;

        case LEJPCB_VAL_NUM_INT:
            if (u_ctx->flag & 2)
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
            if (u_ctx->flag & 2)
            {
                u_ctx->flag ^= 2;
                if (u_ctx->vidx >= 4)
                {
                    append_message(
                        "%d,%d,%d,%d", u_ctx->region.p0_x, u_ctx->region.p0_y, u_ctx->region.p1_x, u_ctx->region.p1_y);
                }
                std::strcat(ws_send_msg, "]");
                u_ctx->flag |= 4;
                if (u_ctx->midx <= 52)
                {
                    u_ctx->ws->cfg->motion.rois[u_ctx->midx] =
                        {u_ctx->region.p0_x, u_ctx->region.p0_y, u_ctx->region.p1_x, u_ctx->region.p1_y};
                    u_ctx->midx++;
                }
            }
            else if (u_ctx->flag & 1)
            {
                u_ctx->flag ^= 1;
                u_ctx->ws->cfg->motion.roi_count = u_ctx->midx;
                std::strcat(ws_send_msg, "]");
                lejp_parser_pop(ctx);
            }
            break;
        }
    }
    return 0;
}

signed char WS::info_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", info_keys[ctx->path_match - 1]);

        switch (ctx->path_match)
        {
        case PNT_INFO_IMP_SYSTEM_VERSION:
            IMPVersion impVersion;
            int ret = IMP_System_GetVersion(&impVersion);
            if (ret)
            {
                append_message(
                    "\"%s\"", impVersion.aVersion);
            }
            else
            {
                append_message(
                    "\"%s\"", "error");
            }
            break;
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::action_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match)
    {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;
        u_ctx->path = u_ctx->root + "." + std::string(ctx->path);

        append_message(
            "%s\"%s\":", u_ctx->s ? "," : "", action_keys[ctx->path_match - 1]);

        switch (ctx->path_match)
        {
        case PNT_STOP_THREAD:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                u_ctx->signal = atoi(ctx->buf);          // stop targets
                u_ctx->signal |= PNT_THREAD_ACTION_STOP; // stop action
            }
            append_message(
                "\"%s\"", "initiated");
            break;
        case PNT_START_THREAD:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                u_ctx->signal = atoi(ctx->buf);           // start targets
                u_ctx->signal |= PNT_THREAD_ACTION_START; // start action
            }
            append_message(
                "\"%s\"", "initiated");
            break;
        case PNT_RESTART_THREAD:
            if (reason == LEJPCB_VAL_NUM_INT)
            {
                u_ctx->signal = atoi(ctx->buf);             // restart targets
                u_ctx->signal |= PNT_THREAD_ACTION_RESTART; // restart action
            }
            append_message(
                "\"%s\"", "initiated");
            break;
        case PNT_SAVE_CONFIG:
            u_ctx->ws->cfg->updateConfig();
            append_message(
                "\"%s\"", "initiated");
            break;
        }

        u_ctx->s = 1;
    }
    else if (reason == LEJPCB_OBJECT_END)
    {
        std::strcat(ws_send_msg, "}");
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

        append_message(
            "%s\"%s\":{", u_ctx->s ? "," : "", root_keys[ctx->path_match - 1]);

        u_ctx->s = 0;

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
            lejp_parser_push(ctx, u_ctx,
                             audio_keys, LWS_ARRAY_SIZE(audio_keys), audio_callback);
            break;
#endif

        case PNT_STREAM0:
            u_ctx->flag = 0;
            lejp_parser_push(ctx, &u_ctx,
                             stream_keys, LWS_ARRAY_SIZE(stream_keys), stream_callback);
            break;
        case PNT_STREAM1:
            u_ctx->flag = 1;
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

int WS::ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

    struct lejp_ctx ctx;

    user_ctx *u_ctx = (user_ctx *)lws_context_user(lws_get_context(wsi));
    u_ctx->s = 0;

    char client_ip[128];
    lws_get_peer_simple(wsi, client_ip, sizeof(client_ip));

    int ws_send_msg_length, url_length;
    char url_token[128];
    memset(url_token, 0, 128);

    std::string json_data((char *)in, len);

    switch (reason)
    {
    case LWS_CALLBACK_ESTABLISHED:
        LOG_DEBUG("LWS_CALLBACK_ESTABLISHED " << client_ip);

        url_length = lws_get_urlarg_by_name_safe(wsi, "token", url_token, sizeof(url_token));
        if (url_length != WEBSOCKET_TOKEN_LENGTH || strcmp(token, url_token) != 0)
        {
            LOG_DEBUG("Unauthenticated websocket connect");
            if (u_ctx->ws->cfg->websocket.secured)
            {
                LOG_DEBUG("Connection refused.");
                return -1;
            }
        }
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        LOG_DEBUG("LWS_CALLBACK_SERVER_WRITEABLE");

        ws_send_msg_length = strlen(ws_send_msg);
        if (ws_send_msg_length)
        {
            LOG_DEBUG("TO " << client_ip << ":  " << ws_send_msg);
            lws_write(wsi, (unsigned char *)ws_send_msg, ws_send_msg_length, LWS_WRITE_TEXT);
            memset(ws_send_msg, 0, sizeof(ws_send_msg));
        }
        break;

    case LWS_CALLBACK_RECEIVE:
        LOG_DEBUG("LWS_CALLBACK_RECEIVE " << client_ip << ", " << json_data);

        // cleanup response buffer
        memset(ws_send_msg, 0, sizeof(ws_send_msg));

        std::strcat(ws_send_msg, "{"); // start response json
        lejp_construct(&ctx, root_callback, u_ctx, root_keys, LWS_ARRAY_SIZE(root_keys));
        lejp_parse(&ctx, (uint8_t *)json_data.c_str(), json_data.length());
        lejp_destruct(&ctx);

        std::strcat(ws_send_msg, "}"); // close response json

        if (u_ctx->signal != 0)
        {
            u_ctx->ws->cfg->main_thread_signal.store(u_ctx->signal);
            u_ctx->ws->cfg->main_thread_signal.notify_one();
            u_ctx->signal = 0;
        }

        lws_callback_on_writable(wsi);
        break;

    case LWS_CALLBACK_CLOSED:
        LOG_DEBUG("LWS_CALLBACK_CLOSED " << client_ip);
        break;

    default:
        break;
    }

    return 0;
}

void WS::run()
{

    int opt;
    char *ip = NULL;

    // create websocket authentication token and write it into /run/
    // only websocket connect with token parameter accepted
    // ws://<ip>:<port>/?token=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    strncpy(token, generateToken(WEBSOCKET_TOKEN_LENGTH).c_str(), WEBSOCKET_TOKEN_LENGTH);
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

    // add current class instances to lws context
    user_ctx u_ctx;
    u_ctx.ws = this;
    u_ctx.s = 0;
    info.user = &u_ctx;

    context = lws_create_context(&info);

    if (!context)
    {
        LOG_ERROR("lws init failed");
    }

    LOG_INFO("Server started on port " << cfg->websocket.port);

    while (1)
    {
        lws_service(context, 50);
    }

    LOG_INFO("Server stopped.");

    lws_context_destroy(context);
}