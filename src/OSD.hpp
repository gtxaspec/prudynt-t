#ifndef OSD_hpp
#define OSD_hpp

#include <map>
#include <string>
#include <vector>
#include <imp/imp_osd.h>
#include "schrift.h"

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
};

class OSD {
public:
	bool init();
	void update();
	void updateDisplayEverySecond();
	int schrift_init();
	void draw_glyph(OSDTextItem *ti, SFT_Image &glyphImage, int *pen_x, int *pen_y, int item_height, int item_width, uint32_t color);
	void draw_outline_glyph(OSDTextItem *ti, SFT_Glyph glyph, int pen_x, int pen_y, int item_height, int item_width, uint32_t stroke_color);
	void draw_glyph_with_outline(OSDTextItem *ti, SFT_Glyph glyph, int *pen_x, int *pen_y, int item_height, int item_width, uint32_t color, uint32_t stroke_color);
	void set_text(OSDTextItem *ti, std::string text);

private:
	SFT sft;
	SFT_Font *font;
	std::vector<uint8_t> fontData;

	OSDTextItem timestamp;
	OSDTextItem userText;
	OSDTextItem uptimeStamp;

	std::map<char, SFT_Glyph> glyphs;
	std::map<char, SFT_Glyph> stroke_glyphs;
};

#endif
