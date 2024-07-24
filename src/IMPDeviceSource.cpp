#include "IMPDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

IMPDeviceSource* IMPDeviceSource::createNew(UsageEnvironment& env) {
    return new IMPDeviceSource(env);
}

IMPDeviceSource::IMPDeviceSource(UsageEnvironment& env)
    : FramedSource(env)
{
    LOG_DEBUG("Device source construct");
    eventTriggerId = envir().taskScheduler()
        .createEventTrigger(reinterpret_cast<TaskFunc *>(+[] (void *dev) {
            reinterpret_cast<IMPDeviceSource *>(dev)->doGetNextFrame();
        }));
}

IMPDeviceSource::~IMPDeviceSource() {
    LOG_DEBUG("Device source destruct");
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
}

void IMPDeviceSource::doGetNextFrame() {
    if (!isCurrentlyAwaitingData()) return;

    H264NALUnit nal;
    if (!encoder->read(&nal)) {
        // nothing to read, wait for event to be triggered
        return;
    }

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
