#include <iostream>
#include <unistd.h>
#include <syslog.h>

// Undefine conflicting macros from syslog.h
#undef LOG_INFO
#undef LOG_DEBUG
#undef LOG_CRIT
#undef LOG_NOTICE

#define MODULE "LOGGER"

#include "Logger.hpp"
#include "Config.hpp"

const char* text_levels[] = {
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG"
};

Logger::Level stringToLogLevel(const std::string& levelStr) {
    if (levelStr == "EMERGENCY") return Logger::EMERGENCY;
    if (levelStr == "ALERT") return Logger::ALERT;
    if (levelStr == "CRITICAL") return Logger::CRIT;
    if (levelStr == "ERROR") return Logger::ERROR;
    if (levelStr == "WARN") return Logger::WARN;
    if (levelStr == "NOTICE") return Logger::NOTICE;
    if (levelStr == "INFO") return Logger::INFO;
    if (levelStr == "DEBUG") return Logger::DEBUG;
    // Default level if unknown string
    return Logger::INFO; // or any default level you prefer
}

Logger::Level Logger::level = stringToLogLevel(Config::singleton()->logLevel);

std::mutex Logger::log_mtx;

bool Logger::init() {
    // Initialize the syslog
    openlog("prudynt", LOG_PID | LOG_NDELAY, LOG_USER);

    LOG_DEBUG("Logger Init.");
    return false;
}

void Logger::setLevel(std::string lvl) {
    Logger::level = stringToLogLevel(lvl);
}

void Logger::log(Level lvl, std::string module, LogMsg msg) {
    std::unique_lock<std::mutex> lck(log_mtx);

    // Always log to syslog regardless of level
    int syslogPriority;
    switch (lvl) {
        case EMERGENCY: syslogPriority = 0; break;
        case ALERT: syslogPriority = 1; break;
        case CRIT: syslogPriority = 2; break;
        case ERROR: syslogPriority = 3; break;
        case WARN: syslogPriority = 4; break;
        case NOTICE: syslogPriority = 5; break;
        case INFO: syslogPriority = 6; break;
        case DEBUG: syslogPriority = 7; break;
        default:    syslogPriority = 7; break; // Default case for undefined levels
    }
    syslog(syslogPriority, "[%s:%s]: %s", text_levels[lvl], module.c_str(), msg.log_str.c_str());

    // Filter what gets printed on the console based on the Logger::level
    if (Logger::level >= lvl) {
        std::stringstream fmt;
        fmt << "[" << text_levels[lvl] << ":" << module << "]: " << msg.log_str << std::endl;
        std::cout << fmt.str();
    }
}

// Remember to close the syslog
// closelog();
