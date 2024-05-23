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
 *      This file implements a process to effect a functional test for
 *      the CHIP BLE layer library error string support interfaces.
 *
 */

#define _CHIP_BLE_BLE_H
#include <ble/BleUUID.h>

#include <gtest/gtest.h>

using namespace chip;
using namespace chip::Ble;

namespace {

TEST(TestBleUUID, CheckUUIDsMatch_NULL)
{
    // Test that NULL pointer UUIDs are not equal
    EXPECT_FALSE(UUIDsMatch(nullptr, nullptr));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID)
{
    // Test positive scenario - CHIP Service UUID
    ChipBleUUID uuid;
    EXPECT_TRUE(StringToUUID("0000FFF6-0000-1000-8000-00805F9B34FB", uuid));
    EXPECT_TRUE(UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID_RandomCase)
{
    // Test that letter case doesn't matter
    ChipBleUUID uuid;
    EXPECT_TRUE(StringToUUID("0000FfF6-0000-1000-8000-00805f9B34Fb", uuid));
    EXPECT_TRUE(UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID_NoSeparators)
{
    // Test that separators don't matter
    ChipBleUUID uuid;
    EXPECT_TRUE(StringToUUID("0000FFF600001000800000805F9B34FB", uuid));
    EXPECT_TRUE(UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

TEST(TestBleUUID, CheckStringToUUID_TooLong)
{
    // Test that even one more digit is too much
    ChipBleUUID uuid;
    EXPECT_FALSE(StringToUUID("0000FFF600001000800000805F9B34FB0", uuid));
}

TEST(TestBleUUID, CheckStringToUUID_TooShort)
{
    // Test that even one less digit is too little
    ChipBleUUID uuid;
    EXPECT_FALSE(StringToUUID("0000FFF600001000800000805F9B34F", uuid));
}

TEST(TestBleUUID, CheckStringToUUID_InvalidChar)
{
    // Test that non-hex digits don't pass
    ChipBleUUID uuid;
    EXPECT_FALSE(StringToUUID("0000GFF6-0000-1000-8000-00805F9B34FB0", uuid));
}

} // namespace
