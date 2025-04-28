#include "JPEGWorker.hpp"

#include "Config.hpp"
#include "Logger.hpp"
#include "WorkerUtils.hpp"
#include "globals.hpp"

#include <fcntl.h>   // For O_RDWR, O_CREAT, O_TRUNC flags
#include <unistd.h>  // For open(), close(), etc.

#define MODULE "JPEGWorker"

JPEGWorker::JPEGWorker(int jpgChnIndex, int impEncoderChn)
    : jpgChn(jpgChnIndex)
    , impEncChn(impEncoderChn)
{
    LOG_DEBUG("JPEGWorker created for JPEG channel index " << jpgChn << " (IMP Encoder Channel "
                                                           << impEncChn << ")");
}

JPEGWorker::~JPEGWorker()
{
    LOG_DEBUG("JPEGWorker destroyed for JPEG channel index " << jpgChn);
}

int JPEGWorker::save_jpeg_stream(int fd, IMPEncoderStream *stream)
{
    int ret, i, nr_pack = stream->packCount;

    for (i = 0; i < nr_pack; i++)
    {
        void *data_ptr;
        size_t data_len;

#if defined(PLATFORM_T31) || defined(PLATFORM_T40) || defined(PLATFORM_T41) || defined(PLATFORM_C100)
        IMPEncoderPack *pack = &stream->pack[i];
        uint32_t remSize = 0; // Declare remSize here
        if (pack->length)
        {
            remSize = stream->streamSize - pack->offset;
            data_ptr = (void *) ((char *) stream->virAddr
                                 + ((remSize < pack->length) ? 0 : pack->offset));
            data_len = (remSize < pack->length) ? remSize : pack->length;
        }
        else
        {
            continue; // Skip empty packs
        }
#elif defined(PLATFORM_T10) || defined(PLATFORM_T20) || defined(PLATFORM_T21) \
    || defined(PLATFORM_T23) || defined(PLATFORM_T30)
        data_ptr = reinterpret_cast<void *>(stream->pack[i].virAddr);
        data_len = stream->pack[i].length;
#endif

        // Write data to file
        ret = write(fd, data_ptr, data_len);
        if (ret != static_cast<int>(data_len))
        {
            printf("Stream write error: %s\n", strerror(errno));
            return -1; // Return error on write failure
        }

#if defined(PLATFORM_T31) || defined(PLATFORM_T40) || defined(PLATFORM_T41) || defined(PLATFORM_C100)
        // Check the condition only under T31 platform, as remSize is used here
        if (remSize && pack->length > remSize)
        {
            ret = write(fd, (void *) ((char *) stream->virAddr), pack->length - remSize);
            if (ret != static_cast<int>(pack->length - remSize))
            {
                printf("Stream write error (remaining part): %s\n", strerror(errno));
                return -1;
            }
        }
#endif
    }

    return 0;
}

// Main processing loop, adapted from Worker::jpeg_grabber
void JPEGWorker::run()
{
    LOG_DEBUG("Start JPEG processing run loop for index " << jpgChn << " (IMP Encoder Channel "
                                                          << impEncChn << ")");

    // Initial target FPS based on idle setting
    int targetFps = global_jpeg[jpgChn]->stream->jpeg_idle_fps;

    // Local stats counters
    uint32_t bps{0}; // Bytes per second
    uint32_t fps{0}; // frames per second

    // timestamp for stream stats calculation
    unsigned long long ms{0};

    // Initialize timestamp for stats calculation (ensure it's set before first use)
    gettimeofday(&global_jpeg[jpgChn]->stream->stats.ts, NULL);
    global_jpeg[jpgChn]->stream->stats.ts.tv_sec -= 10;

    while (global_jpeg[jpgChn]->running)
    {
        /*
        * if jpeg_idle_fps = 0, the thread is put into sleep until a client is connected.
        * if jpeg_idle_fps > 0, we try to reach a frame rate of stream.jpeg_idle_fps. enen if no client is connected.
        * if a client is connected via WS / HTTP we try to reach a framerate of stream.fps
        * the thread will fallback into idle / sleep mode if no client request was made for more than a second
        */
        auto now = steady_clock::now();

        std::unique_lock lck(mutex_main);
        bool request_or_overrun = global_jpeg[jpgChn]->request_or_overrun();
        lck.unlock();

        if (request_or_overrun || targetFps)
        {
            auto diff_last_image
                = duration_cast<milliseconds>(now - global_jpeg[jpgChn]->last_image).count();

            // we remove targetFps/10 millisecond's as image creation time
            // by this we get besser FPS results
            if (targetFps && diff_last_image >= ((1000 / targetFps) - targetFps / 10))
            {
                // check if current jpeg channal is running if not start it
                if (!global_video[global_jpeg[jpgChn]->streamChn]->active)
                {
                    /* required video channel was not running, we need to start it  
                    * and set run_for_jpeg as a reason.
                    */
                    std::unique_lock<std::mutex> lock_stream{mutex_main};
                    global_video[global_jpeg[jpgChn]->streamChn]->run_for_jpeg = true;
                    global_video[global_jpeg[jpgChn]->streamChn]->should_grab_frames.notify_one();
                    lock_stream.unlock();
                    global_video[global_jpeg[jpgChn]->streamChn]->is_activated.acquire();
                }

                // subscriber is connected
                if (request_or_overrun)
                {
                    if (targetFps != global_jpeg[jpgChn]->stream->fps)
                        targetFps = global_jpeg[jpgChn]->stream->fps;
                }
                // no subscriber is connected
                else
                {
                    if (targetFps != global_jpeg[jpgChn]->stream->jpeg_idle_fps)
                        targetFps = global_jpeg[jpgChn]->stream->jpeg_idle_fps;
                }

                if (IMP_Encoder_PollingStream(global_jpeg[jpgChn]->encChn,
                                              cfg->general.imp_polling_timeout)
                    == 0)
                {
                    IMPEncoderStream stream;
                    if (IMP_Encoder_GetStream(global_jpeg[jpgChn]->encChn,
                                              &stream,
                                              GET_STREAM_BLOCKING)
                        == 0)
                    {
                        fps++;
                        bps += stream.pack->length;

                        //  Check for success
                        const char *tempPath = "/tmp/snapshot.tmp"; // Temporary path
                        const char *finalPath = global_jpeg[jpgChn]
                                                    ->stream
                                                    ->jpeg_path; // Final path for the JPEG snapshot

                        // Open and create temporary file with read and write permissions
                        int snap_fd = open(tempPath, O_RDWR | O_CREAT | O_TRUNC, 0666);
                        if (snap_fd >= 0)
                        {
                            // Save the JPEG stream to the file
                            save_jpeg_stream(snap_fd, &stream);

                            // Close the temporary file after writing is done
                            close(snap_fd);

                            // Atomically move the temporary file to the final destination
                            if (rename(tempPath, finalPath) != 0)
                            {
                                LOG_ERROR("Failed to move JPEG snapshot from " << tempPath << " to "
                                                                               << finalPath);
                                std::remove(
                                    tempPath); // Attempt to remove the temporary file if rename fails
                            }
                            else
                            {
                                // LOG_DEBUG("JPEG snapshot successfully updated");
                            }
                        }
                        else
                        {
                            LOG_ERROR("Failed to open JPEG snapshot for writing: " << tempPath);
                        }

                        IMP_Encoder_ReleaseStream(global_jpeg[jpgChn]->encChn,
                                                  &stream); // Release stream after saving
                    }

                    ms = WorkerUtils::tDiffInMs(&global_jpeg[jpgChn]->stream->stats.ts);
                    if (ms > 1000)
                    {
                        global_jpeg[jpgChn]->stream->stats.fps = fps;
                        global_jpeg[jpgChn]->stream->stats.bps = bps;
                        fps = 0;
                        bps = 0;
                        gettimeofday(&global_jpeg[jpgChn]->stream->stats.ts, NULL);

                        LOG_DDEBUG("JPG " << jpgChn
                                          << " fps: " << global_jpeg[jpgChn]->stream->stats.fps
                                          << " bps: " << global_jpeg[jpgChn]->stream->stats.bps
                                          << " diff_last_image: " << diff_last_image
                                          << " request_or_overrun: " << request_or_overrun
                                          << " targetFps: " << targetFps << " ms: " << ms);
                    }
                }

                global_jpeg[jpgChn]->last_image = steady_clock::now();
            }
            else
            {
                usleep(1000);
            }
        }
        else
        {
            LOG_DDEBUG("JPEG LOCK" << " channel:" << jpgChn);

            global_jpeg[jpgChn]->stream->stats.bps = 0;
            global_jpeg[jpgChn]->stream->stats.fps = 0;
            targetFps = 0;

            std::unique_lock<std::mutex> lock_stream{mutex_main};
            global_jpeg[jpgChn]->active = false;
            global_video[global_jpeg[jpgChn]->streamChn]->run_for_jpeg = false;
            while (!global_jpeg[jpgChn]->request_or_overrun() && !global_restart_video)
                global_jpeg[jpgChn]->should_grab_frames.wait(lock_stream);

            targetFps = global_jpeg[jpgChn]->stream->fps;

            global_jpeg[jpgChn]->is_activated.release();
            global_jpeg[jpgChn]->active = true;

            LOG_DDEBUG("JPEG UNLOCK" << " channel:" << jpgChn);
        }
    }

    LOG_DEBUG("Exiting JPEG processing run loop for index " << jpgChn);
}

// Static entry point for creating the thread
void *JPEGWorker::thread_entry(void *arg)
{
    LOG_DEBUG("Start jpeg_grabber thread.");

    StartHelper *sh = static_cast<StartHelper *>(arg);
    int jpgChn = sh->encChn - 2;
    int ret;

    /* do not use the live config variable
    */
    global_jpeg[jpgChn]->streamChn = global_jpeg[jpgChn]->stream->jpeg_channel;

    if (global_jpeg[jpgChn]->streamChn == 0)
    {
        cfg->stream2.width = cfg->stream0.width;
        cfg->stream2.height = cfg->stream0.height;
    }
    else
    {
        cfg->stream2.width = cfg->stream1.width;
        cfg->stream2.height = cfg->stream1.height;
    }

    global_jpeg[jpgChn]->imp_encoder = IMPEncoder::createNew(global_jpeg[jpgChn]->stream,
                                                             sh->encChn,
                                                             global_jpeg[jpgChn]->streamChn,
                                                             "stream2");

    // inform main that initialization is complete
    sh->has_started.release();

    ret = IMP_Encoder_StartRecvPic(global_jpeg[jpgChn]->encChn);
    LOG_DEBUG_OR_ERROR(ret, "IMP_Encoder_StartRecvPic(" << global_jpeg[jpgChn]->encChn << ")");
    if (ret != 0)
        return 0;

    global_jpeg[jpgChn]->active = true;
    global_jpeg[jpgChn]->running = true;
    JPEGWorker worker(jpgChn, sh->encChn);
    worker.run();

    if (global_jpeg[jpgChn]->imp_encoder)
    {
        global_jpeg[jpgChn]->imp_encoder->deinit();

        delete global_jpeg[jpgChn]->imp_encoder;
        global_jpeg[jpgChn]->imp_encoder = nullptr;
    }

    return 0;
}
