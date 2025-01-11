#include "IMPSystem.hpp"
#include "Config.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#define MODULE "IMP_SYSTEM"

/* first sensor */
#define FIRST_SNESOR_NAME 			"gc4653"			//sensor name (match with snesor driver name)
#define	FIRST_I2C_ADDR				0x29				//sensor i2c address
#define	FIRST_I2C_ADAPTER_ID		1					//sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH			2560				//sensor width
#define FIRST_SENSOR_HEIGHT			1440				//sensor height
#define	FIRST_RST_GPIO				GPIO_PC(27)			//sensor reset gpio
#define	FIRST_PWDN_GPIO				-1					//sensor pwdn gpio
#define	FIRST_POWER_GPIO			-1					//sensor power gpio
#define	FIRST_SENSOR_ID				0					//sensor index
#define	FIRST_VIDEO_INTERFACE		IMPISP_SENSOR_VI_MIPI_CSI0 	//sensor interface type (dvp/csi0/csi1)
#define	FIRST_MCLK					IMPISP_SENSOR_MCLK1		//sensor clk source (mclk0/mclk1/mclk2)
#define	FIRST_DEFAULT_BOOT			0					//sensor default mode(0/1/2/3/4)
#define DUALSENSOR_MODE				IMPISP_DUALSENSOR_DUAL_ALLCACHED_MODE	//dualsensor mode ()

// Define our static sensor configuration
static IMPSensorInfo Def_Sensor_Info = {
    .name = "gc4653",
    .cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
    .i2c = {
        .type = "gc4653",
        .addr = 0x29,
        .i2c_adapter_id = 1
    },
    .rst_gpio = GPIO_PC(27),    // This should match your hardware
    .pwdn_gpio = -1,            // Set to appropriate GPIO if you have PWDN connected
    .power_gpio = -1,           // Set to appropriate GPIO if you have POWER control
    .sensor_id = FIRST_SENSOR_ID,
    .video_interface = FIRST_VIDEO_INTERFACE,
    .mclk = FIRST_MCLK,
    .default_boot = FIRST_DEFAULT_BOOT
};
static IMPSensorInfo sensor_info[3];  // Array for up to 3 sensors

static IMPISPCameraInputMode mode = {
    .sensor_num = IMPISP_TOTAL_ONE,
    .dual_mode = DUALSENSOR_MODE,
    .dual_mode_switch = {
        .en = IMPISP_TUNING_OPS_MODE_DISABLE,  // Use proper enum instead of 0
    },
    .joint_mode = IMPISP_NOT_JOINT,
};

static bool sensor_bypass[3] = {0, 0, 1};

// You'll also need the channel config structure
extern struct chn_conf chn[];  // This should be defined in your equivalent of sample-common.h


IMPSensorInfo IMPSystem::create_sensor_info(const char *sensor_name)
{
    IMPSensorInfo out;
    memset(&out, 0, sizeof(IMPSensorInfo));

    // Name and bus settings
    strncpy(out.name, "gc4653", sizeof(out.name) - 1);
    out.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C;

    // I2C settings
    strncpy(out.i2c.type, "gc4653", sizeof(out.i2c.type) - 1);
    out.i2c.addr = 0x29;
    out.i2c.i2c_adapter_id = 1;  // Added this to use I2C1 instead of I2C0

    // GPIO settings
    out.rst_gpio = GPIO_PC(27);
    out.pwdn_gpio = -1;
    out.power_gpio = -1;

    // Sensor identification
    out.sensor_id = 0;

    // Interface settings
    out.video_interface = IMPISP_SENSOR_VI_MIPI_CSI0;  // Using CSI0 MIPI interface
    out.mclk = IMPISP_SENSOR_MCLK0;                    // Using primary MCLK source

    // Boot settings
    out.default_boot = 0;

    return out;
}

int IMPSystem::setupSensorGPIO() {
    int ret;

    // Configure reset GPIO (PC27)
    // First export the GPIO if not already exported
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd >= 0) {
        write(fd, "91", 2); // PC27 is GPIO 91 (you may need to adjust this number based on your platform)
        close(fd);
    }

    // Set direction to out
    fd = open("/sys/class/gpio/gpio91/direction", O_WRONLY);
    if (fd < 0) {
        IMP_LOG_ERR(TAG, "Failed to open GPIO direction file\n");
        return -1;
    }
    write(fd, "out", 3);
    close(fd);

    // Toggle reset
    fd = open("/sys/class/gpio/gpio91/value", O_WRONLY);
    if (fd < 0) {
        IMP_LOG_ERR(TAG, "Failed to open GPIO value file\n");
        return -1;
    }

    // Reset sequence
    write(fd, "0", 1);  // Assert reset
    usleep(10000);      // Hold for 10ms
    write(fd, "1", 1);  // Release reset
    usleep(10000);      // Wait for 10ms

    close(fd);

    return 0;
}

IMPSystem *IMPSystem::createNew()
{
    return new IMPSystem();
}

int IMPSystem::init()
{
    int ret = 0;

    // Initialize OSD memory pool
    IMP_ISP_Tuning_SetOsdPoolSize(512*1024);

    // Initialize sensor info with the same pattern as sample
    memset(&sensor_info, 0, sizeof(sensor_info));
    if(mode.sensor_num == IMPISP_TOTAL_ONE) {
        memcpy(&sensor_info[0], &Def_Sensor_Info, sizeof(IMPSensorInfo));
    }

    ret = IMP_ISP_Open();
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "failed to open ISP\n");
        return -1;
    }

    // Handle bypass mode for each sensor (from sample)
    for(int i = 0; i < mode.sensor_num; i++) {
        IMPVI_NUM vinum = (i == 0) ? IMPVI_MAIN : (i == 1) ? IMPVI_SEC : IMPVI_THR;

        if(!sensor_bypass[i]) {
            continue;
        }

        // Set ISP bypass mode
        IMPISPTuningOpsMode bypass_en = IMPISP_TUNING_OPS_MODE_ENABLE;
        ret = IMP_ISP_Tuning_SetISPBypass(vinum, &bypass_en);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPBypass(%d) failed\n", i);
            return -1;
        }
    }

    // Set camera input mode if needed
    if(mode.sensor_num > IMPISP_TOTAL_ONE) {
        ret = IMP_ISP_SetCameraInputMode(&mode);
        if(ret < 0) {
            IMP_LOG_ERR(TAG, "failed to set camera input mode!\n");
            return -1;
        }
    }

    ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info[0]);
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info[0]);
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_System_Init();
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
        return -1;
    }

    ret = IMP_ISP_EnableTuning();
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
        return -1;
    }

    return 0;
}

int IMPSystem::destroy() {
    int ret;
    IMPVI_NUM vinum = IMPVI_MAIN;

    ret = IMP_ISP_DisableSensor(vinum);
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "Failed to disable sensor\n");
    }

    ret = IMP_ISP_DelSensor(vinum, &sensor_info[0]);  // Using the correct sensor info array
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "Failed to delete sensor\n");
    }

    ret = IMP_ISP_DisableTuning();
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "Failed to disable tuning\n");
    }

    ret = IMP_ISP_Close();
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "Failed to close ISP\n");
    }

    return 0;
}