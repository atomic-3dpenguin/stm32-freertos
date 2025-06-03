// test_static_state_machine.cpp
#include "static_state_machine.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <string>

//----------------------------------------
// Output Capture for Tests
//----------------------------------------
class OutputCapture {
public:
    static std::stringstream& stream() {
        static std::stringstream ss;
        return ss;
    }
    static void clear() {
        stream().str("");
        stream().clear();
    }
    static std::string str() {
        return stream().str();
    }
};

//----------------------------------------
// MotorControllerContext and States
//----------------------------------------
class MotorControllerContext;

template <>
class ConcreteState<StateID::IDLE, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::IDLE, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { OutputCapture::stream() << "Enter IDLE\n"; }
    void onExit(MotorControllerContext&)  { OutputCapture::stream() << "Exit IDLE\n"; }
    void onUpdate(MotorControllerContext&) { OutputCapture::stream() << "Update IDLE\n"; }
};

template <>
class ConcreteState<StateID::SELF_TEST, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::SELF_TEST, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { OutputCapture::stream() << "Enter SELF_TEST\n"; }
    void onExit(MotorControllerContext&)  { OutputCapture::stream() << "Exit SELF_TEST\n"; }
    void onUpdate(MotorControllerContext&) { OutputCapture::stream() << "Update SELF_TEST\n"; }
};

template <>
class ConcreteState<StateID::AUTOMATED, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::AUTOMATED, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { OutputCapture::stream() << "Enter AUTOMATED\n"; }
    void onExit(MotorControllerContext&)  { OutputCapture::stream() << "Exit AUTOMATED\n"; }
    void onUpdate(MotorControllerContext&) { OutputCapture::stream() << "Update AUTOMATED\n"; }
};

template <>
class ConcreteState<StateID::MANUAL, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::MANUAL, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { OutputCapture::stream() << "Enter MANUAL\n"; }
    void onExit(MotorControllerContext&)  { OutputCapture::stream() << "Exit MANUAL\n"; }
    void onUpdate(MotorControllerContext&) { OutputCapture::stream() << "Update MANUAL\n"; }
};

template <>
class ConcreteState<StateID::ERROR, MotorControllerContext> :
    public StateBase<ConcreteState<StateID::ERROR, MotorControllerContext>, MotorControllerContext> {
public:
    void onEnter(MotorControllerContext&) { OutputCapture::stream() << "Enter ERROR\n"; }
    void onExit(MotorControllerContext&)  { OutputCapture::stream() << "Exit ERROR\n"; }
    void onUpdate(MotorControllerContext&) { OutputCapture::stream() << "Update ERROR\n"; }
};

class MotorControllerContext : public StaticStateContext<MotorControllerContext> {
public:
    template <StateID From, StateID To>
    bool canTransitionImpl() const {
        if constexpr (From == StateID::SELF_TEST && To == StateID::AUTOMATED)
            return false;
        return true;
    }
};

//----------------------------------------
// Google Test Cases
//----------------------------------------

TEST(MotorControllerStateMachineTest, StartsInIdle) {
    OutputCapture::clear();
    MotorControllerContext ctx;
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::IDLE);
    EXPECT_EQ(OutputCapture::str(), "Update IDLE\n");
}

TEST(MotorControllerStateMachineTest, ValidTransitionsWithGuardFailureToError) {
    OutputCapture::clear();
    MotorControllerContext ctx;
    ctx.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::SELF_TEST);

    // Guard prevents SELF_TEST -> AUTOMATED
    ctx.dispatchEvent<EventID::START_AUTOMATION>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::ERROR);

    std::string out = OutputCapture::str();
    EXPECT_NE(out.find("Enter SELF_TEST"), std::string::npos);
    EXPECT_NE(out.find("Enter ERROR"), std::string::npos);
}

TEST(MotorControllerStateMachineTest, FullValidPathToManual) {
    OutputCapture::clear();
    MotorControllerContext ctx;

    ctx.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::SELF_TEST);

    // Skip the blocked transition and go to error
    ctx.dispatchEvent<EventID::START_AUTOMATION>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::ERROR);

    ctx.dispatchEvent<EventID::RESET>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::IDLE);

    // New valid path
    ctx.dispatchEvent<EventID::BEGIN_SELF_TEST>();
    ctx.update();
    ctx.setState<StateID::AUTOMATED>(); // Bypass guard manually for test
    ctx.dispatchEvent<EventID::ENTER_MANUAL>();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::MANUAL);

    std::string out = OutputCapture::str();
    EXPECT_NE(out.find("Enter MANUAL"), std::string::npos);
    EXPECT_NE(out.find("Enter IDLE"), std::string::npos);
    EXPECT_NE(out.find("Enter ERROR"), std::string::npos);
}

TEST(MotorControllerStateMachineTest, HandlesMultipleUpdatesPerState) {
    OutputCapture::clear();
    MotorControllerContext ctx;
    ctx.update();
    ctx.update();
    ctx.update();
    EXPECT_EQ(ctx.currentID(), StateID::IDLE);
    std::string out = OutputCapture::str();
    EXPECT_EQ(std::count(out.begin(), out.end(), '\n'), 3);
}
