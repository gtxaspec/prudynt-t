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
    Worker::connect_sink(this, "IMPDeviceSource", encChn);

    if (eventTriggerId == 0)
    {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }

    LOG_DEBUG("IMPDeviceSource construct, encoder channel:" << encChn);
}

IMPDeviceSource::~IMPDeviceSource()
{
    deinit();
}

void IMPDeviceSource::deinit()
{
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);

    Worker::remove_sink(encChn);
}

void IMPDeviceSource::doGetNextFrame()
{
    deliverFrame();
}

void IMPDeviceSource::deliverFrame0(void *clientData)
{
    ((IMPDeviceSource *)clientData)->deliverFrame();
}

H264NALUnit IMPDeviceSource::wait_read()
{
    std::unique_lock<std::mutex> lock(queueMutex);
    queueHasData.wait(lock, [this]{ return !nalQueue.empty(); });

    H264NALUnit nal = nalQueue.front();
    nalQueue.pop();
    return nal;
}

void IMPDeviceSource::deliverFrame()
{
    if (!isCurrentlyAwaitingData())
        return;

    std::unique_lock<std::mutex> lock(queueMutex);

    if (!nalQueue.empty())
    {

        H264NALUnit nal = nalQueue.front();
        nalQueue.pop();
        lock.unlock();

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
        lock.unlock();
        fFrameSize = 0;
    }
}

int IMPDeviceSource::on_data_available(const H264NALUnit &nal)
{
    std::unique_lock<std::mutex> lock(queueMutex);

    int dropCount = 0;
    while (nalQueue.size() >= 30)
    {
        nalQueue.pop();
        dropCount++;
    }

    nalQueue.push(nal);
    if (needNotify)
    {
        queueHasData.notify_all();
    }
    lock.unlock();

    envir().taskScheduler().triggerEvent(eventTriggerId, this);

    return dropCount;
}
