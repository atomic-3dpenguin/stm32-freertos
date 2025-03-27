// main.cpp
#include "system_manager.hpp"
#include <memory>

int main() {
    auto system = std::make_unique<SystemManager>();
    system->init();
    system->run();
}
