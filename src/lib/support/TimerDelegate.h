/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

namespace chip {

class TimerContext
{
public:
    virtual ~TimerContext() {}
    virtual void TimerFired() = 0;
};

/// @brief This class acts as an interface between the report scheduler and the system timer to reduce dependencies on the
/// system layer.
class TimerDelegate
{
public:
    virtual ~TimerDelegate() {}
    /// @brief Start a timer for a given context. The report scheduler must always cancel an existing timer for a context (using
    /// CancelTimer) before starting a new one for that context.
    /// @param context context to pass to the timer callback.
    /// @param aTimeout time in milliseconds before the timer expires
    virtual CHIP_ERROR StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) = 0;
    /// @brief Cancel a timer for a given context
    /// @param context used to identify the timer to cancel
    virtual void CancelTimer(TimerContext * context)                = 0;
    virtual bool IsTimerActive(TimerContext * context)              = 0;
    virtual System::Clock::Timestamp GetCurrentMonotonicTimestamp() = 0;
};

} // namespace chip
