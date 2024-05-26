#include "WS.hpp"
#include <random>
#include <fstream>
#include "Config.hpp"
#include "libwebsockets.h"

#define MODULE "WEBSOCKET"

extern void restart_encoder();
extern std::atomic<int> enc_thread_signal;

/* ROOT */
enum {
    PNT_GENERAL=1,
    PNT_RTSP,
    PNT_SENSOR,
    PNT_STREAM0,
    PNT_STREAM1,
    PNT_OSD,
    PNT_MOTION,
    PNT_INFO,
    PNT_ACTION
};

static const char * const root_keys[] = {
    "general",
    "rtsp",
    "sensor",
    "stream0",
    "stream1",
    "osd",
    "motion",
    "info",
    "action"
};

/* GENERAL */
enum {
   PNT_GENERAL_LOGLEVEL = 1,
   PNT_TEST,
   PNT_TEST2
};

static const char * const general_keys[] = {
    "loglevel",
    "test",
    "xxx"
};

/* RTSP */
enum {
   PNT_RTSP_PORT = 1,
   PNT_RTSP_NAME
};

static const char * const rtsp_keys[] = {
    "port",
    "name"
};

/* SENSOR */
enum {
   PNT_SENSOR_MODEL = 1
};

static const char * const sensor_keys[] = {
    "model"
};

/* STREAM0 */
enum {
   PNT_STREAM0_RTSP_ENDBOINT = 1
};

static const char * const stream0_keys[] = {
    "rtsp_endpoint"
};

/* STREAM1 */
enum {
   PNT_STREAM1_JPEG_ENABLED = 1
};

static const char * const stream1_keys[] = {
    "jpeg_enabled"
};

/* OSD */
enum {
   PNT_OSD_ENABLED = 1
};

static const char * const osd_keys[] = {
    "enabled"
};

/* MOTION */
enum {
   PNT_MOTION_ENABLED = 1
};

static const char * const motion_keys[] = {
    "enabled"
};

/* INFO */
enum {
   PNT_INFO_IMP_SYSTEM_VERSION = 1
};

static const char * const info_keys[] = {
    "imp_system_version"
};

/* ACTION */
enum {
   PNT_RESTART_ENCODER = 1
};

static const char * const action_keys[] = {
    "restart_encoder"
};

char token[WEBSOCKET_TOKEN_LENGTH+1]{0};
char ws_send_msg[2048]{0};

struct user_ctx {
    bool s = 0;
};

std::string generateToken(int length) {
    static const char characters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static const int maxIndex = sizeof(characters) - 1;
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, maxIndex);
    
    std::string randomString;
    randomString.reserve(length);
    
    for (int i = 0; i < length; i++) {
        randomString += characters[distribution(generator)];
    }
    
    return randomString;
}  

template<typename... Args>
void append_message(const char *t, Args&&... a) {

    char message[128]{0};
    snprintf(message, sizeof(message), t, std::forward<Args>(a)...);
    std::strcat(ws_send_msg, message);   
}

signed char WS::general_callback(struct lejp_ctx *ctx, char reason)
{
    if ((reason & LEJP_FLAG_CB_IS_VALUE) && ctx->path_match) {
        
        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", general_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_GENERAL_LOGLEVEL:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "\"%s\"", Config::singleton()->logLevel.c_str());
                break;
            case PNT_TEST:
                append_message(
                    "\"%s\"", "OK");
                break;                                                    
            case PNT_TEST2:
                append_message(
                    "\"%s\"", "OK");
                break;                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);        
    }

    return 0;
}

signed char WS::rtsp_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", rtsp_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_RTSP_PORT:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "%d", Config::singleton()->rtspPort);
                break;  
            case PNT_RTSP_NAME:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "\"%s\"", Config::singleton()->rtspName.c_str());                
                break;                                                                 
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::sensor_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", sensor_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_SENSOR_MODEL:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "\"%s\"", Config::singleton()->sensorModel.c_str());                
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::stream0_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", stream0_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_STREAM0_RTSP_ENDBOINT:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "\"%s\"", Config::singleton()->stream0endpoint.c_str());                 
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::stream1_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", stream1_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_STREAM1_JPEG_ENABLED:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "%s", Config::singleton()->stream1jpegEnable?"true":"false");                 
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::osd_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", osd_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_OSD_ENABLED:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "%s", Config::singleton()->OSDEnable?"true":"false");
                break;                 
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::motion_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", general_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_MOTION_ENABLED:
                if(reason != LEJPCB_VAL_NULL) {

                }
                append_message(
                    "%s", Config::singleton()->motionEnable?"true":"false");                
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::info_callback(struct lejp_ctx *ctx, char reason)
{    
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", info_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_INFO_IMP_SYSTEM_VERSION:
                IMPVersion impVersion;
                int ret = IMP_System_GetVersion(&impVersion);
                if(ret) {
                    append_message(
                        "\"%s\"", impVersion.aVersion);
                } else {
                    append_message(
                        "\"%s\"", "error");                    
                }
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::action_callback(struct lejp_ctx *ctx, char reason)
{
    if (reason & LEJP_FLAG_CB_IS_VALUE && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":", u_ctx->s?",":"", action_keys[ctx->path_match-1]);

        switch (ctx->path_match) {
            case PNT_RESTART_ENCODER:
                //...... tbd
                append_message(
                    "\"%s\"", "done");  
                break;                                                  
            }

        u_ctx->s = 1;

    } else if(reason == LEJPCB_OBJECT_END) {
        std::strcat(ws_send_msg, "}"); 
        lejp_parser_pop(ctx);
    }

    return 0;
}

signed char WS::root_callback(struct lejp_ctx *ctx, char reason)
{
    if ((reason & LEJPCB_OBJECT_START) && ctx->path_match) {

        struct user_ctx *u_ctx = (struct user_ctx *)ctx->user;

        append_message(
            "%s\"%s\":{", u_ctx->s?",":"", root_keys[ctx->path_match-1]);

        u_ctx->s = 0;

        switch (ctx->path_match) {
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
            case PNT_STREAM0:
                lejp_parser_push(ctx, u_ctx, 
                    stream0_keys, LWS_ARRAY_SIZE(stream0_keys), stream0_callback);
                break;
            case PNT_STREAM1:
                lejp_parser_push(ctx, u_ctx, 
                    stream1_keys, LWS_ARRAY_SIZE(stream1_keys), stream1_callback);
                break;
            case PNT_OSD:
                lejp_parser_push(ctx, u_ctx, 
                    osd_keys, LWS_ARRAY_SIZE(osd_keys), osd_callback);
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

int WS::ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

    struct lejp_ctx ctx;
    struct user_ctx u_ctx;

    int ws_send_msg_length, url_length;
    char url_token[128];
    memset(url_token, 0, 128);

    std::string json_data((char*)in, len);

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            LOG_DEBUG("LWS_CALLBACK_ESTABLISHED");
            url_length = lws_get_urlarg_by_name_safe(wsi, "token", url_token, sizeof(url_token));
            if(url_length != WEBSOCKET_TOKEN_LENGTH || strcmp(token, url_token) != 0) {
                LOG_DEBUG("Unauthenticated websocket connect.");
                //return -1;
            }               
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            LOG_DEBUG("LWS_CALLBACK_SERVER_WRITEABLE");

            ws_send_msg_length = strlen(ws_send_msg);
            if(ws_send_msg_length) {
                std::cout << "SEND MESSAGE: " << ws_send_msg << std::endl;
                lws_write(wsi, (unsigned char *)ws_send_msg, ws_send_msg_length, LWS_WRITE_TEXT);
                memset(ws_send_msg, 0, sizeof(ws_send_msg));
            }
            break;

        case LWS_CALLBACK_RECEIVE:
            LOG_DEBUG("LWS_CALLBACK_RECEIVE");

            std::strcat(ws_send_msg, "{"); //start response json

            lejp_construct(&ctx, root_callback, &u_ctx, root_keys, LWS_ARRAY_SIZE(root_keys));
            lejp_parse(&ctx, (uint8_t *)json_data.c_str(), json_data.length());
            lejp_destruct(&ctx);

            std::strcat(ws_send_msg, "}"); //close response json

            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLOSED:
            LOG_DEBUG("LWS_CALLBACK_CLOSED");
            break;

        default:
            break;
    }
    return 0;
}

void WS::run(CFG *cfg) {

    int opt;
    char *ip = NULL;
    int port = 8089;

    //create websocket authentication token and write it into /run/
    //only websocket connect with token parameter accepted
    //ws://<ip>:<port>/?token=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    strncpy(token, generateToken(WEBSOCKET_TOKEN_LENGTH).c_str(), WEBSOCKET_TOKEN_LENGTH);
    std::ofstream outFile("/run/prudynt_websocket_token");
    outFile << token;
    outFile.close();

    LOG_DEBUG("WS TOKEN::" << token);

    protocols.name = "WSS prudynt";
    protocols.callback = ws_callback;
    protocols.per_session_data_size = 0;
    protocols.rx_buffer_size = 65536;

    memset(&info, 0, sizeof(info));
    info.port = port;
    info.iface = ip;
    info.protocols = &protocols;
    info.gid = -1;
    info.uid = -1;

    context = lws_create_context(&info);

    if (!context) {
        LOG_ERROR("lws init failed");
    }

    LOG_INFO("Server started on port " << port);

    while (1) {
        lws_service(context, 50);
    }

    LOG_INFO("Server stopped.");

    lws_context_destroy(context);
}