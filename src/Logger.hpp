#ifndef Logger_hpp
#define Logger_hpp

#include <cstring>
#include <sstream>
#include <mutex>
#include "Config.hpp"

#define FILENAME (strrchr("/" __FILE__, '/') + 1)
#define LOG_EMER(str) Logger::log(Logger::EMERGENCY, FILENAME, LogMsg() << str)
#define LOG_ALER(str) Logger::log(Logger::ALERT, FILENAME, LogMsg() << str)
#define LOG_CRIT(str) Logger::log(Logger::CRITICAL, FILENAME, LogMsg() << str)
#define LOG_ERROR(str) Logger::log(Logger::ERROR, FILENAME, LogMsg() << str)
#define LOG_WARN(str) Logger::log(Logger::WARN, FILENAME, LogMsg() << str)
#define LOG_NOTICE(str) Logger::log(Logger::NOTICE, FILENAME, LogMsg() << str)
#define LOG_INFO(str) Logger::log(Logger::INFO, FILENAME, LogMsg() << str)

#if defined(DDEBUG)
#define LOG_DDEBUG(str) Logger::log(Logger::DEBUG, FILENAME, LogMsg() << str)
#else
#define LOG_DDEBUG(str) ((void)0)
#endif

#if defined(ENABLE_LOG_DEBUG)
#define LOG_DEBUG(str) Logger::log(Logger::DEBUG, FILENAME, LogMsg() << str)
#define LOG_DEBUG_OR_ERROR(condition, str) \
    ((condition) == 0 ? Logger::log(Logger::DEBUG, FILENAME, LogMsg() << str) : Logger::log(Logger::ERROR, FILENAME, LogMsg() << str))
#define LOG_DEBUG_OR_ERROR_AND_EXIT(condition, str)                                            \
    if ((condition) == 0)                                                                      \
    {                                                                                          \
        Logger::log(Logger::DEBUG, FILENAME, LogMsg() << str << " = " << condition); \
    }                                                                                          \
    else                                                                                       \
    {                                                                                          \
        Logger::log(Logger::ERROR, FILENAME, LogMsg() << str << " = " << condition); \
        return condition;                                                                      \
    }
#else
#define LOG_DEBUG(str) ((void)0)
#define LOG_DEBUG_OR_ERROR(condition, str) ((void)0);
#define LOG_DEBUG_OR_ERROR_AND_EXIT(condition, str) ((void)0);
#endif

struct LogMsg
{
    LogMsg() = default;
    std::string log_str;
    LogMsg &operator<<(std::string a)
    {
        log_str.append(a);
        return *this;
    }

    LogMsg &operator<<(int a)
    {
        std::stringstream ss;
        ss << a;
        log_str.append(ss.str());
        return *this;
    }
};

class Logger
{
public:
    enum Level
    {
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
