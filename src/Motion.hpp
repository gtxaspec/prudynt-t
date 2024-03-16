#ifndef Motion_hpp
#define Motion_hpp

#include <memory>
#include <thread>
#include <atomic>
#include "imp/imp_ivs.h"
#include "imp/imp_ivs_move.h"
#include "Encoder.hpp"

class Motion {
public:
    static void detect_start(Motion *m);
    void detect();
    void run();
    bool init();

public:
    static std::atomic<bool> moving;
    static std::atomic<bool> indicator;

private:
    IMP_IVS_MoveParam move_param;
    IMPIVSInterface *move_intf;
    static std::thread detect_thread;
};

#endif /* Motion_hpp */
