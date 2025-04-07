#include "BackchannelSink.hpp"

#include "Logger.hpp"
#include "globals.hpp"

#define MODULE "BackchannelSink"

#define TIMEOUT_MICROSECONDS 500000 // Timeout set to 500ms

BackchannelSink *BackchannelSink::createNew(UsageEnvironment &env,
                                            unsigned clientSessionId,
                                            IMPBackchannelFormat format)
{
    return new BackchannelSink(env, clientSessionId, format);
}

BackchannelSink::BackchannelSink(UsageEnvironment &env,
                                 unsigned clientSessionId,
                                 IMPBackchannelFormat format)
    : MediaSink(env)
    , fRTPSource(nullptr)
    , fReceiveBufferSize(1024)
    , fIsActive(false)
    , fAfterFunc(nullptr)
    , fAfterClientData(nullptr)
    , fClientSessionId(clientSessionId)
    , fTimeoutTask(nullptr)
    , fIsSending(false)
    , fFormat(format)
{
    fReceiveBuffer = new u_int8_t[fReceiveBufferSize];
    if (fReceiveBuffer == nullptr)
    {
        LOG_ERROR("Failed to allocate receive buffer (Session: " << fClientSessionId << ")");
    }
}

BackchannelSink::~BackchannelSink()
{
    sendBackchannelStopFrame();
    delete[] fReceiveBuffer;
}

Boolean BackchannelSink::startPlaying(FramedSource &source,
                                      MediaSink::afterPlayingFunc *afterFunc,
                                      void *afterClientData)
{
    if (fIsActive)
    {
        LOG_WARN("startPlaying called while already active for session " << fClientSessionId);
        return False;
    }

    fRTPSource = &source;
    fAfterFunc = afterFunc;
    fAfterClientData = afterClientData;
    fIsActive = True;

    LOG_DEBUG("Sink starting consumption for session " << fClientSessionId);

    return continuePlaying();
}

void BackchannelSink::stopPlaying()
{
    if (!fIsActive)
    {
        return;
    }

    LOG_DEBUG("Sink stopping consumption for session " << fClientSessionId);

    // Set inactive *first* to prevent re-entrancy
    fIsActive = False;

    sendBackchannelStopFrame();

    envir().taskScheduler().unscheduleDelayedTask(fTimeoutTask);
    fTimeoutTask = nullptr;

    if (fRTPSource != nullptr)
    {
        fRTPSource->stopGettingFrames();
    }

    if (fAfterFunc != nullptr)
    {
        (*fAfterFunc)(fAfterClientData);
    }

    fRTPSource = nullptr;
    fAfterFunc = nullptr;
    fAfterClientData = nullptr;
}

Boolean BackchannelSink::continuePlaying()
{
    if (!fIsActive || fRTPSource == nullptr)
    {
        return False;
    }

    fRTPSource
        ->getNextFrame(fReceiveBuffer, fReceiveBufferSize, afterGettingFrame, this, nullptr, this);

    return True;
}

void BackchannelSink::afterGettingFrame(void *clientData,
                                        unsigned frameSize,
                                        unsigned numTruncatedBytes,
                                        struct timeval presentationTime,
                                        unsigned /*durationInMicroseconds*/)
{
    BackchannelSink *sink = static_cast<BackchannelSink *>(clientData);
    if (sink != nullptr)
    {
        sink->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime);
    }
    else
    {
        LOG_ERROR("afterGettingFrame called with invalid clientData");
    }
}

void BackchannelSink::afterGettingFrame1(unsigned frameSize,
                                         unsigned numTruncatedBytes,
                                         struct timeval presentationTime)
{
    if (!fIsActive)
    {
        return;
    }

    if (numTruncatedBytes > 0)
    {
        LOG_WARN("Received truncated frame (" << frameSize << " bytes, " << numTruncatedBytes
                                              << " truncated) for session " << fClientSessionId
                                              << ". Discarding.");
    }
    else if (frameSize > 0)
    {
        sendBackchannelFrame(fReceiveBuffer, frameSize);
    }

    // Reschedule the timeout check after receiving any frame (even size 0 or
    // truncated) This resets the timer as long as *something* is coming from the
    // source.
    envir().taskScheduler().unscheduleDelayedTask(fTimeoutTask);
    scheduleTimeoutCheck();

    if (fIsActive)
    {
        continuePlaying();
    }
}

void BackchannelSink::scheduleTimeoutCheck()
{
    fTimeoutTask = envir().taskScheduler().scheduleDelayedTask(TIMEOUT_MICROSECONDS,
                                                               (TaskFunc *) timeoutCheck,
                                                               this);
}

void BackchannelSink::timeoutCheck(void *clientData)
{
    BackchannelSink *sink = static_cast<BackchannelSink *>(clientData);
    if (sink)
    {
        sink->timeoutCheck1();
    }
}

void BackchannelSink::timeoutCheck1()
{
    fTimeoutTask = nullptr;

    if (!fIsActive)
    {
        return;
    }

    LOG_INFO("Audio data timeout detected for session " << fClientSessionId
                                                        << ". Sending stop frame.");
    sendBackchannelStopFrame();
}

void BackchannelSink::sendBackchannelFrame(const uint8_t *payload, unsigned payloadSize)
{
    if (!global_backchannel)
    {
        LOG_ERROR("global_backchannel is null, cannot queue BackchannelFrame! (Session: "
                  << fClientSessionId << ")");
        return;
    }

    if (!fIsSending)
    {
        fIsSending = true;
        global_backchannel->is_sending.fetch_add(1, std::memory_order_relaxed);
    }

    BackchannelFrame bcFrame;
    bcFrame.format = fFormat;
    bcFrame.clientSessionId = fClientSessionId;
    bcFrame.payload.assign(payload, payload + payloadSize);

    if (!global_backchannel->inputQueue->write(std::move(bcFrame)))
    {
        LOG_WARN("Input queue full for session " << fClientSessionId << ". Frame dropped.");
    }
    else
    {
        global_backchannel->should_grab_frames.notify_one();
    }
}

void BackchannelSink::sendBackchannelStopFrame()
{
    if (!fIsSending)
    {
        return;
    }

    if (global_backchannel)
    {
        BackchannelFrame stopFrame;
        stopFrame.format = fFormat;
        stopFrame.clientSessionId = fClientSessionId;
        stopFrame.payload.clear(); // Zero-size payload indicates stop/timeout
        if (!global_backchannel->inputQueue->write(std::move(stopFrame)))
        {
            LOG_WARN("Input queue full when trying to send stop frame for session "
                     << fClientSessionId);
        }
        else
        {
            global_backchannel->should_grab_frames.notify_one();
            fIsSending = false;
            global_backchannel->is_sending.fetch_sub(1, std::memory_order_relaxed);
            LOG_INFO("Sent stop frame (zero-payload frame) for session " << fClientSessionId);
        }
    }
    else
    {
        LOG_ERROR("global_backchannel is null, cannot send stop frame for session "
                  << fClientSessionId);
    }
}
