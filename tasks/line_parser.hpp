// line_parser.hpp
#pragma once

#include <array>
#include <cstddef>
#include <cstring>

/**
 * @brief Utility class for parsing complete lines from a stream buffer.
 */
class LineParser {
public:
    static constexpr std::size_t MaxSize = 256;

    LineParser() = default;

    /**
     * @brief Feed a character into the parser.
     * @return true if a complete line is ready
     */
    bool push(char c) {
        if (c == '\r' || c == '\n') {
            buffer[size] = '\0';
            size = 0;
            ready = true;
            return true;
        }
        if (size < buffer.size() - 1) {
            buffer[size++] = c;
        }
        return false;
    }

    /**
     * @brief Check if a line is ready.
     */
    bool hasLine() const {
        return ready;
    }

    /**
     * @brief Retrieve the parsed line.
     */
    const char* getLine() {
        ready = false;
        return buffer.data();
    }

private:
    std::array<char, MaxSize> buffer{};
    std::size_t size = 0;
    bool ready = false;
};
