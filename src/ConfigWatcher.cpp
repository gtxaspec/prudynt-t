#include "ConfigWatcher.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "globals.hpp"

#include <sys/inotify.h>
#include <sys/stat.h>

#define MODULE "ConfigWatcher"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

ConfigWatcher::ConfigWatcher()
{
    LOG_DEBUG("ConfigWatcher created.");
}

ConfigWatcher::~ConfigWatcher()
{
    LOG_DEBUG("ConfigWatcher destroyed.");
}

void ConfigWatcher::run()
{
#ifdef __linux__ // Check if on Linux where inotify is expected
    watch_using_notify();
#else
    // Fallback to polling on non-Linux systems or if inotify fails
    watch_using_poll();
#endif
}

void *ConfigWatcher::thread_entry(void *arg)
{
    (void) arg; // Mark unused
    LOG_DEBUG("Starting config watch thread.");
    ConfigWatcher watcher;
    watcher.run();
    LOG_DEBUG("Exiting config watch thread.");
    return nullptr;
}

void ConfigWatcher::watch_using_notify()
{
    int inotifyFd = inotify_init();
    if (inotifyFd < 0)
    {
        LOG_ERROR("inotify_init() failed");
        return;
    }

    int watchDescriptor = inotify_add_watch(inotifyFd,
                                            cfg->filePath.c_str(),
                                            IN_MODIFY | IN_ALL_EVENTS);
    if (watchDescriptor == -1)
    {
        LOG_ERROR("inotify_add_watch() failed");
        close(inotifyFd);
        return;
    }

    char buffer[EVENT_BUF_LEN];

    LOG_DEBUG("Monitoring file for changes: " << cfg->filePath);

    while (true)
    {
        int length = read(inotifyFd, buffer, EVENT_BUF_LEN);
        if (length < 0)
        {
            LOG_ERROR("Error reading file change notification.");
            break;
        }

        int i = 0;
        while (i < length)
        {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];

            if (event->mask & IN_MODIFY)
            {
                cfg->load();
                LOG_INFO("Config file changed, the config is reloaded from: " << cfg->filePath);
            }

            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(inotifyFd, watchDescriptor);
    close(inotifyFd);
    return;
}

void ConfigWatcher::watch_using_poll()
{
    struct stat fileInfo;
    time_t lastModifiedTime = 0;

    while (true)
    {
        if (stat(cfg->filePath.c_str(), &fileInfo) == 0)
        {
            if (lastModifiedTime == 0)
            {
                lastModifiedTime = fileInfo.st_mtime;
            }
            else if (fileInfo.st_mtime != lastModifiedTime)
            {
                lastModifiedTime = fileInfo.st_mtime;
                cfg->load();
                LOG_INFO("Config file changed, the config is reloaded from: " << cfg->filePath);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
