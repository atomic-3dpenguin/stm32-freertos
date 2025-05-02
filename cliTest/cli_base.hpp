/*
 * cli_base.hpp
 *
 *  Created on: May 1, 2025
 *      Author: cavem
 */

#ifndef CLI_BASE_HPP_
#define CLI_BASE_HPP_
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <array>
#include "cmsis_os2.h"
#include "cli_common.hpp"

namespace cli {

// CRTP base for all transports
// Derived must implement getCharImpl() and putCharImpl(char)
template<typename Derived, size_t MAX_SUBSYS = 8>
class CLIBase {
public:
    using Handler = std::function<void(const std::vector<std::string>&)>;

    explicit CLIBase(const std::array<osEventFlagsId_t, MAX_SUBSYS>& events)
      : events_(events) {}

    void registerCommand(CommandId id, const std::string& name, Handler h) {
        cmds_[name] = { id, h };
    }

    void listCommands() {
        for (auto& kv : cmds_) {
            write((kv.first + "\r\n").c_str());
        }
    }

    std::string readLine() {
        std::string line;
        while (true) {
            char c = static_cast<Derived*>(this)->getCharImpl();
            if (c=='\r'||c=='\n') {
                putCharImpl('\r'); putCharImpl('\n');
                break;
            }
            if (c=='\b'||c==0x7F) {
                if (!line.empty()) {
                    line.pop_back();
                    putCharImpl('\b'); putCharImpl(' '); putCharImpl('\b');
                }
                continue;
            }
            line.push_back(c);
            putCharImpl(c);
        }
        return line;
    }

    void run() {
        stop_ = false;
        while (!stop_) {
            auto tokens = tokenize(readLine());
            if (tokens.empty()) continue;

            auto it = cmds_.find(tokens[0]);
            if (it!=cmds_.end()) {
                auto [id,h] = it->second;
                h(tokens);
                uint8_t ss = static_cast<uint8_t>((id & SUBSYS_MASK)>>SUBSYS_SHIFT);
                if (ss<events_.size() && events_[ss])
                    osEventFlagsSet(events_[ss], id);
            } else {
                write("Error: Unknown command\r\n");
            }
        }
    }

    void stop() { stop_ = true; }

protected:
    void write(const char* s) {
        for (; *s; ++s)
            static_cast<Derived*>(this)->putCharImpl(*s);
    }

private:
    std::vector<std::string> tokenize(const std::string& l) {
        std::istringstream iss(l);
        std::vector<std::string> t;
        std::string x;
        while (iss>>x) t.push_back(x);
        return t;
    }

    std::unordered_map<std::string,std::pair<CommandId,Handler>> cmds_;
    std::array<osEventFlagsId_t, MAX_SUBSYS> events_;
    bool stop_{false};
};

} // namespace cli
#endif /* CLI_BASE_HPP_ */
