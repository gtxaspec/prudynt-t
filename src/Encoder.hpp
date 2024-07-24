#ifndef Encoder_hpp
#define Encoder_hpp

#include <array>
#include <memory>
#include <ctime>
#include <map>
#include <functional>

#include <sys/file.h>

#include <imp/imp_framesource.h>
#include <imp/imp_system.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include <imp/imp_osd.h>

#include "MsgChannel.hpp"
#include "Logger.hpp"
#include "OSD.hpp"


static const std::array<int, 64> jpeg_chroma_quantizer = {{
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
}};

static const std::array<int, 64> jpeg_luma_quantizer = {{
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
}};

struct H264NALUnit {
    std::vector<uint8_t> data;
    struct timeval time;
    int64_t imp_ts;
    int64_t duration;
};

using CallbackFunction = std::function<void(void)>;

class Encoder {
public:
    Encoder();
    bool init();
    void run();
    void jpeg_snap();

    static void flush() {
        IMP_Encoder_RequestIDR(0);
        IMP_Encoder_FlushStream(0);
    }

    void set_output_channel(std::shared_ptr<MsgChannel<H264NALUnit>> chn) {
        LOG_DEBUG("Encoder::set_output_channel " << chn.use_count());
        this->output_chn = chn;
        Encoder::flush();
    }

    void set_data_callback(CallbackFunction cb) {
        this->onDataCallback = cb;
    }

private:
    OSD osd;

    int system_init();
    int framesource_init();
    int encoder_init();
    struct timeval imp_time_base;
public:
    std::shared_ptr<MsgChannel<H264NALUnit>> output_chn = nullptr;
    bool IDR = false;
    CallbackFunction onDataCallback = [](){};
};

#endif
