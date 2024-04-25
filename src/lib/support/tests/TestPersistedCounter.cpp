/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      Unit tests for the Chip Persisted Storage API.
 *
 */

#include <gtest/gtest.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PersistedCounter.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/PersistedStorage.h>

using namespace chip;

namespace {

TEST(TestPersistedCounter, TestOutOfBox)
{
    TestPersistentStorageDelegate storage;
    PersistedCounter<uint64_t> counter;

    // When initializing the first time out of the box, we should have a count of 0 and a value of 0x10000 for the next starting
    // value in persistent storage.
    EXPECT_EQ(counter.Init(&storage, DefaultStorageKeyAllocator::IMEventNumber(), 0x10000), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 0ULL);
}

TEST(TestPersistedCounter, TestReboot)
{
    chip::TestPersistentStorageDelegate storage;
    chip::PersistedCounter<uint64_t> counter, counter2;

    // When initializing the first time out of the box, we should have a count of 0.
    EXPECT_EQ(counter.Init(&storage, chip::DefaultStorageKeyAllocator::IMEventNumber(), 0x10000), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 0ULL);

    // Now we "reboot", and we should get a count of 0x10000.
    EXPECT_EQ(counter2.Init(&storage, chip::DefaultStorageKeyAllocator::IMEventNumber(), 0x10000), CHIP_NO_ERROR);
    EXPECT_EQ(counter2.GetValue(), 0x10000ULL);
}

TEST(TestPersistedCounter, TestkWriteNextCounterStart)
{
    chip::TestPersistentStorageDelegate storage;
    chip::PersistedCounter<uint64_t> counter;

    // When initializing the first time out of the box, we should have
    // a count of 0.
    EXPECT_EQ(counter.Init(&storage, chip::DefaultStorageKeyAllocator::IMEventNumber(), 0x10000), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 0ULL);

    // Verify that we write out the next starting counter value after
    // we've exhausted the counter's range.
    for (int32_t i = 0; i < 0x10000; i++)
    {
        EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    }
    EXPECT_EQ(counter.GetValue(), 0x10000ULL);

    for (int32_t i = 0; i < 0x10000; i++)
    {
        EXPECT_EQ(counter.Advance(), CHIP_NO_ERROR);
    }
    EXPECT_EQ(counter.GetValue(), 0x20000ULL);
}

TEST(TestPersistedCounter, TestSetValue)
{
    chip::TestPersistentStorageDelegate storage;
    chip::PersistedCounter<uint64_t> counter;

    // Test values
    constexpr uint64_t startingEpochValue = 65535;
    constexpr uint64_t newValue           = 2 * startingEpochValue + 156;
    constexpr uint64_t newEpochValue      = 3 * startingEpochValue;
    uint64_t storedValue                  = 0;
    uint16_t size                         = sizeof(storedValue);

    // When initializing the first time out of the box, we should have a count of 0.
    EXPECT_EQ(counter.Init(&storage, chip::DefaultStorageKeyAllocator::IMEventNumber(), startingEpochValue), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), 0ULL);

    // Set New value
    EXPECT_EQ(counter.SetValue(newValue), CHIP_NO_ERROR);
    EXPECT_EQ(counter.GetValue(), newValue);

    // Check new Epoch value was persisted
    EXPECT_EQ(storage.SyncGetKeyValue(chip::DefaultStorageKeyAllocator::IMEventNumber().KeyName(), &storedValue, size),
              CHIP_NO_ERROR);
    EXPECT_EQ(sizeof(storedValue), size);
    storedValue = Encoding::LittleEndian::HostSwap<uint64_t>(storedValue);

    EXPECT_EQ(newEpochValue, storedValue);
}

} // namespace
