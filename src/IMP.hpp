#ifndef IMP_hpp
#define IMP_hpp

#ifdef PLATFORM_T31
    #include <imp_t31/imp_framesource.h>
    #include <imp_t31/imp_system.h>
    #include <imp_t31/imp_isp.h>
    #include <imp_t31/imp_system.h>
#elif PLATFORM_T20
    #include <imp_t20/imp_framesource.h>
    #include <imp_t20/imp_system.h>
    #include <imp_t20/imp_isp.h>
    #include <imp_t20/imp_system.h>
#elif PLATFORM_T21
    #include <imp_t21/imp_framesource.h>
    #include <imp_t21/imp_system.h>
    #include <imp_t21/imp_isp.h>
    #include <imp_t21/imp_system.h>
#elif PLATFORM_T30
    #include <imp_t30/imp_framesource.h>
    #include <imp_t30/imp_system.h>
    #include <imp_t30/imp_isp.h>
    #include <imp_t30/imp_system.h>
#endif

#include <sysutils/su_base.h>

class IMP {
public:
    static bool init();
public:
    static const int FRAME_RATE;
private:
    static int system_init();
    static int framesource_init();
    static IMPFSChnAttr create_fs_attr();
    static IMPSensorInfo create_sensor_info(std::string sensor);
};

#endif
