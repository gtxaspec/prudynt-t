
#include <cmath>
#include "OSD.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#if defined(PLATFORM_T31)
#define IMPEncoderCHNAttr IMPEncoderChnAttr
#define IMPEncoderCHNStat IMPEncoderChnStat
#endif

#if defined(PLATFORM_T31)
#define picWidth uWidth
#define picHeight uHeight
#endif

unsigned long getSystemUptime()
{
    struct sysinfo info;
    if (sysinfo(&info) != 0)
    {
        return 0;
    }
    return info.uptime;
}

int getIp(char *addressBuffer)
{
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
        {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET)
        { // check it is IP4
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
        }
    }
    if (ifAddrStruct != nullptr)
        freeifaddrs(ifAddrStruct);
    return 0;
}

const char * OSD::getConfigPath(const char *itemName) {
    static std::string buffer;
    buffer.clear();
    buffer = std::string(parent) + ".osd." + itemName;
    return buffer.c_str();
}

int autoFontSize(int pWidth)
{
    double m = 1.0 / 60.0;
    double b = 21.33;
    return m * pWidth + b + 1;
}

const char *replace(const char *str, const char *oldToken, const char *newToken)
{
    int strLen = 0;
    while (str[strLen] != '\0')
        strLen++;

    int oldTokenLen = 0;
    while (oldToken[oldTokenLen] != '\0')
        oldTokenLen++;

    int newTokenLen = 0;
    while (newToken[newTokenLen] != '\0')
        newTokenLen++;

    const char *pos = nullptr;
    for (int i = 0; i <= strLen - oldTokenLen; ++i)
    {
        bool match = true;
        for (int j = 0; j < oldTokenLen; ++j)
        {
            if (str[i + j] != oldToken[j])
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            pos = str + i;
            break;
        }
    }

    if (!pos)
    {
        return str;
    }

    int newStrLen = strLen - oldTokenLen + newTokenLen;

    char *newStr = new char[newStrLen + 1];

    int i = 0;
    for (; str + i != pos; ++i)
    {
        newStr[i] = str[i];
    }

    for (int j = 0; j < newTokenLen; ++j, ++i)
    {
        newStr[i] = newToken[j];
    }

    for (int j = 0; pos[j + oldTokenLen] != '\0'; ++j, ++i)
    {
        newStr[i] = pos[j + oldTokenLen];
    }

    newStr[i] = '\0';

    return newStr;
}

void OSD::rotateBGRAImage(uint8_t *&inputImage, int &width, int &height, int angle, bool del = true)
{
    double angleRad = angle * (M_PI / 180.0);

    int originalCorners[4][2] = {
        {0, 0},
        {width, 0},
        {0, height},
        {width, height}};

    int minX = INT_MAX;
    int maxX = INT_MIN;
    int minY = INT_MAX;
    int maxY = INT_MIN;

    /*
    for (int i = 0; i < 4; ++i)
    {
        int x = originalCorners[i][0];
        int y = originalCorners[i][1];
    */
    for (auto & originalCorner : originalCorners) {
		int x = originalCorner[0];
		int y = originalCorner[1];

        int newX = static_cast<int>(x * cos(angleRad) - y * sin(angleRad));
        int newY = static_cast<int>(x * sin(angleRad) + y * cos(angleRad));

        if (newX < minX)
            minX = newX;
        if (newX > maxX)
            maxX = newX;
        if (newY < minY)
            minY = newY;
        if (newY > maxY)
            maxY = newY;
    }

    int newWidth = maxX - minX + 1;
    int newHeight = maxY - minY + 1;

    int centerX = width / 2;
    int centerY = height / 2;

    int newCenterX = newWidth / 2;
    int newCenterY = newHeight / 2;

    auto *rotatedImage = new uint8_t[newWidth * newHeight * 4]();

    for (int y = 0; y < newHeight; ++y)
    {
        for (int x = 0; x < newWidth; ++x)
        {
            int newX = x - newCenterX;
            int newY = y - newCenterY;

            int origX = static_cast<int>(newX * cos(angleRad) + newY * sin(angleRad)) + centerX;
            int origY = static_cast<int>(-newX * sin(angleRad) + newY * cos(angleRad)) + centerY;

            if (origX >= 0 && origX < width && origY >= 0 && origY < height)
            {
                for (int c = 0; c < 4; ++c)
                {
                    rotatedImage[(y * newWidth + x) * 4 + c] = inputImage[(origY * width + origX) * 4 + c];
                }
            }
        }
    }

    if (del)
        delete[] inputImage;
    inputImage = rotatedImage;
    width = newWidth;
    height = newHeight;
}

int OSD::get_abs_pos(int max, int size, int pos)
{
    if (pos == 0){
        return max / 2 - size / 2;
    }
    if (pos < 0){
        return max - size - 1 + pos;
    }
    return pos;
}

void OSD::set_pos(IMPOSDRgnAttr *rgnAttr, int x, int y, int width, int height, int encChn)
{
    // picWidth, picHeight cpp macro !!
    IMPEncoderCHNAttr chnAttr;
    IMP_Encoder_GetChnAttr(encChn, &chnAttr);

    if (width == 0 || height == 0)
    {
        width = rgnAttr->rect.p1.x - rgnAttr->rect.p0.x + 1;
        height = rgnAttr->rect.p1.y - rgnAttr->rect.p0.y + 1;
    }

    if (x > (int)chnAttr.encAttr.picWidth - width)
        x = (int)chnAttr.encAttr.picWidth - width;

    if (y > (int)chnAttr.encAttr.picHeight - height)
        y = (int)chnAttr.encAttr.picHeight - height;

    rgnAttr->rect.p0.x = get_abs_pos(chnAttr.encAttr.picWidth, width, x);
    rgnAttr->rect.p0.y = get_abs_pos(chnAttr.encAttr.picHeight, height, y);
    rgnAttr->rect.p1.x = rgnAttr->rect.p0.x + width - 1;
    rgnAttr->rect.p1.y = rgnAttr->rect.p0.y + height - 1;
}

int OSD::freetype_init()
{
    int error;

    error = FT_Init_FreeType(&freetype);
    if (error)
    {
        return error;
    }

    error = FT_New_Face(
        freetype,
        osd->font_path,
        0,
        &fontface);
    if (error)
    {
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
        72 * 64,
        96,
        96);

    FT_Stroker_Set(
        stroker,
        osd->font_stroke_size,
        FT_STROKER_LINECAP_SQUARE,
        FT_STROKER_LINEJOIN_ROUND,
        0);
    FT_Set_Char_Size(fontface, 0, 32 * 64, osd->font_size, osd->font_size);

    // Prerender glyphs needed for displaying date & time.
    const char *prerender_list;
    if (osd->user_text_enabled)
    {
        prerender_list = "0123456789 /:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()-_=+[]{}|;:'\",.<>?`~";
    }
    else
    {
        prerender_list = "0123456789 /APM:";
    }

    int prerender_list_length = strlen(prerender_list);
    for (unsigned int i = 0; i < prerender_list_length; ++i)
    {
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
                     uint32_t color)
{
    int rel_pen_x = *pen_x + bmg->left;
    int rel_pen_y = *pen_y + bmg->top;
    unsigned int row = 0;
    while (row < bmg->bitmap.rows)
    {
        rel_pen_x = *pen_x + bmg->left;
        for (unsigned int x = 0; x < bmg->bitmap.width; ++x, ++rel_pen_x)
        {
            if (rel_pen_x < 0 || rel_pen_y < 0 || rel_pen_x > item_width - 1 || rel_pen_y > item_height - 1)
                continue;
            int data_index = (item_height - rel_pen_y) * item_width * 4 + rel_pen_x * 4;
            if (data_index + 3 >= item_height * item_width * 4)
                continue;
            int glyph_index = row * bmg->bitmap.pitch + x;

            uint8_t red, green, blue, alpha;
            uint8_t alpha_a = bmg->bitmap.buffer[glyph_index];
            uint8_t alpha_b = data[data_index + 3];
            // Exit early if alpha is zero.
            if (alpha_a == 0)
                continue;

            if (alpha_a == 0xFF || alpha_b == 0)
            {
                red = (color >> 16) & 0xFF;
                green = (color >> 8) & 0xFF;
                blue = (color >> 0) & 0xFF;
                alpha = alpha_a;
            }
            else
            {
                // Alpha composite
                float faa = alpha_a / 256.0;
                float fab = alpha_b / 256.0;
                float fra = ((color >> 16) & 0xFF) / 256.0;
                float frb = data[data_index + 2] / 256.0;
                float fga = ((color >> 8) & 0xFF) / 256.0;
                float fgb = data[data_index + 1] / 256.0;
                float fba = ((color >> 0) & 0xFF) / 256.0;
                float fbb = data[data_index + 0] / 256.0;

                float alpha_o = faa + fab * (1.0 - faa);
                fra = (fra * faa + frb * fab * (1.0 - faa)) / alpha_o;
                red = (uint8_t)(fra * 256.0);
                fga = (fga * faa + fgb * fab * (1.0 - faa)) / alpha_o;
                green = (uint8_t)(fra * 256.0);
                fba = (fba * faa + fbb * fab * (1.0 - faa)) / alpha_o;
                blue = (uint8_t)(fba * 256.0);
                alpha = 0xFF;
            }
            data[data_index + 0] = blue;
            data[data_index + 1] = green;
            data[data_index + 2] = red;
            data[data_index + 3] = alpha;
        }
        ++row;
        --rel_pen_y;
    }

    *pen_x += ((FT_Glyph)bmg)->advance.x >> 16;
    *pen_y += ((FT_Glyph)bmg)->advance.y >> 16;
}

void OSD::set_text(OSDItem *osdItem, IMPOSDRgnAttr *rgnAttr, const char *text, int posX, int posY, int angle)
{

    // First, calculate the size of the bitmap surface we need
    int text_length = strlen(text);
    FT_BBox total_bbox = {0, 0, 0, 0};
    for (unsigned int i = 0; i < text_length; ++i)
    {
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

    // OSD Quirk: The item width must be divisible by 2.
    // If not, the OSD item shifts rapidly side-to-side.
    // Bad timings?
    if (item_width % 2 != 0)
        ++item_width;

    free(osdItem->data);
    osdItem->data = (uint8_t *)malloc(item_width * item_height * 4);
    memset(osdItem->data, 0, item_width * item_height * 4);

    // Then, render the stroke & text
    for (unsigned int i = 0; i < text_length; ++i)
    {
        int cpx = pen_x;
        int cpy = pen_y;

        if (osd->font_stroke_enabled)
        {
            draw_glyph(osdItem->data, stroke_bitmaps[text[i]], &cpx, &cpy, item_height, item_width, osd->font_stroke_color);
        }
        draw_glyph(osdItem->data, bitmaps[text[i]], &pen_x, &pen_y, item_height, item_width, osd->font_color);
    }

    if (angle)
    {
        rotateBGRAImage(osdItem->data, item_width, item_height, angle);
    }

    set_pos(rgnAttr, posX, posY, item_width, item_height, encChn);
    rgnAttr->data.picData.pData = osdItem->data;
}

unsigned char *loadBGRAImage(const char *filepath, size_t &length)
{
    // Datei öffnen
    FILE *file = fopen(filepath, "rb");
    if (!file)
    {
        printf("Failed to open the OSD logo file.\n");
        return nullptr;
    }

    // Dateiende suchen, um die Größe zu ermitteln
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Speicher für das Bild allokieren
    unsigned char *data = (unsigned char *)malloc(length);
    if (!data)
    {
        printf("Failed to allocate memory for the image.\n");
        fclose(file);
        return nullptr;
    }

    // Bilddaten lesen
    if (fread(data, 1, length, file) != length)
    {
        printf("Failed to read OSD logo data.\n");
        free(data);
        fclose(file);
        return nullptr;
    }

    // Datei schließen
    fclose(file);
    return data;
}

OSD *OSD::createNew(
    _osd *osd,
    std::shared_ptr<CFG> cfg,
    int osdGrp,
    int encChn,
    const char *parent)
{
    return new OSD(osd, cfg, osdGrp, encChn, parent);
}

void OSD::init()
{

    int ret = 0;
    LOG_DEBUG("OSD init for  begin");

    // cfg = _cfg;
    last_updated_second = -1;

    ret = IMP_Encoder_GetChnAttr(osdGrp, &channelAttributes);
    if (ret < 0)
    {
        LOG_DEBUG("IMP_Encoder_GetChnAttr() == " << ret);
        // return true;
    }
    // picWidth, picHeight cpp macro !!
    LOG_DEBUG("IMP_Encoder_GetChnAttr read. Stream resolution: " << channelAttributes.encAttr.picWidth
                                                                 << "x" << channelAttributes.encAttr.picHeight);

    ret = IMP_OSD_CreateGroup(osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_CreateGroup(" << osdGrp << ")");

    int autoOffset = round((float)(channelAttributes.encAttr.picWidth * 0.004));
    if (osd->font_size == OSD_AUTO_VALUE)
    {
        int fontSize = autoFontSize(channelAttributes.encAttr.picWidth);
        
        //use cfg->set to set noSave, so auto values will not written to config
        cfg->set<int>(getConfigPath("font_size"), fontSize, true);
        cfg->set<int>(getConfigPath("font_stroke_size"), fontSize, true);
    }

    if (freetype_init())
    {
        LOG_DEBUG("FREETYPE init failed.");
        // return true;
    }

    if (osd->time_enabled)
    {
        /* OSD Time */
        if (osd->pos_time_x == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_time_x"), autoOffset, true);
        }
        if (osd->pos_time_y == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_time_y"), autoOffset, true);
        }
 
        osdTime.data = nullptr;
        osdTime.imp_rgn = IMP_OSD_CreateRgn(nullptr);
        IMP_OSD_RegisterRgn(osdTime.imp_rgn, osdGrp, nullptr);
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

    if (osd->user_text_enabled)
    {

        getIp(ip);
        gethostname(hostname, 64);

        /* OSD Usertext */
        if (osd->pos_user_text_x == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_user_text_x"), 0, true);            
        }
        if (osd->pos_user_text_y == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_user_text_y"), autoOffset, true);            
        }

        osdUser.data = nullptr;
        osdUser.imp_rgn = IMP_OSD_CreateRgn(nullptr);
        IMP_OSD_RegisterRgn(osdUser.imp_rgn, osdGrp, nullptr);
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
        grpRgnAttr.fgAlhpa = osd->user_text_transparency;
        IMP_OSD_SetGrpRgnAttr(osdUser.imp_rgn, osdGrp, &grpRgnAttr);
    }

    if (osd->uptime_enabled)
    {

        /* OSD Uptime */
        if (osd->pos_uptime_x == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_uptime_x"), -autoOffset, true);            
        }
        if (osd->pos_uptime_y == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_uptime_y"), autoOffset, true);            
        }

        osdUptm.data = nullptr;
        osdUptm.imp_rgn = IMP_OSD_CreateRgn(nullptr);
        IMP_OSD_RegisterRgn(osdUptm.imp_rgn, osdGrp, nullptr);
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
        grpRgnAttr.fgAlhpa = osd->uptime_transparency;
        IMP_OSD_SetGrpRgnAttr(osdUptm.imp_rgn, osdGrp, &grpRgnAttr);
    }

    if (osd->logo_enabled)
    {

        /* OSD Logo */
        if (osd->pos_logo_x == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_logo_x"), -autoOffset, true);             
        }
        if (osd->pos_logo_y == OSD_AUTO_VALUE)
        {
            //use cfg->set to set noSave, so auto values will not written to config
            cfg->set<int>(getConfigPath("pos_logo_y"), -autoOffset, true);             
        }

        size_t imageSize;
        auto imageData = loadBGRAImage(osd->logo_path, imageSize);

        osdLogo.data = nullptr;
        osdLogo.imp_rgn = IMP_OSD_CreateRgn(nullptr);
        IMP_OSD_RegisterRgn(osdLogo.imp_rgn, osdGrp, nullptr);
        osd->regions.logo = osdLogo.imp_rgn;

        IMPOSDRgnAttr rgnAttr;
        memset(&rgnAttr, 0, sizeof(IMPOSDRgnAttr));

        // Verify OSD logo size vs dimensions
        if ((osd->logo_width * osd->logo_height * 4) == imageSize)
        {
            rgnAttr.type = OSD_REG_PIC;
            rgnAttr.fmt = PIX_FMT_BGRA;
            rgnAttr.data.picData.pData = imageData;

            // Logo rotation
            int logo_width = osd->logo_width;
            int logo_height = osd->logo_height;
            if (osd->logo_rotation)
            {
                uint8_t *imageData = static_cast<uint8_t *>(rgnAttr.data.picData.pData);
                rotateBGRAImage(imageData, logo_width,
                                logo_height, osd->logo_rotation, false);
                rgnAttr.data.picData.pData = imageData;
            }

            set_pos(&rgnAttr, osd->pos_logo_x,
                    osd->pos_logo_y, logo_width, logo_height, encChn);
        }
        else
        {

            LOG_ERROR("Invalid OSD logo dimensions. Imagesize=" << imageSize << ", " << osd->logo_width
                << "*" << osd->logo_height << "*4=" << (osd->logo_width * osd->logo_height * 4));
        }
        IMP_OSD_SetRgnAttr(osdLogo.imp_rgn, &rgnAttr);

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

int OSD::exit()
{

    int ret;

    ret = IMP_OSD_ShowRgn(osdTime.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdTime.imp_rgn, " << osdGrp << ", 0)");

    ret = IMP_OSD_ShowRgn(osdUser.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdUser.imp_rgn, " << osdGrp << ", 0)");

    ret = IMP_OSD_ShowRgn(osdUptm.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdUptm.imp_rgn, " << osdGrp << ", 0)");

    ret = IMP_OSD_ShowRgn(osdLogo.imp_rgn, osdGrp, 0);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_ShowRgn(osdLogo.imp_rgn, " << osdGrp << ", 0)");

    ret = IMP_OSD_UnRegisterRgn(osdTime.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdTime.imp_rgn, " << osdGrp << ")");

    ret = IMP_OSD_UnRegisterRgn(osdUser.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUser.imp_rgn, " << osdGrp << ")");

    ret = IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, " << osdGrp << ")");

    ret = IMP_OSD_UnRegisterRgn(osdLogo.imp_rgn, osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_UnRegisterRgn(osdUptm.imp_rgn, " << osdGrp << ")");

    IMP_OSD_DestroyRgn(osdTime.imp_rgn);
    IMP_OSD_DestroyRgn(osdUser.imp_rgn);
    IMP_OSD_DestroyRgn(osdUptm.imp_rgn);
    IMP_OSD_DestroyRgn(osdLogo.imp_rgn);

    ret = IMP_OSD_DestroyGroup(osdGrp);
    LOG_DEBUG_OR_ERROR(ret, "IMP_OSD_DestroyGroup(" << osdGrp << ")");

    // cleanup osd image data
    free(osdTime.data);
    free(osdUser.data);
    free(osdUptm.data);
    free(osdLogo.data);

    ret = FT_Done_FreeType(freetype);
    LOG_DEBUG_OR_ERROR(ret, "FT_Done_FreeType(freetype)");

    return 0;
}

void OSD::updateDisplayEverySecond()
{
    time_t current = time(nullptr);
    struct tm *ltime = localtime(&current);

    // Check if we have moved to a new second
    if (ltime->tm_sec != last_updated_second)
    {

        // Format and update system time
        if (osd->time_enabled)
        {

            char timeFormatted[256];
            strftime(timeFormatted, sizeof(timeFormatted), osd->time_format, ltime);

            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdTime.imp_rgn, &rgnAttr);
            set_text(&osdTime, &rgnAttr, timeFormatted,
                     osd->pos_time_x, osd->pos_time_y, osd->time_rotation);
            IMP_OSD_SetRgnAttr(osdTime.imp_rgn, &rgnAttr);
        }

        // Format and update user text !! every 30 seconds !!
        if (osd->user_text_enabled || last_updated_second == -1)
        {

            const char *user_text = osd->user_text_format;
            if (strstr(user_text, "%hostname") != nullptr)
            {
                user_text = replace(user_text, "%hostname", hostname);
            }

            if (strstr(user_text, "%ipaddress") != nullptr)
            {
                user_text = replace(user_text, "%ipaddress", ip);
            }

            if (strstr(user_text, "%fps") != nullptr)
            {
                char fps[4];
                snprintf(fps, 4, "%3d", osd->stats.fps);
                user_text = replace(user_text, "%fps", fps);
            }

            if (strstr(user_text, "%bps") != nullptr)
            {
                char bps[8];
                snprintf(bps, 8, "%5d", osd->stats.bps);
                user_text = replace(user_text, "%bps", bps);
            }

            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdUser.imp_rgn, &rgnAttr);
            set_text(&osdUser, &rgnAttr, user_text,
                     osd->pos_user_text_x, osd->pos_user_text_y, osd->user_text_rotation);
            IMP_OSD_SetRgnAttr(osdUser.imp_rgn, &rgnAttr);

            delete user_text;
        }

        // Format and update uptime
        if (osd->uptime_enabled)
        {

            unsigned long currentUptime = getSystemUptime();
            unsigned long hours = currentUptime / 3600;
            unsigned long minutes = (currentUptime % 3600) / 60;
            unsigned long seconds = currentUptime % 60;

            char uptimeFormatted[256];
            snprintf(uptimeFormatted, sizeof(uptimeFormatted), osd->uptime_format, hours, minutes, seconds);

            IMPOSDRgnAttr rgnAttr;
            IMP_OSD_GetRgnAttr(osdUptm.imp_rgn, &rgnAttr);
            set_text(&osdUptm, &rgnAttr, uptimeFormatted,
                     osd->pos_uptime_x, osd->pos_uptime_y, osd->uptime_rotation);
            IMP_OSD_SetRgnAttr(osdUptm.imp_rgn, &rgnAttr);
        }

        last_updated_second = ltime->tm_sec; // Update the last second tracker
    }
}

void* OSD::updateWrapper(void* arg) {
    OSD* osd = static_cast<OSD*>(arg);
    return osd->update();
}

void* OSD::update()
{
    while(osd->thread_signal.load()){

        updateDisplayEverySecond();
        usleep(1000*1000);
    }

    return nullptr;
}
