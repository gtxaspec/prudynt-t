#include "Config.hpp"
#include "Logger.hpp"
#include "Opus.hpp"
#include <atomic>
#include "RTSPStatus.hpp"

namespace { std::atomic<uint32_t> g_opusMismatches{0}; }

Opus* Opus::createNew(int sampleRate, int numChn)
{
    return new Opus(sampleRate, numChn);
}

Opus::~Opus()
{
    close();
}

int Opus::open()
{
    int opusError;

    encoder = opus_encoder_create(sampleRate, numChn, OPUS_APPLICATION_AUDIO, &opusError);
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to create Opus encoder: " << opus_strerror(opusError));
        return -1;
    }

    // Configure encoder for maximum quality at the configured bitrate
    int bitrate = cfg->audio.input_bitrate * 1000; // bps
    opusError = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to set bitrate (" << bitrate << ") for Opus encoder: " << opus_strerror(opusError));
    }

    // Highest complexity for quality (CPU usage is negligible at 1ch/48k on this SoC)
    opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
    // Make VBR explicit (better quality at target rate)
    opus_encoder_ctl(encoder, OPUS_SET_VBR(1));
    // Hint fullband capability
    opus_encoder_ctl(encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
    // Content is typically music/ambience on cams; this helps tuning psychoacoustics
    opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));

    opusError = opus_encoder_ctl(encoder, OPUS_GET_BITRATE(&bitrate));
    if (opusError != OPUS_OK)
    {
        LOG_ERROR("Failed to get bitrate from Opus encoder: " << opus_strerror(opusError));
        return -1;
    }

    LOG_INFO("Encoder bitrate: " << bitrate);

    return 0;
}

int Opus::close()
{
    if (encoder)
    {
        opus_encoder_destroy(encoder);
    }
    encoder = nullptr;
    return 0;
}

int Opus::encode(IMPAudioFrame *data, unsigned char *outbuf, int *outLen)
{
    // Calculate samples per channel
    int samples_per_channel = (data->len / sizeof(int16_t)) / numChn;

    // Debug: Log frame details for first few frames
    static int frame_count = 0;
    if (frame_count < 10) {
        LOG_DEBUG("Opus encode frame " << frame_count << ": len=" << data->len
                 << " bytes, samples_per_ch=" << samples_per_channel
                 << " (20ms at " << sampleRate << "Hz)");
        frame_count++;
    }

    // Calculate expected samples for 20ms at current sample rate
    int expected_samples = sampleRate * 0.020;
    if (samples_per_channel != expected_samples) {
        uint64_t cnt = ++g_opusMismatches;
        // Expose metric (single audio channel assumed as audio0)
        RTSPStatus::writeCustomParameter("audio0", "opus_mismatch_count", std::to_string(cnt));

        if (samples_per_channel < expected_samples) {
            if (cnt <= 10 || (cnt % 100) == 0) {
                LOG_WARN("Opus underfilled frame: got " << samples_per_channel
                         << ", expected " << expected_samples
                         << " (20ms@" << sampleRate << "Hz) — dropping to preserve framing");
            }
            return -1; // let upstream accumulate more
        } else {
            if (cnt <= 10 || (cnt % 100) == 0) {
                LOG_WARN("Opus oversized frame: got " << samples_per_channel
                         << ", expected " << expected_samples
                         << " (20ms@" << sampleRate << "Hz) — dropping unexpected size");
            }
            return -1; // unexpected; do not encode mis-sized frame
        }
    }

    opus_int32 bytesEncoded = opus_encode(
        encoder,
        reinterpret_cast<const opus_int16*>(data->virAddr),
        samples_per_channel,
        reinterpret_cast<unsigned char*>(outbuf),
        1024);

    if (bytesEncoded < 0)
    {
        LOG_WARN("Encoding failed with error code: " << bytesEncoded);
        return -1;
    }

    *outLen = bytesEncoded;

    return 0;
}
