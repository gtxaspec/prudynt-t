#include "BackchannelProcessor.hpp"

#include "Logger.hpp"
#include "globals.hpp"

#include <cassert>
#include <cmath>
#include <vector>

#include <imp/imp_audio.h>

#define MODULE "BackchannelProcessor"

BackchannelProcessor::BackchannelProcessor()
    : currentSessionId(0)
    , fPipe(nullptr)
    , fPipeFd(-1)
{}

BackchannelProcessor::~BackchannelProcessor()
{
    closePipe();
}

static int getFrequency(IMPBackchannelFormat format)
{
#define RETURN_FREQUENCY(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        if (IMPBackchannelFormat::EnumName == format) \
            return Frequency; \
    }
    X_FOREACH_BACKCHANNEL_FORMAT(RETURN_FREQUENCY)
#undef RETURN_FREQUENCY
    return 8000;
}

static const char *getFormatName(IMPBackchannelFormat format)
{
#define RETURN_NAME(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        if (IMPBackchannelFormat::EnumName == format) \
            return NameString; \
    }
    X_FOREACH_BACKCHANNEL_FORMAT(RETURN_NAME)
#undef RETURN_NAME
    return "UNKNOWN";
}

std::vector<int16_t> BackchannelProcessor::resampleLinear(const std::vector<int16_t> &input_pcm,
                                                          int input_rate,
                                                          int output_rate)
{
    assert(input_rate != output_rate);

    double ratio = static_cast<double>(output_rate) / input_rate;
    size_t output_size = static_cast<size_t>(
        std::max(1.0, std::round(static_cast<double>(input_pcm.size()) * ratio)));

    std::vector<int16_t> output_pcm(output_size);
    size_t input_size = input_pcm.size();

    for (size_t i = 0; i < output_size; ++i)
    {
        double input_pos = static_cast<double>(i) / ratio;
        size_t index1 = static_cast<size_t>(input_pos);

        if (index1 >= input_size)
        {
            index1 = input_size - 1;
        }

        int16_t sample1 = input_pcm[index1];
        int16_t sample2 = (index1 + 1 < input_size) ? input_pcm[index1 + 1] : sample1;

        double factor = input_pos - static_cast<double>(index1);

        double interpolated_sample = static_cast<double>(sample1) * (1.0 - factor)
                                     + static_cast<double>(sample2) * factor;

        if (interpolated_sample > INT16_MAX)
            interpolated_sample = INT16_MAX;
        if (interpolated_sample < INT16_MIN)
            interpolated_sample = INT16_MIN;

        output_pcm[i] = static_cast<int16_t>(interpolated_sample);
    }

    return output_pcm;
}

bool BackchannelProcessor::initPipe()
{
    if (fPipe)
    {
        LOG_DEBUG("Pipe already initialized.");
        return true;
    }
    LOG_DEBUG("Opening pipe to: /bin/iac -s");
    fPipe = popen("/bin/iac -s", "w");
    if (fPipe == nullptr)
    {
        LOG_ERROR("popen failed: " << strerror(errno));
        fPipeFd = -1;
        return false;
    }

    fPipeFd = fileno(fPipe);
    if (fPipeFd == -1)
    {
        LOG_ERROR("fileno failed: " << strerror(errno));
        closePipe();
        return false;
    }

    int flags = fcntl(fPipeFd, F_GETFL, 0);
    if (flags == -1)
    {
        LOG_ERROR("fcntl(F_GETFL) failed: " << strerror(errno));
        closePipe();
        return false;
    }

    if (fcntl(fPipeFd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        LOG_ERROR("fcntl(F_SETFL, O_NONBLOCK) failed: " << strerror(errno));
        closePipe();
        return false;
    }

    LOG_DEBUG("Pipe opened successfully (fd=" << fPipeFd << ").");
    return true;
}

void BackchannelProcessor::closePipe()
{
    if (fPipe)
    {
        LOG_DEBUG("Closing pipe (fd=" << fPipeFd << ").");
        int ret = pclose(fPipe);
        fPipe = nullptr;
        fPipeFd = -1;
        if (ret == -1)
        {
            LOG_ERROR("pclose() failed: " << strerror(errno));
        }
        else
        {
            if (WIFEXITED(ret))
            {
                LOG_DEBUG("Pipe process exited with status: " << WEXITSTATUS(ret));
            }
            else if (WIFSIGNALED(ret))
            {
                LOG_WARN("Pipe process terminated by signal: " << WTERMSIG(ret));
            }
            else
            {
                LOG_WARN("Pipe process stopped for unknown reason.");
            }
        }
    }
}

bool BackchannelProcessor::decodeFrame(const uint8_t *payload,
                                       size_t payloadSize,
                                       IMPBackchannelFormat format,
                                       std::vector<int16_t> &outPcmBuffer)
{
    IMPAudioStream stream_in;
    stream_in.stream = const_cast<uint8_t *>(payload);
    stream_in.len = static_cast<int>(payloadSize);

    int adChn = (int) format;
    int ret = IMP_ADEC_SendStream(adChn, &stream_in, BLOCK);
    if (ret != 0)
    {
        LOG_ERROR("IMP_ADEC_SendStream failed for channel " << adChn << ": " << ret);
        return false;
    }

    IMPAudioStream stream_out;
    ret = IMP_ADEC_GetStream(adChn, &stream_out, BLOCK);
    if (ret == 0 && stream_out.len > 0 && stream_out.stream != nullptr)
    {
        size_t num_samples = stream_out.len / sizeof(int16_t);
        if (stream_out.len % sizeof(int16_t) != 0)
        {
            LOG_WARN("Decoded stream length (" << stream_out.len
                                               << ") not multiple of int16_t size. Truncating.");
        }
        outPcmBuffer.assign(reinterpret_cast<int16_t *>(stream_out.stream),
                            reinterpret_cast<int16_t *>(stream_out.stream) + num_samples);
        IMP_ADEC_ReleaseStream(adChn, &stream_out);
        return true;
    }
    else if (ret != 0)
    {
        LOG_ERROR("IMP_ADEC_GetStream failed for channel " << adChn << ": " << ret);
        return false;
    }

    LOG_DEBUG("ADEC_GetStream succeeded but produced no data.");
    outPcmBuffer.clear();
    return true;
}

bool BackchannelProcessor::writePcmToPipe(const std::vector<int16_t> &pcmBuffer)
{
    if (fPipeFd == -1 || fPipe == nullptr)
    {
        LOG_ERROR("Pipe is closed (fd=" << fPipeFd << "), cannot write PCM data.");
        return false;
    }
    if (pcmBuffer.empty())
    {
        LOG_DEBUG("Attempted to write empty PCM buffer to pipe.");
        return true;
    }

    size_t bytesToWrite = pcmBuffer.size() * sizeof(int16_t);
    const uint8_t *dataPtr = reinterpret_cast<const uint8_t *>(pcmBuffer.data());

    ssize_t bytesWritten = write(fPipeFd, dataPtr, bytesToWrite);

    if (bytesWritten == static_cast<ssize_t>(bytesToWrite))
    {
        // Bytes written match expected size
    }
    else if (bytesWritten >= 0)
    {
        LOG_WARN("Partial write to pipe (" << bytesWritten << "/" << bytesToWrite
                                           << "). Assuming pipe clogged.");
        return true;
    }
    else
    {
        int saved_errno = errno;
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK)
        {
            LOG_WARN("Pipe clogged (EAGAIN/EWOULDBLOCK). Discarding PCM chunk.");
            return true;
        }
        else if (saved_errno == EPIPE)
        {
            LOG_ERROR("write() failed: Broken pipe (EPIPE). Assuming pipe closed by "
                      "reader.");
            closePipe();
            return false;
        }
        else
        {
            LOG_ERROR("write() failed: errno=" << saved_errno << ": " << strerror(saved_errno)
                                               << ". Assuming pipe closed.");
            closePipe();
            return false;
        }
    }

    return true;
}

bool BackchannelProcessor::processFrame(const BackchannelFrame &frame)
{
    if (!cfg->audio.output_enabled)
    {
        return true;
    }

    std::vector<int16_t> decoded_pcm;
    if (!decodeFrame(frame.payload.data(), frame.payload.size(), frame.format, decoded_pcm))
    {
        // Error already logged in decodeFrame
        return true; // Continue processing loop, maybe next frame works
    }

    if (decoded_pcm.empty())
    {
        LOG_WARN("decodeFrame returned empty PCM buffer.");
        return true; // Nothing to process
    }

    // Resample only if necessary
    int input_rate = getFrequency(frame.format);
    int target_rate = cfg->audio.output_sample_rate;
    const std::vector<int16_t> *buffer_to_write = &decoded_pcm;
    std::vector<int16_t> resampled_pcm;

    if (input_rate != target_rate)
    {
        resampled_pcm = resampleLinear(decoded_pcm, input_rate, target_rate);
        buffer_to_write = &resampled_pcm;
    }

    // Write the final mono PCM to the pipe
    if (buffer_to_write != nullptr && !buffer_to_write->empty())
    {
        if (!writePcmToPipe(*buffer_to_write))
        {
            // Error writing to pipe, likely closed. Stop processing loop.
            return false;
        }
    }

    return true;
}

void BackchannelProcessor::run()
{
    if (!global_backchannel)
    {
        LOG_ERROR("Cannot run BackchannelProcessor: global_backchannel is null.");
        return;
    }

    LOG_INFO("Processor thread running...");

    global_backchannel->running = true;
    while (global_backchannel->running)
    {
        // Wait for condition: running and at least one sink is sending
        {
            std::unique_lock<std::mutex> lock(global_backchannel->mutex);
            global_backchannel->should_grab_frames.wait(lock, [&] {
                return !global_backchannel->running
                       || global_backchannel->is_sending.load(std::memory_order_acquire) > 0;
            });
        }

        if (!global_backchannel->running)
        {
            break;
        }

        BackchannelFrame frame = global_backchannel->inputQueue->wait_read();

        if (!global_backchannel->running)
        {
            break;
        }

        // Handle Zero-Payload Frame (Stop Signal)
        if (frame.payload.empty())
        {
            LOG_DEBUG("Received stop signal (zero-payload) from session " << frame.clientSessionId);
            if (frame.clientSessionId == currentSessionId && currentSessionId != 0)
            {
                LOG_INFO("Current session " << currentSessionId << " stopped. Closing pipe.");
                closePipe();
                currentSessionId = 0;
            }
            else if (currentSessionId == 0)
            {
                LOG_DEBUG("Stop signal received but no current session. Ignoring.");
            }
            else
            {
                LOG_WARN("Stop signal from non-current session "
                         << frame.clientSessionId << " (Current: " << currentSessionId
                         << "). Ignoring.");
            }
            continue;
        }

        // Handle Playback Frame (Data)
        if (currentSessionId == 0)
        {
            // No current session, this frame's sender becomes the current one
            currentSessionId = frame.clientSessionId;
            LOG_INFO("New current session " << currentSessionId << " playing "
                                            << getFormatName(frame.format) << ". Opening pipe.");
            if (!initPipe())
            {
                LOG_ERROR("Failed to open pipe for new session " << currentSessionId
                                                                 << ". Resetting.");
                currentSessionId = 0;
                continue;
            }
            // Pipe is open
            if (!processFrame(frame))
            {
                // processFrame returns false if pipe write fails and closes pipe
                LOG_WARN("processFrame failed for initial frame of session " << currentSessionId
                                                                             << ". Pipe closed.");
                currentSessionId = 0;
            }
        }
        else if (frame.clientSessionId == currentSessionId)
        {
            // Frame is from the current session
            if (!fPipe)
            { // Ensure pipe is open (it might have closed unexpectedly)
                LOG_WARN("Pipe was closed unexpectedly for current session " << currentSessionId
                                                                             << ". Reopening.");
                if (!initPipe())
                {
                    LOG_ERROR("Failed to reopen pipe for session " << currentSessionId
                                                                   << ". Resetting.");
                    currentSessionId = 0;
                    continue;
                }
            }
            // Pipe should be open
            if (!processFrame(frame))
            {
                // processFrame returns false if pipe write fails and closes pipe
                LOG_WARN("processFrame failed for session " << currentSessionId << ". Pipe closed.");
                currentSessionId = 0;
            }
        }
        else
        {
            // Frame is from a different session, ignore it
            LOG_DEBUG("Discarding frame from non-current session "
                      << frame.clientSessionId << " (Current: " << currentSessionId << ")");
        }
    }

    LOG_INFO("Processor thread stopping.");
    closePipe();
}
