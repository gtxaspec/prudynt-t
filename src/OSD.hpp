#ifndef OSD_hpp
#define OSD_hpp

#include <map>

#include <imp/imp_osd.h>
#include <imp/imp_encoder.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#if defined(PLATFORM_T31)
	#define IMPEncoderCHNAttr IMPEncoderChnAttr
	#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

struct OSDTextItem {
    IMPRgnHandle imp_rgn;
    IMPOSDRgnAttr imp_attr;
    IMPOSDGrpRgnAttr imp_grp_attr;
    std::string text;
    uint8_t *data;
    int point_size;
    int stroke;
    uint32_t color;
    uint32_t stroke_color;
    uint8_t update_intervall;  //update intervall in seconds
    uint32_t last_update;      //second of the day (last update)
    int x;
    int y;
};

class OSD {
public:
    bool init();
    bool exit();
    void update();
    void updateDisplayEverySecond();
private:
    int freetype_init();
    void draw_glyph(OSDTextItem *ti, FT_BitmapGlyph bmg,
                     int *pen_x, int *pen_y,
                    int item_height, int item_width,
                   uint32_t color);
    void set_text(OSDTextItem *ti, std::string text);
    void set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, int width, int height);
    int get_abs_pos(int max, int size, int pos);

    FT_Library freetype;
    FT_Face fontface;
    FT_Stroker stroker;

    OSDTextItem timestamp;
    OSDTextItem userText;
    OSDTextItem uptimeStamp;

    IMPOSDRgnAttr OSDLogo;
    IMPRgnHandle OSDLogoHandle;

    IMPEncoderCHNAttr channelAttributes;

    std::map<char,FT_BitmapGlyph> bitmaps;
    std::map<char,FT_BitmapGlyph> stroke_bitmaps;
    std::map<char,FT_BBox> boxes;
    std::map<char,FT_Vector> advances;
};

#endif
