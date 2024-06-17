#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "Encoder.hpp"
#include <mutex>
#include <queue>

class IMPDeviceSource: public FramedSource {
public:
    static IMPDeviceSource* createNew(UsageEnvironment& env, int encChn);

    H264NALUnit wait_read();
    virtual void doGetNextFrame() override;

public:
    void signal_new_data();
    int on_data_available(const H264NALUnit& nalu);
    uint32_t sink_id;

protected:
    IMPDeviceSource(UsageEnvironment& env, int encChn);
    virtual ~IMPDeviceSource();

private:
    
    static void deliverFrame0(void* clientData);
    void deliverFrame();
 
    int encChn;
    EventTriggerId eventTriggerId;
    std::queue<H264NALUnit> nalQueue;
    std::mutex queueMutex;  
    bool doLog; 
    IMPDeviceSource * _IMPDeviceSource = NULL;
};

#endif
