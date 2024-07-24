#ifndef RTSP_hpp
#define RTSP_hpp

#include <memory>

#include "MsgChannel.hpp"
#include "Encoder.hpp"
#include "Logger.hpp"

class RTSP {
public:
    RTSP() {};
    void run();

    void set_input_channel(std::shared_ptr<MsgChannel<H264NALUnit>> chn) {
        this->input_chn = chn;
    }
private:
    std::shared_ptr<MsgChannel<H264NALUnit>> input_chn;
};

#endif
