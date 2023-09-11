/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief defines a generic time source interface that uses a real clock
 *        at runtime but can be substituted by a test one for unit tests.
 */

#pragma once

#include <lib/support/CodeUtils.h>
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
    System::Clock::Timestamp GetMonotonicTimestamp();
};

/**
 * A system time source, based on the system platform layer.
 */
template <>
class TimeSource<Source::kSystem>
{
public:
    System::Clock::Timestamp GetMonotonicTimestamp() { return System::SystemClock().GetMonotonicTimestamp(); }
};

/**
 * A test time source. Allows setting the current time.
 */
template <>
class TimeSource<Source::kTest>
{
public:
    System::Clock::Timestamp GetMonotonicTimestamp() { return mCurrentTime; }

    void SetMonotonicTimestamp(System::Clock::Timestamp value)
    {
        VerifyOrDie(value >= mCurrentTime);
        mCurrentTime = value;
    }

private:
    System::Clock::Timestamp mCurrentTime = System::Clock::kZero;
};

} // namespace Time
} // namespace chip
