#ifndef IMPSystem_hpp
#define IMPSystem_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <memory>
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
        init();
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