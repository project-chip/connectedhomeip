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

#include <ble/BleUUID.h>
#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

using namespace chip;
using namespace chip::Ble;

namespace {

void CheckStringToUUID_ChipUUID(nlTestSuite * inSuite, void * inContext)
{
    // Test positive scenario - CHIP Service UUID
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, StringToUUID("0000FFF6-0000-1000-8000-00805F9B34FB", uuid));
    NL_TEST_ASSERT(inSuite, UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

void CheckStringToUUID_ChipUUID_RandomCase(nlTestSuite * inSuite, void * inContext)
{
    // Test that letter case doesn't matter
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, StringToUUID("0000FfF6-0000-1000-8000-00805f9B34Fb", uuid));
    NL_TEST_ASSERT(inSuite, UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

void CheckStringToUUID_ChipUUID_NoSeparators(nlTestSuite * inSuite, void * inContext)
{
    // Test that separators don't matter
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, StringToUUID("0000FFF600001000800000805F9B34FB", uuid));
    NL_TEST_ASSERT(inSuite, UUIDsMatch(&uuid, &CHIP_BLE_SVC_ID));
}

void CheckStringToUUID_TooLong(nlTestSuite * inSuite, void * inContext)
{
    // Test that even one more digit is too much
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, !StringToUUID("0000FFF600001000800000805F9B34FB0", uuid));
}

void CheckStringToUUID_TooShort(nlTestSuite * inSuite, void * inContext)
{
    // Test that even one less digit is too little
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, !StringToUUID("0000FFF600001000800000805F9B34F", uuid));
}

void CheckStringToUUID_InvalidChar(nlTestSuite * inSuite, void * inContext)
{
    // Test that non-hex digits don't pass
    ChipBleUUID uuid;
    NL_TEST_ASSERT(inSuite, !StringToUUID("0000GFF6-0000-1000-8000-00805F9B34FB0", uuid));
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("CheckStringToUUID_ChipUUID", CheckStringToUUID_ChipUUID),
    NL_TEST_DEF("CheckStringToUUID_ChipUUID_RandomCase", CheckStringToUUID_ChipUUID_RandomCase),
    NL_TEST_DEF("CheckStringToUUID_ChipUUID_NoSeparators", CheckStringToUUID_ChipUUID_NoSeparators),
    NL_TEST_DEF("CheckStringToUUID_TooLong", CheckStringToUUID_TooLong),
    NL_TEST_DEF("CheckStringToUUID_TooShort", CheckStringToUUID_TooShort),
    NL_TEST_DEF("CheckStringToUUID_InvalidChar", CheckStringToUUID_InvalidChar),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestBleUUID()
{
    nlTestSuite theSuite = { "BleUUID", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBleUUID)
