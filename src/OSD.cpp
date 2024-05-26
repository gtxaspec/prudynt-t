#include <iostream>
#include <vector>
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
	#define picWidth uWidth
	#define picHeight uHeight
#endif

int OSD::get_abs_pos(int max, int size, int pos) {

    if(pos==0) return max / 2 - size / 2;
    if(pos<0) return max - size + pos;
    return pos;
}

void OSD::set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, int width, int height) {
    //picWidth, picHeight cpp macro !!
    rgnAttr->rect.p0.x = get_abs_pos(channelAttributes.encAttr.picWidth, width, x);
    rgnAttr->rect.p0.y = get_abs_pos(channelAttributes.encAttr.picHeight, height, y);
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
        Config::singleton()->OSDFontPath.c_str(),
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
        Config::singleton()->OSDFontStrokeSize,
        FT_STROKER_LINECAP_SQUARE,
        FT_STROKER_LINEJOIN_ROUND,
        0
    );
    FT_Set_Char_Size(fontface, 0, 32*64, Config::singleton()->OSDFontSize, Config::singleton()->OSDFontSize);

    //Prerender glyphs needed for displaying date & time.
    std::string prerender_list;

    if (Config::singleton()->OSDUserTextEnable) {
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

void OSD::draw_glyph(OSDTextItem *ti, FT_BitmapGlyph bmg,
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
            uint8_t alpha_b = ti->data[data_index+3];
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
                float frb = ti->data[data_index+2] / 256.0;
                float fga = ((color >> 8) & 0xFF) / 256.0;
                float fgb = ti->data[data_index+1] / 256.0;
                float fba = ((color >> 0) & 0xFF) / 256.0;
                float fbb = ti->data[data_index+0] / 256.0;

                float alpha_o = faa + fab*(1.0 - faa);
                fra = (fra*faa + frb*fab*(1.0 - faa)) / alpha_o;
                red = (uint8_t)(fra * 256.0);
                fga = (fga*faa + fgb*fab*(1.0 - faa)) / alpha_o;
                green = (uint8_t)(fra * 256.0);
                fba = (fba*faa + fbb*fab*(1.0 - faa)) / alpha_o;
                blue = (uint8_t)(fba * 256.0);
                alpha = 0xFF;
            }
            ti->data[data_index+0] = blue;
            ti->data[data_index+1] = green;
            ti->data[data_index+2] = red;
            ti->data[data_index+3] = alpha;
        }
        ++row;
        --rel_pen_y;
    }

    *pen_x += ((FT_Glyph)bmg)->advance.x >> 16;
    *pen_y += ((FT_Glyph)bmg)->advance.y >> 16;
}

void OSD::set_text(OSDTextItem *ti, std::string text) {
    ti->text = text;

    //First, calculate the size of the bitmap surface we need
    FT_BBox total_bbox = {0,0,0,0};
    for (unsigned int i = 0; i < ti->text.length(); ++i) {
        FT_BBox bbox = boxes[ti->text[i]];

        if (bbox.yMin < total_bbox.yMin)
            total_bbox.yMin = bbox.yMin;
        if (bbox.yMax > total_bbox.yMax)
            total_bbox.yMax = bbox.yMax;
        if (bbox.xMin < total_bbox.xMin)
            total_bbox.xMin = bbox.xMin;
        total_bbox.xMax += advances[ti->text[i]].x >> 16;
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

    set_pos(&ti->imp_attr, ti->x, ti->y, item_width, item_height);

    free(ti->data);
    ti->data = (uint8_t*)malloc(item_width * item_height * 4);
    ti->imp_attr.data.picData.pData = ti->data;
    memset(ti->data, 0, item_width * item_height * 4);

    
    //Then, render the stroke & text
    for (unsigned int i = 0; i < ti->text.length(); ++i) {
        int cpx = pen_x;
        int cpy = pen_y;

    if (Config::singleton()->OSDFontStrokeEnable) {
        draw_glyph(ti, stroke_bitmaps[ti->text[i]], &cpx, &cpy, item_height, item_width, Config::singleton()->OSDFontStrokeColor);
    }
        draw_glyph(ti, bitmaps[ti->text[i]], &pen_x, &pen_y, item_height, item_width, Config::singleton()->OSDFontColor);
    }
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

    if (freetype_init()) {
        LOG_DEBUG("FREETYPE init failed.");
        return true;
    }

    ret = IMP_Encoder_GetChnAttr(0, &channelAttributes);
    if (ret < 0) {
        LOG_DEBUG("IMP_Encoder_GetChnAttr() == " + std::to_string(ret));
        return true;
    }
    LOG_DEBUG(
        printf("IMP_Encoder_GetChnAttr read. Stream resolution: %d x %d\n", 
                channelAttributes.encAttr.picWidth, channelAttributes.encAttr.picHeight)); //picWidth, picHeight cpp macro !!

    ret = IMP_OSD_CreateGroup(0);
    if (ret < 0) {
        LOG_DEBUG("IMP_OSD_CreateGroup() == " + std::to_string(ret));
        return true;
    }
    LOG_DEBUG("IMP_OSD_CreateGroup group 0 created");

    if (Config::singleton()->OSDTimeEnable) {

        timestamp.x = Config::singleton()->stream0osdPosTimeX;
        timestamp.y = Config::singleton()->stream0osdPosTimeY;

        memset(&timestamp.imp_rgn, 0, sizeof(timestamp.imp_rgn));
        timestamp.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(timestamp.imp_rgn, 0, NULL);
        timestamp.imp_attr.type = OSD_REG_PIC;
        timestamp.imp_attr.fmt = PIX_FMT_BGRA;
        timestamp.imp_attr.data.picData.pData = timestamp.data;

        memset(&timestamp.imp_grp_attr, 0, sizeof(timestamp.imp_grp_attr));
        IMP_OSD_SetRgnAttr(timestamp.imp_rgn, &timestamp.imp_attr);
        IMP_OSD_GetGrpRgnAttr(timestamp.imp_rgn, 0, &timestamp.imp_grp_attr);
        timestamp.imp_grp_attr.show = 1;
        timestamp.imp_grp_attr.layer = 1;
        timestamp.imp_grp_attr.scalex = 0;
        timestamp.imp_grp_attr.scaley = 0;
        timestamp.imp_grp_attr.gAlphaEn = 1;
        timestamp.imp_grp_attr.fgAlhpa = Config::singleton()->stream0osdTimeAlpha;
        IMP_OSD_SetGrpRgnAttr(timestamp.imp_rgn, 0, &timestamp.imp_grp_attr);
    }

    if (Config::singleton()->OSDUserTextEnable) {

        userText.x = Config::singleton()->stream0osdUserTextPosX;
        userText.y = Config::singleton()->stream0osdUserTextPosY;

        memset(&userText.imp_rgn, 0, sizeof(userText.imp_rgn));
        userText.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(userText.imp_rgn, 0, NULL);
        userText.imp_attr.type = OSD_REG_PIC;
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
        userText.imp_grp_attr.gAlphaEn = 1;
        userText.imp_grp_attr.fgAlhpa = Config::singleton()->stream0osdUserTextAlpha;
        IMP_OSD_SetGrpRgnAttr(userText.imp_rgn, 0, &userText.imp_grp_attr);

        userText.update_intervall = 10;
        userText.last_update = 0;      
    }

    if (Config::singleton()->OSDUptimeEnable) {

        uptimeStamp.x = Config::singleton()->stream0osdUptimeStampPosX;
        uptimeStamp.y = Config::singleton()->stream0osdUptimeStampPosY;

        memset(&uptimeStamp.imp_rgn, 0, sizeof(uptimeStamp.imp_rgn));
        uptimeStamp.imp_rgn = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_RegisterRgn(uptimeStamp.imp_rgn, 0, NULL);
        uptimeStamp.imp_attr.type = OSD_REG_PIC;
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
        uptimeStamp.imp_grp_attr.gAlphaEn = 1;
        uptimeStamp.imp_grp_attr.fgAlhpa = Config::singleton()->stream0osdUptimeAlpha;
        IMP_OSD_SetGrpRgnAttr(uptimeStamp.imp_rgn, 0, &uptimeStamp.imp_grp_attr);        
    }

    if (Config::singleton()->OSDLogoEnable) {
        size_t imageSize;
        auto imageData = loadBGRAImage(Config::singleton()->OSDLogoPath.c_str(), imageSize);
        
        memset(&OSDLogo, 0, sizeof(OSDLogo));
        OSDLogo.type = OSD_REG_PIC;
        OSDLogo.fmt = PIX_FMT_BGRA;
        OSDLogo.data.picData.pData = imageData.get();
    
        int picw = Config::singleton()->OSDLogoWidth + 1;
        int pich = Config::singleton()->OSDLogoHeight + 1;

        set_pos(&OSDLogo, 
                Config::singleton()->stream0osdLogoPosX, 
                Config::singleton()->stream0osdLogoPosY, 
                picw-1, pich-1);

        IMPRgnHandle OSDLogoHandle = IMP_OSD_CreateRgn(NULL);
        IMP_OSD_SetRgnAttr(OSDLogoHandle, &OSDLogo);
        IMPOSDGrpRgnAttr OSDLogoGroup;
        memset(&OSDLogoGroup, 0, sizeof(OSDLogoGroup));
        OSDLogoGroup.show = 1;
        OSDLogoGroup.layer = 4;
        OSDLogoGroup.scalex = 1;
        OSDLogoGroup.scaley = 1;
        OSDLogoGroup.gAlphaEn = 1;
        OSDLogoGroup.fgAlhpa = Config::singleton()->stream0osdLogoAlpha;

        IMP_OSD_RegisterRgn(OSDLogoHandle, 0, &OSDLogoGroup);                
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

bool OSD::exit() {

    int ret;

    ret = IMP_OSD_ShowRgn(timestamp.imp_rgn, 0, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_ShowRgn close timeStamp error");
    }

    ret = IMP_OSD_ShowRgn(userText.imp_rgn, 0, 0);
    if (ret < 0) {
        LOG_ERROR( "IMP_OSD_ShowRgn close user text error");
    }

    ret = IMP_OSD_ShowRgn(uptimeStamp.imp_rgn, 0, 0);
    if (ret < 0) {
        LOG_ERROR( "IMP_OSD_ShowRgn close uptime error");
    }

    ret = IMP_OSD_ShowRgn(OSDLogoHandle, 0, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_ShowRgn close Rect error");
    }

    ret = IMP_OSD_UnRegisterRgn(timestamp.imp_rgn, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_UnRegisterRgn timeStamp error");
    }

    ret = IMP_OSD_UnRegisterRgn(userText.imp_rgn, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_UnRegisterRgn user text error");
    }

    ret = IMP_OSD_UnRegisterRgn(uptimeStamp.imp_rgn, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_UnRegisterRgn Cover error");
    }

    ret = IMP_OSD_UnRegisterRgn(OSDLogoHandle, 0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_UnRegisterRgn Rect error");
    }

    IMP_OSD_DestroyRgn(timestamp.imp_rgn);
    IMP_OSD_DestroyRgn(userText.imp_rgn);
    IMP_OSD_DestroyRgn(uptimeStamp.imp_rgn);
    IMP_OSD_DestroyRgn(OSDLogoHandle);

    ret = IMP_OSD_DestroyGroup(0);
    if (ret < 0) {
        LOG_ERROR("IMP_OSD_DestroyGroup(0) error");
        return -1;
    }

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

// Var to keep track of the last second updated
static int last_updated_second = -1;

void OSD::updateDisplayEverySecond() {
    time_t current = time(NULL);
    struct tm *ltime = localtime(&current);

    // Check if we have moved to a new second
    if (ltime->tm_sec != last_updated_second) {
        last_updated_second = ltime->tm_sec;  // Update the last second tracker

        //get second of the day
        uint32_t sod = (ltime->tm_hour * 3600) + (ltime->tm_min * 60) + ltime->tm_sec;
        
        // Now we ensure both updates are performed as close together as possible
        //Format user text
        if (Config::singleton()->OSDUserTextEnable && 
            ( sod < userText.last_update || sod > ( userText.last_update + userText.update_intervall ))) {

            userText.last_update = sod;

            std::string sTextFormated = Config::singleton()->OSDUserTextString;

            std::size_t tokenPos = sTextFormated.find("%hostname");
            if (tokenPos != std::string::npos) {
                char hostname[64];
                gethostname(hostname, 64);
                sTextFormated.replace(tokenPos, 9, std::string(hostname));
            }

            tokenPos = sTextFormated.find("%ipaddress");
            if (tokenPos != std::string::npos) {
                char ip[INET_ADDRSTRLEN];
                getIp(ip);
                sTextFormated.replace(tokenPos, 10, std::string(ip));
            }

            set_text(&userText, sTextFormated);
            IMP_OSD_SetRgnAttr(userText.imp_rgn, &userText.imp_attr);
        }

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
    //Called every frame by the encoder.
    updateDisplayEverySecond();
}
