#include <iostream>
#include <unistd.h>
#include <syslog.h>

// Undefine conflicting macros from syslog.h
#undef LOG_INFO
#undef LOG_DEBUG

#define MODULE "LOGGER"

#include "Logger.hpp"

const char* text_levels[] = {
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

Logger::Level Logger::level = DEBUG;
std::mutex Logger::log_mtx;

bool Logger::init() {
    // Initialize the syslog
    openlog("prudynt", LOG_PID | LOG_NDELAY, LOG_USER);

    LOG_INFO("Logger Init.");
    return false;
}

void Logger::log(Level lvl, std::string module, LogMsg msg) {
    if (Logger::level < lvl) {
        return;
    }
    std::unique_lock<std::mutex> lck(log_mtx);
    std::stringstream fmt;
    fmt << "[" << text_levels[lvl] << ":" << module << "]: " << msg.log_str << std::endl;

    // Output to console
    std::cout << fmt.str();

    // Output to syslog
    int syslogPriority;
    switch (lvl) {
        case ERROR: syslogPriority = 3; break;
        case WARN:  syslogPriority = 4; break;
        case INFO:  syslogPriority = 6; break;
        case DEBUG: syslogPriority = 7; break;
        default:    syslogPriority = 7; break;
    }

    syslog(syslogPriority, "[%s:%s]: %s", text_levels[lvl], module.c_str(), msg.log_str.c_str());
}

// Remember to close the syslog
// closelog();
