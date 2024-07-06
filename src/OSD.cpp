#include <iostream>
#include <vector>
#include "OSD.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <fstream>
#include <memory>
#include "schrift.h"

int OSD::schrift_init() {
	// Load the font file into memory
	std::ifstream fontFile(Config::singleton()->OSDFontPath, std::ios::binary | std::ios::ate);
	if (!fontFile.is_open()) {
		return -1;
	}

	size_t fileSize = fontFile.tellg();
	fontFile.seekg(0, std::ios::beg);
	fontData.resize(fileSize);
	fontFile.read(reinterpret_cast<char*>(fontData.data()), fileSize);
	fontFile.close();

	font = sft_loadmem(fontData.data(), fontData.size());
	if (!font) {
		return -1;
	}

	sft.font = font;
	sft.xScale = 32.0;
	sft.yScale = 32.0;
	sft.xOffset = 0.0;
	sft.yOffset = 0.0;
	sft.flags = SFT_DOWNWARD_Y;

	return 0;
}

void OSD::draw_glyph(OSDTextItem *ti, SFT_Image &glyphImage, int *pen_x, int *pen_y, int item_height, int item_width, uint32_t color) {
	int rel_pen_x = *pen_x;
	int rel_pen_y = *pen_y + glyphImage.height;
	for (int row = 0; row < glyphImage.height; ++row) {
		rel_pen_x = *pen_x;
		for (int col = 0; col < glyphImage.width; ++col, ++rel_pen_x) {
			if (rel_pen_x < 0 || rel_pen_y < 0 || rel_pen_x >= item_width || rel_pen_y >= item_height)
				continue;

			int data_index = (item_height - rel_pen_y) * item_width * 4 + rel_pen_x * 4;
			if (data_index + 3 >= item_height * item_width * 4)
				continue;

			uint8_t alpha_a = ((uint8_t*)glyphImage.pixels)[row * glyphImage.width + col];
			if (alpha_a == 0) {
				continue;
			}

			uint8_t red = (color >> 16) & 0xFF;
			uint8_t green = (color >> 8) & 0xFF;
			uint8_t blue = (color >> 0) & 0xFF;
			uint8_t alpha = alpha_a;

			ti->data[data_index + 0] = blue;
			ti->data[data_index + 1] = green;
			ti->data[data_index + 2] = red;
			ti->data[data_index + 3] = alpha;
		}
		--rel_pen_y;
	}
}

void OSD::draw_outline_glyph(OSDTextItem *ti, SFT_Glyph glyph, int pen_x, int pen_y, int item_height, int item_width, uint32_t stroke_color) {
	// Render the outline by scaling the glyph slightly larger
	SFT sft_temp = sft;
	sft_temp.xScale += outline_thickness; // Use outline_thickness for adjustable outline
	sft_temp.yScale += outline_thickness;

	SFT_Image outlineImage;
	SFT_GMetrics metrics;
	sft_gmetrics(&sft_temp, glyph, &metrics);
	outlineImage.width = metrics.minWidth;
	outlineImage.height = metrics.minHeight;
	outlineImage.pixels = calloc(1, outlineImage.width * outlineImage.height);

	if (sft_render(&sft_temp, glyph, outlineImage) == 0) {
		draw_glyph(ti, outlineImage, &pen_x, &pen_y, item_height, item_width, stroke_color);
	}
	free(outlineImage.pixels);
}

void OSD::draw_glyph_with_outline(OSDTextItem *ti, SFT_Glyph glyph, int *pen_x, int *pen_y, int item_height, int item_width, uint32_t color, uint32_t stroke_color) {
	// Store original pen positions
	int original_pen_x = *pen_x;
	int original_pen_y = *pen_y;

	// Draw the outline glyph
	draw_outline_glyph(ti, glyph, original_pen_x, original_pen_y, item_height, item_width, stroke_color);

	// Draw the filled glyph at the same position
	SFT_Image filledImage;
	SFT_GMetrics metrics;
	sft_gmetrics(&sft, glyph, &metrics);
	filledImage.width = metrics.minWidth;
	filledImage.height = metrics.minHeight;
	filledImage.pixels = calloc(1, filledImage.width * filledImage.height);

	if (sft_render(&sft, glyph, filledImage) == 0) {
		draw_glyph(ti, filledImage, pen_x, pen_y, item_height, item_width, color);
	}
	free(filledImage.pixels);

	// Update pen position
	*pen_x = original_pen_x + metrics.minWidth;
}

void OSD::set_text(OSDTextItem *ti, std::string text) {
	ti->text = text;

	// First, calculate the size of the bitmap surface we need
	int total_width = 0;
	int max_height = 0;
	std::vector<SFT_Glyph> glyphs;
	std::vector<SFT_GMetrics> metrics_list;
	for (char c : text) {
		SFT_Glyph glyph;

		if (sft_lookup(&sft, c, &glyph) == 0 && glyph != 0) {
			SFT_GMetrics metrics;
			sft_gmetrics(&sft, glyph, &metrics);
			glyphs.push_back(glyph);
			metrics_list.push_back(metrics);
			total_width += metrics.minWidth;
			max_height = std::max(max_height, metrics.minHeight);
		}
	}

	int item_height = max_height;
	int item_width = total_width;

	// OSD Quirk: The item width must be divisible by 2.
	// If not, the OSD item shifts rapidly side-to-side.
	// Bad timings?
	if (item_width % 2 != 0)
		++item_width;

	ti->imp_attr.rect.p1.x = ti->imp_attr.rect.p0.x + item_width - 1;
	ti->imp_attr.rect.p1.y = ti->imp_attr.rect.p0.y + item_height - 1;

	free(ti->data);
	ti->data = (uint8_t*)malloc(item_width * item_height * 4);
	ti->imp_attr.data.picData.pData = ti->data;
	memset(ti->data, 0, item_width * item_height * 4);

	// Then, render the text
	int pen_x = 0;
	int pen_y = 0;
	for (size_t i = 0; i < text.length(); ++i) {
		draw_glyph_with_outline(ti, glyphs[i], &pen_x, &pen_y, item_height, item_width, Config::singleton()->OSDFontColor, Config::singleton()->OSDFontStrokeColor);
	}
}

void OSD::set_outline_thickness(double thickness) {
	outline_thickness = thickness;
}

std::unique_ptr<unsigned char[]> loadBGRAImage(const std::string& filepath, size_t& length) {
	std::ifstream file(filepath, std::ios::binary | std::ios::ate); // Open file at the end to get the size
	if (!file.is_open()) {
		LOG_ERROR("Failed to open the OSD logo file.");
		return nullptr;
	}

	length = file.tellg(); // Get file size
	file.seekg(0, std::ios::beg); // Seek back to start of file

	// Allocate memory for the image
	auto data = std::make_unique<unsigned char[]>(length);

	// Read the image data
	if (!file.read(reinterpret_cast<char*>(data.get()), length)) {
		LOG_ERROR("Failed to read OSD logo data.");
		return nullptr; // Memory is automatically freed if allocation was successful
	}

	return data;
}

bool OSD::init() {
	int ret;
	LOG_DEBUG("OSD init begin");

	if (schrift_init()) {
		LOG_DEBUG("libschrift init failed.");
		return true;
	}

    set_outline_thickness(10.0); // Adjust the outline thickness as needed, doesn't scale right yet.

	ret = IMP_OSD_CreateGroup(0);
	if (ret < 0) {
		LOG_DEBUG("IMP_OSD_CreateGroup() == " + std::to_string(ret));
		return true;
	}
	LOG_DEBUG("IMP_OSD_CreateGroup group 0 created");

	memset(&timestamp.imp_rgn, 0, sizeof(timestamp.imp_rgn));
	timestamp.imp_rgn = IMP_OSD_CreateRgn(NULL);
	IMP_OSD_RegisterRgn(timestamp.imp_rgn, 0, NULL);
	timestamp.imp_attr.type = OSD_REG_PIC;
	timestamp.imp_attr.rect.p0.x = Config::singleton()->stream0osdPosTimeX;
	timestamp.imp_attr.rect.p0.y = Config::singleton()->stream0osdPosTimeY;
	timestamp.imp_attr.fmt = PIX_FMT_BGRA;
	timestamp.imp_attr.data.picData.pData = timestamp.data;

	memset(&timestamp.imp_grp_attr, 0, sizeof(timestamp.imp_grp_attr));
	IMP_OSD_SetRgnAttr(timestamp.imp_rgn, &timestamp.imp_attr);
	IMP_OSD_GetGrpRgnAttr(timestamp.imp_rgn, 0, &timestamp.imp_grp_attr);
	timestamp.imp_grp_attr.show = 1;
	timestamp.imp_grp_attr.layer = 1;
	timestamp.imp_grp_attr.scalex = 0;
	timestamp.imp_grp_attr.scaley = 0;
	IMP_OSD_SetGrpRgnAttr(timestamp.imp_rgn, 0, &timestamp.imp_grp_attr);

	if (Config::singleton()->OSDUserTextEnable) {
		int userTextPosX = (Config::singleton()->stream0osdUserTextPosX == 0) ?
					Config::singleton()->stream0width / 2 :
					Config::singleton()->stream0osdUserTextPosX;
		memset(&userText.imp_rgn, 0, sizeof(userText.imp_rgn));
		userText.imp_rgn = IMP_OSD_CreateRgn(NULL);
		IMP_OSD_RegisterRgn(userText.imp_rgn, 0, NULL);
		userText.imp_attr.type = OSD_REG_PIC;
		userText.imp_attr.rect.p0.x = userTextPosX;
		userText.imp_attr.rect.p0.y = Config::singleton()->stream0osdUserTextPosY;
		userText.imp_attr.fmt = PIX_FMT_BGRA;
		userText.data = NULL;
		userText.imp_attr.data.picData.pData = userText.data;
		set_text(&userText, Config::singleton()->OSDUserTextString);

		memset(&userText.imp_grp_attr, 0, sizeof(userText.imp_grp_attr));
		IMP_OSD_SetRgnAttr(userText.imp_rgn, &userText.imp_attr);
		IMP_OSD_GetGrpRgnAttr(userText.imp_rgn, 0, &userText.imp_grp_attr);
		userText.imp_grp_attr.show = 1;
		userText.imp_grp_attr.layer = 2;
		userText.imp_grp_attr.scalex = 1;
		userText.imp_grp_attr.scaley = 1;
		IMP_OSD_SetGrpRgnAttr(userText.imp_rgn, 0, &userText.imp_grp_attr);
	}

	if (Config::singleton()->OSDUptimeEnable) {
		int uptimePosX = (Config::singleton()->stream0osdUptimeStampPosX == 0) ?
					Config::singleton()->stream0width - 240 :
					Config::singleton()->stream0osdUptimeStampPosX;
		memset(&uptimeStamp.imp_rgn, 0, sizeof(uptimeStamp.imp_rgn));
		uptimeStamp.imp_rgn = IMP_OSD_CreateRgn(NULL);
		IMP_OSD_RegisterRgn(uptimeStamp.imp_rgn, 0, NULL);
		uptimeStamp.imp_attr.type = OSD_REG_PIC;
		uptimeStamp.imp_attr.rect.p0.x = uptimePosX;
		uptimeStamp.imp_attr.rect.p0.y = Config::singleton()->stream0osdUptimeStampPosY;
		uptimeStamp.imp_attr.fmt = PIX_FMT_BGRA;
		uptimeStamp.data = NULL;
		uptimeStamp.imp_attr.data.picData.pData = uptimeStamp.data;

		memset(&uptimeStamp.imp_grp_attr, 0, sizeof(uptimeStamp.imp_grp_attr));
		IMP_OSD_SetRgnAttr(uptimeStamp.imp_rgn, &uptimeStamp.imp_attr);
		IMP_OSD_GetGrpRgnAttr(uptimeStamp.imp_rgn, 0, &uptimeStamp.imp_grp_attr);
		uptimeStamp.imp_grp_attr.show = 1;
		uptimeStamp.imp_grp_attr.layer = 3;
		uptimeStamp.imp_grp_attr.scalex = 1;
		uptimeStamp.imp_grp_attr.scaley = 1;
		IMP_OSD_SetGrpRgnAttr(uptimeStamp.imp_rgn, 0, &uptimeStamp.imp_grp_attr);
	}

	if (Config::singleton()->OSDLogoEnable) {
		size_t imageSize;
		auto imageData = loadBGRAImage(Config::singleton()->OSDLogoPath.c_str(), imageSize);

		int OSDLogoX = (Config::singleton()->stream0osdLogoPosX == 0) ?
						(Config::singleton()->stream0width - Config::singleton()->OSDLogoWidth - 20) :
						Config::singleton()->stream0osdLogoPosX;

		int OSDLogoY = (Config::singleton()->stream0osdLogoPosY == 0) ?
						(Config::singleton()->stream0height - Config::singleton()->OSDLogoHeight - 10):
						Config::singleton()->stream0osdLogoPosY;

		IMPOSDRgnAttr OSDLogo;
		memset(&OSDLogo, 0, sizeof(OSDLogo));
		OSDLogo.type = OSD_REG_PIC;
		int picw = Config::singleton()->OSDLogoWidth;
		int pich = Config::singleton()->OSDLogoHeight;
		OSDLogo.rect.p0.x = OSDLogoX;
		OSDLogo.rect.p0.y = OSDLogoY;
		OSDLogo.rect.p1.x = OSDLogo.rect.p0.x + picw - 1;
		OSDLogo.rect.p1.y = OSDLogo.rect.p0.y + pich - 1;
		OSDLogo.fmt = PIX_FMT_BGRA;
		OSDLogo.data.picData.pData = imageData.get();

		IMPRgnHandle handle = IMP_OSD_CreateRgn(NULL);
		IMP_OSD_SetRgnAttr(handle, &OSDLogo);
		IMPOSDGrpRgnAttr OSDLogoGroup;
		memset(&OSDLogoGroup, 0, sizeof(OSDLogoGroup));
		OSDLogoGroup.show = 1;
		OSDLogoGroup.layer = 4;
		OSDLogoGroup.scalex = 1;
		OSDLogoGroup.scaley = 1;
		OSDLogoGroup.gAlphaEn = 1;
		OSDLogoGroup.fgAlhpa = Config::singleton()->stream0osdLogoAlpha;

		IMP_OSD_RegisterRgn(handle, 0, &OSDLogoGroup);
	}

	ret = IMP_OSD_Start(0);
	if (ret < 0) {
		LOG_DEBUG("IMP_OSD_Start() == " + std::to_string(ret));
		return true;
	}
	LOG_DEBUG("IMP_OSD_Start succeed");

	LOG_DEBUG("OSD init completed");
	return false;
}

unsigned long getSystemUptime() {
	std::ifstream uptimeFile("/proc/uptime");
	std::string line;
	if (std::getline(uptimeFile, line)) {
		std::istringstream iss(line);
		double uptimeSeconds;
		if (iss >> uptimeSeconds) {
			// Uptime in seconds
			return static_cast<unsigned long>(uptimeSeconds);
		}
	}
	return 0; // Default to 0 if unable to read uptime
}

// Var to keep track of the last second updated
static int last_updated_second = -1;

void OSD::updateDisplayEverySecond() {
	time_t current = time(NULL);
	struct tm *ltime = localtime(&current);

	// Check if we have moved to a new second
	if (ltime->tm_sec != last_updated_second) {
		last_updated_second = ltime->tm_sec;  // Update the last second tracker

		// Now we ensure both updates are performed as close together as possible
		// Format and update system time
		if (Config::singleton()->OSDTimeEnable) {
			char timeFormatted[256];
			strftime(timeFormatted, sizeof(timeFormatted), Config::singleton()->OSDFormat.c_str(), ltime);
			set_text(&timestamp, std::string(timeFormatted));
			IMP_OSD_SetRgnAttr(timestamp.imp_rgn, &timestamp.imp_attr);
		}

		// Calculate and update system uptime
		if (Config::singleton()->OSDUptimeEnable) {
			unsigned long currentUptime = getSystemUptime();
			unsigned long hours = currentUptime / 3600;
			unsigned long minutes = (currentUptime % 3600) / 60;
			unsigned long seconds = currentUptime % 60;
			char uptimeFormatted[256];
			snprintf(uptimeFormatted, sizeof(uptimeFormatted), Config::singleton()->OSDUptimeFormat.c_str(), hours, minutes, seconds);
			set_text(&uptimeStamp, std::string(uptimeFormatted));
			IMP_OSD_SetRgnAttr(uptimeStamp.imp_rgn, &uptimeStamp.imp_attr);
		}
	}
}

void OSD::update() {
	// Called every frame by the encoder.
	updateDisplayEverySecond();
}
