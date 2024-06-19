#ifndef WS_hpp
#define WS_hpp

#include <atomic>
#include "Logger.hpp"
#include "Config.hpp"
#include <memory>
#include "libwebsockets.h"
#include <imp/imp_system.h>
#include <imp/imp_common.h>

#define WEBSOCKET_PORT 8089
#define WEBSOCKET_TOKEN_LENGTH 32

// WebSocket
class WS {
    
    public:
        void run();  
        WS(std::shared_ptr<CFG> _cfg, std::shared_ptr<std::atomic<int>> _mts) : cfg(_cfg), main_thread_signal(_mts) {}
        void restartEncoder();

    private:
        
        std::shared_ptr<CFG> cfg;
        std::shared_ptr<std::atomic<int>> main_thread_signal;
        
        lws_protocols protocols;
        struct lws_context_creation_info info;     
        struct lws_context *context;  

        static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

        static signed char root_callback(struct lejp_ctx *ctx, char reason);
        static signed char general_callback(struct lejp_ctx *ctx, char reason);
        static signed char rtsp_callback(struct lejp_ctx *ctx, char reason);
        static signed char sensor_callback(struct lejp_ctx *ctx, char reason);
        static signed char image_callback(struct lejp_ctx *ctx, char reason);
#if defined(AUDIO_SUPPORT)           
        static signed char audio_callback(struct lejp_ctx *ctx, char reason);
#endif
        static signed char stream_callback(struct lejp_ctx *ctx, char reason);
        static signed char stream1_callback(struct lejp_ctx *ctx, char reason);
        static signed char osd_callback(struct lejp_ctx *ctx, char reason);
        static signed char motion_callback(struct lejp_ctx *ctx, char reason);
        static signed char motion_roi_callback(struct lejp_ctx *ctx, char reason);
        static signed char info_callback(struct lejp_ctx *ctx, char reason);
        static signed char action_callback(struct lejp_ctx *ctx, char reason);

};

#endif
