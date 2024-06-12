#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

IMPDeviceSource* IMPDeviceSource::createNew(UsageEnvironment& env, int encChn) {
    return new IMPDeviceSource(env, encChn);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment& env, int encChn)
    : FramedSource(env), encChn(encChn)
{
    LOG_DEBUG("Device source construct " << encChn);
    if(encChn == 0) {
        sink_id = Encoder::connect_sink(this, "IMPDeviceSource 0", encChn);
    } else {
        sink_id = Encoder::connect_low_sink(this, "IMPDeviceSource 1", encChn);
    }
}

IMPDeviceSource::~IMPDeviceSource() {
    LOG_DEBUG("Device source destruct " << encChn);
    if(encChn == 0) {
        Encoder::remove_sink(sink_id);
    } else {
        Encoder::remove_low_sink(sink_id);
    }
}

void IMPDeviceSource::doGetNextFrame() {
    if (!isCurrentlyAwaitingData()) return;

    //XXX: We aren't supposed to block here. The correct thing would be
    //to return early if we don't have a frame available, and then signal
    //an event to deliver the frame when it becomes available.
    //See DeviceSource.cpp in 3rdparty/live/liveMedia/
    H264NALUnit nal = encoder->wait_read();

    if (nal.data.size() > fMaxSize) {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = nal.data.size() - fMaxSize;
    }
    else {
        fFrameSize = nal.data.size();
    }
    fPresentationTime = nal.time;
    fDurationInMicroseconds = nal.duration;
    memcpy(fTo, &nal.data[0], fFrameSize);

    FramedSource::afterGetting(this);
    std::this_thread::yield();
}
