#ifndef IMPSystem_hpp
#define IMPSystem_hpp

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include "Logger.hpp"
#include "Config.hpp"

#define TAG "IMP_SYSTEM"


// We need to match the sample's structure - define this in header
struct chn_conf {
    int index;
    IMPFSChnAttr fs_chn_attr;
    // Add other fields as needed from sample-common.h
};

class IMPSystem {
public:
    static IMPSystem *createNew();

    IMPSystem() {
        if(init() != 0) {
            throw std::invalid_argument("Error initializing the imp system");
        }

        struct timespec timeSinceBoot;
        clock_gettime(CLOCK_MONOTONIC, &timeSinceBoot);
        uint64_t imp_time_base = (timeSinceBoot.tv_sec * 1000000) + (timeSinceBoot.tv_nsec / 1000);
        IMP_System_RebaseTimeStamp(imp_time_base);
    }

    ~IMPSystem() {
        destroy();
    }

    int init();
    int destroy();

private:
    IMPSensorInfo sinfo{};
    IMPSensorInfo create_sensor_info(const char *sensor_name);
    int setupSensorGPIO();
};

#endif