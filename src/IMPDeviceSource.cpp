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
    : FramedSource(env), encChn(encChn), eventTriggerId(0), stream{stream}, name{name}
{
    std::lock_guard lock_stream {stream->lock};
    stream->onDataCallback = [this]()
    { this->on_data_available(); };

    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    stream->should_grab_frames.notify_one();
    LOG_DEBUG("IMPDeviceSource " << name << " constructed, encoder channel:" << encChn);
}

template<typename FrameType, typename Stream>
void IMPDeviceSource<FrameType, Stream>::deinit()
{
    std::lock_guard lock_stream {stream->lock};
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
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
        if (nal.data.size() > fMaxSize)
        {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = nal.data.size() - fMaxSize;
        }
        else
        {
            fFrameSize = nal.data.size();
        }

        fPresentationTime = nal.time;
        
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
