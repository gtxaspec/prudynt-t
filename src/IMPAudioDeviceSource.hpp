#ifndef IMPAudioDeviceSource_hpp
#define IMPAudioDeviceSource_hpp

#include "FramedSource.hh"
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <cstdint>
#include "worker.hpp"

class IMPAudioDeviceSource : public FramedSource {
public:
    static IMPAudioDeviceSource* createNew(UsageEnvironment& env, int audioChn);

    AudioFrame wait_read();
    int on_data_available(const AudioFrame& frame);
    void deinit();
    int audioChn;

    EventTriggerId eventTriggerId;
    void initializationComplete() { needNotify = false; }

protected:
    IMPAudioDeviceSource(UsageEnvironment& env, int audioChn);
    virtual ~IMPAudioDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void* clientData);
    void deliverFrame();

    std::queue<AudioFrame> frameQueue;
    std::mutex queueMutex;
    std::condition_variable queueHasData;
    bool needNotify = true;
};

#endif