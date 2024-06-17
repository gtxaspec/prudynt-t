#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"
#include <chrono>

IMPDeviceSource* IMPDeviceSource::createNew(UsageEnvironment& env, int encChn) {
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment& env, int encChn)
    : FramedSource(env), encChn(encChn), eventTriggerId(0), doLog(0)
{
    sink_id = Encoder::connect_sink(this, "IMPDeviceSource", encChn);

    if (eventTriggerId == 0) {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }

    _IMPDeviceSource = static_cast<IMPDeviceSource *>(this);

    LOG_DEBUG("Device source construct, encChn:" << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 
}

IMPDeviceSource::~IMPDeviceSource() {
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    Encoder::remove_sink(sink_id);
    LOG_DEBUG("Device source destruct, encChn:" << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 
}

void IMPDeviceSource::doGetNextFrame()
{
    deliverFrame();
}

void IMPDeviceSource::deliverFrame0(void* clientData)
{
    ((IMPDeviceSource*)clientData)->deliverFrame();
}

H264NALUnit IMPDeviceSource::wait_read() {

    LOG_DEBUG("H264NALUnit IMPDeviceSource::wait_read()");

    while (nalQueue.empty()) {
        usleep(1000*1000);
        LOG_DEBUG("H264NALUnit IMPDeviceSource::wait_read(), nalQueue.empty(), wait for data.");
    }

    H264NALUnit nal = nalQueue.front();
    nalQueue.pop();
    return nal;
}

void IMPDeviceSource::deliverFrame() {

    if(doLog) LOG_DEBUG("void IMPDeviceSource::deliverFrame() START, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 

    if (!isCurrentlyAwaitingData()) {
        return;
    }

    if(doLog) LOG_DEBUG("void IMPDeviceSource::deliverFrame() STEP1, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 

    std::unique_lock<std::mutex> lock(queueMutex);

    if(doLog) LOG_DEBUG("void IMPDeviceSource::deliverFrame() STEP2, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 

    if (!nalQueue.empty()) {
        H264NALUnit nal = nalQueue.front();
        nalQueue.pop();
        lock.unlock();

        if(doLog) LOG_DEBUG("void IMPDeviceSource::deliverFrame() STEP3, encChn:" 
            << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId << ", data.size():" << nal.data.size()); 

        if (nal.data.size() > fMaxSize) {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = nal.data.size() - fMaxSize;
        } else {
            fFrameSize = nal.data.size();
        }
        fPresentationTime = nal.time;
        fDurationInMicroseconds = nal.duration;
        memcpy(fTo, &nal.data[0], fFrameSize);

        if(fFrameSize > 0) {
            FramedSource::afterGetting(this);
        }
    } else {
        lock.unlock();
        fFrameSize = 0;
    }

    if(doLog) LOG_DEBUG("void IMPDeviceSource::deliverFrame() EXIT, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 
}

void IMPDeviceSource::signal_new_data() {
    envir().taskScheduler().triggerEvent(eventTriggerId, _IMPDeviceSource);
}

int IMPDeviceSource::on_data_available(const H264NALUnit& nal) {

    if(doLog) LOG_DEBUG("int IMPDeviceSource::on_data_available(const H264NALUnit& nal) START, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 

    std::unique_lock<std::mutex> lock(queueMutex);
    int r = 0;
    while(nalQueue.size() >= 30) {
      nalQueue.pop();
      r++;
      doLog = true;
    }

    nalQueue.push(nal);
    lock.unlock();

    //envir().taskScheduler().triggerEvent(eventTriggerId, this);
    _IMPDeviceSource->signal_new_data();

    if(doLog) LOG_DEBUG("int IMPDeviceSource::on_data_available(const H264NALUnit& nal) START, encChn:" 
        << encChn << ", sink_id:" << sink_id << ", enevtTriggerId:" << eventTriggerId); 
    return r;
}