#ifndef IMPEncoder_hpp
#define IMPEncoder_hpp

#include <array>
#include "Logger.hpp"
#include "Config.hpp"
#include "OSD.hpp"
#include <imp/imp_encoder.h>
#include <imp/imp_system.h>

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

static const std::array<int, 64> jpeg_chroma_quantizer = {{17, 18, 24, 47, 99, 99, 99, 99,
                                                           18, 21, 26, 66, 99, 99, 99, 99,
                                                           24, 26, 56, 99, 99, 99, 99, 99,
                                                           47, 66, 99, 99, 99, 99, 99, 99,
                                                           99, 99, 99, 99, 99, 99, 99, 99,
                                                           99, 99, 99, 99, 99, 99, 99, 99,
                                                           99, 99, 99, 99, 99, 99, 99, 99,
                                                           99, 99, 99, 99, 99, 99, 99, 99}};

static const std::array<int, 64> jpeg_luma_quantizer = {{16, 11, 10, 16, 24, 40, 51, 61,
                                                         12, 12, 14, 19, 26, 58, 60, 55,
                                                         14, 13, 16, 24, 40, 57, 69, 56,
                                                         14, 17, 22, 29, 51, 87, 80, 62,
                                                         18, 22, 37, 56, 68, 109, 103, 77,
                                                         24, 35, 55, 64, 81, 104, 113, 92,
                                                         49, 64, 78, 87, 103, 121, 120, 101,
                                                         72, 92, 95, 98, 112, 100, 103, 99}};

class IMPEncoder
{
public:
    static IMPEncoder *createNew(_stream *stream, std::shared_ptr<CFG> cfg, int encChn, int encGrp, const char *name);

    IMPEncoder(_stream *stream, std::shared_ptr<CFG> cfg, int encChn, int encGrp, const char *name) : stream(stream), cfg(cfg), encChn(encChn), encGrp(encGrp), name(name)
    {
        init();
    }

    ~IMPEncoder(){
        destroy();
    };

    int init();
    int deinit();
    int destroy();

    OSD *osd = nullptr;

private:
    
    std::shared_ptr<CFG> cfg;
    const char *name;

    IMPEncoderCHNAttr chnAttr;
    void initProfile();

    IMPCell fs = {DEV_ID_FS, encGrp, 0};
    IMPCell enc = {DEV_ID_ENC, encGrp, 0};
    IMPCell osd_cell = {DEV_ID_OSD, encGrp, 0};

    _stream *stream;
    int encChn;
    int encGrp;
};

#endif