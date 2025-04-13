#ifndef BACKCHANNEL_PROCESSOR_HPP
#define BACKCHANNEL_PROCESSOR_HPP

// Processes audio frames, decodes them, handles session management (who is
// "current"), resamples, and sends PCM data to a pipe.

#include "IMPBackchannel.hpp"
#include "globals.hpp"

#include <cstdint>
#include <cstdio>

class BackchannelProcessor
{
public:
    BackchannelProcessor();
    ~BackchannelProcessor();

    void run();

private:
    std::vector<int16_t> resampleLinear(const std::vector<int16_t> &input_pcm,
                                        int input_rate,
                                        int output_rate);

    bool initPipe();
    void closePipe();

    bool processFrame(const BackchannelFrame &frame);
    bool decodeFrame(const uint8_t *payload,
                     size_t payloadSize,
                     IMPBackchannelFormat format,
                     std::vector<int16_t> &outPcmBuffer);
    bool writePcmToPipe(const std::vector<int16_t> &pcmBuffer);

    unsigned int currentSessionId;

    FILE *fPipe;
    int fPipeFd;

    BackchannelProcessor(const BackchannelProcessor &) = delete;
    BackchannelProcessor &operator=(const BackchannelProcessor &) = delete;
};

#endif // BACKCHANNEL_PROCESSOR_HPP
