#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"
#include <thread>
#include <chrono>

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
    : FramedSource(env), encChn(encChn), stream{stream}, name{name}, eventTriggerId(0), 
      firstFrame(true), base_timestamp(0), timestamp_initialized(false), droppedFrames(0)
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
        // Add frame pacing delays to prevent overwhelming the network stack
        // This is critical for proper stream delivery, especially for I-frames
        bool isIFrame = false;
        if constexpr (std::is_same_v<FrameType, H264NALUnit>) {
            // Check if this is an I-frame (typically much larger)
            if (nal.data.size() > 0 && nal.data.size() > 10000) {
                // I-frames are typically much larger, use a longer delay
                isIFrame = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
            } else {
                // Regular inter frames get a smaller delay
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        // Check if the frame is too large for the buffer
        if (nal.data.size() > fMaxSize)
        {
            // Track dropped frames for diagnostics
            droppedFrames++;
            
            // If we drop too many frames in succession, log a warning
            if (droppedFrames % 10 == 1) {
                LOG_ERROR("Frame size " << nal.data.size() << " exceeds buffer size " << fMaxSize << 
                          ". Dropped " << droppedFrames << " frames to avoid corruption.");
            }
            
            fFrameSize = 0;
            FramedSource::afterGetting(this);
            return;
        }
        else
        {
            fFrameSize = nal.data.size();
            // Reset the dropped frames counter when we successfully process a frame
            if (droppedFrames > 0) {
                droppedFrames = 0;
            }
        }

        // Timestamp normalization: ensure the first frame has zero timestamp
        // and all subsequent frames use normalized timestamps
        if (firstFrame) {
            // First frame ALWAYS has timestamp zero for clean synchronization
            struct timeval zero_time;
            zero_time.tv_sec = 0;
            zero_time.tv_usec = 0;  // Exactly zero for first frame
            fPresentationTime = zero_time;
            firstFrame = false;
            
            // Initialize our base timestamp for future normalization
            if constexpr (std::is_same_v<FrameType, H264NALUnit>) {
                base_timestamp = nal.imp_ts;
            } else {
                // For audio frames, convert the timeval to microseconds
                base_timestamp = (nal.time.tv_sec * 1000000LL) + nal.time.tv_usec;
            }
            timestamp_initialized = true;
        } else {
            // For subsequent frames, use normalized timestamps that start from zero
            struct timeval normalized_time;
            
            if (timestamp_initialized) {
                int64_t frame_ts;
                
                if constexpr (std::is_same_v<FrameType, H264NALUnit>) {
                    frame_ts = nal.imp_ts;
                } else {
                    // For audio frames, convert the timeval to microseconds
                    frame_ts = (nal.time.tv_sec * 1000000LL) + nal.time.tv_usec;
                }
                
                // Calculate normalized timestamp (relative to base timestamp)
                int64_t normalized_ts = frame_ts - base_timestamp;
                
                // Ensure we never use negative timestamps
                if (normalized_ts < 0) normalized_ts = 0;
                
                // Convert back to timeval format
                normalized_time.tv_sec = normalized_ts / 1000000;
                normalized_time.tv_usec = normalized_ts % 1000000;
                
                fPresentationTime = normalized_time;
            } else {
                // Fallback if timestamp wasn't initialized (shouldn't happen)
                fPresentationTime = nal.time;
            }
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
