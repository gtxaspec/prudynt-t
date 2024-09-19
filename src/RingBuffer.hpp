#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

class RingBuffer
{
public:
    RingBuffer(size_t capacity)
        : buffer(new uint8_t[capacity]), capacity(capacity), head(0), tail(0), size(0)
    {}

    ~RingBuffer()
    {
        delete[] buffer;
    }

    void push(const uint8_t* data, size_t dataSize)
    {
        if (dataSize > capacity - size)
        {
            throw std::overflow_error("Ring buffer overflow");
        }

        size_t spaceAtEnd = capacity - tail;
        if (dataSize <= spaceAtEnd)
        {
            // If we can fit the data without wrapping
            std::memcpy(buffer + tail, data, dataSize);
        }
        else
        {
            // If the data wraps around the buffer
            std::memcpy(buffer + tail, data, spaceAtEnd);
            std::memcpy(buffer, data + spaceAtEnd, dataSize - spaceAtEnd);
        }

        tail = (tail + dataSize) % capacity;
        size += dataSize;
    }

    void fetch(uint8_t* output, size_t count)
    {
        if (count > size)
        {
            throw std::underflow_error("Ring buffer underflow");
        }

        size_t spaceAtEnd = capacity - head;
        if (count <= spaceAtEnd)
        {
            // If the data can be copied without wrapping
            std::memcpy(output, buffer + head, count);
        }
        else
        {
            // If the data wraps around the buffer
            std::memcpy(output, buffer + head, spaceAtEnd);
            std::memcpy(output + spaceAtEnd, buffer, count - spaceAtEnd);
        }

        head = (head + count) % capacity;
        size -= count;
    }

    bool isEmpty() const { return size == 0; }
    size_t getSize() const { return size; }

private:
    uint8_t* buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
};
