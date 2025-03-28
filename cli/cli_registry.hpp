// cli_registry.hpp
#pragma once

#include <functional>
#include <unordered_map>
#include <string_view>
#include <string>

/**
 * @brief CLI command registry to associate strings with command functions.
 */
class CLIRegistry {
public:
    using CommandHandler = std::function<std::string(std::string_view)>;

    /**
     * @brief Register a command string with a callback handler.
     *
     * @param name Command name (e.g. "echo")
     * @param handler Function that takes input args and returns a response
     */
    void registerCommand(std::string_view name, CommandHandler handler) {
        commands[std::string(name)] = std::move(handler);
    }

    /**
     * @brief Execute a command by parsing the full input line.
     *
     * @param line Full line of input (e.g. "echo hello")
     * @return Result string to send back over UART
     */
    std::string execute(std::string_view line) const {
        const auto space = line.find(' ');
        const auto name = line.substr(0, space);
        const auto args = space == std::string_view::npos ? std::string_view{} : line.substr(space + 1);

        if (auto it = commands.find(std::string(name)); it != commands.end()) {
            return it->second(args);
        } else {
            return "Unknown command: " + std::string(name) + "\r\n";
        }
    }

private:
    std::unordered_map<std::string, CommandHandler> commands;
};
