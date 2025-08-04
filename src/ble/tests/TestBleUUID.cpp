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

#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::Ble;

namespace {

constexpr ChipBleUUID expectedUUID = { { 0x00, 0x00, 0xFF, 0xF6, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34,
                                         0xFB } };

TEST(TestBleUUID, CheckUUIDsMatch_NULL)
{
    // Test that NULL pointer UUIDs are not equal
    EXPECT_FALSE(UUIDsMatch(nullptr, nullptr));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID)
{
    // Test positive scenario - CHIP Service UUID
    ChipBleUUID uuid = StringToUUIDConstexpr("0000FFF6-0000-1000-8000-00805F9B34FB");
    EXPECT_TRUE(UUIDsMatch(&uuid, &expectedUUID));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID_RandomCase)
{
    // Test that letter case doesn't matter
    ChipBleUUID uuid = StringToUUIDConstexpr("0000FfF6-0000-1000-8000-00805f9B34Fb");
    EXPECT_TRUE(UUIDsMatch(&uuid, &expectedUUID));
}

TEST(TestBleUUID, CheckStringToUUID_ChipUUID_NoSeparators)
{
    // Test that separators don't matter
    ChipBleUUID uuid = StringToUUIDConstexpr("0000FFF600001000800000805F9B34FB");
    EXPECT_TRUE(UUIDsMatch(&uuid, &expectedUUID));
}

TEST(TestBleUUID, CheckStringToUUID_TooLong)
{
    // Test that even one more digit is too much
    auto result = StringToUUID("0000FFF600001000800000805F9B34FB0");
    EXPECT_FALSE(result.first);
}

TEST(TestBleUUID, CheckStringToUUID_InvalidChar)
{
    // Test that non-hex digits don't pass
    auto result = StringToUUID("0000GFF6-0000-1000-8000-00805F9B34FB");
    EXPECT_FALSE(result.first);
}

} // namespace
