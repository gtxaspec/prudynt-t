#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <libconfig.h++>
#include <variant>

#include "Config.hpp"
#include "Logger.hpp"

#define MODULE "CONFIG"

namespace fs = std::filesystem;

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
            return 0; // Exit if configuration file is missing
        }
    } catch (const libconfig::ParseException &pex) {
        LOG_WARN("Parse error at " + std::string(pex.getFile()) + ":" + std::to_string(pex.getLine()) + " - " + pex.getError());
        return 0; // Exit on parsing error
    }

    return 1;
}

 bool CFG::updateConfig() {

    if( config_loaded ) {

    }
    return 1;
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
    }
}
