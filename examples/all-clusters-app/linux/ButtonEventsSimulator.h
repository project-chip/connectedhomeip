/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <functional>
#include <stdbool.h>

#include <lib/core/DataModelTypes.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

namespace chip {
namespace app {

/**
 * State machine to emit button sequences. Configure with `SetXxx()` methods
 * and then call `Execute()` with a functor to be called when done.
 *
 * The implementation has dependencies on SystemLayer (to start timers) and on
 * EventLogging.
 *
 */
class ButtonEventsSimulator
{
public:
    enum class Mode
    {
        kModeLongPress,
        kModeMultiPress,
        kModeMultiPressNonAs
    };

    using DoneCallback = std::function<void()>;

    ButtonEventsSimulator() = default;

    // Returns true on success to start execution, false on something going awry.
    // `doneCallback` is called only if execution got started.
    bool Execute(DoneCallback && doneCallback);

    ButtonEventsSimulator & SetLongPressDelayMillis(System::Clock::Milliseconds32 longPressDelayMillis)
    {
        mLongPressDelayMillis = longPressDelayMillis;
        return *this;
    }

    ButtonEventsSimulator & SetLongPressDurationMillis(System::Clock::Milliseconds32 longPressDurationMillis)
    {
        mLongPressDurationMillis = longPressDurationMillis;
        return *this;
    }

    ButtonEventsSimulator & SetMultiPressPressedTimeMillis(System::Clock::Milliseconds32 multiPressPressedTimeMillis)
    {
        mMultiPressPressedTimeMillis = multiPressPressedTimeMillis;
        return *this;
    }

    ButtonEventsSimulator & SetMultiPressReleasedTimeMillis(System::Clock::Milliseconds32 multiPressReleasedTimeMillis)
    {
        mMultiPressReleasedTimeMillis = multiPressReleasedTimeMillis;
        return *this;
    }

    ButtonEventsSimulator & SetMultiPressNumPresses(uint8_t multiPressNumPresses)
    {
        mMultiPressNumPresses = multiPressNumPresses;
        return *this;
    }

    ButtonEventsSimulator & SetIdleButtonId(uint8_t idleButtonId)
    {
        mIdleButtonId = idleButtonId;
        return *this;
    }

    ButtonEventsSimulator & SetPressedButtonId(uint8_t pressedButtonId)
    {
        mPressedButtonId = pressedButtonId;
        return *this;
    }

    ButtonEventsSimulator & SetMode(Mode mode)
    {
        mMode = mode;
        return *this;
    }

    ButtonEventsSimulator & SetEndpointId(EndpointId endpointId)
    {
        mEndpointId = endpointId;
        return *this;
    }

    ButtonEventsSimulator & SetFeatureMap(uint32_t featureMap)
    {
        mFeatureMap = featureMap;
        return *this;
    }

    ButtonEventsSimulator & SetMultiPressMax(uint8_t multiPressMax)
    {
        mMultiPressMax = multiPressMax;
        return *this;
    }

private:
    enum class State
    {
        kIdle = 0,

        kEmitStartOfLongPress = 1,
        kEmitLongPress        = 2,
        kEmitLongRelease      = 3,

        kEmitStartOfMultiPress = 4,
        kEmitEndOfMultiPress   = 5,

        kMultiPressButtonRelease = 6,
    };

    static void OnTimerDone(System::Layer * layer, void * appState);
    void SetState(State newState);
    void Next();
    void StartTimer(System::Clock::Timeout duration);

    DoneCallback mDoneCallback;
    System::Clock::Milliseconds32 mLongPressDelayMillis{};
    System::Clock::Milliseconds32 mLongPressDurationMillis{};
    System::Clock::Milliseconds32 mMultiPressPressedTimeMillis{};
    System::Clock::Milliseconds32 mMultiPressReleasedTimeMillis{};
    uint8_t mMultiPressNumPresses{ 1 };
    uint8_t mMultiPressPressesDone{ 0 };
    uint8_t mIdleButtonId{ 0 };
    uint8_t mPressedButtonId{ 1 };
    EndpointId mEndpointId{ 1 };
    uint32_t mFeatureMap{ 0 };
    uint8_t mMultiPressMax{ 0 };

    Mode mMode{ Mode::kModeLongPress };
    State mState{ State::kIdle };
};

} // namespace app
} // namespace chip
