#ifndef IMPSystem_hpp
#define IMPSystem_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <memory>
#include <sys/time.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <../sysutils/su_base.h>

class IMPSystem
{
public:
    static IMPSystem *createNew();

    IMPSystem()
    {
        if( init() != 0 && Logger::level != Logger::DEBUG) {

            throw std::invalid_argument("error initializing the imp system.");
        };

        /*
        / https://github.com/rara64/prudynt-t/commit/7eda99252b0d1309cbe134dc4143182eda9c21bd
        */

        struct timespec timeSinceBoot;
        clock_gettime(CLOCK_MONOTONIC, &timeSinceBoot);

        uint64_t imp_time_base = (timeSinceBoot.tv_sec * 1000000) + (timeSinceBoot.tv_nsec / 1000);
        IMP_System_RebaseTimeStamp(imp_time_base);

        LOG_DEBUG("IMP_System_RebaseTimeStamp(" << imp_time_base << ");");
    }

    ~IMPSystem()
    {
        destroy();
    };

    int init();
    int destroy();

private:
    IMPSensorInfo sinfo{};
    IMPSensorInfo create_sensor_info(const char *sensor_name);
};

#endif