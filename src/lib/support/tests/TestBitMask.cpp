/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/support/BitMask.h>
#include <lib/support/UnitTestRegistration.h>

#include <algorithm>
#include <cstring>
#include <initializer_list>
#include <nlunit-test.h>

using namespace chip;

namespace {

enum class TestEnum : uint16_t
{
    kZero = 0x0000, // not a valid mask or bit flag, here anyway

    kBit_0      = 0x0001,
    kBits_1_2   = 0x0006,
    kBits_4_7   = 0x00F0,
    kBits_High4 = 0xF000,
    kBits_High8 = 0xFF00,
};

void TestBitMaskOperations(nlTestSuite * inSuite, void * inContext)
{
    BitMask<TestEnum> mask;

    NL_TEST_ASSERT(inSuite, mask.Raw() == 0);

    mask.SetField(TestEnum::kBits_1_2, 2);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0x0004);

    mask.SetRaw(0);
    mask.SetField(TestEnum::kBits_4_7, 0x0B);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0x00B0);

    mask.SetRaw(0);

    for (uint16_t i = 0; i < 0x10; i++)
    {
        mask.SetField(TestEnum::kBits_High4, i);
        NL_TEST_ASSERT(inSuite, mask.Raw() == (i << 12));
    }

    mask.SetField(TestEnum::kBits_High8, 0x23);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0x2300);

    mask.SetField(TestEnum::kBits_High4, 0xA);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0xA300);
}

void TestBitFieldLogic(nlTestSuite * inSuite, void * inContext)
{
    BitMask<TestEnum> mask;

    // some general logic that still applies for bit fields just in case
    NL_TEST_ASSERT(inSuite, !mask.HasAny(TestEnum::kBits_High4));
    NL_TEST_ASSERT(inSuite, !mask.HasAny(TestEnum::kBits_High8));

    // setting something non-zero in the upper 4 bits sets "something" in both
    // upper and 4 and 8 bits
    mask.SetField(TestEnum::kBits_High4, 0x01);
    NL_TEST_ASSERT(inSuite, mask.HasAny(TestEnum::kBits_High4));
    NL_TEST_ASSERT(inSuite, mask.HasAny(TestEnum::kBits_High8));

    // sets something visible in high 8 bits, but not high 4 bits
    mask.SetField(TestEnum::kBits_High8, 0x01);
    NL_TEST_ASSERT(inSuite, !mask.HasAny(TestEnum::kBits_High4));
    NL_TEST_ASSERT(inSuite, mask.HasAny(TestEnum::kBits_High8));
}

void TestBitMaskInvalid(nlTestSuite * inSuite, void * inContext)
{
    BitMask<TestEnum> mask;

    // This generally tests for no infinite loops. Nothing to set here
    mask.SetField(TestEnum::kZero, 0x01);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0);

    mask.SetRaw(0x1234);
    mask.SetField(TestEnum::kZero, 0x01);
    NL_TEST_ASSERT(inSuite, mask.Raw() == 0x1234);
}

const nlTest sTests[] = {
    NL_TEST_DEF("BitMask operations", TestBitMaskOperations), //
    NL_TEST_DEF("BitFields logic", TestBitFieldLogic),        //
    NL_TEST_DEF("Invalid operations", TestBitMaskInvalid),    //
    NL_TEST_SENTINEL()                                        //
};

} // namespace

int TestBitMask()
{
    nlTestSuite theSuite = { "BitMask tests", &sTests[0], nullptr, nullptr };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestBitMask)
