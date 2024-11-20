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

#include <stdint.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <protocols/secure_channel/CheckInCounter.h>

using namespace chip;
using namespace chip::Protocols::SecureChannel;

namespace {

enum class CheckInCounterOperations : uint8_t
{
    kInvalidateHalf = 1,
    kInvalidateAll  = 2,
};

/**
 * @brief Helper function that validates CheckInCounter value after an operation for configurable input values
 *
 * @param startValue Starting value of the Check-In counter
 * @param expectedValue Expected value after the InvalidateHalfCheckInCounterValues call
 */
void VerifyCheckInCounterValues(uint32_t startValue, uint32_t expectedValue, CheckInCounterOperations operation)
{
    // Test Configuration
    TestPersistentStorageDelegate storage;
    CheckInCounter counter;

    // Init Value of the counter is random
    EXPECT_EQ(counter.Init(&storage, chip::DefaultStorageKeyAllocator::IMEventNumber(), 0x10000), CHIP_NO_ERROR);

    // Set starting value
    uint32_t currentValue = counter.GetValue();
    uint32_t delta        = startValue - currentValue;

    EXPECT_EQ(counter.AdvanceBy(delta), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), startValue);

    // Test operation
    switch (operation)
    {
    case CheckInCounterOperations::kInvalidateHalf: {
        EXPECT_EQ(counter.InvalidateHalfCheckInCounterValues(), CHIP_NO_ERROR);
        break;
    }
    case CheckInCounterOperations::kInvalidateAll: {
        EXPECT_EQ(counter.InvalidateAllCheckInCounterValues(), CHIP_NO_ERROR);
        break;
    }
    default: {
        FAIL();
    }
    };

    EXPECT_EQ(counter.GetValue(), expectedValue);
}

TEST(TestCheckInCounter, TestInvalidateHalfCheckInCounterValues)
{

    constexpr uint32_t startValue    = 1;
    constexpr uint32_t expectedValue = 2147483648;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateHalf);
}

TEST(TestCheckInCounter, TestInvalidateHalfCheckInCounterValuesRollover)
{

    constexpr uint32_t startValue    = 4294967295;
    constexpr uint32_t expectedValue = 2147483646;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateHalf);
}

TEST(TestCheckInCounter, TestInvalidateHalfCheckInCounterValues0Value)
{
    constexpr uint32_t startValue    = 0;
    constexpr uint32_t expectedValue = 2147483647;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateHalf);
}

TEST(TestCheckInCounter, TestInvalidateAllCheckInCounterValues)
{
    constexpr uint32_t startValue    = 105;
    constexpr uint32_t expectedValue = 104;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateAll);
}

TEST(TestCheckInCounter, TestInvalidateAllCheckInCounterValues0Value)
{
    constexpr uint32_t startValue    = 0;
    constexpr uint32_t expectedValue = UINT32_MAX;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateAll);
}

TEST(TestCheckInCounter, TestInvalidateAllCheckInCounterValuesRollover)
{
    constexpr uint32_t startValue    = UINT32_MAX;
    constexpr uint32_t expectedValue = UINT32_MAX - 1;

    VerifyCheckInCounterValues(startValue, expectedValue, CheckInCounterOperations::kInvalidateAll);
}

} // namespace
