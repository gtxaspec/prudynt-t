#include "RTSPStatus.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

// For logging compatibility with prudynt-t
#ifdef LOG_DEBUG
    #define RTSP_STATUS_LOG_DEBUG(msg) LOG_DEBUG("RTSPStatus: " << msg)
    #define RTSP_STATUS_LOG_ERROR(msg) LOG_ERROR("RTSPStatus: " << msg)
    #define RTSP_STATUS_LOG_INFO(msg) LOG_INFO("RTSPStatus: " << msg)
#else
    #define RTSP_STATUS_LOG_DEBUG(msg) std::cout << "DEBUG RTSPStatus: " << msg << std::endl
    #define RTSP_STATUS_LOG_ERROR(msg) std::cerr << "ERROR RTSPStatus: " << msg << std::endl
    #define RTSP_STATUS_LOG_INFO(msg) std::cout << "INFO RTSPStatus: " << msg << std::endl
#endif

// Static member definitions
const std::string RTSPStatus::STATUS_BASE_DIR = "/run/prudynt/rtsp/";
std::mutex RTSPStatus::statusMutex;
std::map<std::string, RTSPStatus::StreamInfo> RTSPStatus::activeStreams;

bool RTSPStatus::initialize() {
    std::lock_guard<std::mutex> lock(statusMutex);

    RTSP_STATUS_LOG_DEBUG("Initializing RTSP status interface at " << STATUS_BASE_DIR);

    if (!ensureBaseDirectory()) {
        RTSP_STATUS_LOG_ERROR("Failed to create base directory: " << STATUS_BASE_DIR);
        return false;
    }

    // Clear any existing stream data
    activeStreams.clear();

    RTSP_STATUS_LOG_INFO("RTSP status interface initialized successfully");
    return true;
}

void RTSPStatus::cleanup() {
    std::lock_guard<std::mutex> lock(statusMutex);

    RTSP_STATUS_LOG_DEBUG("Cleaning up RTSP status interface");

    try {
        if (std::filesystem::exists(STATUS_BASE_DIR)) {
            std::filesystem::remove_all(STATUS_BASE_DIR);
            RTSP_STATUS_LOG_DEBUG("Removed status directory: " << STATUS_BASE_DIR);
        }
    } catch (const std::exception& e) {
        RTSP_STATUS_LOG_ERROR("Failed to cleanup status directory: " << e.what());
    }

    activeStreams.clear();
    RTSP_STATUS_LOG_INFO("RTSP status interface cleaned up");
}

bool RTSPStatus::updateStreamStatus(const std::string& streamName, const StreamInfo& info) {
    std::lock_guard<std::mutex> lock(statusMutex);

    RTSP_STATUS_LOG_DEBUG("Updating status for stream: " << streamName);

    if (!ensureBaseDirectory()) {
        RTSP_STATUS_LOG_ERROR("Base directory not available");
        return false;
    }

    if (!createStreamDirectory(streamName)) {
        RTSP_STATUS_LOG_ERROR("Failed to create stream directory for: " << streamName);
        return false;
    }

    // Write all parameters
    bool success = true;
    success &= writeParameter(streamName, "format", info.format);
    success &= writeParameter(streamName, "fps", std::to_string(info.fps));
    success &= writeParameter(streamName, "width", std::to_string(info.width));
    success &= writeParameter(streamName, "height", std::to_string(info.height));
    success &= writeParameter(streamName, "endpoint", info.endpoint);
    success &= writeParameter(streamName, "url", info.url);
    success &= writeParameter(streamName, "bitrate", std::to_string(info.bitrate));
    success &= writeParameter(streamName, "mode", info.mode);
    success &= writeParameter(streamName, "enabled", info.enabled ? "true" : "false");

    if (success) {
        activeStreams[streamName] = info;
        RTSP_STATUS_LOG_INFO("Updated status for stream " << streamName
                           << " (" << info.format << " " << info.width << "x" << info.height
                           << "@" << info.fps << "fps)");
    } else {
        RTSP_STATUS_LOG_ERROR("Failed to update some parameters for stream: " << streamName);
    }

    return success;
}

bool RTSPStatus::removeStreamStatus(const std::string& streamName) {
    std::lock_guard<std::mutex> lock(statusMutex);

    RTSP_STATUS_LOG_DEBUG("Removing status for stream: " << streamName);

    bool success = removeStreamDirectory(streamName);

    if (success) {
        activeStreams.erase(streamName);
        RTSP_STATUS_LOG_INFO("Removed status for stream: " << streamName);
    } else {
        RTSP_STATUS_LOG_ERROR("Failed to remove status for stream: " << streamName);
    }

    return success;
}

RTSPStatus::StreamInfo RTSPStatus::getStreamStatus(const std::string& streamName) {
    std::lock_guard<std::mutex> lock(statusMutex);

    auto it = activeStreams.find(streamName);
    if (it != activeStreams.end()) {
        return it->second;
    }

    return StreamInfo(); // Return empty info if not found
}

bool RTSPStatus::isAvailable() {
    std::lock_guard<std::mutex> lock(statusMutex);
    return std::filesystem::exists(STATUS_BASE_DIR) && std::filesystem::is_directory(STATUS_BASE_DIR);
}

std::vector<std::string> RTSPStatus::getActiveStreams() {
    std::lock_guard<std::mutex> lock(statusMutex);

    std::vector<std::string> streams;
    for (const auto& pair : activeStreams) {
        if (pair.second.enabled) {
            streams.push_back(pair.first);
        }
    }

    return streams;
}

bool RTSPStatus::createStreamDirectory(const std::string& streamName) {
    std::string streamDir = STATUS_BASE_DIR + streamName + "/";

    try {
        std::filesystem::create_directories(streamDir);
        RTSP_STATUS_LOG_DEBUG("Created stream directory: " << streamDir);
        return true;
    } catch (const std::exception& e) {
        RTSP_STATUS_LOG_ERROR("Failed to create stream directory " << streamDir << ": " << e.what());
        return false;
    }
}

bool RTSPStatus::writeParameter(const std::string& streamName,
                               const std::string& parameter,
                               const std::string& value) {
    std::string filePath = STATUS_BASE_DIR + streamName + "/" + parameter;

    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            RTSP_STATUS_LOG_ERROR("Failed to open file for writing: " << filePath);
            return false;
        }

        file << value << std::endl;
        file.close();

        RTSP_STATUS_LOG_DEBUG("Wrote " << parameter << "=" << value << " to " << filePath);
        return true;
    } catch (const std::exception& e) {
        RTSP_STATUS_LOG_ERROR("Failed to write parameter " << parameter << " to " << filePath << ": " << e.what());
        return false;
    }
}

bool RTSPStatus::removeStreamDirectory(const std::string& streamName) {
    std::string streamDir = STATUS_BASE_DIR + streamName + "/";

    try {
        if (std::filesystem::exists(streamDir)) {
            std::filesystem::remove_all(streamDir);
            RTSP_STATUS_LOG_DEBUG("Removed stream directory: " << streamDir);
        }
        return true;
    } catch (const std::exception& e) {
        RTSP_STATUS_LOG_ERROR("Failed to remove stream directory " << streamDir << ": " << e.what());
        return false;
    }
}

bool RTSPStatus::ensureBaseDirectory() {
    try {
        if (!std::filesystem::exists(STATUS_BASE_DIR)) {
            std::filesystem::create_directories(STATUS_BASE_DIR);
            RTSP_STATUS_LOG_DEBUG("Created base directory: " << STATUS_BASE_DIR);
        }
        return true;
    } catch (const std::exception& e) {
        RTSP_STATUS_LOG_ERROR("Failed to create base directory " << STATUS_BASE_DIR << ": " << e.what());
        return false;
    }
}
