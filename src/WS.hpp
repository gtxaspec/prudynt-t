#ifndef WS_hpp
#define WS_hpp

#include "Logger.hpp"
#include "libwebsockets.h"

// WebSocket
class WS {
    
    public:
        void run();    

    private:
        lws_protocols protocols;
        struct lws_context_creation_info info;     
        struct lws_context *context;  
};

#endif
