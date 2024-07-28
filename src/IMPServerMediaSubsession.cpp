#include <iostream>
#include <memory>
#include "IMPServerMediaSubsession.hpp"
#include "IMPDeviceSource.hpp"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "GroupsockHelper.hh"
#include "Config.hpp"

extern std::shared_ptr<CFG> cfg;

// Modify method to accept pointers for the NAL units
IMPServerMediaSubsession *IMPServerMediaSubsession::createNew(
    UsageEnvironment &env,
    H264NALUnit *vps, // Change to pointer to make optional
    H264NALUnit sps,
    H264NALUnit pps,
    int encChn)
{
    // Pass along the pointers; they may be nullptr
    return new IMPServerMediaSubsession(env, vps, sps, pps, encChn);
}

// Modify the constructor accordingly
IMPServerMediaSubsession::IMPServerMediaSubsession(
    UsageEnvironment &env,
    H264NALUnit *vps, // Change to pointer to make optional
    H264NALUnit sps,
    H264NALUnit pps,
    int encChn)
    : OnDemandServerMediaSubsession(env, true),
      vps(vps ? new H264NALUnit(*vps) : nullptr), // Copy if not nullptr
      sps(sps), pps(pps), encChn(encChn)
{
}

// Destructor - we should delete the VPS if it was allocated
IMPServerMediaSubsession::~IMPServerMediaSubsession()
{
    delete vps; // Safe to delete nullptr if vps is not set
}

FramedSource *IMPServerMediaSubsession::createNewStreamSource(
    unsigned clientSessionId,
    unsigned &estBitrate)
{
    LOG_DEBUG("Create Stream Source. ");
    estBitrate = cfg->rtsp.est_bitrate; // The expected bitrate?

    IMPDeviceSource *imp = IMPDeviceSource::createNew(envir(), encChn);
    // Here we need to decide based on the format whether to use H264 or H265 framer
    if (vps)
    {
        return H265VideoStreamDiscreteFramer::createNew(envir(), imp, false, false);
    }
    else
    { // Let's assume the default is H264 if not H265
        return H264VideoStreamDiscreteFramer::createNew(envir(), imp, false, false);
    }
}

// Modify RTP Sink creation to conditionally include VPS
RTPSink *IMPServerMediaSubsession::createNewRTPSink(
    Groupsock *rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource *fs)
{
    increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), cfg->rtsp.send_buffer_size);
    // Use VPS only if it's available (non-nullptr, and we are in H265 mode)
    if (vps)
    {
        return H265VideoRTPSink::createNew(
            envir(),
            rtpGroupsock,
            rtpPayloadTypeIfDynamic,
            &vps->data[0], vps->data.size(), // Now using pointer, check and dereference
            &sps.data[0], sps.data.size(),
            &pps.data[0], pps.data.size());
    }
    else
    {
        // For H264 or other formats, VPS is not used
        return H264VideoRTPSink::createNew(
            envir(),
            rtpGroupsock,
            rtpPayloadTypeIfDynamic,
            &sps.data[0], sps.data.size(),
            &pps.data[0], pps.data.size());
    }

    delete[] fSDPLines; fSDPLines = NULL;
}
