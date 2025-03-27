// cli_command_base.hpp
#pragma once
#include <cstddef>

// CRTP Command Base

template <typename Derived>
struct CLICommand {
    static void executor(const char* args, char* response, size_t maxLen) {
        Derived::execute(args, response, maxLen);
    }
};