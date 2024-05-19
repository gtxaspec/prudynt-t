#include "WS.hpp"
#include "libwebsockets.h"

#define MODULE "WEBSOCKET"

int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {

    int m;
    std::string sendMsg = "PRUDYNT";

    char* sendChar = new char[sendMsg.length() + 1];
    strcpy(sendChar, sendMsg.c_str());

    int strLength = strlen(sendChar) + 1; 

    unsigned char *sendBuffer = 
        (unsigned char *)malloc(LWS_PRE + strLength);
    memcpy (sendBuffer + LWS_PRE, sendChar, strLength );

    switch (reason) {

        case LWS_CALLBACK_ESTABLISHED:
            LOG_DEBUG("LWS_CALLBACK_ESTABLISHED");      
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            LOG_DEBUG("LWS_CALLBACK_SERVER_WRITEABLE");
            m = lws_write(wsi, sendBuffer + LWS_PRE, strLength, LWS_WRITE_TEXT);
            free(sendBuffer);
            break;

        case LWS_CALLBACK_RECEIVE:
            LOG_DEBUG("LWS_CALLBACK_RECEIVE length: " << (int)len << ", text: " << (char *)in);
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

void WS::run() {

    int opt;
    char *ip = NULL;
    int port = 8089;

    protocols.name = "WSS prudynt";
    protocols.callback = callback;
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