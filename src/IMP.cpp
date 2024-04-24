#include <iostream>
#include <cstring>
#include "IMP.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#define OSDPoolSize 131072

bool IMP::init() {
    int ret;

    ret = system_init();
    if (ret < 0) {
        LOG_ERROR("System Init Failed");
        return true;
    }

    ret = framesource_init();
    if (ret < 0) {
        LOG_ERROR("Framesource Init Failed");
        return true;
    }

    return false;
}

IMPSensorInfo IMP::create_sensor_info(std::string sensor) {
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));
    LOG_INFO("Sensor: " << Config::singleton()->sensorModel.c_str());
        std::strcpy(out.name, Config::singleton()->sensorModel.c_str());
        out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;
        std::strcpy(out.i2c.type, Config::singleton()->sensorModel.c_str());
        out.i2c.addr = Config::singleton()->sensorI2Caddress;
        return out;
}

IMPFSChnAttr IMP::create_fs_attr() {
    IMPFSChnAttr out;
    memset(&out, 0, sizeof(IMPFSChnAttr));

    //Seems to only support the following (channel enable fails otherwise)
    //PIX_FMT_YUYV422
    //PIX_FMT_UYVY422
    //PIX_FMT_NV12
    //Of those, I have only gotten PIX_FMT_NV12 to produce frames.
    out.pixFmt = PIX_FMT_NV12;
    out.outFrmRateNum = Config::singleton()->stream0fps;
    out.outFrmRateDen = 1;
    out.nrVBs = Config::singleton()->stream0buffers;
    out.type = FS_PHY_CHANNEL;
    out.crop.enable = 0;
    out.crop.top = 0;
    out.crop.left = 0;
    out.crop.width = Config::singleton()->stream0width;
    out.crop.height = Config::singleton()->stream0height;
    out.scaler.enable = Config::singleton()->stream0scaleEnable;
    out.scaler.outwidth = Config::singleton()->stream0scaleWidth;
    out.scaler.outheight = Config::singleton()->stream0scaleHeight;
    out.picWidth = Config::singleton()->stream0width;  // Testing stream size sync
    out.picHeight = Config::singleton()->stream0height;
    return out;
}

int IMP::framesource_init() {
    int ret = 0;
    int rotation = Config::singleton()->stream0rotation;
    int rot_height = Config::singleton()->stream0height;
    int rot_width = Config::singleton()->stream0width;

    IMPFSChnAttr fs_chn_attr = create_fs_attr();

#if !defined(KERNEL_VERSION_4)
#if defined(PLATFORM_T31)
    // Set rotate before FS creation
    // IMP_Encoder_SetFisheyeEnableStatus(0, 1);

    ret = IMP_FrameSource_SetChnRotate(0, rotation, rot_height, rot_width);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_SetChnRotate() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnRotate == " + std::to_string(rotation));
#endif
#endif

    ret = IMP_FrameSource_CreateChn(0, &fs_chn_attr);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_CreateChn() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_CreateChn created");

    ret = IMP_FrameSource_SetChnAttr(0, &fs_chn_attr);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_SetChnAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnAttr set");

    IMPFSChnFifoAttr fifo;

    ret = IMP_FrameSource_GetChnFifoAttr(0, &fifo);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_GetChnFifoAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_GetChnFifoAttr set");

    fifo.maxdepth = 0;
    ret = IMP_FrameSource_SetChnFifoAttr(0, &fifo);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_SetChnFifoAttr() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetChnFifoAttr set");

    ret = IMP_FrameSource_SetFrameDepth(0, 0);
    if (ret < 0) {
        LOG_DEBUG("IMP_FrameSource_SetFrameDepth() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_FrameSource_SetFrameDepth set");

    return ret;
}

int IMP::system_init() {
    int ret = 0;

    // If you are having problems with video quality, please make
    // sure you are linking against the latest version of libimp
    // available for the target soc

    IMPVersion impVersion;
    ret = IMP_System_GetVersion(&impVersion);
    LOG_INFO("LIBIMP Version " << impVersion.aVersion);

    SUVersion suVersion;
    ret = SU_Base_GetVersion(&suVersion);
    LOG_INFO("SYSUTILS Version: " << suVersion.chr);

    const char* cpuInfo = IMP_System_GetCPUInfo();
    LOG_INFO("CPU Information: " << cpuInfo);

    ret = IMP_OSD_SetPoolSize(OSDPoolSize);
    if (ret < 0) {
        LOG_DEBUG("Error: IMP_OSD_SetPoolSize == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_OSD_SetPoolSize == " + std::to_string(OSDPoolSize));

    ret = IMP_ISP_Open();
    if (ret < 0) {
        LOG_DEBUG("Error: IMP_ISP_Open() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("ISP Opened!");

    IMPSensorInfo sinfo = create_sensor_info(Config::singleton()->sensorModel.c_str());
    ret = IMP_ISP_AddSensor(&sinfo);
    if (ret < 0) {
        LOG_DEBUG("Error: IMP_ISP_AddSensor() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("Sensor Added");

    ret = IMP_ISP_EnableSensor();
    if (ret < 0) {
        LOG_DEBUG("Error: IMP_ISP_EnableSensor() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("Sensor Enabled");

    ret = IMP_System_Init();
    if (ret < 0) {
        LOG_DEBUG("Error: IMP_System_Init() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP System Initialized");

    //Enable tuning.
    //This is necessary to customize the sensor's image output.
    //Denoising, WDR, Night Mode, and FPS customization require this.
    ret = IMP_ISP_EnableTuning();
    if (ret < 0) {
        LOG_DEBUG("ERROR: IMP_ISP_EnableTuning() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_EnableTuning enabled");

    /* Set tuning defaults; on some SoC platforms, if we don't do this, the stream will be dark until manually set */
    IMP_ISP_Tuning_SetContrast(128);
    IMP_ISP_Tuning_SetSharpness(128);
    IMP_ISP_Tuning_SetSaturation(128);
    IMP_ISP_Tuning_SetBrightness(128);
    IMP_ISP_Tuning_SetISPBypass(IMPISP_TUNING_OPS_MODE_ENABLE);
    IMP_ISP_Tuning_SetAntiFlickerAttr(IMPISP_ANTIFLICKER_60HZ);
    LOG_DEBUG("ISP Tuning Defaults set");

    ret = IMP_ISP_Tuning_SetSensorFPS(Config::singleton()->sensorFps, 1);
    if (ret < 0) {
        LOG_DEBUG("ERROR: IMP_ISP_Tuning_SetSensorFPS() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_Tuning_SetSensorFPS == " + std::to_string(Config::singleton()->sensorFps));

    // Set the ISP to DAY on launch
    ret = IMP_ISP_Tuning_SetISPRunningMode(IMPISP_RUNNING_MODE_DAY);
    if (ret < 0) {
        LOG_DEBUG("ERROR: IMP_ISP_Tuning_SetISPRunningMode() == " + std::to_string(ret));
        return ret;
    }
    LOG_DEBUG("IMP_ISP_Tuning_SetISPRunningMode == " + std::to_string(IMPISP_RUNNING_MODE_DAY));


    return ret;
}
