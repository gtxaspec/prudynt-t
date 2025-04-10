#ifndef JPEG_WORKER_HPP
#define JPEG_WORKER_HPP

#include "IMPEncoder.hpp"

class JPEGWorker
{
public:
    explicit JPEGWorker(int jpgChnIndex, int impEncChn);
    ~JPEGWorker();

    static void *thread_entry(void *arg);

private:
    void run();
    int save_jpeg_stream(int fd, IMPEncoderStream *stream);

    int jpgChn;
    int impEncChn;
};

#endif // JPEG_PROCESSOR_HPP
