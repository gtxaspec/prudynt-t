#ifndef CONFIG_WATCHER_HPP
#define CONFIG_WATCHER_HPP

class ConfigWatcher
{
public:
    ConfigWatcher();
    ~ConfigWatcher();

    static void *thread_entry(void *arg);

private:
    void run();
    void watch_using_notify();
    void watch_using_poll();
};

#endif // CONFIG_WATCHER_HPP
