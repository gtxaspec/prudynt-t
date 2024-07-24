#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "worker.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>
#include "globals.hpp"

class IMPDeviceSource : public FramedSource
{
public:
    static IMPDeviceSource *createNew(UsageEnvironment &env, int encChn);

    void on_data_available()
    {
        if (eventTriggerId != 0)
        {
            envir().taskScheduler().triggerEvent(eventTriggerId, this);
        }
    }
    void deinit();
    int encChn;
    EventTriggerId eventTriggerId;

    IMPDeviceSource(UsageEnvironment &env, int encChn);
    virtual ~IMPDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void *clientData);
    void deliverFrame();
};

#endif
