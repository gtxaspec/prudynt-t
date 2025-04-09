#ifndef BACKCHANNEL_STREAM_STATE_HPP
#define BACKCHANNEL_STREAM_STATE_HPP

// Manages the state of a backchannel audio stream, associating RTP source,
// sink, groupsocks, and transport details.

#include "BackchannelSink.hpp"

#include <liveMedia.hh>

struct UdpTransportDetails
{
    struct sockaddr_storage destAddr;
    Port rtpDestPort;
    Port rtcpDestPort;
};

struct TcpTransportDetails
{
    int tcpSocketNum;
    unsigned char rtpChannelId;
    unsigned char rtcpChannelId;
    TLSState *tlsState;
};

union TransportSpecificDetails {
    UdpTransportDetails u;
    TcpTransportDetails t;

    TransportSpecificDetails() {}
    ~TransportSpecificDetails() {}
};

class BackchannelStreamState
{
    // Restore friendship to allow subsession access to private members
    friend class BackchannelServerMediaSubsession;

public:
    // Constructor takes UsageEnvironment and CNAME instead of master reference
    BackchannelStreamState(UsageEnvironment &env,
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
                           TLSState *_tlsState);

    ~BackchannelStreamState();

    // Configures transport and starts the sink playing
    void startPlaying(TaskFunc *rtcpRRHandler,
                      void *rtcpRRHandlerClientData,
                      ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
                      void *serverRequestAlternativeByteHandlerClientData);

private:
    UsageEnvironment &fEnv;
    char const *fCNAME; // CNAME for RTCP reports
    RTPSource *rtpSource;
    BackchannelSink *mediaSink;
    Groupsock *rtpGS;
    Groupsock *rtcpGS;          // Groupsock for RTCP
    RTCPInstance *rtcpInstance; // RTCP instance associated with the stream
    unsigned clientSessionId;   // ID of the client for this stream

    Boolean fIsTCP;                      // Flag indicating TCP or UDP transport
    TransportSpecificDetails fTransport; // Union holding transport-specific details
};

#endif // BACKCHANNEL_STREAM_STATE_HPP
