#ifndef WORKERUTILS_HPP
#define WORKERUTILS_HPP

#include <semaphore>

#include <sys/time.h>

// Struct used for signaling thread startup completion
struct StartHelper
{
    int encChn;
    std::binary_semaphore has_started{0};
};

// WorkerUtils provides utility functions that are shared across different worker classes.
// This includes time-related functions that use a monotonic clock to avoid issues
// with system time changes (e.g., from NTP).
namespace WorkerUtils {

void getMonotonicTimeOfDay(struct timeval *tv);
unsigned long long getMonotonicTimeDiffInMs(struct timeval *startTime);

} // namespace WorkerUtils

#endif // WORKERUTILS_HPP
