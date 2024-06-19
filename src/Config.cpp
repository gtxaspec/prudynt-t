#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <functional>
#include <libconfig.h++>
#include <variant>

#include "Config.hpp"
#include "Logger.hpp"

#define MODULE "CONFIG"

namespace fs = std::filesystem;

#if defined(OLD_CONFIG)
bool CFG::readConfig() {

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.cfg";
    filePath = cfgFilePath;

    // Try to load the configuration file from the specified paths
    try {
        lc.readFile(cfgFilePath.c_str());
        LOG_INFO("Loaded configuration from " + cfgFilePath.string());
    } catch (const libconfig::FileIOException &) {
        fs::path etcPath = "/etc/prudynt.cfg";
        filePath = etcPath;
        try {
            lc.readFile(etcPath.c_str());
            LOG_INFO("Loaded configuration from " + etcPath.string());
        } catch (...) {
            LOG_WARN("Failed to load prudynt configuration file from /etc.");
            return 0; // Exit if configuration file is missing
        }
    } catch (const libconfig::ParseException &pex) {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return 0; // Exit on parsing error
    }

    return 1;
}

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
        //LOG_WARN("Missing configuration for " + item.path + ", using default.");
    } else if (!item.validate(item.value)) {
        //LOG_ERROR(item.errorMessage); // Log validation error
        item.value = item.defaultValue; // Revert to default if validation fails
    }

    //std::cout << item.path << " = " << item.value << std::endl;
}

template<typename T>
void handleConfigItem2(libconfig::Config &lc, ConfigItem<T> &item) {
    
    T configValue;
    bool readFromConfig = lc.lookupValue(item.path, configValue);
    bool readFromProc = false;
    bool addToConfig = false;

    size_t pos = item.path.find('.');
    std::string sect = item.path.substr(0, pos);
    std::string entr = item.path.substr(pos + 1);
    
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
        }
    }

    if(item.value != configValue && !readFromProc) {

        if(lc.lookup(sect).exists(entr)) 
            lc.lookup(sect).remove(entr);

        if constexpr (std::is_same_v<T, bool>) {
            lc.lookup(sect).add(entr, libconfig::Setting::TypeBoolean) = item.value;;
        } else if constexpr (std::is_same_v<T, std::string>) {
            lc.lookup(sect).add(entr, libconfig::Setting::TypeString) = item.value;;
        } else if constexpr (std::is_same_v<T, int>) {
            lc.lookup(sect).add(entr, libconfig::Setting::TypeInt) = item.value;
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            libconfig::Setting &setting = lc.lookup(sect).add(entr, libconfig::Setting::TypeInt) = (long)item.value;
            setting.setFormat(libconfig::Setting::FormatHex);
        }
    }

    //cleanup do not write default
    if(item.value == item.defaultValue) {
        if(lc.lookup(sect).exists(entr)) 
            lc.lookup(sect).remove(entr);
    }    
}

bool CFG::updateConfig() { 
    
    config_loaded = readConfig();

    if( config_loaded ) {

        // Process all configuration items
        for (auto &item : boolItems) handleConfigItem2(lc, item);
        for (auto &item : stringItems) handleConfigItem2(lc, item);
        for (auto &item : intItems) handleConfigItem2(lc, item);
        for (auto &item : uintItems) handleConfigItem2(lc, item);

        libconfig::Setting &root = lc.getRoot();

        if(root.exists("rois"))
            root.remove("rois");

        libconfig::Setting &rois = root.add("rois", libconfig::Setting::TypeGroup);

        for (int i=0; i<motion.roi_count; i++) {

            libconfig::Setting &entry = rois.add("roi_" + std::to_string(i), libconfig::Setting::TypeArray);
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p0_x;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p0_y;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p1_x;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p1_y; 
        }     
        
    }    

    lc.writeFile(filePath);
    LOG_DEBUG("Config is written to " << filePath);

    return true;
};

CFG::CFG() {
    
    config_loaded = readConfig();

    if( config_loaded ) {

        // Process all configuration items
        for (auto &item : boolItems) handleConfigItem(lc, item, missingConfigs);
        for (auto &item : stringItems) handleConfigItem(lc, item, missingConfigs);
        for (auto &item : intItems) handleConfigItem(lc, item, missingConfigs);
        for (auto &item : uintItems) handleConfigItem(lc, item, missingConfigs);

        libconfig::Setting &root = lc.getRoot();

        if(root.exists("rois")) {

            libconfig::Setting &rois = root.lookup("rois");

            for (int i=0; i<motion.roi_count; i++) {

                int n = atoi(std::regex_replace(rois[i].getName(),
                            std::regex("[^0-9]*([0-9]+).*"), std::string("$1")).c_str());

                if(rois[i].getLength() == 4) {
                    motion.rois[n].p0_x = rois[i][0];
                    motion.rois[n].p0_y = rois[i][1];
                    motion.rois[n].p1_x = rois[i][2];
                    motion.rois[n].p1_y = rois[i][3];
                }
            }
        }        
    }
}

#else  //defined(OLD_CONFIG)

bool read_proc(std::string procPath, std::string &line) {

    std::ifstream procFile(procPath);

    if (procFile) {
        if (std::getline(procFile, line)) {
            return true;
        }
    }
    return false;
}

bool CFG::readConfig() {

    // Construct the path to the configuration file in the same directory as the program binary
    fs::path binaryPath = fs::read_symlink("/proc/self/exe").parent_path();
    fs::path cfgFilePath = binaryPath / "prudynt.cfg";
    filePath = cfgFilePath;

    // Try to load the configuration file from the specified paths
    try {
        lc.readFile(cfgFilePath.c_str());
        LOG_INFO("Loaded configuration from " + cfgFilePath.string());
    } catch (const libconfig::FileIOException &) {
        fs::path etcPath = "/etc/prudynt.cfg";
        filePath = etcPath;
        try {
            lc.readFile(etcPath.c_str());
            LOG_INFO("Loaded configuration from " + etcPath.string());
        } catch (...) {
            LOG_WARN("Failed to load prudynt configuration file from /etc.");
            return 0; // Exit if configuration file is missing
        }
    } catch (const libconfig::ParseException &pex) {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return 0; // Exit on parsing error
    }

    return 1;
}

bool CFG::updateConfig() {

    config_loaded = readConfig();

    if( config_loaded ) {

        libconfig::Setting &root = lc.getRoot();

        for (auto &item : settings) {
            
            std::string path = item.first;

            size_t pos = path.find('.');
            std::string sect = path.substr(0, pos);
            std::string entr = path.substr(pos + 1);

            if (!root.exists(sect)) {
                root.add(sect, libconfig::Setting::TypeGroup);
            }
            
            std::visit([&path,&root,&sect,&entr,this](const auto& e) {

                using T = std::decay_t<decltype(e.value)>;
                using U = std::decay_t<decltype(e)>;

                std::string line;
                int state = 0;

                if (!state && !e.procPath.empty()) 
                    state |= read_proc(e.procPath, line)?2:0;

                if(lc.lookup(sect).exists(entr)) 
                    lc.lookup(sect).remove(entr);

                if constexpr (std::is_same_v<U, bolEntry>) {
                    if constexpr (std::is_same_v<T, bool>) {
                        bool value = e.defaultValue;
                        state |= lc.lookupValue(path, value);                   
                        if(value != e.value && e.value != e.defaultValue && state != 2) {
                            root.lookup(sect).add(entr, libconfig::Setting::TypeBoolean) = e.value;
                        }
                    }
                } else if constexpr (std::is_same_v<U, intEntry>) {
                    if constexpr (std::is_same_v<T, int>) {
                        int value = e.defaultValue;
                        state |= lc.lookupValue(path, value);                          
                        if(value != e.value && e.value != e.defaultValue && state != 2) {
                            root.lookup(sect).add(entr, libconfig::Setting::TypeInt) = e.value;
                        }
                    }
                } else if constexpr (std::is_same_v<U, strEntry>) {
                    if constexpr (std::is_same_v<T, std::string>) {
                        std::string value = e.defaultValue;
                        state |= lc.lookupValue(path, value);                         
                        if(value != e.value && e.value != e.defaultValue && state != 2) {
                            root.lookup(sect).add(entr, libconfig::Setting::TypeString) = e.value;
                        }
                    }
                }else if constexpr (std::is_same_v<U, uintEntry>) {
                    if constexpr (std::is_same_v<T, unsigned int>) {
                        unsigned int value = e.defaultValue;
                        state |= lc.lookupValue(path, value);                                               
                        if(value != e.value && e.value != e.defaultValue && state != 2) {
                            libconfig::Setting &setting = root.lookup(sect).add(entr, libconfig::Setting::TypeInt) = (long)e.value;
                            setting.setFormat(libconfig::Setting::FormatHex);                            
                        }
                    }
                }
            }, item.second);
        }

        if(root.exists("rois"))
            root.remove("rois");

        libconfig::Setting &rois = root.add("rois", libconfig::Setting::TypeGroup);

        for (int i=0; i<motion.roi_count; i++) {

            libconfig::Setting &entry = rois.add("roi_" + std::to_string(i), libconfig::Setting::TypeArray);
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p0_x;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p0_y;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p1_x;
            entry.add(libconfig::Setting::TypeInt) = motion.rois[i].p1_y; 
        }             
    }

    lc.writeFile(filePath);
    LOG_DEBUG("Config is written to " << filePath);

    return true;
}

CFG::CFG() {
    
    config_loaded = readConfig();

    if( config_loaded ) {

        for (auto &item : settings) {
            
            std::string path = item.first;

            std::visit([&path,this](const auto& e) {

                using T = std::decay_t<decltype(e.value)>;
                using U = std::decay_t<decltype(e)>;
                
                T value; 
                std::string line;
                int state = lc.lookupValue(path, value);

                if (!state && !e.procPath.empty()) 
                    state |= read_proc(e.procPath, line)?2:0;

                if constexpr (std::is_same_v<U, bolEntry>) {
                    if constexpr (std::is_same_v<T, bool>) {
                        if(state){ 
                            if(e.isValid(value)) { 
                                e.value = value;
                            } else {
                                std::cout << "[CFG:Config.cpp] " << path << ", " << e.message << std::endl;
                                e.value = e.defaultValue;
                            }
                        } else {
                            e.value = e.defaultValue;
                        }
                    }
                } else if constexpr (std::is_same_v<U, intEntry>) {
                    if constexpr (std::is_same_v<T, int>) {
                        if(state==2) {
                            std::istringstream iss(line);
                            iss >> value;
                            std::cout << "[CFG:Config.cpp] " << path << ", Use config from procfs." << std::endl;
                        }
                        if(state){ 
                            if(e.isValid(value)) { 
                                e.value = value;
                            } else {
                                std::cout << "[CFG:Config.cpp] " << path << ", " << e.message << std::endl;
                                e.value = e.defaultValue;
                            }
                        } else {
                            e.value = e.defaultValue;
                        }
                    }
                } else if constexpr (std::is_same_v<U, strEntry>) {
                    if constexpr (std::is_same_v<T, std::string>) {
                        if(state == 2) { //not read from config but found in procfs
                            value = line;
                            std::cout << "[CFG:Config.cpp] " << path << ", use config from procfs." << std::endl;
                        }
                        if(state){ 
                            if(e.isValid(value)) { 
                                e.value = value;
                            } else {
                                std::cout << "[CFG:Config.cpp] " << path << ", " << e.message << std::endl;
                                e.value = e.defaultValue;
                            }
                        } else {
                            e.value = e.defaultValue;
                        }
                    }
                }else if constexpr (std::is_same_v<U, uintEntry>) {
                    if constexpr (std::is_same_v<T, unsigned int>) {
                        if(state==2) {
                            std::istringstream iss(line);
                            if (line.find("0x") == 0) { 
                                iss >> std::hex >> value;
                            } else {
                                iss >> value;
                            }
                            std::cout << "[CFG:Config.cpp] " << path << ", Use config from procfs." << std::endl;
                        }
                        if(state){ 
                            if(e.isValid(value)) { 
                                e.value = value;
                            } else {
                                std::cout << "[CFG:Config.cpp]" << path << ", " << e.message << std::endl;
                                e.value = e.defaultValue;
                            }
                        } else {
                            e.value = e.defaultValue;
                        }
                    }
                }
            }, item.second);
        }

        libconfig::Setting &root = lc.getRoot();

        if(root.exists("rois")) {

            libconfig::Setting &rois = root.lookup("rois");

            for (int i=0; i<motion.roi_count; i++) {

                int n = atoi(std::regex_replace(rois[i].getName(),
                            std::regex("[^0-9]*([0-9]+).*"), std::string("$1")).c_str());

                if(rois[i].getLength() == 4) {
                    motion.rois[n].p0_x = rois[i][0];
                    motion.rois[n].p0_y = rois[i][1];
                    motion.rois[n].p1_x = rois[i][2];
                    motion.rois[n].p1_y = rois[i][3];
                }
            }
        }
    }
}
#endif //defined(OLD_CONFIG)

