#ifndef Motion_hpp
#define Motion_hpp

#include <memory>
#include <thread>
#include <atomic>
#include "imp/imp_ivs.h"
#include "imp/imp_ivs_move.h"
#include "Config.hpp"
#include "Logger.hpp"
#include <imp/imp_system.h>

class Motion {
    public:
        static void detect_start(Motion *m);
        void detect();
        void run();
        bool init(std::shared_ptr<CFG> _cfg);
        bool exit();

    private:
        std::atomic<bool> moving;
        std::atomic<bool> indicator;    
        std::shared_ptr<CFG> cfg;
        IMP_IVS_MoveParam move_param;
        IMPIVSInterface *move_intf;
        std::thread detect_thread;

        IMPCell fs = { DEV_ID_FS, 0, 1 };
        IMPCell ivs_cell = { DEV_ID_IVS, 0, 0 };    
};

#endif /* Motion_hpp */
