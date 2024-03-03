#pragma once

#include <string>

enum NightMode {
    NIGHT_MODE_SUN_TRACK
};

class Config {
public:
    static Config* singleton();
private:
    Config();
    void loadDefaults();
    bool validateConfig();

public:
    int streamFps;
    int sensorI2Caddress;
    int sensorFps;
    int stream0buffers;
    int stream0height;
    int stream0width;
    int stream0bitrate;
    int stream0osdPosWidth;
    int stream0osdPosHeight;
    std::string OSDFontPath;
    int OSDFontSize;
    int OSDFontStrokeSize;
    std::string sensorModel;
    bool nightEnabled;
    bool nightInfrared;
    bool nightColor;
    std::string nightModeString;
    NightMode nightMode;
    double sunTrackLatitude;
    double sunTrackLongitude;
    std::string rtspUsername;
    std::string rtspPassword;
    std::string rtspName;
    int rtspPort;
    int OSDEnabled;
    bool rtspAuthRequired;
    bool motionEnabled;
    int motionPreTime;
    int motionPostTime;
    int motionSensitivity;
    int motionDebounce;
    bool motionStrictIDR;
    bool cvrEnabled;
    int cvrRotateTime;
private:
    static Config* instance;
};
