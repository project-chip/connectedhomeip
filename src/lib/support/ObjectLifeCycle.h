/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <cstdint>

namespace chip {

/**
 * Track the life cycle of an object.
 *
 * <pre>
 *
 *      Construction
 *          ↓
 *      Uninitialized ‹─┐
 *          ↓           │
 *      Initializing    │
 *          ↓           │
 *      Initialized     │
 *          ↓           │
 *      ShuttingDown    │
 *          ↓           │
 *      Shutdown ───────┘
 *          ↓
 *      Destroyed
 *
 * </pre>
 */
class ObjectLifeCycle
{
public:
    enum class State : uint8_t
    {
        Uninitialized = 0, ///< Pre-initialized state.
        Initializing  = 1, ///< State during intialization.
        Initialized   = 2, ///< Initialized (active) state.
        ShuttingDown  = 3, ///< State during shutdown.
        Shutdown      = 4, ///< Post-shutdown state.
        Destroyed     = 5, ///< Post-destructor state.
    };

    ObjectLifeCycle() : mState(State::Uninitialized) {}
    ~ObjectLifeCycle() { mState = State::Destroyed; }

    /**
     * @returns true if and only if the object is in the Initialized state.
     */
    bool IsInitialized() const { return mState == State::Initialized; }

    /*
     * State transitions.
     *
     * Typical use is `VerifyOrReturnError(state.SetInitializing(), CHIP_ERROR_INCORRECT_STATE)`; these functions return `bool`
     * rather than a `CHIP_ERROR` so that error source tracking will record the call point rather than this function itself.
     */
    bool SetInitializing() { return Transition(State::Uninitialized, State::Initializing); }
    bool SetInitialized() { return Transition(State::Initializing, State::Initialized); }
    bool SetShuttingDown() { return Transition(State::Initialized, State::ShuttingDown); }
    bool SetShutdown() { return Transition(State::ShuttingDown, State::Shutdown); }
    bool Reset() { return Transition(State::Shutdown, State::Uninitialized); }

    // Skip steps when a class's shutdown code has nothing useful to do in between.
    bool ResetFromShuttingDown() { return Transition(State::ShuttingDown, State::Uninitialized); }
    bool ResetFromInitialized() { return Transition(State::Initialized, State::Uninitialized); }

    /**
     * Transition from Uninitialized or Shutdown to Destroyed.
     *
     * Typical use is `VerifyOrReturnError(state.Destroy(), CHIP_ERROR_INCORRECT_STATE)`; this function returns `bool` rather than
     * a `CHIP_ERROR` so that error source tracking will record the call point rather than this function itself.
     *
     * @return true     if the state was Uninitialized or Shutdown and is now Destroyed.
     * @return false    otherwise.
     */
    bool Destroy()
    {
        if (mState == State::Uninitialized || mState == State::Shutdown)
        {
            mState = State::Destroyed;
            return true;
        }
        return false;
    }

    State GetState() const { return mState; }

private:
    bool Transition(State from, State to)
    {
        if (mState == from)
        {
            mState = to;
            return true;
        }
        return false;
    }

    State mState;
};

} // namespace chip
