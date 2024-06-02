#ifndef OSD_hpp
#define OSD_hpp

#include <map>
#include <memory>
#include "Config.hpp"
#include <imp/imp_osd.h>
#include <imp/imp_encoder.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#if defined(PLATFORM_T31)
	#define IMPEncoderCHNAttr IMPEncoderChnAttr
	#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

struct OSDItem {
    IMPRgnHandle imp_rgn;
    uint8_t *data;
};

class OSD {
    public:
        //OSD(std::shared_ptr<CFG> &_cfg) : cfg(&_cfg) {}
        bool init(std::shared_ptr<CFG> _cfg);
        bool exit();
        void update();
        void updateDisplayEverySecond();
        
        static void rotateBGRAImage(uint8_t*& inputImage, int& width, int& height, int angle, bool del);
        static void set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, int width = 0, int height = 0);
        static int get_abs_pos(int max, int size, int pos);            
    private:
        std::shared_ptr<CFG> cfg;

        int last_updated_second;

        OSDItem osdTime;   
        OSDItem osdUser;
        OSDItem osdUptm;
        OSDItem osdLogo;
        
        int freetype_init();
        void draw_glyph(uint8_t *data, FT_BitmapGlyph bmg,
                        int *pen_x, int *pen_y,
                        int item_height, int item_width,
                    uint32_t color);
        void set_text(OSDItem *osdItem, IMPOSDRgnAttr *rgnAttr, std::string text, int posX, int posY, int angle);

        FT_Library freetype;
        FT_Face fontface;
        FT_Stroker stroker;

        IMPEncoderCHNAttr channelAttributes;

        std::map<char,FT_BitmapGlyph> bitmaps;
        std::map<char,FT_BitmapGlyph> stroke_bitmaps;
        std::map<char,FT_BBox> boxes;
        std::map<char,FT_Vector> advances;

        bool initialized{0};
};

#endif
