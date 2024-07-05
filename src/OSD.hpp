#ifndef OSD_hpp
#define OSD_hpp

#include <map>
#include <memory>
#include "Config.hpp"
#include <imp/imp_osd.h>
#include <imp/imp_encoder.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_STROKER_H

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

struct OSDItem
{
    IMPRgnHandle imp_rgn;
    uint8_t *data;
    uint16_t width;
    uint16_t height;
    IMPOSDRgnAttrData *rgnAttrData;
};

class OSD
{
public:
    static OSD *createNew(_osd *osd, std::shared_ptr<CFG> cfg, int osdGrp, int encChn, const char *parent);
    OSD(_osd *osd, std::shared_ptr<CFG> cfg, int osdGrp, int encChn, const char *parent) : osd(osd), cfg(cfg), osdGrp(osdGrp), encChn(encChn), parent(parent)
    {
        init();
    }
    void init();
    int exit();
    //void update();
    void* update();
    static void* updateWrapper(void* arg);
    void updateDisplayEverySecond();

    void rotateBGRAImage(uint8_t *&inputImage, uint16_t &width, uint16_t &height, int angle, bool del);
    static void set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, uint16_t width, uint16_t height, const uint16_t max_width, const uint16_t max_height);
    static uint16_t get_abs_pos(const uint16_t max,const uint16_t size,const int pos);

private:

    _osd *osd;
    std::shared_ptr<CFG> cfg;
    int last_updated_second;
    const char *parent;

    OSDItem osdTime{};
    OSDItem osdUser{};
    OSDItem osdUptm{};
    OSDItem osdLogo{};

    int freetype_init();
    void draw_glyph(uint8_t *data, FT_BitmapGlyph bmg,
                    int *pen_x, int *pen_y,
                    int item_height, int item_width,
                    uint32_t color);
    void set_text(OSDItem *osdItem, IMPOSDRgnAttr *rgnAttr, const char *text, int posX, int posY, int angle);
    const char * getConfigPath(const char *itemName);

    FT_Library freetype{};
    FT_Face fontface{};
    FT_Stroker stroker{};

    IMPEncoderCHNAttr channelAttributes;

    std::map<char, FT_BitmapGlyph> bitmaps{};
    std::map<char, FT_BitmapGlyph> stroke_bitmaps{};
    std::map<char, FT_BBox> boxes{};
    std::map<char, FT_Vector> advances{};

    bool initialized{0};
    int osdGrp{};
    int encChn{};

    char hostname[64];
    char ip[INET_ADDRSTRLEN]{};

    uint16_t stream_width;
    uint16_t stream_height;
};

#endif
