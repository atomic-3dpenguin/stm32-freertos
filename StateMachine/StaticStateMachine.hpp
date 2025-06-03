// static_state_machine.hpp (C++17)
#pragma once
#include <iostream>
#include <type_traits>
#include <new>

//----------------------------------------
// MotorController State and Event Definitions
//----------------------------------------
enum class StateID {
    IDLE,
    SELF_TEST,
    AUTOMATED,
    MANUAL,
    ERROR
};

enum class EventID {
    BEGIN_SELF_TEST,
    START_AUTOMATION,
    ENTER_MANUAL,
    FAULT,
    RESET
};

//----------------------------------------
// Forward Declaration
//----------------------------------------
template <typename Derived>
class StaticStateContext;

//----------------------------------------
// State Base CRTP
//----------------------------------------
template <typename DerivedState, typename Context>
class StateBase {
public:
    void onEnter(Context& ctx) { static_cast<DerivedState*>(this)->onEnter(ctx); }
    void onExit(Context& ctx)  { static_cast<DerivedState*>(this)->onExit(ctx); }
    void onUpdate(Context& ctx) { static_cast<DerivedState*>(this)->onUpdate(ctx); }
};

//----------------------------------------
// State Specializations (Override as needed)
//----------------------------------------
template <StateID ID, typename Context>
class ConcreteState;

//----------------------------------------
// Compile-Time Transition Map
//----------------------------------------
template <StateID From, EventID Event>
struct Transition;

// Valid Transitions

template <> struct Transition<StateID::IDLE, EventID::BEGIN_SELF_TEST> {
    static constexpr StateID To = StateID::SELF_TEST;
};

template <> struct Transition<StateID::SELF_TEST, EventID::START_AUTOMATION> {
    static constexpr StateID To = StateID::AUTOMATED;
};

template <> struct Transition<StateID::AUTOMATED, EventID::ENTER_MANUAL> {
    static constexpr StateID To = StateID::MANUAL;
};

template <> struct Transition<StateID::MANUAL, EventID::FAULT> {
    static constexpr StateID To = StateID::ERROR;
};

template <> struct Transition<StateID::ERROR, EventID::RESET> {
    static constexpr StateID To = StateID::IDLE;
};

//----------------------------------------
// Validity Checker (SFINAE friendly)
//----------------------------------------
template <typename T, typename = void>
struct is_valid_transition : std::false_type {};

template <typename T>
struct is_valid_transition<T, std::void_t<decltype(T::To)>> : std::true_type {};

//----------------------------------------
// CRTP Context with Static Storage and Guards
//----------------------------------------
template <typename Derived>
class StaticStateContext {
public:
    using ThisType = Derived;

    template <StateID ID>
    using StateT = ConcreteState<ID, ThisType>;

    StaticStateContext() {
        construct<StateID::IDLE>();
    }

    template <StateID NewID>
    void setState() {
        getActive()->onExit(*static_cast<ThisType*>(this));
        construct<NewID>();
        getActive()->onEnter(*static_cast<ThisType*>(this));
    }

    void update() {
        getActive()->onUpdate(*static_cast<ThisType*>(this));
    }

    StateID currentID() const { return current_id_; }

    // Dispatch using current runtime state and a static event
    template <EventID Ev>
    void dispatchEvent() {
        switch (current_id_) {
            case StateID::IDLE:
                dispatchFrom<StateID::IDLE, Ev>(); break;
            case StateID::SELF_TEST:
                dispatchFrom<StateID::SELF_TEST, Ev>(); break;
            case StateID::AUTOMATED:
                dispatchFrom<StateID::AUTOMATED, Ev>(); break;
            case StateID::MANUAL:
                dispatchFrom<StateID::MANUAL, Ev>(); break;
            case StateID::ERROR:
                dispatchFrom<StateID::ERROR, Ev>(); break;
        }
    }

    template <StateID From, EventID Ev>
    void dispatchFrom() {
        static_assert(is_valid_transition<Transition<From, Ev>>::value, "Invalid transition");
        constexpr StateID To = Transition<From, Ev>::To;

        if (canTransition<From, To>()) {
            setState<To>();
        } else {
            setState<StateID::ERROR>();
        }
    }

    // Entry/exit guards can be overridden in the context class
    template <StateID From, StateID To>
    bool canTransition() const {
        return static_cast<const ThisType*>(this)->template canTransitionImpl<From, To>();
    }

private:
    static constexpr size_t MaxSize = std::max({
        sizeof(StateT<StateID::IDLE>),
        sizeof(StateT<StateID::SELF_TEST>),
        sizeof(StateT<StateID::AUTOMATED>),
        sizeof(StateT<StateID::MANUAL>),
        sizeof(StateT<StateID::ERROR>)
    });

    static constexpr size_t MaxAlign = std::max({
        alignof(StateT<StateID::IDLE>),
        alignof(StateT<StateID::SELF_TEST>),
        alignof(StateT<StateID::AUTOMATED>),
        alignof(StateT<StateID::MANUAL>),
        alignof(StateT<StateID::ERROR>)
    });

    using Storage = std::aligned_storage_t<MaxSize, MaxAlign>;
    Storage storage_;
    StateID current_id_ = StateID::IDLE;

    void* raw() { return &storage_; }

    template <StateID ID>
    void construct() {
        current_id_ = ID;
        new (raw()) StateT<ID>();
    }

    StateBase<void, ThisType>* getActive() {
        return reinterpret_cast<StateBase<void, ThisType>*>(raw());
    }
};
