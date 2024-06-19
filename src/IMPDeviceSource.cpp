#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"
#include <chrono>

IMPDeviceSource *IMPDeviceSource::createNew(UsageEnvironment &env, int encChn)
{
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment &env, int encChn)
    : FramedSource(env), encChn(encChn), eventTriggerId(0)
{
    sinkId = Encoder::connect_sink(this, "IMPDeviceSource", encChn);
    streamStart = std::chrono::high_resolution_clock::now();

    if (eventTriggerId == 0)
    {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }

    LOG_DEBUG("IMPDeviceSource construct, encoder channel:" << encChn << ", id:" << sinkId);
}

IMPDeviceSource::~IMPDeviceSource()
{
    deinit();
}

void IMPDeviceSource::deinit() 
{
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);

    Encoder::remove_sink(sinkId);

    auto streamEnd = std::chrono::high_resolution_clock::now();

    auto h = duration_cast<std::chrono::hours>(streamEnd - streamStart);
    auto m = duration_cast<std::chrono::minutes>(streamEnd - streamStart - h);
    auto s = duration_cast<std::chrono::seconds>(streamEnd - streamStart - h - m);
    auto ms = duration_cast<std::chrono::milliseconds>(streamEnd - streamStart - h - m - s);

    LOG_DEBUG("IMPDeviceSource destruct after , encoder channel:" << encChn << ", id:" << sinkId << " after:" 
        << h.count() << ":" << m.count() << ":" << s.count() << "." << ms.count());    
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