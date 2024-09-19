#ifndef AAC_SINK_HPP
#define AAC_SINK_HPP

#include <liveMedia.hh>

class AACSink : public MPEG4GenericRTPSink {
public:
    static AACSink* createNew(UsageEnvironment& env, Groupsock* RTPgs,
                              u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
                              unsigned numChannels);

protected:
    AACSink(UsageEnvironment& env, Groupsock* RTPgs,
            u_int8_t rtpPayloadFormat, u_int32_t rtpTimestampFrequency,
            unsigned numChannels);
    virtual ~AACSink();

private:
    char* config;
};

#endif // AAC_SINK_HPP
