#include "SystemSensor.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <filesystem>

// For logging compatibility with prudynt-t
#ifdef LOG_DEBUG
    #define SYSTEM_SENSOR_LOG_DEBUG(msg) LOG_DEBUG("SystemSensor: " << msg)
    #define SYSTEM_SENSOR_LOG_ERROR(msg) LOG_ERROR("SystemSensor: " << msg)
    #define SYSTEM_SENSOR_LOG_INFO(msg) LOG_INFO("SystemSensor: " << msg)
#else
    #define SYSTEM_SENSOR_LOG_DEBUG(msg) std::cout << "DEBUG SystemSensor: " << msg << std::endl
    #define SYSTEM_SENSOR_LOG_ERROR(msg) std::cerr << "ERROR SystemSensor: " << msg << std::endl
    #define SYSTEM_SENSOR_LOG_INFO(msg) std::cout << "INFO SystemSensor: " << msg << std::endl
#endif

// Define the sensor proc directory path
const std::string SystemSensor::SENSOR_PROC_DIR = "/proc/jz/sensor/";

SystemSensor::SensorInfo SystemSensor::getSensorInfo() {
    SYSTEM_SENSOR_LOG_DEBUG("Getting sensor information from /proc/jz/sensor/");

    if (!isAvailable()) {
        throw std::runtime_error("Sensor proc filesystem /proc/jz/sensor/ is not accessible");
    }

    SensorInfo info;

    // Read basic sensor information
    info.name = readProcString("name");
    info.chip_id = readProcString("chip_id");
    info.i2c_addr = readProcString("i2c_addr");
    info.version = readProcString("version");

    // Read numeric values with defaults from constructor
    info.width = readProcInt("width", info.width);
    info.height = readProcInt("height", info.height);
    info.min_fps = readProcInt("min_fps", info.min_fps);
    info.max_fps = readProcInt("max_fps", info.max_fps);
    info.i2c_bus = readProcInt("i2c_bus", info.i2c_bus);
    info.boot = readProcInt("boot", info.boot);
    info.mclk = readProcInt("mclk", info.mclk);
    info.video_interface = readProcInt("video_interface", info.video_interface);
    info.reset_gpio = readProcInt("reset_gpio", info.reset_gpio);

    // Parse I2C address if available
    if (!info.i2c_addr.empty()) {
        info.i2c_address = parseHexString(info.i2c_addr);
    }

    // Set default FPS to max_fps
    info.fps = info.max_fps;

    SYSTEM_SENSOR_LOG_INFO("Successfully retrieved sensor info: " << info.name
                          << " (" << info.width << "x" << info.height
                          << "@" << info.max_fps << "fps)");

    return info;
}

bool SystemSensor::isAvailable() {
    // Check if /proc/jz/sensor/ directory exists
    return std::filesystem::exists(SENSOR_PROC_DIR) && std::filesystem::is_directory(SENSOR_PROC_DIR);
}

std::string SystemSensor::readProcString(const std::string& filename) {
    std::string fullPath = SENSOR_PROC_DIR + filename;
    std::ifstream file(fullPath);

    if (!file.is_open()) {
        SYSTEM_SENSOR_LOG_DEBUG("Failed to open " << fullPath);
        return "";
    }

    std::string line;
    if (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        SYSTEM_SENSOR_LOG_DEBUG("Read from " << fullPath << ": " << line);
        return line;
    }

    SYSTEM_SENSOR_LOG_DEBUG("No content in " << fullPath);
    return "";
}

int SystemSensor::readProcInt(const std::string& filename, int defaultValue) {
    std::string value = readProcString(filename);

    if (value.empty()) {
        SYSTEM_SENSOR_LOG_DEBUG("Using default value " << defaultValue << " for " << filename);
        return defaultValue;
    }

    try {
        int result = std::stoi(value);
        SYSTEM_SENSOR_LOG_DEBUG("Parsed " << filename << " as int: " << result);
        return result;
    } catch (const std::exception& e) {
        SYSTEM_SENSOR_LOG_ERROR("Failed to parse '" << value << "' as int from " << filename << ": " << e.what());
        return defaultValue;
    }
}

unsigned int SystemSensor::parseHexString(const std::string& hexStr) {
    if (hexStr.empty()) {
        return 0;
    }

    try {
        // Handle both "0x37" and "37" formats
        if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X") {
            return static_cast<unsigned int>(std::stoul(hexStr, nullptr, 16));
        } else {
            return static_cast<unsigned int>(std::stoul(hexStr, nullptr, 16));
        }
    } catch (const std::exception& e) {
        SYSTEM_SENSOR_LOG_ERROR("Failed to parse hex string '" << hexStr << "': " << e.what());
        return 0;
    }
}
