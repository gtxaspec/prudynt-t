#ifndef BACKCHANNEL_SINK_HPP
#define BACKCHANNEL_SINK_HPP

// Receives audio data, encapsulates it in BackchannelFrame, enqueues it, and
// handles inactivity timeouts by sending stop frames.

#include "IMPBackchannel.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <liveMedia.hh>

class BackchannelSink : public MediaSink
{
public:
    static BackchannelSink *createNew(UsageEnvironment &env,
                                      unsigned clientSessionId,
                                      IMPBackchannelFormat format);

    Boolean startPlaying(FramedSource &source,
                         MediaSink::afterPlayingFunc *afterFunc,
                         void *afterClientData);
    void stopPlaying();

protected:
    BackchannelSink(UsageEnvironment &env, unsigned clientSessionId, IMPBackchannelFormat format);
    virtual ~BackchannelSink();

    virtual Boolean continuePlaying();

private:
    void scheduleTimeoutCheck();
    static void timeoutCheck(void *clientData);
    void timeoutCheck1();

    static void afterGettingFrame(void *clientData,
                                  unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    void afterGettingFrame1(unsigned frameSize,
                            unsigned numTruncatedBytes,
                            struct timeval presentationTime);

    void sendBackchannelFrame(const uint8_t *payload, unsigned payloadSize);
    void sendBackchannelStopFrame();

    FramedSource *fRTPSource;
    u_int8_t *fReceiveBuffer;
    int fReceiveBufferSize;

    bool fIsActive;
    MediaSink::afterPlayingFunc *fAfterFunc;
    void *fAfterClientData;

    unsigned fClientSessionId;
    TaskToken fTimeoutTask;

    bool fIsSending;
    const IMPBackchannelFormat fFormat;
};

#endif // BACKCHANNEL_SINK_HPP
