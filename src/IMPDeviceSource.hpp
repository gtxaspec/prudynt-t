#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "worker.hpp"
#include <mutex>
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

protected:
    IMPDeviceSource(UsageEnvironment &env, int encChn);
    virtual ~IMPDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void *clientData);
    void deliverFrame();

    std::queue<H264NALUnit> nalQueue;
    std::mutex queueMutex;
};

#endif
