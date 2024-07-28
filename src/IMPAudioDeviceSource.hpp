#ifndef IMPAudioDeviceSource_hpp
#define IMPAudioDeviceSource_hpp

#include "FramedSource.hh"
#include "worker.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>
#include "globals.hpp"

class IMPAudioDeviceSource : public FramedSource
{
public:
    static IMPAudioDeviceSource *createNew(UsageEnvironment &env, int aiChn);

    void on_data_available()
    {
        if (eventTriggerId != 0)
        {
            envir().taskScheduler().triggerEvent(eventTriggerId, this);
        }
    }
    void deinit();
    int aiChn;
    EventTriggerId eventTriggerId;

    IMPAudioDeviceSource(UsageEnvironment &env, int aiChn);
    virtual ~IMPAudioDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void *clientData);
    void deliverFrame();
};

#endif
