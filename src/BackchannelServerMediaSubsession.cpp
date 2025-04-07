#include "BackchannelServerMediaSubsession.hpp"

#include "BackchannelSink.hpp"
#include "BackchannelStreamState.hpp"
#include "Config.hpp"
#include "IMPBackchannel.hpp"
#include "Logger.hpp"
#include "globals.hpp"

#include <GroupsockHelper.hh>

#define MODULE "BackchannelSubsession"

BackchannelServerMediaSubsession *BackchannelServerMediaSubsession::createNew(
    UsageEnvironment &env, IMPBackchannelFormat format)
{
    return new BackchannelServerMediaSubsession(env, format);
}

BackchannelServerMediaSubsession::BackchannelServerMediaSubsession(UsageEnvironment &env,
                                                                   IMPBackchannelFormat format)
    : OnDemandServerMediaSubsession(env, False)
    , fSDPLines(nullptr)
    , fInitialPortNum(6970)
    , fMultiplexRTCPWithRTP(false)
    , fFormat(format)
{
    LOG_DEBUG("Subsession created for channel " << static_cast<int>(fFormat));
    gethostname(fCNAME, MAX_CNAME_LEN);
    fCNAME[MAX_CNAME_LEN] = '\0';

    // Adjust initial port number for RTCP if not multiplexing
    if (!fMultiplexRTCPWithRTP)
    {
        fInitialPortNum = (fInitialPortNum + 1) & ~1;
    }
}

BackchannelServerMediaSubsession::~BackchannelServerMediaSubsession()
{
    LOG_DEBUG("Subsession destroyed");
    delete[] fSDPLines;
}

char const *BackchannelServerMediaSubsession::sdpLines(int /*addressFamily*/)
{
    if (fSDPLines == nullptr)
    {
        unsigned int sdpLinesSize = 400;
        fSDPLines = new char[sdpLinesSize];
        if (fSDPLines == nullptr)
            return nullptr;

        const char *formatName;
        int payloadType;
        unsigned frequency;

#define APPLY_SDP_IF(EnumName, NameString, PayloadType, Frequency, MimeType) \
    if (fFormat == IMPBackchannelFormat::EnumName) \
    { \
        formatName = NameString; \
        payloadType = PayloadType; \
        frequency = Frequency; \
    }

        X_FOREACH_BACKCHANNEL_FORMAT(APPLY_SDP_IF)
#undef APPLY_SDP_IF

        LOG_DEBUG("Generating SDP for format " << formatName << " (Payload Type: " << payloadType
                                               << ")");

        snprintf(fSDPLines,
                 sdpLinesSize,
                 "m=audio 0 RTP/AVP %d\r\n"
                 "c=IN IP4 0.0.0.0\r\n"
                 "b=AS:%u\r\n"
                 "a=rtpmap:%d %s/%u/%d\r\n"
                 "a=control:%s\r\n"
                 "a=sendonly\r\n",
                 payloadType,
                 estimatedBitrate(),
                 payloadType,
                 formatName,
                 frequency,
                 1,
                 trackId());

        fSDPLines[sdpLinesSize - 1] = '\0'; // Ensure null termination
    }
    return fSDPLines;
}

char const *BackchannelServerMediaSubsession::getAuxSDPLine(RTPSink * /*rtpSink*/,
                                                            FramedSource * /*inputSource*/)
{
    // No auxiliary SDP line needed
    return nullptr;
}

MediaSink *BackchannelServerMediaSubsession::createNewStreamDestination(unsigned clientSessionId,
                                                                        unsigned & /*estBitrate*/)
{
    LOG_DEBUG("Creating BackchannelSink for channel: " << static_cast<int>(fFormat));
    return BackchannelSink::createNew(envir(), clientSessionId, fFormat);
}

RTPSource *BackchannelServerMediaSubsession::createNewRTPSource(
    Groupsock *rtpGroupsock, unsigned char /*rtpPayloadTypeIfDynamic*/, MediaSink * /*outputSink*/)
{
    const char *mimeType;
    int payloadType;
    unsigned frequency;

#define APPLY_SOURCE_IF(EnumName, NameString, PayloadType, Frequency, MimeType) \
    if (fFormat == IMPBackchannelFormat::EnumName) \
    { \
        mimeType = MimeType; \
        payloadType = PayloadType; \
        frequency = Frequency; \
    }
    X_FOREACH_BACKCHANNEL_FORMAT(APPLY_SOURCE_IF)
#undef APPLY_SOURCE_IF

    return SimpleRTPSource::createNew(envir(),
                                      rtpGroupsock,
                                      payloadType,
                                      frequency,
                                      mimeType,
                                      0,      // numChannels - currently always 0
                                      False); // allowMultipleFramesPerPacket
}

void BackchannelServerMediaSubsession::getStreamParameters(
    unsigned clientSessionId,
    struct sockaddr_storage const &clientAddress,
    Port const &clientRTPPort,
    Port const &clientRTCPPort,
    int tcpSocketNum,
    unsigned char rtpChannelId,
    unsigned char rtcpChannelId,
    TLSState *tlsState,
    struct sockaddr_storage &destinationAddress,
    u_int8_t & /*destinationTTL*/,
    Boolean &isMulticast,
    Port &serverRTPPort,
    Port &serverRTCPPort,
    void *&streamToken)
{
    isMulticast = False; // This subsession is always unicast
    streamToken = nullptr;

    // Set destination address if not provided by client
    if (addressIsNull(destinationAddress))
    {
        destinationAddress = clientAddress;
    }

    // Validate transport parameters
    if (clientRTPPort.num() == 0 && tcpSocketNum < 0)
    {
        LOG_ERROR(
            "getStreamParameters: Invalid parameters (no client ports or TCP socket) for session "
            << clientSessionId);
        return;
    }

    // Create MediaSink
    unsigned streamBitrate = 0;
    MediaSink *mediaSink = createNewStreamDestination(clientSessionId, streamBitrate);
    if (mediaSink == nullptr)
    {
        LOG_ERROR("getStreamParameters: createNewStreamDestination FAILED for session "
                  << clientSessionId);
        return;
    }

    // Initialize transport variables
    Groupsock *rtpGroupsock = nullptr;
    Groupsock *rtcpGroupsock = nullptr;
    Boolean isTCP = (tcpSocketNum >= 0);

    // Set up transport (UDP or TCP) and create Groupsocks
    if (!isTCP)
    { // UDP
        if (clientRTCPPort.num() == 0)
        {
            // This might be okay if the client doesn't intend to send RTCP Sender Reports
            LOG_WARN("Client requested UDP streaming but provided no RTCP port for session "
                     << clientSessionId);
        }

        // Allocate UDP ports and create groupsocks
        if (!allocateUdpPorts(serverRTPPort, serverRTCPPort, rtpGroupsock, rtcpGroupsock))
        {
            LOG_ERROR("getStreamParameters: Failed to allocate UDP ports for session "
                      << clientSessionId);
            Medium::close(mediaSink);
            // allocateUdpPorts cleans up its own groupsocks on failure
            return;
        }
    }
    else
    { // TCP
        serverRTPPort = 0;
        serverRTCPPort = 0;
        struct sockaddr_storage dummyAddr;
        memset(&dummyAddr, 0, sizeof dummyAddr);
        dummyAddr.ss_family = AF_INET;
        Port dummyPort(0);

        // Create dummy groupsocks
        rtpGroupsock = new Groupsock(envir(), dummyAddr, dummyPort, 0);
        if (rtpGroupsock == nullptr || rtpGroupsock->socketNum() < 0)
        {
            LOG_ERROR("getStreamParameters: Failed to create dummy RTP Groupsock for TCP session "
                      << clientSessionId);
            Medium::close(mediaSink);
            delete rtpGroupsock; // Safe even if null
            return;
        }

        rtcpGroupsock = new Groupsock(envir(), dummyAddr, dummyPort, 0);
        if (rtcpGroupsock == nullptr || rtcpGroupsock->socketNum() < 0)
        {
            LOG_ERROR("getStreamParameters: Failed to create dummy RTCP Groupsock for TCP session "
                      << clientSessionId);
            Medium::close(mediaSink);
            delete rtpGroupsock;
            delete rtcpGroupsock; // Safe even if null
            return;
        }
    }

    // Create the RTPSource (which receives data) using the established groupsock
    RTPSource *rtpSource = createNewRTPSource(rtpGroupsock, 0, mediaSink);
    if (rtpSource == nullptr)
    {
        LOG_ERROR("getStreamParameters: createNewRTPSource FAILED for session " << clientSessionId);
        Medium::close(mediaSink);
        delete rtpGroupsock;
        if (rtcpGroupsock != rtpGroupsock) // Avoid double delete if multiplexing
            delete rtcpGroupsock;
        return;
    }

    // Create our custom stream state object
    BackchannelStreamState *state = new BackchannelStreamState(envir(),
                                                               fCNAME,
                                                               rtpSource,
                                                               (BackchannelSink *) mediaSink,
                                                               rtpGroupsock,
                                                               rtcpGroupsock,
                                                               clientSessionId,
                                                               destinationAddress,
                                                               clientRTPPort,
                                                               clientRTCPPort,
                                                               tcpSocketNum,
                                                               rtpChannelId,
                                                               rtcpChannelId,
                                                               tlsState);

    if (state == nullptr)
    {
        LOG_ERROR("getStreamParameters: Failed to create BackchannelStreamState for session "
                  << clientSessionId);
        Medium::close(rtpSource);
        Medium::close(mediaSink);
        delete rtpGroupsock;
        if (rtcpGroupsock != rtpGroupsock)
            delete rtcpGroupsock;
        return;
    }

    // Success: Assign the state object to the stream token
    streamToken = (void *) state;
}

bool BackchannelServerMediaSubsession::allocateUdpPorts(Port &serverRTPPort,
                                                        Port &serverRTCPPort,
                                                        Groupsock *&rtpGroupsock,
                                                        Groupsock *&rtcpGroupsock)
{
    NoReuse dummy(envir());
    portNumBits serverPortNum = fInitialPortNum;

    while (1)
    {
        serverRTPPort = serverPortNum;
        struct sockaddr_storage nullAddr;
        memset(&nullAddr, 0, sizeof(nullAddr));
        nullAddr.ss_family = AF_INET;
        rtpGroupsock = createGroupsock(nullAddr, serverRTPPort);
        if (rtpGroupsock->socketNum() < 0)
        {
            delete rtpGroupsock;
            rtpGroupsock = nullptr;
            serverPortNum += (fMultiplexRTCPWithRTP ? 1 : 2);
            if (serverPortNum == 0)
                return False; // Port wrap-around check
            continue;
        }

        if (fMultiplexRTCPWithRTP)
        {
            serverRTCPPort = serverRTPPort;
            rtcpGroupsock = rtpGroupsock;
        }
        else
        {
            serverRTCPPort = ++serverPortNum;
            if (serverPortNum == 0)
            { // Port wrap-around check
                delete rtpGroupsock;
                rtpGroupsock = nullptr;
                return False;
            }
            rtcpGroupsock = createGroupsock(nullAddr, serverRTCPPort);
            if (rtcpGroupsock->socketNum() < 0)
            {
                delete rtpGroupsock;
                rtpGroupsock = nullptr;
                delete rtcpGroupsock;
                rtcpGroupsock = nullptr;
                ++serverPortNum; // Port pair failed, try next
                if (serverPortNum == 0)
                    return False; // Port wrap-around check
                continue;
            }
        }
        LOG_DEBUG("UDP port allocation succeeded. RTP=" << ntohs(serverRTPPort.num()) << ", RTCP="
                                                        << ntohs(serverRTCPPort.num()));
        return True; // Success
    }
}

void BackchannelServerMediaSubsession::startStream(
    unsigned clientSessionId,
    void *streamToken,
    TaskFunc *rtcpRRHandler,
    void *rtcpRRHandlerClientData,
    unsigned short &rtpSeqNum,
    unsigned &rtpTimestamp,
    ServerRequestAlternativeByteHandler *serverRequestAlternativeByteHandler,
    void *serverRequestAlternativeByteHandlerClientData)
{
    BackchannelStreamState *state = (BackchannelStreamState *) streamToken;

    if (state == nullptr)
    {
        LOG_DEBUG("Client setup/probe initiated (NULL streamToken) for session " << clientSessionId);
        return;
    }

    state->startPlaying(rtcpRRHandler,
                        rtcpRRHandlerClientData,
                        serverRequestAlternativeByteHandler,
                        serverRequestAlternativeByteHandlerClientData);

    // Set initial RTP seq num and timestamp
    rtpSeqNum = 0;
    rtpTimestamp = 0;
    RTPSource *rtpSource = state->rtpSource;
    if (rtpSource != nullptr)
    {
        rtpSeqNum = rtpSource->curPacketMarkerBit() ? (rtpSource->curPacketRTPSeqNum() + 1)
                                                    : rtpSource->curPacketRTPSeqNum();
    }
}

void BackchannelServerMediaSubsession::deleteStream(unsigned clientSessionId, void *&streamToken)
{
    BackchannelStreamState *state = (BackchannelStreamState *) streamToken;
    if (state != nullptr)
    {
        // Deleting the state object triggers its destructor for cleanup
        delete state;
        streamToken = nullptr;
    }
}

void BackchannelServerMediaSubsession::getRTPSinkandRTCP(void *streamToken,
                                                         RTPSink *&rtpSink,
                                                         RTCPInstance *&rtcp)
{
    // This subsession only receives, so no RTPSink or RTCP for sending.
    rtpSink = nullptr;
    rtcp = nullptr;
}

int BackchannelServerMediaSubsession::estimatedBitrate()
{
    return 64;
}

FramedSource *BackchannelServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/,
                                                                      unsigned & /*estBitrate */)
{
    // This subsession receives, it doesn't provide a source to an RTPSink.
    return nullptr;
}

RTPSink *BackchannelServerMediaSubsession::createNewRTPSink(Groupsock * /*rtpGroupsock*/,
                                                            unsigned char /*rtpPayloadTypeIfDynamic*/,
                                                            FramedSource * /*inputSource*/)
{
    // This subsession receives, it doesn't create an RTPSink for sending.
    return nullptr;
}
