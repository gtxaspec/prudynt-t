#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"
#include <chrono>

IMPDeviceSource *IMPDeviceSource::createNew(UsageEnvironment &env, int encChn)
{
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment &env, int encChn)
    : FramedSource(env), encChn(encChn), eventTriggerId(0), startTime(clock())
{
    sinkId = Encoder::connect_sink(this, "IMPDeviceSource", encChn);

    if (eventTriggerId == 0)
    {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }

    LOG_DEBUG("IMPDeviceSource construct, encoder channel:" << encChn << ", id:" << sinkId);
}

IMPDeviceSource::~IMPDeviceSource()
{
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);

    Encoder::remove_sink(sinkId);

    LOG_DEBUG("IMPDeviceSource destruct, encoder channel:" << encChn << ", id:" << sinkId);
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

    LOG_DEBUG("wait_read()");

    while (nalQueue.empty())
    {

        usleep(1000 * 1000);
        LOG_DEBUG("wait_read(), nalQueue.empty(), wait for data.");
    }

    H264NALUnit nal = nalQueue.front();
    nalQueue.pop();
    return nal;
}

void IMPDeviceSource::deliverFrame()
{

    if (!isCurrentlyAwaitingData()) return;

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
        fDurationInMicroseconds = nal.duration;
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
    lock.unlock();

    envir().taskScheduler().triggerEvent(eventTriggerId, this);

    return dropCount;
}