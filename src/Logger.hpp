#ifndef Logger_hpp
#define Logger_hpp

#include <cstring>
#include <sstream>
#include <mutex>
#include "Config.hpp"

#define __FILENAME__ (strrchr("/" __FILE__, '/') + 1)
#define LOG_EMER(str) Logger::log(Logger::EMERGENCY, __FILENAME__, LogMsg() << str)
#define LOG_ALER(str)     Logger::log(Logger::ALERT, __FILENAME__, LogMsg() << str)
#define LOG_CRIT(str)  Logger::log(Logger::CRITICAL, __FILENAME__, LogMsg() << str)
#define LOG_ERROR(str)     Logger::log(Logger::ERROR, __FILENAME__, LogMsg() << str)
#define LOG_WARN(str)      Logger::log(Logger::WARN, __FILENAME__, LogMsg() << str)
#define LOG_NOTICE(str)    Logger::log(Logger::NOTICE, __FILENAME__, LogMsg() << str)
#define LOG_INFO(str)      Logger::log(Logger::INFO, __FILENAME__, LogMsg() << str)

#if defined(ENABLE_LOG_DEBUG)
    #define LOG_DEBUG(str)     Logger::log(Logger::DEBUG, __FILENAME__, LogMsg() << str)
    #define LOG_DEBUG_OR_ERROR(condition, str)  \
        ((condition) == 0 ? Logger::log(Logger::DEBUG, __FILENAME__, LogMsg() << str) : \
        Logger::log(Logger::ERROR, __FILENAME__, LogMsg() << str))
    #define LOG_DEBUG_OR_ERROR_AND_EXIT(condition, str) \
        if ((condition) == 0) { \
            Logger::log(Logger::DEBUG, __FILENAME__, LogMsg() << str); \
        } else { \
            Logger::log(Logger::ERROR, __FILENAME__, LogMsg() << str << " returns " << condition); \
            return condition; \
        }
#else
    #define LOG_DEBUG(str) ((void)0)
    #define LOG_DEBUG_OR_ERROR(condition, str)  \
        ((condition) == 0 ? (void)0 : \
        Logger::log(Logger::ERROR, __FILENAME__, LogMsg() << str))
    #define LOG_DEBUG_OR_ERROR_AND_EXIT(condition, str) \
        if ((condition) == 0) { \
            (void)0 \
        } else { \
            Logger::log(Logger::ERROR, __FILENAME__, LogMsg() << str << " returns " << condition); \
            return condition; \
        }
#endif

struct LogMsg {
    LogMsg() {};
    std::string log_str;
    LogMsg& operator<<(std::string a) {
        log_str.append(a);
        return *this;
    }

    LogMsg& operator<<(int a) {
        std::stringstream ss;
        ss << a;
        log_str.append(ss.str());
        return *this;
    }
};

class Logger {
public:
    enum Level {
        EMERGENCY,
        ALERT,
        CRIT,
        ERROR,
        WARN,
        NOTICE,
        INFO,
        DEBUG
    };

    static bool init(std::string logLevel);
    static void log(Level level, std::string module, LogMsg msg);

    static void setLevel(std::string lvl);
    static Level level;

private:
    static std::mutex log_mtx;
    
};

#endif
