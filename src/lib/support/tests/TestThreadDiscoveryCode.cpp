/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/support/ThreadDiscoveryCode.h>

namespace {

using namespace chip;

class TestThreadDiscoveryCode : public ::testing::Test
{
};

TEST_F(TestThreadDiscoveryCode, TestLongDiscriminator)
{
    // Long discriminator 0xABC (12 bits)
    // Packing should be: 4D 54 00 00 00 00 BC 0A
    Thread::DiscoveryCode code(static_cast<uint16_t>(0xABC));

    EXPECT_EQ(code.AsUInt64(), 0x4D5400000000BC0AULL);
}

TEST_F(TestThreadDiscoveryCode, TestShortDiscriminator)
{
    // Short discriminator 0xA (4 bits)
    // Initialized using uint8_t constructor: code(0xA) calls uint16_t constructor with 0xA << 8 = 0xA00
    // Packing for 0xA00: discLow8 = 0, discHigh4 = 0xA
    // Result: 4D 54 00 00 00 00 00 0A
    Thread::DiscoveryCode code(static_cast<uint8_t>(0xA));

    EXPECT_EQ(code.AsUInt64(), 0x4D5400000000000AULL);
}

TEST_F(TestThreadDiscoveryCode, TestAsUInt64Short)
{
    // Long discriminator 0xABC
    // Full: 4D 54 00 00 00 00 BC 0A
    // Short: 4D 54 00 00 00 00 00 0A
    Thread::DiscoveryCode code(static_cast<uint16_t>(0xABC));

    EXPECT_EQ(code.AsUInt64Short(), 0x4D5400000000000AULL);
}

} // namespace
