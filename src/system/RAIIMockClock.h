/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

// Include configuration headers
#include <system/SystemClock.h>

namespace chip {
namespace System {
namespace Clock {
namespace Internal {

// A RAII wrapper for MockClock that sets the system clock to the mock clock and restores the original clock on destruction.
class RAIIMockClock : public MockClock
{
public:
    RAIIMockClock() : mRealClock{ &System::SystemClock() } { System::Clock::Internal::SetSystemClockForTesting(this); }

    RAIIMockClock(const RAIIMockClock &)             = delete;
    RAIIMockClock & operator=(const RAIIMockClock &) = delete;
    RAIIMockClock(RAIIMockClock &&)                  = delete;
    RAIIMockClock & operator=(RAIIMockClock &&)      = delete;

    ~RAIIMockClock() { System::Clock::Internal::SetSystemClockForTesting(mRealClock); }

private:
    System::Clock::ClockBase * mRealClock{ nullptr };
};

} // namespace Internal
} // namespace Clock
} // namespace System
} // namespace chip
