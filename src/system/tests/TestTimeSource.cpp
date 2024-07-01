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
 * @file
 *    This is a unit test suite for <tt>chip::Time::TimeSource</tt>. Tests mainly
 *    the ability to compile and use the test implementation of the time source.
 */

#include <pw_unit_test/framework.h>

#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemConfig.h>
#include <system/TimeSource.h>

TEST(TestTimeSource, TestTimeSourceSetAndGet)
{

    chip::Time::TimeSource<chip::Time::Source::kTest> source;

    EXPECT_EQ(source.GetMonotonicTimestamp(), chip::System::Clock::kZero);

    constexpr chip::System::Clock::Milliseconds64 k1234 = chip::System::Clock::Milliseconds64(1234);
    source.SetMonotonicTimestamp(k1234);
    EXPECT_EQ(source.GetMonotonicTimestamp(), k1234);
}

TEST(TestTimeSource, SystemTimeSourceGet)
{

    chip::Time::TimeSource<chip::Time::Source::kSystem> source;

    chip::System::Clock::Timestamp oldValue = source.GetMonotonicTimestamp();

    // a basic monotonic test. This is likely to take less than 1ms, so the
    // actual test value lies mostly in ensuring things compile.
    for (int i = 0; i < 100; i++)
    {
        chip::System::Clock::Timestamp newValue = source.GetMonotonicTimestamp();
        EXPECT_GE(newValue, oldValue);
        oldValue = newValue;
    }
}
