#pragma once

#include <string>

class Config {
public:
    // We only need one instance of Config
    static Config* singleton();

private:
    // Private constructor to prevent instantiation
    Config();
public:
    // Public configuration settings
    std::string sensorModel;
    unsigned int sensorI2Caddress;
    int sensorFps;
    int sensorHeight;
    int sensorWidth;

    std::string stream0format;
    int stream0fps;
    int stream0gop;
    int stream0maxGop;
    int stream0buffers;
    int stream0height;
    int stream0width;
    int stream0bitrate;
    int stream0osdPosX;
    int stream0osdPosY;
    int stream0osdUserTextPosX;
    int stream0osdUserTextPosY;
    int stream0osdUptimeStampPosX;
    int stream0osdUptimeStampPosY;
    int stream0osdLogoPosX;
    int stream0osdLogoPosY;
    std::string stream0jpegPath;
    bool stream0jpegEnable;
    int stream0jpegQuality;
    int stream0jpegRefresh;
    int motionDebounce;
    int motionPostTime;
    int motionCooldownTime;
    std::string motionScriptPath;
    int motionInitTime;
    int motionSensitivity;
    int motionSkipFrameCnt;
    int motionFrameWidth;
    int motionFrameHeight;
    int motionRoi0X;
    int motionRoi0Y;
    int motionRoi1X;
    int motionRoi1Y;
    int roiCnt;
    bool motionEnable;

    std::string rtspUsername;
    std::string rtspPassword;
    std::string rtspName;
    int rtspPort;
    bool rtspAuthRequired;
    int rtspEstBitRate;
    int rtspOutBufferSize;
    int rtspSendBufferSize;

    std::string OSDFontPath;
    std::string OSDFormat;
    std::string OSDUptimeFormat;
    std::string OSDLogoPath;
    int OSDFontSize;
    int OSDFontStrokeSize;
    int OSDLogoWidth;
    int OSDLogoHeight;
    bool OSDEnable;
    unsigned int OSDFontColor;
    unsigned int OSDFontStrokeColor;
    bool OSDFontStrokeEnable;
    bool OSDUserTextEnable;
    std::string OSDUserTextString;
    bool OSDUptimeEnable;
    std::string logLevel;
    bool OSDLogoEnable;
    bool OSDTimeEnable;
private:
    // Holds the singleton instance
    static Config* instance;
};
