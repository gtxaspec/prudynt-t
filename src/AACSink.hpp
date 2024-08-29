#ifndef AAC_SINK_HPP
#define AAC_SINK_HPP

#include <liveMedia.hh>

class AACSink : public MPEG4GenericRTPSink {
public:
    static AACSink* createNew(UsageEnvironment& env, Groupsock* RTPgs,
                              u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
                              unsigned samplingFrequency, unsigned numChannels);

protected:
    AACSink(UsageEnvironment& env, Groupsock* RTPgs,
            u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
            unsigned samplingFrequency, unsigned numChannels);
    virtual ~AACSink();
};

#endif // AAC_SINK_HPP
