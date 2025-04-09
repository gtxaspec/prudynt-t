#ifndef BACKCHANNEL_SERVER_MEDIA_SUBSESSION_HPP
#define BACKCHANNEL_SERVER_MEDIA_SUBSESSION_HPP

// Handles the server side of a backchannel audio stream within an RTSP session,
// creating the sink, managing stream parameters, and handling a single format
// for all clients.

#define MAX_CNAME_LEN 100

#include "BackchannelSink.hpp"
#include "BackchannelStreamState.hpp"
#include "IMPBackchannel.hpp"

#include <liveMedia.hh>

class BackchannelServerMediaSubsession : public OnDemandServerMediaSubsession
{
    friend class BackchannelStreamState;

public:
    static BackchannelServerMediaSubsession *createNew(UsageEnvironment &env,
                                                       IMPBackchannelFormat format);

    virtual ~BackchannelServerMediaSubsession();

protected:
    BackchannelServerMediaSubsession(UsageEnvironment &env, IMPBackchannelFormat format);

    virtual char const *sdpLines(int addressFamily);
    virtual char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource);

    virtual MediaSink *createNewStreamDestination(unsigned clientSessionId, unsigned &estBitrate);
    virtual RTPSource *createNewRTPSource(Groupsock *rtpGroupsock,
                                          unsigned char rtpPayloadTypeIfDynamic,
                                          MediaSink *outputSink);
    virtual void getStreamParameters(unsigned clientSessionId,
                                     struct sockaddr_storage const &clientAddress,
                                     Port const &clientRTPPort,
                                     Port const &clientRTCPPort,
                                     int tcpSocketNum,
                                     unsigned char rtpChannelId,
                                     unsigned char rtcpChannelId,
                                     TLSState *tlsState,
                                     struct sockaddr_storage &destinationAddress,
                                     u_int8_t &destinationTTL,
                                     Boolean &isMulticast,
                                     Port &serverRTPPort,
                                     Port &serverRTCPPort,
                                     void *&streamToken);

    virtual void startStream(unsigned clientSessionId,
                             void *streamToken,
                             TaskFunc *rtcpRRHandler,
                             void *rtcpRRHandlerClientData,
                             unsigned short &rtpSeqNum,
                             unsigned &rtpTimestamp,
                             ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
                             void *serverRequestAlternativeByteHandlerClientData);
    virtual void deleteStream(unsigned clientSessionId, void *&streamToken);

    virtual void getRTPSinkandRTCP(void *streamToken, RTPSink *&rtpSink, RTCPInstance *&rtcp);
    virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
    virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource *inputSource);

private:
    bool allocateUdpPorts(Port &serverRTPPort,
                          Port &serverRTCPPort,
                          Groupsock *&rtpGroupsock,
                          Groupsock *&rtcpGroupsock);
    int estimatedBitrate();

    char *fSDPLines = nullptr;      // Cached SDP lines
    char fCNAME[MAX_CNAME_LEN + 1]; // For RTCP
    portNumBits fInitialPortNum;    // Starting port for UDP allocation
    bool fMultiplexRTCPWithRTP;     // Whether to multiplex RTCP with RTP
    IMPBackchannelFormat fFormat;   // Audio format for this subsession
};

#endif // BACKCHANNEL_SERVER_MEDIA_SUBSESSION_HPP
