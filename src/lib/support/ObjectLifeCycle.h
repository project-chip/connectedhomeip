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
 *                  ┌───────────────┐  Init  ┌─────────────┐  Shutdown  ┌──────────┐  Destroy  ┌───────────┐
 * Construction ───>│ Uninitialized ├───────>│ Initialized ├───────────>│ Shutdown ├──────────>│ Destroyed │
 *                  └──────────┬────┘        └─────────────┘            └─────┬────┘           └───────────┘
 *                      ^      │                               Reset          │
 *                      └──────┴<─────────────────────────────────────────────┘
 * </pre>
 */
struct ObjectLifeCycle
{
    /**
     * @returns true if and only if the object is in the Initialized state.
     */
    bool IsInitialized() const { return mState == State::Initialized; }

    /**
     * Transition from Uninitialized to Initialized.
     *
     * Typical use is `VerifyOrReturnError(state.Init(), CHIP_ERROR_INCORRECT_STATE)`; this function returns `bool` rather than
     * a `CHIP_ERROR` so that error source tracking will record the call point rather than this function itself.
     *
     * @return true     if the state was Uninitialized and is now Initialized.
     * @return false    otherwise.
     */
    bool Init()
    {
        if (mState == State::Uninitialized)
        {
            mState = State::Initialized;
            return true;
        }
        return false;
    }

    /**
     * Transition from Initialized to Shutdown.
     *
     * Typical use is `VerifyOrReturnError(state.Shutdown(), CHIP_ERROR_INCORRECT_STATE)`; this function returns `bool` rather than
     * a `CHIP_ERROR` so that error source tracking will record the call point rather than this function itself.
     *
     * @return true     if the state was Initialized and is now Shutdown.
     * @return false    otherwise.
     */
    bool Shutdown()
    {
        if (mState == State::Initialized)
        {
            mState = State::Shutdown;
            return true;
        }
        return false;
    }

    /**
     * Transition from Shutdown back to Uninitialized, or remain Uninitialized.
     *
     * Typical use is `VerifyOrReturnError(state.Reset(), CHIP_ERROR_INCORRECT_STATE)`; this function returns `bool` rather than
     * a `CHIP_ERROR` so that error source tracking will record the call point rather than this function itself.
     *
     * @return true     if the state was Uninitialized or Shutdown and is now Uninitialized.
     * @return false    otherwise.
     */
    bool Reset()
    {
        if (mState == State::Shutdown)
        {
            mState = State::Uninitialized;
        }
        return mState == State::Uninitialized;
    }

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

    /**
     * Return the current state as an integer. This is intended for troubleshooting or logging, since there is no code access to
     * the meaning of the integer value.
     */
    explicit operator int() const { return static_cast<int>(mState); }

private:
    enum class State : uint8_t
    {
        Uninitialized = 0, /**< Pre-initialized state; */
        Initialized   = 1, /**< Initialized state. */
        Shutdown      = 2, /**< Post-Shutdown state. */
        Destroyed     = 3, /**< Post-destructor state. */
    };
    State mState = State::Uninitialized;
};

} // namespace chip
