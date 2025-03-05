#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

// explicit instantiation
template class IMPDeviceSource<H264NALUnit, video_stream>;
template class IMPDeviceSource<AudioFrame, audio_stream>;

template<typename FrameType, typename Stream>
IMPDeviceSource<FrameType, Stream> *IMPDeviceSource<FrameType, Stream>::createNew(UsageEnvironment &env, int encChn, std::shared_ptr<Stream> stream, const char *name)
{
    return new IMPDeviceSource<FrameType, Stream>(env, encChn, stream, name);
}

template<typename FrameType, typename Stream>
IMPDeviceSource<FrameType, Stream>::IMPDeviceSource(UsageEnvironment &env, int encChn, std::shared_ptr<Stream> stream, const char *name)
    : FramedSource(env), encChn(encChn), stream{stream}, name{name}, eventTriggerId(0), firstFrame(true)     
{
    std::lock_guard lock_stream {mutex_main};
    std::lock_guard lock_callback {stream->onDataCallbackLock};
    stream->onDataCallback = [this]()
    { this->on_data_available(); };
    stream->hasDataCallback = true;

    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    stream->should_grab_frames.notify_one();
    LOG_DEBUG("IMPDeviceSource " << name << " constructed, encoder channel:" << encChn);
}

template<typename FrameType, typename Stream>
void IMPDeviceSource<FrameType, Stream>::deinit()
{
    std::lock_guard lock_stream {mutex_main};
    std::lock_guard lock_callback {stream->onDataCallbackLock};
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    stream->hasDataCallback = false;
    stream->onDataCallback = nullptr;
    LOG_DEBUG("IMPDeviceSource " << name << " destructed, encoder channel:" << encChn);
}

template<typename FrameType, typename Stream>
IMPDeviceSource<FrameType, Stream>::~IMPDeviceSource()
{
    deinit();
}

template<typename FrameType, typename Stream>
void IMPDeviceSource<FrameType, Stream>::doGetNextFrame()
{
    deliverFrame();
}

template <typename FrameType, typename Stream>
void IMPDeviceSource<FrameType, Stream>::deliverFrame0(void *clientData)
{
    ((IMPDeviceSource<FrameType, Stream> *)clientData)->deliverFrame();
}

template <typename FrameType, typename Stream>
void IMPDeviceSource<FrameType, Stream>::deliverFrame()
{
    if (!isCurrentlyAwaitingData())
        return;

    FrameType nal;
    if (stream->msgChannel->read(&nal))
    {
        // Check if the NAL unit is too large for the buffer
        if (nal.data.size() > fMaxSize)
        {
            // If we're truncating the data, it's better to discard the frame entirely
            // to avoid corrupted video rather than sending a partial frame
            LOG_DEBUG("Frame size " << nal.data.size() << " exceeds buffer size " << fMaxSize << 
                      ". Dropping frame to avoid corruption.");
            fFrameSize = 0;
            
            // Signal that we've processed the frame without actually delivering it
            // This is better than delivering a corrupted frame
            FramedSource::afterGetting(this);
            return;
        }
        else
        {
            fFrameSize = nal.data.size();
        }

        // Use timestamps that are guaranteed to increment
        if (firstFrame) {
            // Start with a small non-zero timestamp to avoid mpv "Invalid video timestamp" errors
            struct timeval first_frame_time;
            first_frame_time.tv_sec = 0;
            first_frame_time.tv_usec = 1000; // Start at 1ms instead of 0
            fPresentationTime = first_frame_time;
            firstFrame = false;
        } else {
            // For subsequent frames, use the encoder-provided timestamp
            fPresentationTime = nal.time;
        }
        
        memcpy(fTo, &nal.data[0], fFrameSize);

        if (fFrameSize > 0)
        {
            FramedSource::afterGetting(this);
        }
    }
    else
    {
        fFrameSize = 0;
    }
}
