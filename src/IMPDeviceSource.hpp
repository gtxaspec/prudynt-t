#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "worker.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>

class IMPDeviceSource : public FramedSource
{
public:
    static IMPDeviceSource *createNew(UsageEnvironment &env, int encChn);

    H264NALUnit wait_read();
    int on_data_available(const H264NALUnit &nalu);
    void deinit();
    int encChn;

    EventTriggerId eventTriggerId;
    void initializationComplete() { needNotify = false; }

protected:
    IMPDeviceSource(UsageEnvironment &env, int encChn);
    virtual ~IMPDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void *clientData);
    void deliverFrame();

    std::queue<H264NALUnit> nalQueue;
    std::mutex queueMutex;
    std::condition_variable queueHasData;
    bool needNotify = true;    // notify is needed only during initialization
};

#endif
