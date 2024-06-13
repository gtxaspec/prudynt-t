#ifndef MsgChannel_hpp
#define MsgChannel_hpp

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

template <class T> class MsgChannel {
public:
    MsgChannel(unsigned int bsize) : write_ptr(0), read_ptr(0), data_available_callback(nullptr) {
        msg_buffer.resize(bsize);
        buffer_size = bsize;
    }

        bool write(T msg) {
        std::unique_lock<std::mutex> lck(cv_mtx);
        if (can_write()) {
            msg_buffer[write_ptr] = msg;
            increment_write();
            write_cv.notify_all();
            if (data_available_callback) {
                data_available_callback();
            }
            return true;
        }
        return false;
    }

    bool read(T* out) {
        std::unique_lock<std::mutex> lck(cv_mtx);
        if (can_read()) {
            *out = msg_buffer[read_ptr];
            msg_buffer[read_ptr] = T();
            increment_read();
            return true;
        }
        return false;
    }

    T wait_read() {
        std::unique_lock<std::mutex> lck(cv_mtx);
        write_cv.wait(lck, [this] { return can_read(); });
        T val = msg_buffer[read_ptr];
        msg_buffer[read_ptr] = T();
        increment_read();
        return val;
    }

    void set_data_available_callback(std::function<void()> callback) {
        data_available_callback = callback;
    }

private:
    bool can_read() const {
        return write_ptr != read_ptr;
    }

    bool can_write() const {
        return (write_ptr + 1) % buffer_size != read_ptr;
    }

    void increment_read() {
        read_ptr = (read_ptr + 1) % buffer_size;
    }

    void increment_write() {
        write_ptr = (write_ptr + 1) % buffer_size;
    }

    std::vector<T> msg_buffer;
    std::atomic<unsigned int> write_ptr;
    std::atomic<unsigned int> read_ptr;
    std::mutex cv_mtx;
    std::condition_variable write_cv;
    unsigned int buffer_size;
    std::function<void()> data_available_callback;
};

#endif
