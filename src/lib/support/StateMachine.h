/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 SmartThings
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <lib/core/Optional.h>
#include <lib/support/Variant.h>

namespace chip {
namespace StateMachine {

/**
 * An extension of the Optional class that removes the explicit requirement
 * for construction from a T value as a convenience to allow auto construction
 * of Optional<T>.
 */
template <class T>
class Optional : public chip::Optional<T>
{
public:
    Optional(const T & value) : chip::Optional<T>(value) {}
    Optional() : chip::Optional<T>() {}
};

/**
 * An extension of the Variant class offering pattern matching of State types
 * to dynamically dispatch execution of the required State interface methods:
 * Enter, Exit, GetName, LogTtransition.
 */
template <typename... Ts>
struct VariantState : Variant<Ts...>
{

private:
    template <typename T>
    void Enter(bool & ever)
    {
        if (!ever && chip::Variant<Ts...>::template Is<T>())
        {
            ever = true;
            chip::Variant<Ts...>::template Get<T>().Enter();
        }
    }

    template <typename T>
    void Exit(bool & ever)
    {
        if (!ever && chip::Variant<Ts...>::template Is<T>())
        {
            ever = true;
            chip::Variant<Ts...>::template Get<T>().Exit();
        }
    }

    template <typename T>
    void GetName(const char ** name)
    {
        if (name && !*name && chip::Variant<Ts...>::template Is<T>())
        {
            *name = chip::Variant<Ts...>::template Get<T>().GetName();
        }
    }

    template <typename T>
    void LogTransition(bool & ever, const char * previous)
    {
        if (!ever && chip::Variant<Ts...>::template Is<T>())
        {
            ever = true;
            chip::Variant<Ts...>::template Get<T>().LogTransition(previous);
        }
    }

public:
    template <typename T, typename... Args>
    static VariantState<Ts...> Create(Args &&... args)
    {
        VariantState<Ts...> instance;
        instance.template Set<T>(std::forward<Args>(args)...);
        return instance;
    }

    void Enter()
    {
        bool ever = false;
        [](...) {}((this->template Enter<Ts>(ever), 0)...);
    }

    void Exit()
    {
        bool ever = false;
        [](...) {}((this->template Exit<Ts>(ever), 0)...);
    }

    const char * GetName()
    {
        const char * name = nullptr;
        [](...) {}((this->template GetName<Ts>(&name), 0)...);
        return name;
    }

    void LogTransition(const char * previous)
    {
        bool ever = false;
        [](...) {}((this->template LogTransition<Ts>(ever, previous), 0)...);
    }
};

/**
 * The interface for dispatching events into the State Machine.
 * @tparam TEvent a variant holding the Events for the State Machine.
 */
template <typename TEvent>
class Context
{
public:
    virtual ~Context() = default;

    /**
     * Dispatch an event to the current state.
     * @param evt a variant holding an Event for the State Machine.
     */
    virtual void Dispatch(const TEvent & evt) = 0;
};

/**
 * This is a functional approach to the State Machine design pattern.  The design is
 * borrowed from http://www.vishalchovatiya.com/state-design-pattern-in-modern-cpp
 * and extended for this application.
 *
 * At a high-level, the purpose of a State Machine is to switch between States.  Each
 * State handles Events.  The handling of Events may lead to Transitions.  The purpose
 * of this design pattern is to decouple States, Events, and Transitions.  For instance,
 * it is desirable to remove knowledge of next/previous States from each individual
 * State.  This allows adding/removing States with minimal code change and leads to a
 * simpler implementation.
 *
 * This State Machine design emulates C++17 features to achieve the functional approach.
 * Instead of using an enum or inheritance for the Events, the Events are defined as
 * structs and placed in a variant.  Likewise, the States are all defined as structs and
 * placed in a variant.  With the Events and States in two different variants, the
 * Transitions table uses the type introspction feature of the variant object to match a
 * given state and event to an optional new-state return.
 *
 * For event dispatch, the State Machine implements the Context interface.  The Context
 * interface is passed to States to allow Dispatch() of events when needed.
 *
 * The State held in the TState must provide four methods to support calls from
 * the State Machine:
 * @code
 *   struct State {
 *     void Enter() { }
 *     void Exit() { }
 *     void LogTransition(const char *) { }
 *     const char *GetName() { return ""; }
 *   }
 * @endcode
 *
 * The TTransitions table type is implemented with an overloaded callable operator method
 * to match the combinations of State / Event variants that may produce a new-state return.
 * This allows the Transition table to define how each State responds to Events.  Below is
 * an example of a Transitions table implemented as a struct:
 *
 * @code
 *   struct Transitions {
 *     using State = chip::StateMachine::VariantState<State1, State2>;
 *     chip::StateMachine::Optional<State> operator()(State &state, Event &event)
 *     {
 *         if (state.Is<State1>() && event.Is<Event2>())
 *         {
 *             return State::Create<State2>();
 *         }
 *         else if (state.Is<State2>() && event.Is<Event1>())
 *         {
 *             return State::Create<State1>();
 *         }
 *         else
 *         {
 *             return {}
 *         }
 *     }
 *   }
 * @endcode
 *
 * The rules for calling Dispatch from within the state machien are as follows:
 *
 * (1) Only the State::Enter method should call Dispatch.  Calls from Exit or
 *     LogTransition will cause an abort.
 * (2) The transitions table may return a new state OR call Dispatch, but must
 *     never do both.  Doing both will cause an abort.
 *
 * @tparam TState a variant holding the States.
 * @tparam TEvent a variant holding the Events.
 * @tparam TTransitions an object that implements the () operator for transitions.
 */
template <typename TState, typename TEvent, typename TTransitions>
class StateMachine : public Context<TEvent>
{
public:
    StateMachine(TTransitions & tr) : mCurrentState(tr.GetInitState()), mTransitions(tr), mSequence(0) {}
    ~StateMachine() override = default;
    void Dispatch(const TEvent & evt) override
    {
        ++mSequence;
        auto prev     = mSequence;
        auto newState = mTransitions(mCurrentState, evt);
        if (newState.HasValue())
        {
            auto oldState = mCurrentState.GetName();
            mCurrentState.Exit();
            mCurrentState = newState.Value();
            mCurrentState.LogTransition(oldState);
            // It is impermissible to dispatch events from Exit() or
            // LogTransition(), or from the transitions table when a transition
            // has also been returned.  Verify that this hasn't occurred.
            VerifyOrDie(prev == mSequence);
            mCurrentState.Enter();
        }
    }
    TState GetState() { return mCurrentState; }

private:
    TState mCurrentState;
    TTransitions & mTransitions;
    unsigned mSequence;
};

} // namespace StateMachine
} // namespace chip
