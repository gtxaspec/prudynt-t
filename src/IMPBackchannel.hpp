#ifndef IMP_BACKCHANNEL_HPP
#define IMP_BACKCHANNEL_HPP

/*
 *  Backchannel Audio Pipeline: Architecture Overview
 *
 *  This module manages backchannel audio, enabling two-way audio
 *  communication from an RTSP client. It involves setting up audio
 *  channels with the IMP audio SDK for decoding received audio.
 *
 *  Key aspects of the pipeline:
 *   1. RTSP Setup and SDP: The RTSP server creates a
 *      BackchannelServerMediaSubsession for each supported audio format.
 *      The SDP (Session Description Protocol) defines the audio stream
 *      parameters (e.g., codec, sample rate) for the client.
 *   2. Audio Reception and Timeout: The BackchannelSink receives
 *      encoded audio data from the RTSP client and implements a timeout
 *      mechanism. If no data is received for a certain period, it sends
 *      a "stop" frame to signal the end of the session.
 *   3. Frame Queueing: The BackchannelSink encapsulates the received data
 *      into BackchannelFrame objects and enqueues them in a global queue
 *      (global_backchannel->inputQueue). These frames can be either
 *      playback frames (containing audio data) or stop frames (with empty
 *      payload, signaling the end of a session).
 *   4. Audio Processing and Session Management: The BackchannelProcessor
 *      dequeues frames from the queue, decodes the audio data using the
 *      IMP audio SDK, and resamples it if necessary. It maintains a
 *      concept of a "current" session, processing only frames from that
 *      session and discarding frames from other sessions.
 *   5. Audio Output: The decoded PCM audio is then sent to a pipe, where
 *      the `/bin/iac` program receives the data and handles the audio
 *      output.
 *
 *  The IMPBackchannel class is responsible for:
 *   - Registering and managing audio decoders (e.g., Opus) with the IMP
 *     audio SDK.
 *   - Creating and destroying the IMP audio channels used for decoding.
 */

#include "Config.hpp"

// Define the list of backchannel formats and their properties
// X(EnumName, NameString, PayloadType, Frequency, MimeType)
#define X_FOREACH_BACKCHANNEL_FORMAT(X) \
    X(AAC, "MPEG4-GENERIC", 97, cfg->audio.output_sample_rate, "audio/mpeg4-generic") \
    X(PCMU, "PCMU", 0, 8000, "audio/PCMU") \
    X(PCMA, "PCMA", 8, 8000, "audio/PCMA") \
    /* Add new formats here */

#define APPLY_ENUM(EnumName, NameString, PayloadType, Frequency, MimeType) EnumName,
enum class IMPBackchannelFormat { UNKNOWN = -1, X_FOREACH_BACKCHANNEL_FORMAT(APPLY_ENUM) };
#undef APPLY_ENUM

class IMPBackchannel
{
public:
    static IMPBackchannel *createNew();
    IMPBackchannel() { init(); }
    ~IMPBackchannel() { deinit(); };
    int init();
    void deinit();

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

    static int getFormatPayloadType(IMPBackchannelFormat format)
    {
#define RETURN_PAYLOADTYPE(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        if (IMPBackchannelFormat::EnumName == format) \
            return PayloadType; \
    }
        X_FOREACH_BACKCHANNEL_FORMAT(RETURN_PAYLOADTYPE)
#undef RETURN_PAYLOADTYPE
        return 96;
    }

    static int getFormatFrequency(IMPBackchannelFormat format)
    {
#define RETURN_FREQUENCY(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        if (IMPBackchannelFormat::EnumName == format) \
            return Frequency; \
    }
        X_FOREACH_BACKCHANNEL_FORMAT(RETURN_FREQUENCY)
#undef RETURN_FREQUENCY
        return 0;
    }

    static const char *getFormatMimeType(IMPBackchannelFormat format)
    {
#define RETURN_MIME_TYPE(EnumName, NameString, PayloadType, Frequency, MimeType) \
    { \
        if (IMPBackchannelFormat::EnumName == format) \
            return MimeType; \
    }
        X_FOREACH_BACKCHANNEL_FORMAT(RETURN_MIME_TYPE)
#undef RETURN_MIME_TYPE
        return "audio/unknown";
    }

private:
    int aacDecoderHandle{-1};
};

#endif // IMP_BACKCHANNEL_HPP
