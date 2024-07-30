#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

IMPDeviceSource *IMPDeviceSource::createNew(UsageEnvironment &env, int encChn)
{
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment &env, int encChn)
    : FramedSource(env), encChn(encChn), eventTriggerId(0)
{
    if(!video[encChn]->onDataCallback) {
        video[encChn]->onDataCallback = [this]()
        { this->on_data_available(); };

        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
    LOG_DEBUG("IMPDeviceSource construct, encoder channel:" << encChn);
}

void IMPDeviceSource::deinit()
{
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    video[encChn]->onDataCallback = nullptr;
    LOG_DEBUG("IMPDeviceSource destruct, encoder channel:" << encChn);
}

IMPDeviceSource::~IMPDeviceSource()
{
    deinit();
}

void IMPDeviceSource::doGetNextFrame()
{
    deliverFrame();
}

void IMPDeviceSource::deliverFrame0(void *clientData)
{
    ((IMPDeviceSource *)clientData)->deliverFrame();
}

void IMPDeviceSource::deliverFrame()
{
    if (!isCurrentlyAwaitingData())
        return;

    H264NALUnit nal;
    if (video[encChn]->msgChannel->read(&nal))
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

        //gettimeofday(&fPresentationTime, NULL);
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
