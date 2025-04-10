#ifndef VIDEO_WORKER_HPP
#define VIDEO_WORKER_HPP

class VideoWorker
{
public:
    explicit VideoWorker(int encChn);
    ~VideoWorker();

    static void *thread_entry(void *arg);

private:
    void run();

    int encChn;
};

#endif // VIDEO_PROCESSOR_HPP
