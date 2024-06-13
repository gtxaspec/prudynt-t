#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"
#include <chrono>

IMPDeviceSource* IMPDeviceSource::createNew(UsageEnvironment& env, int encChn) {
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment& env, int encChn)
    : FramedSource(env), encChn(encChn)
{
    LOG_DEBUG("Device source construct " << encChn);
    sink_id = Encoder::connect_sink(this, "IMPDeviceSource", encChn);
}

IMPDeviceSource::~IMPDeviceSource() {
    LOG_DEBUG("Device source destruct " << encChn);
    Encoder::remove_sink(sink_id);
}

void IMPDeviceSource::doGetNextFrame() {
    if (!isCurrentlyAwaitingData()) return;

    H264NALUnit nal;
    if (encoder->read(&nal)) {

        if (nal.data.size() > fMaxSize) {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = nal.data.size() - fMaxSize;
        } else {
            fFrameSize = nal.data.size();
        }
        fPresentationTime = nal.time;
        fDurationInMicroseconds = nal.duration;
        memcpy(fTo, &nal.data[0], fFrameSize);

        FramedSource::afterGetting(this);
    }
}

void IMPDeviceSource::on_data_available() {

    envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)afterGetting, this);
}
