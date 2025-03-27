// system_manager.hpp
#pragma once

#include "logger_task.hpp"
#include "cli_task.hpp"
#include "cli_commands.hpp"
#include "fdcan_tasks.hpp"
#include <memory>

class SystemManager {
public:
    void init();
    void run();

private:
    void registerCLICommands();

    std::unique_ptr<LoggerTask> logger;
    std::unique_ptr<CLITask> cli;
    std::unique_ptr<FDCANTask> fdcan1Task;
    std::unique_ptr<FDCANTask> fdcan2Task;
};
