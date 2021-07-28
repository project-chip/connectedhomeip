/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @brief defines a generic time source interface that uses a real clock
 *        at runtime but can be substituted by a test one for unit tests.
 */

#pragma once

#include <stdlib.h>
#include <system/SystemClock.h>

namespace chip {
namespace Time {

enum class Source
{
    kSystem, // System time source
    kTest,   // Test time source
};

/**
 * Defines a generic time source within a system. System time and test times
 * are available.
 */
template <Source kSource>
class TimeSource
{
public:
    /**
     * Returns a monotonically increasing time in milliseconds since an arbitrary, platform-defined
     * epoch.
     *
     * Maintains requirements for the System::Platform::Layer clock implementation:
     *
     *  - Return a value that is ever-increasing (i.e. never * wraps) between reboots of the system.
     *  - The underlying time source is required to tick continuously during any system sleep modes
     *    such that the values do not entail a restart upon wake.
     *  - This function is expected to be thread-safe on any platform that employs threading.
     */
    uint64_t GetCurrentMonotonicTimeMs();
};

/**
 * A system time source, based on the system platform layer.
 */
template <>
class TimeSource<Source::kSystem>
{
public:
    uint64_t GetCurrentMonotonicTimeMs() { return System::Clock::GetMonotonicMicroseconds(); }
};

/**
 * A test time source. Allows setting the current time.
 */
template <>
class TimeSource<Source::kTest>
{
public:
    uint64_t GetCurrentMonotonicTimeMs() { return mCurrentTimeMs; }

    void SetCurrentMonotonicTimeMs(uint64_t value)
    {
        if (value < mCurrentTimeMs)
        {
            abort();
        }
        mCurrentTimeMs = value;
    }

private:
    uint64_t mCurrentTimeMs = 0;
};

} // namespace Time
} // namespace chip
