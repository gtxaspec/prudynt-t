#include "IMPAudioDeviceSource.hpp"
#include <iostream>
#include "GroupsockHelper.hh"

IMPAudioDeviceSource* IMPAudioDeviceSource::createNew(UsageEnvironment& env, int audioChn) {
    return new IMPAudioDeviceSource(env, audioChn);
}

IMPAudioDeviceSource::IMPAudioDeviceSource(UsageEnvironment& env, int audioChn)
    : FramedSource(env), audioChn(audioChn), eventTriggerId(0) {
    if (eventTriggerId == 0) {
        eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
    }
}

IMPAudioDeviceSource::~IMPAudioDeviceSource() {
    deinit();
}

void IMPAudioDeviceSource::deinit() {
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
}

void IMPAudioDeviceSource::doGetNextFrame() {
    deliverFrame();
}

void IMPAudioDeviceSource::deliverFrame0(void* clientData) {
    ((IMPAudioDeviceSource*)clientData)->deliverFrame();
}

AudioFrame IMPAudioDeviceSource::wait_read() {
    std::unique_lock<std::mutex> lock(queueMutex);
    queueHasData.wait(lock, [this]{ return !frameQueue.empty(); });

    AudioFrame frame = frameQueue.front();
    frameQueue.pop();
    return frame;
}

void IMPAudioDeviceSource::deliverFrame() {
    if (!isCurrentlyAwaitingData())
        return;

    std::unique_lock<std::mutex> lock(queueMutex);

    if (!frameQueue.empty()) {
        AudioFrame frame = frameQueue.front();
        frameQueue.pop();
        lock.unlock();

        if (frame.data.size() > fMaxSize) {
            fFrameSize = fMaxSize;
            fNumTruncatedBytes = frame.data.size() - fMaxSize;
        } else {
            fFrameSize = frame.data.size();
        }
        fPresentationTime = frame.time;
        memcpy(fTo, frame.data.data(), fFrameSize);

        if (fFrameSize > 0) {
            FramedSource::afterGetting(this);
        }
    } else {
        lock.unlock();
        fFrameSize = 0;
    }
}

int IMPAudioDeviceSource::on_data_available(const AudioFrame& frame) {
    std::unique_lock<std::mutex> lock(queueMutex);

    int dropCount = 0;
    while (frameQueue.size() >= 30) {
        frameQueue.pop();
        dropCount++;
    }

    frameQueue.push(frame);
    if (needNotify) {
        queueHasData.notify_all();
    }
    lock.unlock();

    envir().taskScheduler().triggerEvent(eventTriggerId, this);

    return dropCount;
}