#include <iostream>
#include "IMPServerMediaSubsession.hpp"
#include "IMPDeviceSource.hpp"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamDiscreteFramer.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "GroupsockHelper.hh"
#include "Config.hpp"

// Modify method to accept pointers for the NAL units
IMPServerMediaSubsession* IMPServerMediaSubsession::createNew(
    UsageEnvironment& env,
    H264NALUnit* vps,  // Change to pointer to make optional
    H264NALUnit sps,
    H264NALUnit pps
) {
    // Pass along the pointers; they may be nullptr
    return new IMPServerMediaSubsession(env, vps, sps, pps);
}

// Modify the constructor accordingly
IMPServerMediaSubsession::IMPServerMediaSubsession(
    UsageEnvironment& env,
    H264NALUnit* vps,  // Change to pointer to make optional
    H264NALUnit sps,
    H264NALUnit pps)
    : OnDemandServerMediaSubsession(env, false),
      vps(vps ? new H264NALUnit(*vps) : nullptr),  // Copy if not nullptr
      sps(sps), pps(pps) {
}

// Destructor - we should delete the VPS if it was allocated
IMPServerMediaSubsession::~IMPServerMediaSubsession() {
    delete vps;  // Safe to delete nullptr if vps is not set
}

FramedSource* IMPServerMediaSubsession::createNewStreamSource(
    unsigned clientSessionId,
    unsigned& estBitrate
) {
    LOG_DEBUG("Create Stream Source.");
    estBitrate = Config::singleton()->rtspEstBitRate; // The expected bitrate?

    IMPDeviceSource* imp = IMPDeviceSource::createNew(envir());
    // Here we need to decide based on the format whether to use H264 or H265 framer
    if (Config::singleton()->stream0format == "H265") {
        return H265VideoStreamDiscreteFramer::createNew(envir(), imp, false, false);
    } else { // Lets assume the default is H264 if not H265
        return H264VideoStreamDiscreteFramer::createNew(envir(), imp, false, false);
    }
}

// Modify RTP Sink creation to conditionally include VPS
RTPSink* IMPServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* fs
) {
    increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), Config::singleton()->rtspSendBufferSize);
    // Use VPS only if it's available (non-nullptr and we are in H265 mode)
    if (Config::singleton()->stream0format == "H265" && vps) {
        return H265VideoRTPSink::createNew(
            envir(),
            rtpGroupsock,
            rtpPayloadTypeIfDynamic,
            &vps->data[0], vps->data.size(),  // Now using pointer, check and dereference
            &sps.data[0], sps.data.size(),
            &pps.data[0], pps.data.size()
        );
    } else {
        // For H264 or other formats, VPS is not used
        return H264VideoRTPSink::createNew(
            envir(),
            rtpGroupsock,
            rtpPayloadTypeIfDynamic,
            &sps.data[0], sps.data.size(),
            &pps.data[0], pps.data.size()
        );
    }
}
