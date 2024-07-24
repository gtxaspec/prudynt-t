#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "MsgChannel.hpp"
#include "Encoder.hpp"

class IMPDeviceSource: public FramedSource {
public:
    static IMPDeviceSource* createNew(UsageEnvironment& env);

    void set_input_channel(std::shared_ptr<MsgChannel<H264NALUnit>> chn) {
        encoder = chn;
    }

    void on_data_callback() {
        if (eventTriggerId != 0) {
            envir().taskScheduler().triggerEvent(eventTriggerId, this);
        }
    }

private:
    EventTriggerId eventTriggerId;

protected:
    IMPDeviceSource(UsageEnvironment& env);
    virtual ~IMPDeviceSource();

private:
    virtual void doGetNextFrame();
    std::shared_ptr<MsgChannel<H264NALUnit>> encoder = nullptr;
};

#endif
