#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <libconfig.h++>

#include "Config.hpp"
#include "Logger.hpp"

#define MODULE "CONFIG"

namespace fs = std::filesystem;
Config* Config::instance = nullptr;



// Utility function to handle configuration lookup, default assignment, and validation
template<typename T>
void handleConfigItem(libconfig::Config &lc, ConfigItem<T> &item, std::vector<std::string> &missingConfigs) {
    bool readFromConfig = lc.lookupValue(item.path, item.value);
    bool readFromProc = false;

    if (!readFromConfig && !item.procPath.empty()) { // If not read from config and procPath is set
        // Attempt to read from the proc filesystem
        std::ifstream procFile(item.procPath);
        if (procFile) {
            T value;
            std::string line;
            if (std::getline(procFile, line)) {
                if constexpr (std::is_same_v<T, std::string>) { // Check if T is std::string
                    value = line;
                    readFromProc = true;
                } else if constexpr (std::is_same_v<T, unsigned int>) { // Check if T is unsigned int
                    std::istringstream iss(line);
                    if (line.find("0x") == 0) { // Check if the line starts with "0x"
                        iss >> std::hex >> value; // Read as hexadecimal
                    } else {
                        iss >> value; // Read as decimal
                    }
                    readFromProc = true;
                } else { // For other types, just read directly if possible
                    std::istringstream iss(line);
                    if (iss >> value) {
                        readFromProc = true;
                    }
                }
            }
            if (readFromProc) {
                item.value = value; // Assign the value read from proc
            }
        }
    }

    if (!readFromConfig && !readFromProc) {
        item.value = item.defaultValue; // Assign default value if not found anywhere
        missingConfigs.push_back(item.path); // Record missing config
        LOG_WARN("Missing configuration for " + item.path + ", using default.");
    } else if (!item.validate(item.value)) {
        LOG_ERROR(item.errorMessage); // Log validation error
        item.value = item.defaultValue; // Revert to default if validation fails
    }
}

Config::Config() {
    libconfig::Config lc;

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.cfg";

    // Try to load the configuration file from the specified paths
    try {
        lc.readFile(cfgFilePath.c_str());
        LOG_INFO("Loaded configuration from " + cfgFilePath.string());
    } catch (const libconfig::FileIOException &) {
        LOG_DEBUG("Failed to load prudynt configuration file from binary directory. Trying /etc...");
        fs::path etcPath = "/etc/prudynt.cfg";
        try {
            lc.readFile(etcPath.c_str());
            LOG_INFO("Loaded configuration from " + etcPath.string());
        } catch (...) {
            LOG_WARN("Failed to load prudynt configuration file from /etc.");
            return; // Exit if configuration file is missing
        }
    } catch (const libconfig::ParseException &pex) {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return; // Exit on parsing error
    }

    // Process all configuration items
    for (auto &item : boolItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : stringItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : intItems) handleConfigItem(lc, item, missingConfigs);
    for (auto &item : uintItems) handleConfigItem(lc, item, missingConfigs);
}

Config* Config::singleton() {
    if (instance == nullptr) {
        instance = new Config();
    }
    return instance;
}
