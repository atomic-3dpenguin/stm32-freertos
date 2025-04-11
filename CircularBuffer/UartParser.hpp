#ifndef UART_PARSER_HPP
#define UART_PARSER_HPP

#include "CircularBuffer.hpp"
#include <string>
#include <cstdint>
#include <cstddef>

extern void appendTerminalText(const std::string& text);
extern void handleCommand(const std::string& cmd);

// UartParser uses a circular buffer to store incoming characters.
// It continuously extracts characters from the buffer, echoes them to the terminal,
// and accumulates text until it finds a newline ('\n' or '\r').
// When a newline is detected, it outputs a new line and calls handleCommand() with the complete command.
class UartParser {
public:
    UartParser(): commandBuffer(""){}

    // Called by the DMA callback to push received data into the ring buffer.
    void pushData(const uint8_t* data, std::size_t length){
        for (std::size_t i = 0; i < length; ++i) {
            // Push each received byte into the ring buffer.
            ringBuffer.push(static_cast<char>(data[i]));
        }
    }

    // Processes data available in the ring buffer.
    // This method extracts characters, echoes them, and accumulates a command.
    // When a newline is encountered, it calls handleCommand() and clears the command buffer.
    void process(){
        char ch = '\0';
        // Process all available characters in the ring buffer.
        while (!ringBuffer.empty()) {
            if (!ringBuffer.pop(ch)) {
                break;
            }
            // Check for newline characters.
            if (ch == '\n' || ch == '\r') {
                appendTerminalText("\r\n");
                if (!commandBuffer.empty()) {
                    handleCommand(commandBuffer);
                    commandBuffer.clear();
                }
            } else {
                // Echo the character and accumulate it.
                commandBuffer.push_back(ch);
                appendTerminalText(std::string(1, ch));
            }
        }
    }
    
private:
    static constexpr std::size_t BUFFER_SIZE = 128;
    CircularBuffer<char, BUFFER_SIZE> ringBuffer;
    std::string commandBuffer;
};

#endif // UART_PARSER_HPP
