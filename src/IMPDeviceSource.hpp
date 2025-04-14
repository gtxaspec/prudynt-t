#ifndef IMPDeviceSource_hpp
#define IMPDeviceSource_hpp

#include "FramedSource.hh"
#include "worker.hpp"
#include <mutex>
#include <condition_variable>
#include <queue>
#include "globals.hpp"

template <typename FrameType, typename Stream>
class IMPDeviceSource : public FramedSource
{
public:
    static IMPDeviceSource *createNew(UsageEnvironment &env, int encChn, std::shared_ptr<Stream> stream, const char *name);

    void on_data_available()
    {
        if (eventTriggerId != 0)
        {
            envir().taskScheduler().triggerEvent(eventTriggerId, this);
        }
    }
    IMPDeviceSource(UsageEnvironment &env, int encChn, std::shared_ptr<Stream> stream, const char *name);
    virtual ~IMPDeviceSource();

private:
    
    virtual void doGetNextFrame() override;
    static void deliverFrame0(void *clientData);
    void deliverFrame();
    void deinit();
    int encChn;
    std::shared_ptr<Stream> stream;
    std::string name;   // for printing
    EventTriggerId eventTriggerId;
    // For synchronization tracking
    bool firstFrame;
    int64_t base_timestamp;
    bool timestamp_initialized;
    // For tracking dropped frames
    unsigned int droppedFrames;
};

#endif
