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
    static IMPSystem *createNew(std::shared_ptr<CFG> _cfg);

    IMPSystem(std::shared_ptr<CFG> _cfg) : cfg(_cfg)
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
    std::shared_ptr<CFG> cfg;
};

#endif