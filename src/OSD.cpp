#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include "OSD.hpp"
#include "Encoder.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <unistd.h>
#include <fstream>
#include <memory>

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#if defined(PLATFORM_T31)
	#define IMPEncoderCHNAttr IMPEncoderChnAttr
	#define IMPEncoderCHNStat IMPEncoderChnStat    
#endif

#if defined(PLATFORM_T31)
	#define picWidth uWidth
	#define picHeight uHeight 
#endif

void OSD::rotateBGRAImage(uint8_t*& inputImage, int& width, int& height, int angle, bool del = true) {

    double angleRad = angle * (M_PI / 180.0);

    int originalCorners[4][2] = {
        {0, 0},
        {width, 0},
        {0, height},
        {width, height}
    };

    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;

    for (int i = 0; i < 4; ++i) {
        int x = originalCorners[i][0];
        int y = originalCorners[i][1];
        
        int newX = static_cast<int>(x * std::cos(angleRad) - y * std::sin(angleRad));
        int newY = static_cast<int>(x * std::sin(angleRad) + y * std::cos(angleRad));

        if (newX < minX) minX = newX;
        if (newX > maxX) maxX = newX;
        if (newY < minY) minY = newY;
        if (newY > maxY) maxY = newY;
    }

    int newWidth = maxX - minX + 1;
    int newHeight = maxY - minY + 1;

    int centerX = width / 2;
    int centerY = height / 2;

    int newCenterX = newWidth / 2;
    int newCenterY = newHeight / 2;

    uint8_t* rotatedImage = new uint8_t[newWidth * newHeight * 4]();

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int newX = x - newCenterX;
            int newY = y - newCenterY;

            int origX = static_cast<int>(newX * std::cos(angleRad) + newY * std::sin(angleRad)) + centerX;
            int origY = static_cast<int>(-newX * std::sin(angleRad) + newY * std::cos(angleRad)) + centerY;

            if (origX >= 0 && origX < width && origY >= 0 && origY < height) {
                for (int c = 0; c < 4; ++c) {
                    rotatedImage[(y * newWidth + x) * 4 + c] = inputImage[(origY * width + origX) * 4 + c];
                }
            }
        }
    }

    if (del) delete[] inputImage;
    inputImage = rotatedImage;
    width = newWidth;
    height = newHeight;
}

int OSD::get_abs_pos(int max, int size, int pos) {

    if(pos==0) return max / 2 - size / 2;
    if(pos<0) return max - size + pos;
    return pos;
}

void OSD::set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, int width, int height, int encChn) {
    //picWidth, picHeight cpp macro !!
    IMPEncoderCHNAttr chnAttr;
    IMP_Encoder_GetChnAttr(encChn, &chnAttr);

    if(width == 0 || height == 0) {
        width = rgnAttr->rect.p1.x - rgnAttr->rect.p0.x + 1;
        height = rgnAttr->rect.p1.y - rgnAttr->rect.p0.y + 1;
    }

    if(x>chnAttr.encAttr.picWidth-width)
        x=chnAttr.encAttr.picWidth-width;
    
    if(y>chnAttr.encAttr.picHeight-height)
        y=chnAttr.encAttr.picHeight-height;

    rgnAttr->rect.p0.x = get_abs_pos(chnAttr.encAttr.picWidth, width, x);
    rgnAttr->rect.p0.y = get_abs_pos(chnAttr.encAttr.picHeight, height, y);
    rgnAttr->rect.p1.x = rgnAttr->rect.p0.x + width - 1;
    rgnAttr->rect.p1.y = rgnAttr->rect.p0.y + height - 1;     
}

void set_glyph_transform(FT_Face fontface, int angle) {
    // Normalize the angle to [0, 360)
    angle %= 360;
    // Initialize transformation matrix
    FT_Matrix matrix;

    switch (angle) {
        case 0:
            // Identity transformation (no rotation)
            matrix.xx = 1 * (1 << 16);  // Fixed-point 1.0
            matrix.xy = 0;
            matrix.yx = 0;
            matrix.yy = 1 * (1 << 16);  // Fixed-point 1.0
            break;
        case 90:
            // 90 degrees clockwise rotation
            matrix.xx = 0;
            matrix.xy = -1 * (1 << 16);  // Fixed-point -1.0
            matrix.yx = 1 * (1 << 16);   // Fixed-point 1.0
            matrix.yy = 0;
            break;
        case 180:
            // 180 degrees rotation
            // Segmentation Fault
            matrix.xx = -1 * (1 << 16);  // Fixed-point -1.0
            matrix.xy = 0;
            matrix.yx = 0;
            matrix.yy = -1 * (1 << 16);  // Fixed-point -1.0
            break;
        case 270:
            // 270 degrees clockwise (or 90 degrees counter-clockwise) rotation
            matrix.xx = 0;
            matrix.xy = 1 * (1 << 16);   // Fixed-point 1.0
            matrix.yx = -1 * (1 << 16);  // Fixed-point -1.0
            matrix.yy = 0;
            break;
        default:
            // For angles not at 0, 90, 180, or 270 degrees, no transformation is applied
            std::cerr << "Angle must be 0, 90, 180, or 270 degrees." << std::endl;
            return;  // Exit the function if an unsupported angle is provided
    }

    // Apply the transformation to the font face
    FT_Set_Transform(fontface, &matrix, nullptr);
}

int OSD::freetype_init() {
    int error;

    error = FT_Init_FreeType(&freetype);
    if (error) {
        return error;
    }
    
    error = FT_New_Face(
        freetype,
        osd->font_path.c_str(),
        0,
        &fontface
    );
    if (error) {
        return error;
    }

   #if 0
        FT_Matrix matrix;  // Transformation matrix
        matrix.xx = 0;
        matrix.xy = -1 * (1 << 16);  // Fixed-point -1.0
        matrix.yx = 1 * (1 << 16);   // Fixed-point 1.0
        matrix.yy = 0;
        FT_Set_Transform(fontface, &matrix, nullptr);
    #endif

    FT_Stroker_New(freetype, &stroker);

    FT_Set_Char_Size(
        fontface,
        0,
        72*64,
        96,
        96
    );

    FT_Stroker_Set(
        stroker,
        osd->font_stroke_size,
        FT_STROKER_LINECAP_SQUARE,
        FT_STROKER_LINEJOIN_ROUND,
        0
    );
    FT_Set_Char_Size(fontface, 0, 32*64, osd->font_size , osd->font_size);

    //Prerender glyphs needed for displaying date & time.
    std::string prerender_list;

    if (osd->user_text_enabled) {
        prerender_list = "0123456789 /:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()-_=+[]{}|;:'\",.<>?`~";
    } else {
        prerender_list = "0123456789 /APM:";
    }

    // set_glyph_transform(fontface, 90);  // Rotate text by 90 degrees

    for (unsigned int i = 0; i < prerender_list.length(); ++i) {
        FT_UInt gindex = FT_Get_Char_Index(fontface, prerender_list[i]);
        FT_Load_Glyph(fontface, gindex, FT_LOAD_DEFAULT);
        FT_Glyph glyph;
        FT_Get_Glyph(fontface->glyph, &glyph);
        FT_Glyph stroked;
        FT_Get_Glyph(fontface->glyph, &stroked);

        FT_Glyph_StrokeBorder(&stroked, stroker, false, true);

        FT_BBox bbox;
        FT_Glyph_Get_CBox(stroked, FT_GLYPH_BBOX_PIXELS, &bbox);
        boxes[prerender_list[i]] = bbox;
        advances[prerender_list[i]] = stroked->advance;

        FT_Glyph_To_Bitmap(&stroked, FT_RENDER_MODE_NORMAL, NULL, true);
        FT_BitmapGlyph strokeBMG = (FT_BitmapGlyph)stroked;
        stroke_bitmaps[prerender_list[i]] = strokeBMG;

        FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, true);
        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;
        bitmaps[prerender_list[i]] = bitmapGlyph;
    }

    return 0;
}

void OSD::draw_glyph(uint8_t *data, FT_BitmapGlyph bmg,
                     int *pen_x, int *pen_y,
                     int item_height, int item_width,
                     uint32_t color) {
    int rel_pen_x = *pen_x + bmg->left;
    int rel_pen_y = *pen_y + bmg->top;
    unsigned int row = 0;
    while (row < bmg->bitmap.rows) {
        rel_pen_x = *pen_x + bmg->left;
        for (unsigned int x = 0; x < bmg->bitmap.width; ++x, ++rel_pen_x) {
            if (rel_pen_x < 0 || rel_pen_y < 0 || rel_pen_x > item_width - 1 || rel_pen_y > item_height - 1)
                continue;
            int data_index = (item_height-rel_pen_y)*item_width*4 + rel_pen_x*4;
            if (data_index+3 >= item_height*item_width*4)
                continue;
            int glyph_index = row*bmg->bitmap.pitch + x;

            uint8_t red, green, blue, alpha;
            uint8_t alpha_a = bmg->bitmap.buffer[glyph_index];
            uint8_t alpha_b = data[data_index+3];
            //Exit early if alpha is zero.
            if (alpha_a == 0)
                continue;

            if (alpha_a == 0xFF || alpha_b == 0) {
                red = (color >> 16) & 0xFF;
                green = (color >> 8) & 0xFF;
                blue = (color >> 0) & 0xFF;
                alpha = alpha_a;
            }
            else {
                //Alpha composite
                float faa = alpha_a / 256.0;
                float fab = alpha_b / 256.0;
                float fra = ((color >> 16) & 0xFF) / 256.0;
                float frb = data[data_index+2] / 256.0;
                float fga = ((color >> 8) & 0xFF) / 256.0;
                float fgb = data[data_index+1] / 256.0;
                float fba = ((color >> 0) & 0xFF) / 256.0;
                float fbb = data[data_index+0] / 256.0;

                float alpha_o = faa + fab*(1.0 - faa);
                fra = (fra*faa + frb*fab*(1.0 - faa)) / alpha_o;
                red = (uint8_t)(fra * 256.0);
                fga = (fga*faa + fgb*fab*(1.0 - faa)) / alpha_o;
                green = (uint8_t)(fra * 256.0);
                fba = (fba*faa + fbb*fab*(1.0 - faa)) / alpha_o;
                blue = (uint8_t)(fba * 256.0);
                alpha = 0xFF;
            }
            data[data_index+0] = blue;
            data[data_index+1] = green;
            data[data_index+2] = red;
            data[data_index+3] = alpha;
        }
        ++row;
        --rel_pen_y;
    }

    *pen_x += ((FT_Glyph)bmg)->advance.x >> 16;
    *pen_y += ((FT_Glyph)bmg)->advance.y >> 16;
}

void OSD::set_text(OSDItem *osdItem, IMPOSDRgnAttr *rgnAttr, std::string text, int posX, int posY, int angle) {

    //First, calculate the size of the bitmap surface we need
    FT_BBox total_bbox = {0,0,0,0};
    for (unsigned int i = 0; i < text.length(); ++i) {
        FT_BBox bbox = boxes[text[i]];

        if (bbox.yMin < total_bbox.yMin)
            total_bbox.yMin = bbox.yMin;
        if (bbox.yMax > total_bbox.yMax)
            total_bbox.yMax = bbox.yMax;
        if (bbox.xMin < total_bbox.xMin)
            total_bbox.xMin = bbox.xMin;
        total_bbox.xMax += advances[text[i]].x >> 16;
    }
    int item_height = total_bbox.yMax - total_bbox.yMin + 1;
    int item_width = total_bbox.xMax - total_bbox.xMin + 1;

    int pen_y = -total_bbox.yMin;
    int pen_x = -total_bbox.xMin;

    //OSD Quirk: The item width must be divisible by 2.
    //If not, the OSD item shifts rapidly side-to-side.
    //Bad timings?
    if (item_width % 2 != 0)
        ++item_width;

    free(osdItem->data);
    osdItem->data = (uint8_t*)malloc(item_width * item_height * 4);
    memset(osdItem->data, 0, item_width * item_height * 4);

    //Then, render the stroke & text
    for (unsigned int i = 0; i < text.length(); ++i) {
        int cpx = pen_x;
        int cpy = pen_y;

    if (osd->font_stroke_enabled) {
        draw_glyph(osdItem->data, stroke_bitmaps[text[i]], &cpx, &cpy, item_height, item_width, osd->font_stroke_color);
    }
        draw_glyph(osdItem->data, bitmaps[text[i]], &pen_x, &pen_y, item_height, item_width, osd->font_color);
    }

    if(angle) {
        rotateBGRAImage(osdItem->data, item_width, item_height, angle);
    }

    set_pos(rgnAttr, posX, posY, item_width, item_height, encChn);
    rgnAttr->data.picData.pData = osdItem->data;
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

OSD* OSD::createNew(
    _osd *osd,
    int osdGrp,
    int encChn
) {
    return new OSD(osd, osdGrp, encChn);
}

void OSD::init() {

    int ret;
    LOG_DEBUG("OSD init for  begin");

    //cfg = _cfg;
    last_updated_second = -1;
    
    if (freetype_init()) {
        LOG_DEBUG("FREETYPE init failed.");
        //return true;
    }

    ret = IMP_Encoder_GetChnAttr(osdGrp, &channelAttributes);
    if (ret < 0) {
        LOG_DEBUG("IMP_Encoder_GetChnAttr() == " + std::to_string(ret));
        //return true;
    }
     //picWidth, picHeight cpp macro !!
    LOG_DEBUG("IMP_Encoder_GetChnAttr read. Stream resolution: " << channelAttributes.encAttr.picWidth << "x" << channelAttributes.encAttr.picHeight);

    ret = IMP_OSD_CreateGroup(osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_CreateGroup(" << osdGrp << ")");

    if (osd->time_enabled) {

        /* OSD Time */
        osdTime.data = NULL;
        osdTime.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(osdTime.imp_rgn, osdGrp, NULL);
        osd->regions.time = osdTime.imp_rgn;

        IMPOSDRgnAttr rgnAttr;
        memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));
        rgnAttr.type = OSD_REG_PIC;
        rgnAttr.fmt = PIX_FMT_BGRA;
        set_text(&osdTime, &rgnAttr, osd->time_format, 
            osd->pos_time_x, osd->pos_time_y, osd->time_rotation);
        IMP_OSD_SetRgnAttr(osdTime.imp_rgn, &rgnAttr);
        
        IMPOSDGrpRgnAttr grpRgnAttr;
        memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
        grpRgnAttr.show = 1;
        grpRgnAttr.layer = 1;
        grpRgnAttr.gAlphaEn = 0;
        grpRgnAttr.fgAlhpa = osd->time_transparency;
        IMP_OSD_SetGrpRgnAttr(osdTime.imp_rgn, osdGrp, &grpRgnAttr);
    }

    if (osd->user_text_enabled) {

        /* OSD Usertext */
        osdUser.data = NULL;
        osdUser.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(osdUser.imp_rgn, osdGrp, NULL);
        osd->regions.user = osdUser.imp_rgn;

        IMPOSDRgnAttr rgnAttr;
        memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));
        rgnAttr.type = OSD_REG_PIC;
        rgnAttr.fmt = PIX_FMT_BGRA;       
        set_text(&osdUser, &rgnAttr, osd->user_text_format,
            osd->pos_user_text_x, osd->pos_user_text_y, osd->user_text_rotation);
        IMP_OSD_SetRgnAttr(osdUser.imp_rgn, &rgnAttr);
        
        IMPOSDGrpRgnAttr grpRgnAttr;
        memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
        grpRgnAttr.show = 1;
        grpRgnAttr.layer = 2;        
        grpRgnAttr.gAlphaEn = 1;
        grpRgnAttr.fgAlhpa = osd->time_transparency;
        IMP_OSD_SetGrpRgnAttr(osdUser.imp_rgn, osdGrp, &grpRgnAttr);
    }

    if (osd->uptime_enabled) {

        /* OSD Uptime */
        osdUptm.data = NULL;
        osdUptm.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(osdUptm.imp_rgn, osdGrp, NULL);
        osd->regions.uptime = osdUptm.imp_rgn;

        IMPOSDRgnAttr rgnAttr;
        memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));
        rgnAttr.type = OSD_REG_PIC;
        rgnAttr.fmt = PIX_FMT_BGRA;     
        set_text(&osdUptm, &rgnAttr, osd->uptime_format,
            osd->pos_uptime_x, osd->pos_uptime_y, osd->uptime_rotation);
        IMP_OSD_SetRgnAttr(osdUptm.imp_rgn, &rgnAttr);
        
        IMPOSDGrpRgnAttr grpRgnAttr;
        memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
        grpRgnAttr.show = 1;
        grpRgnAttr.layer = 3;       
        grpRgnAttr.gAlphaEn = 1;
        grpRgnAttr.fgAlhpa = osd->time_transparency;
        IMP_OSD_SetGrpRgnAttr(osdUptm.imp_rgn, osdGrp, &grpRgnAttr);
    }

   if (osd->logo_enabled) {

        /* OSD Logo */

        size_t imageSize;
        auto imageData = loadBGRAImage(osd->logo_path.c_str(), imageSize);

        osdLogo.data = NULL;
        osdLogo.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(osdLogo.imp_rgn, osdGrp, NULL);
        osd->regions.logo = osdLogo.imp_rgn;

        IMPOSDRgnAttr rgnAttr;
        memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));

        //Verify OSD logo size vs dimensions
        if ((osd->logo_width*osd->logo_height*4) == imageSize) {

            rgnAttr.type = OSD_REG_PIC;
            rgnAttr.fmt = PIX_FMT_BGRA;
            rgnAttr.data.picData.pData = imageData.get();

            //Logo rotation
            int logo_width = osd->logo_width;
            int logo_height = osd->logo_height;
            if(osd->logo_rotation) {
                uint8_t* imageData = static_cast<uint8_t*>(rgnAttr.data.picData.pData);
                rotateBGRAImage(imageData, logo_width, 
                    logo_height, osd->logo_rotation, false);
                rgnAttr.data.picData.pData = imageData;
            }

            set_pos(&rgnAttr, osd->pos_logo_x, 
                osd->pos_logo_y, logo_width, logo_height, encChn);
        } else {

            LOG_ERROR("Invalid OSD logo dimensions. Imagesize=" << imageSize << ", " << osd->logo_width << "*" << 
                osd->logo_height << "*4=" << (osd->logo_width*osd->logo_height*4));
        }  
        IMP_OSD_SetRgnAttr(osdLogo.imp_rgn, &rgnAttr);
        
        LOG_DEBUG(rgnAttr.rect.p0.x << " x " << rgnAttr.rect.p0.y);

        IMPOSDGrpRgnAttr grpRgnAttr;
        memset(&grpRgnAttr, 0, sizeof(IMPOSDGrpRgnAttr));
        grpRgnAttr.show = 1;
        grpRgnAttr.layer = 4;       
        grpRgnAttr.gAlphaEn = 1;
        grpRgnAttr.fgAlhpa = osd->logo_transparency;
        IMP_OSD_SetGrpRgnAttr(osdLogo.imp_rgn, osdGrp, &grpRgnAttr);     
    }

    ret = IMP_OSD_Start(osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_Start(" << osdGrp << ")");

    initialized = true;
}

int OSD::exit() {

    int ret;

    ret = IMP_OSD_ShowRgn(osdTime.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdTime.imp_rgn, "<<osdGrp<<", 0)");

    ret = IMP_OSD_ShowRgn(osdUser.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdUser.imp_rgn, "<<osdGrp<<", 0)");

    ret = IMP_OSD_ShowRgn(osdUptm.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdUptm.imp_rgn, "<<osdGrp<<", 0)");

    ret = IMP_OSD_ShowRgn(osdLogo.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdLogo.imp_rgn, "<<osdGrp<<", 0)");

    ret = IMP_OSD_UnRegisterRgn(osdTime.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdTime.imp_rgn, "<<osdGrp<<")");

    ret = IMP_OSD_UnRegisterRgn(osdUser.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUser.imp_rgn, "<<osdGrp<<")");

    ret = IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, "<<osdGrp<<")");

    ret = IMP_OSD_UnRegisterRgn(osdLogo.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, "<<osdGrp<<")");

    IMP_OSD_DestroyRgn(osdTime.imp_rgn);
    IMP_OSD_DestroyRgn(osdUser.imp_rgn);
    IMP_OSD_DestroyRgn(osdUptm.imp_rgn);
    IMP_OSD_DestroyRgn(osdLogo.imp_rgn);

    ret = IMP_OSD_DestroyGroup(osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_DestroyGroup("<<osdGrp<<")");

    // cleanup osd image data
    free(osdTime.data);
    free(osdUser.data);
    free(osdUptm.data);
    free(osdLogo.data);

    ret = FT_Done_FreeType(freetype);
    LOG_DEBUG_OR_ERROR(ret, "FT_Done_FreeType(freetype)");

    return 0;    
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

int getIp (char * addressBuffer) {
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            //char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //snprintf(buff, sizeof(buff), "%s", addressBuffer); 
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return 0;
}

void OSD::updateDisplayEverySecond() {
    time_t current = time(NULL);
    struct tm *ltime = localtime(&current);

    // Check if we have moved to a new second
    if (ltime->tm_sec != last_updated_second) {

        // Format and update system time 
        if (osd->time_enabled) {

            char timeFormatted[256];
            strftime(timeFormatted, sizeof(timeFormatted), osd->time_format.c_str(), ltime);
            
            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdTime.imp_rgn, &rgnAttr);
            set_text(&osdTime, &rgnAttr, std::string(timeFormatted), 
                osd->pos_time_x, osd->pos_time_y, osd->time_rotation);
            IMP_OSD_SetRgnAttr(osdTime.imp_rgn, &rgnAttr);
        }

        // Format and update user text !! every 30 seconds !!
        if (osd->user_text_enabled && (!(ltime->tm_sec % 30) || last_updated_second == -1)) {

            std::string user_text = osd->user_text_format;

            std::size_t tokenPos = user_text.find("%hostname");
            if (tokenPos != std::string::npos) {
                char hostname[64];
                gethostname(hostname, 64);
                user_text.replace(tokenPos, 9, std::string(hostname));
            }

            tokenPos = user_text.find("%ipaddress");
            if (tokenPos != std::string::npos) {
                char ip[INET_ADDRSTRLEN];
                getIp(ip);
                user_text.replace(tokenPos, 10, std::string(ip));
            }

            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdUser.imp_rgn, &rgnAttr);
            set_text(&osdUser, &rgnAttr, user_text,
                osd->pos_user_text_x, osd->pos_user_text_y, osd->user_text_rotation);
            IMP_OSD_SetRgnAttr(osdUser.imp_rgn, &rgnAttr);
        }

        // Format and update uptime
        if (osd->uptime_enabled) {

            unsigned long currentUptime = getSystemUptime();
            unsigned long hours = currentUptime / 3600;
            unsigned long minutes = (currentUptime % 3600) / 60;
            unsigned long seconds = currentUptime % 60;

            char uptimeFormatted[256];
            snprintf(uptimeFormatted, sizeof(uptimeFormatted), osd->uptime_format.c_str(), hours, minutes, seconds);

            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdUptm.imp_rgn, &rgnAttr);
            set_text(&osdUptm, &rgnAttr, std::string(uptimeFormatted),
                osd->pos_uptime_x, osd->pos_uptime_y, osd->uptime_rotation);
            IMP_OSD_SetRgnAttr(osdUptm.imp_rgn, &rgnAttr);
        }  

        last_updated_second = ltime->tm_sec;  // Update the last second tracker              
    }
}

void OSD::update() {
    //Called every frame by the encoder.
    updateDisplayEverySecond();
}
