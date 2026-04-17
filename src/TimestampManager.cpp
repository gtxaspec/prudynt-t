#include "TimestampManager.hpp"
#include "Logger.hpp"
#include "IMPSystem.hpp"
#include <time.h>
#include <map>
#include <cstdlib>
#include <chrono>
#include <imp_system.h>

#define MODULE "TIMESTAMP_MANAGER"

TimestampManager& TimestampManager::getInstance() {
    static TimestampManager instance;
    return instance;
}

int TimestampManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);

    if (initialized) {
        LOG_WARN("TimestampManager already initialized");
        return 0;
    }

    // No baseline needed - we use IMP_System_GetTimeStamp() directly
    // This ensures SINGLE SOURCE OF TRUTH with hardware timestamps
    initialized = true;

    LOG_INFO("TimestampManager initialized - using IMP hardware timestamps directly");
    return 0;
}

uint64_t TimestampManager::getTimestampNs() {
    if (!initialized) {
        LOG_ERROR("TimestampManager not initialized!");
        return 0;
    }

    // SINGLE SOURCE OF TRUTH: Use IMP hardware timestamp (already monotonic)
    // IMP_System_GetTimeStamp() returns microseconds since the baseline we set
    int64_t impTimestamp = IMP_System_GetTimeStamp();

    // Convert to nanoseconds for high precision
    return static_cast<uint64_t>(impTimestamp) * 1000ULL;
}

uint64_t TimestampManager::getTimestampUs() {
    if (!initialized) {
        LOG_ERROR("TimestampManager not initialized!");
        return 0;
    }

    // SINGLE SOURCE OF TRUTH: Use IMP hardware timestamp directly
    return static_cast<uint64_t>(IMP_System_GetTimeStamp());
}

void TimestampManager::getTimestamp(struct timeval* tv) {
    if (!tv) {
        LOG_ERROR("TimestampManager::getTimestamp: null timeval pointer");
        return;
    }

    if (!initialized) {
        LOG_ERROR("TimestampManager not initialized!");
        tv->tv_sec = 0;
        tv->tv_usec = 0;
        return;
    }

    // SINGLE SOURCE OF TRUTH: Use IMP hardware timestamp directly
    int64_t impTimestamp = IMP_System_GetTimeStamp();
    tv->tv_sec = impTimestamp / 1000000;
    tv->tv_usec = impTimestamp % 1000000;

    // TIMESTAMP DEBUG: Log TimestampManager output (rate-limited)
    static int log_count = 0;
    if (log_count < 10 || log_count % 100 == 0) {
        LOG_DEBUG("TIMESTAMP_MANAGER_0_SOURCE: impTimestamp=" << impTimestamp << " tv_sec=" << tv->tv_sec << " tv_usec=" << tv->tv_usec);
        log_count++;
    } else {
        log_count++;
    }
}


