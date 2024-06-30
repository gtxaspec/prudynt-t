#ifndef IMPSystem_hpp
#define IMPSystem_hpp

#include "Logger.hpp"
#include "Config.hpp"
#include <memory>
#include <imp/imp_isp.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <../sysutils/su_base.h>

class IMPSystem
{
public:
    static IMPSystem *createNew(_image *image, _sensor *sensor);

    IMPSystem(_image *image, _sensor *sensor) : image(image), sensor(sensor)
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
    _image *image{};
    _sensor *sensor{};
    IMPSensorInfo sinfo{};
    IMPSensorInfo create_sensor_info(const char *sensor_name);
};

#endif