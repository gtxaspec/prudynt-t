#pragma once

#include <string>

class Config {
public:
    static Config* singleton();
private:
    Config();
    void loadDefaults();
    bool validateConfig();

public:
    std::string sensorModel;
    int sensorI2Caddress;
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

    std::string rtspUsername;
    std::string rtspPassword;
    std::string rtspName;
    int rtspPort;
    bool rtspAuthRequired;

    std::string OSDFontPath;
    std::string OSDFormat;
    int OSDFontSize;
    int OSDFontStrokeSize;
    int OSDEnabled;
private:
    static Config* instance;
};
