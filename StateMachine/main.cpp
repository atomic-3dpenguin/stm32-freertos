#include "static_state_machine.hpp"
#include <iostream>

// MotorControllerContext forward declaration
class MotorControllerContext;

// Define concrete states
template <> class ConcreteState<StateID::IDLE, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::IDLE, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "→ IDLE\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "← IDLE\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "[IDLE]...\n"; }
};

template <> class ConcreteState<StateID::SELF_TEST, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::SELF_TEST, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "→ SELF_TEST\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "← SELF_TEST\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "[SELF_TEST] running...\n"; }
};

template <> class ConcreteState<StateID::AUTOMATED, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::AUTOMATED, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "→ AUTOMATED\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "← AUTOMATED\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "[AUTOMATED] active...\n"; }
};

template <> class ConcreteState<StateID::MANUAL, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::MANUAL, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "→ MANUAL\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "← MANUAL\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "[MANUAL] ready...\n"; }
};

template <> class ConcreteState<StateID::ERROR, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::ERROR, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "!!! ERROR STATE ENTERED\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Recovering from ERROR...\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "[ERROR] fault handling\n"; }
};

// Context with guard logic
class MotorControllerContext : public StaticStateContext<MotorControllerContext> {
public:
    template <StateID From, StateID To>
    bool canTransitionImpl() const {
        // Block SELF_TEST → AUTOMATED
        if constexpr (From == StateID::SELF_TEST && To == StateID::AUTOMATED)
            return false;
        return true;
    }
};

int main() {
    MotorControllerContext motor;

    std::cout << "--- START FSM ---\n";
    motor.update();

    std::cout << "\n>> BEGIN_SELF_TEST\n";
    motor.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    motor.update();

    std::cout << "\n>> START_AUTOMATION (Blocked by guard)\n";
    motor.dispatchEvent<EventID::START_AUTOMATION>();
    motor.update();

    std::cout << "\n>> RESET from ERROR\n";
    motor.dispatchEvent<EventID::RESET>();
    motor.update();

    std::cout << "\n>> Valid Path: SELF_TEST → AUTOMATED → MANUAL (manual override)\n";
    motor.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    motor.setState<StateID::AUTOMATED>();  // manual override (e.g. calibration complete)
    motor.dispatchEvent<EventID::ENTER_MANUAL>();
    motor.update();

    std::cout << "--- END FSM ---\n";
    return 0;
}



// FreeRTOS Task Notification example
// void MotorTask(void* arg) {
//     MotorControllerContext fsm;

//     for (;;) {
//         EventBits_t events = xEventGroupWaitBits(...);

//         if (events & EVENT_SELF_TEST_START) {
//             fsm.dispatchEvent<EventID::BEGIN_SELF_TEST>();
//         } else if (events & EVENT_FAULT) {
//             fsm.dispatchEvent<EventID::FAULT>();
//         }

//         fsm.update(); // call periodically or conditionally
//     }
// }
