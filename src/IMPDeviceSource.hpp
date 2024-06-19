#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "Encoder.hpp"
#include <mutex>
#include <queue>
#include <chrono>

class IMPDeviceSource: public FramedSource {
public:
    static IMPDeviceSource* createNew(UsageEnvironment& env, int encChn);

    H264NALUnit wait_read();
    int on_data_available(const H264NALUnit& nalu);
    void deinit();
    int sinkId;
    int encChn;
    std::chrono::high_resolution_clock::time_point streamStart;

    EventTriggerId eventTriggerId;

protected:
    IMPDeviceSource(UsageEnvironment& env, int encChn);
    virtual ~IMPDeviceSource();

private:
    virtual void doGetNextFrame();
    static void deliverFrame0(void* clientData);
    void deliverFrame();
 
    std::queue<H264NALUnit> nalQueue;
    std::mutex queueMutex;  
};

#endif
