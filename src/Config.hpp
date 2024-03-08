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

    std::string stream0format;
    int stream0fps;
    int stream0gop;
    int stream0buffers;
    int stream0height;
    int stream0width;
    int stream0bitrate;
    int stream0osdPosWidth;
    int stream0osdPosHeight;
    int stream0osdUserTextPosWidth;
    int stream0osdUserTextPosHeight;

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
    int OSDFontSize;
    int OSDFontStrokeSize;
    bool OSDEnabled;
    unsigned int OSDFontColor;
    unsigned int OSDFontStrokeColor;
    bool OSDFontStrokeEnable;
    bool OSDUserTextEnable;
    std::string OSDUserTextString;

    std::string LogLevel;
private:
    // Holds the singleton instance
    static Config* instance;
};
