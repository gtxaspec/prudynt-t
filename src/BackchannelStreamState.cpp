#include "BackchannelStreamState.hpp"

#include "Logger.hpp"

#define MODULE "BackchannelStreamState"

BackchannelStreamState::BackchannelStreamState(UsageEnvironment &env,
                                               char const *cname,
                                               RTPSource *_rtpSource,
                                               BackchannelSink *_mediaSink,
                                               Groupsock *_rtpGS,
                                               Groupsock *_rtcpGS,
                                               unsigned _clientSessionId,
                                               struct sockaddr_storage const &_destAddr,
                                               Port const &_rtpDestPort,
                                               Port const &_rtcpDestPort,
                                               int _tcpSocketNum,
                                               unsigned char _rtpChannelId,
                                               unsigned char _rtcpChannelId,
                                               TLSState *_tlsState)
    : fEnv(env)
    , fCNAME(cname)
    , rtpSource(_rtpSource)
    , mediaSink(_mediaSink)
    , rtpGS(_rtpGS)
    , rtcpGS(_rtcpGS)
    , rtcpInstance(nullptr)
    , clientSessionId(_clientSessionId)
    , fIsTCP(_tcpSocketNum >= 0)
{
    // Initialize the appropriate part of the transport union
    if (fIsTCP)
    {
        fTransport.t.tcpSocketNum = _tcpSocketNum;
        fTransport.t.rtpChannelId = _rtpChannelId;
        fTransport.t.rtcpChannelId = _rtcpChannelId;
        fTransport.t.tlsState = _tlsState;
    }
    else
    {
        fTransport.u.destAddr = _destAddr;
        fTransport.u.rtpDestPort = _rtpDestPort;
        fTransport.u.rtcpDestPort = _rtcpDestPort;
    }
}
BackchannelStreamState::~BackchannelStreamState()
{
    LOG_DEBUG("Destroyed for session " << clientSessionId);
    Medium::close(rtcpInstance);
    Medium::close(rtpSource);
    Medium::close(mediaSink);

    // Delete groupsocks
    delete rtpGS;
    if (rtcpGS != nullptr && rtcpGS != rtpGS)
    { // Avoid double delete if multiplexing
        delete rtcpGS;
    }
}

void BackchannelStreamState::startPlaying(
    TaskFunc *rtcpRRHandler,
    void *rtcpRRHandlerClientData,
    ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
    void *serverRequestAlternativeByteHandlerClientData)
{
    if (mediaSink && rtpSource)
    {
        // Create RTCP instance using stored env and CNAME
        rtcpInstance = RTCPInstance::createNew(fEnv,
                                               rtcpGS,
                                               64 /*est BW kbps*/,
                                               (unsigned char *) fCNAME,
                                               nullptr /*sink*/,
                                               rtpSource /*source*/,
                                               True /*is server*/);
        if (rtcpInstance)
        {
            rtcpInstance->setRRHandler(rtcpRRHandler, rtcpRRHandlerClientData);

            // Configure transport based on stored flag and union data
            if (fIsTCP)
            {
                LOG_INFO("Configuring stream for TCP (session "
                         << clientSessionId << ", socket " << fTransport.t.tcpSocketNum
                         << ", RTP ch " << (int) fTransport.t.rtpChannelId << ", RTCP ch "
                         << (int) fTransport.t.rtcpChannelId << ")");
                rtpSource->setStreamSocket(fTransport.t.tcpSocketNum,
                                           fTransport.t.rtpChannelId,
                                           fTransport.t.tlsState);
                rtcpInstance->addStreamSocket(fTransport.t.tcpSocketNum,
                                              fTransport.t.rtcpChannelId,
                                              fTransport.t.tlsState);

                // Use stored env
                RTPInterface::setServerRequestAlternativeByteHandler(
                    fEnv,
                    fTransport.t.tcpSocketNum,
                    serverRequestAlternativeByteHandler,
                    serverRequestAlternativeByteHandlerClientData);

                // Set specific RR handler for TCP
                struct sockaddr_storage tcpSocketNumAsAddress;
                memset(&tcpSocketNumAsAddress, 0, sizeof(tcpSocketNumAsAddress));
                tcpSocketNumAsAddress.ss_family = AF_INET;
                ((struct sockaddr_in &) tcpSocketNumAsAddress).sin_addr.s_addr = htonl(
                    fTransport.t.tcpSocketNum);
                rtcpInstance->setSpecificRRHandler(tcpSocketNumAsAddress,
                                                   fTransport.t.rtcpChannelId,
                                                   rtcpRRHandler,
                                                   rtcpRRHandlerClientData);
            }
            else
            { // UDP
                LOG_INFO("Configuring stream for UDP (session " << clientSessionId << ")");
                if (rtpGS)
                    rtpGS->addDestination(fTransport.u.destAddr,
                                          fTransport.u.rtpDestPort,
                                          clientSessionId);
                if (rtcpGS)
                    rtcpGS->addDestination(fTransport.u.destAddr,
                                           fTransport.u.rtcpDestPort,
                                           clientSessionId);
                rtcpInstance->setSpecificRRHandler(fTransport.u.destAddr,
                                                   fTransport.u.rtcpDestPort,
                                                   rtcpRRHandler,
                                                   rtcpRRHandlerClientData);
            }

            rtcpInstance->sendReport(); // Send initial RTCP report
        }
        else
        {
            LOG_WARN("Failed to create RTCPInstance for session " << clientSessionId);
        }

        // Connect the sink to the source
        if (!mediaSink->startPlaying(*rtpSource, nullptr, this))
        {
            LOG_ERROR("mediaSink->startPlaying failed for client session " << clientSessionId);
            Medium::close(rtcpInstance);
            rtcpInstance = nullptr;
        }
    }
    else
    {
        if (!mediaSink)
            LOG_ERROR("Cannot start playing - mediaSink is NULL for client session "
                      << clientSessionId);
        else if (!rtpSource)
            LOG_ERROR("Cannot start playing - rtpSource is NULL for client session "
                      << clientSessionId);
        else
            LOG_ERROR("Cannot start playing - unknown reason for client session "
                      << clientSessionId);
    }
}
