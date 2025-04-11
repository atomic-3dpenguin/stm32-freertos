#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <array>
#include <cstddef>

template <typename T, std::size_t Size>
class CircularBuffer {
public:
    CircularBuffer() : head(0), tail(0), full(false) {}

    // Push an item into the ring buffer.
    // Overwrites the oldest data if the buffer is full.
    void push(T item) {
        buffer[head] = item;
        if(full) {
            tail = (tail + 1) % Size; // Overwrite oldest element.
        }
        head = (head + 1) % Size;
        full = (head == tail);
    }

    // Try to pop an item from the ring buffer.
    // Returns false if the buffer is empty.
    bool pop(T &item) {
        if(empty()) {
            return false;
        }
        item = buffer[tail];
        full = false;
        tail = (tail + 1) % Size;
        return true;
    }

    // Check if the ring buffer is empty.
    bool empty() const {
        return (!full && (head == tail));
    }

    // Clear the ring buffer.
    void clear() {
        head = tail;
        full = false;
    }

    // Return the number of stored items.
    std::size_t size() const {
        if(full)
            return Size;
        if(head >= tail)
            return head - tail;
        return Size + head - tail;
    }

private:
    std::array<T, Size> buffer;
    std::size_t head;
    std::size_t tail;
    bool full;
};

#endif // CIRCULAR_BUFFER_HPP
