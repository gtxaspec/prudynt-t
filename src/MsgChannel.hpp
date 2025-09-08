#ifndef MsgChannel_hpp
#define MsgChannel_hpp

#include <iostream>
#include <deque>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

/* Implementation of the MsgChannel API, except that it keeps
 * the most recent bsize elements in the queue.
 */
template <class T> class MsgChannel {
public:
    MsgChannel(unsigned int bsize) : buffer_size{bsize} { }

    bool write(T msg) {
        std::unique_lock<std::mutex> lck(cv_mtx);
        msg_buffer.push_front(msg);
        if (msg_buffer.size() > buffer_size) {
            msg_buffer.pop_back();
            return false;
        }
        write_cv.notify_all();
        return true;
    }

    bool read(T *out) {
        std::unique_lock<std::mutex> lck(cv_mtx);
        if (can_read()) {
            *out = msg_buffer.back();
            msg_buffer.pop_back();
            return true;
        }
        return false;
    }

    T wait_read() {
        std::unique_lock<std::mutex> lck(cv_mtx);
        while (!can_read()) {
            write_cv.wait(lck);
        };
        T val = msg_buffer.back();
        msg_buffer.pop_back();
        return val;
    }

private:
    bool can_read() {
        return !msg_buffer.empty();
    }

    std::deque<T> msg_buffer;
    std::mutex cv_mtx;
    std::condition_variable write_cv;
    unsigned int buffer_size;
};

#endif
