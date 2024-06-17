#ifndef RTSP_hpp
#define RTSP_hpp

#include <memory>

#include "Encoder.hpp"
#include "Logger.hpp"
#include <queue>

class RTSP {
public:
    RTSP(std::shared_ptr<CFG> _cfg) : cfg(_cfg) {};
    void run();

private:
    std::shared_ptr<CFG> cfg;


};

#endif
