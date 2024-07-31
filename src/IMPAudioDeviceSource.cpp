#include "IMPAudioDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

IMPAudioDeviceSource *IMPAudioDeviceSource::createNew(UsageEnvironment &env, int aiChn)
{
    return new IMPAudioDeviceSource(env, aiChn);
}

IMPAudioDeviceSource::IMPAudioDeviceSource(UsageEnvironment &env, int aiChn)
    : FramedSource(env), aiChn(aiChn), eventTriggerId(0)
{
    std::lock_guard lock_stream {global_audio[aiChn]->lock};
    global_audio[aiChn]->onDataCallback = [this]()
    { this->on_data_available(); };

    eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    global_audio[aiChn]->should_grab_frames.notify_one();

    LOG_DEBUG("IMPAudioDeviceSource construct, encoder channel:" << aiChn);
}

void IMPAudioDeviceSource::deinit()
{
    std::lock_guard lock_stream {global_audio[aiChn]->lock};
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    global_audio[aiChn]->onDataCallback = nullptr;
    LOG_DEBUG("IMPAudioDeviceSource destruct, encoder channel:" << aiChn);
}

IMPAudioDeviceSource::~IMPAudioDeviceSource()
{
    deinit();
}

void IMPAudioDeviceSource::doGetNextFrame()
{
    deliverFrame();
}

void IMPAudioDeviceSource::deliverFrame0(void *clientData)
{
    ((IMPAudioDeviceSource *)clientData)->deliverFrame();
}

void IMPAudioDeviceSource::deliverFrame()
{
    if (!isCurrentlyAwaitingData())
        return;

    AudioFrame af;
    if (global_audio[aiChn]->msgChannel->read(&af))
    {
        if (af.data.size() > fMaxSize)
        {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = af.data.size() - fMaxSize;
        }
        else
        {
            fFrameSize = af.data.size();
        }

        fPresentationTime = af.time;
        
        memcpy(fTo, &af.data[0], fFrameSize);

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
