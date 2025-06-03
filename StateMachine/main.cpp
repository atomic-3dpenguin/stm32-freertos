#include "static_state_machine.hpp"
#include <iostream>

// Declare MotorControllerContext forward for state specializations
class MotorControllerContext;

// Basic state behavior printing to console
template <> class ConcreteState<StateID::IDLE, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::IDLE, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "Enter IDLE\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Exit IDLE\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "Update IDLE\n"; }
};

template <> class ConcreteState<StateID::SELF_TEST, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::SELF_TEST, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "Enter SELF_TEST\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Exit SELF_TEST\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "Update SELF_TEST\n"; }
};

template <> class ConcreteState<StateID::AUTOMATED, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::AUTOMATED, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "Enter AUTOMATED\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Exit AUTOMATED\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "Update AUTOMATED\n"; }
};

template <> class ConcreteState<StateID::MANUAL, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::MANUAL, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "Enter MANUAL\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Exit MANUAL\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "Update MANUAL\n"; }
};

template <> class ConcreteState<StateID::ERROR, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::ERROR, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { std::cout << "Enter ERROR\n"; }
    void onExit(MotorControllerContext&)  { std::cout << "Exit ERROR\n"; }
    void onUpdate(MotorControllerContext&) { std::cout << "Update ERROR\n"; }
};

// Define the context
class MotorControllerContext : public StaticStateContext<MotorControllerContext> {};

int main() {
    MotorControllerContext motor;

    motor.update();
    motor.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    motor.update();
    motor.dispatchEvent<EventID::START_AUTOMATION>();
    motor.update();
    motor.dispatchEvent<EventID::ENTER_MANUAL>();
    motor.update();
    motor.dispatchEvent<EventID::FAULT>();
    motor.update();
    motor.dispatchEvent<EventID::RESET>();
    motor.update();

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
