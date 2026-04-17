#ifndef TIMESTAMP_MANAGER_HPP
#define TIMESTAMP_MANAGER_HPP

#include <sys/time.h>
#include <cstdint>
#include <mutex>
#include <time.h>

/**
 * TimestampManager - DeepSeek's Recommended 64-bit Monotonic Timestamp Implementation
 *
 * Implements Option 1 from DeepSeek's recommendations:
 * - 64-bit nanosecond precision using CLOCK_MONOTONIC
 * - 584+ year overflow period (never worry about overflow)
 * - Immune to system time changes (NTP adjustments)
 * - Single source of truth for all audio/video timestamps
 * - High precision timing for professional streaming
 */
class TimestampManager {
public:
    /**
     * Get the singleton instance of TimestampManager
     */
    static TimestampManager& getInstance();

    /**
     * Initialize the timestamp manager.
     * Sets up the 64-bit monotonic timebase.
     *
     * @return 0 on success, non-zero on error
     */
    int initialize();

    /**
     * Get current 64-bit monotonic timestamp in nanoseconds.
     * This is the primary high-precision timestamp method.
     *
     * @return Current monotonic timestamp in nanoseconds (64-bit)
     */
    uint64_t getTimestampNs();

    /**
     * Get current monotonic timestamp in microseconds for compatibility.
     *
     * @return Current monotonic timestamp in microseconds
     */
    uint64_t getTimestampUs();

    /**
     * Get current monotonic timestamp as timeval for legacy compatibility.
     *
     * @param tv Pointer to timeval structure to fill
     */
    void getTimestamp(struct timeval* tv);

    /**
     * Check if the timestamp manager has been initialized
     *
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return initialized; }

private:
    TimestampManager() = default;
    ~TimestampManager() = default;

    // Prevent copying
    TimestampManager(const TimestampManager&) = delete;
    TimestampManager& operator=(const TimestampManager&) = delete;

    bool initialized = false;
    mutable std::mutex mutex;           // Thread safety
};

#endif // TIMESTAMP_MANAGER_HPP
