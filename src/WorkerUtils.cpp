#include "WorkerUtils.hpp"

#include <cstddef>

namespace WorkerUtils {

unsigned long long tDiffInMs(struct timeval *startTime)
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long seconds = currentTime.tv_sec - startTime->tv_sec;
    long microseconds = currentTime.tv_usec - startTime->tv_usec;

    unsigned long long milliseconds = (seconds * 1000) + (microseconds / 1000);

    return milliseconds;
}

} // namespace WorkerUtils
