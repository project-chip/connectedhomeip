/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <controller/SetUpCodePairer.h>
#include <setup_payload/Base38Decode.h>
#include <setup_payload/Base38Encode.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/PersistentStorageMacros.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Controller;

namespace {

std::string to_hex_string(uint8_t value)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
    return ss.str();
}

class SetUpCodePairerTest : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
};

TEST_F(SetUpCodePairerTest, SuccessfulyPopulatesPayloadFromQRSetUpCode)
{
    constexpr char kDefaultPayloadQRCode[] = "MT:M5L90MP500K64J00000";

    std::vector<uint8_t> decoded{};
    ASSERT_EQ(CHIP_NO_ERROR, chip::base38Decode("M5L90MP500K64J00000", decoded));

    std::string decoded_base38_str{};

    for (uint8_t i = 0; i < decoded.size(); ++i)
    {
        decoded_base38_str += to_hex_string(decoded[i]);
    }

    ASSERT_EQ(decoded_base38_str, "6000080020001000100000");

    char out_buff[19];
    chip::MutableCharSpan out_span{ out_buff };

    uint8_t input[11] = { 0x60, 0x00, 0x08, 0x00, 0x20, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00 };
    char encodedBuf[64];
    MutableByteSpan inputSpan(input);
    MutableCharSpan encodedSpan(encodedBuf);

    ASSERT_EQ(CHIP_NO_ERROR, base38Encode(inputSpan, encodedSpan));

    std::string encoded_base38_str{};
    for (uint8_t i = 0; i < encodedSpan.size(); ++i)
    {
        encoded_base38_str += encodedSpan[i];
    }

    ASSERT_EQ(encoded_base38_str, "M5L90MP500K64J00000");

    // decoded base38 payload:
    // | 01100000 | 00000000 | 00001000 | 00000000 | 00100000 | 00000000 | 00010000 | 00000000 | 00010000 | 00000000 | 00000000 |
    //     0x60       0x00       0x08       0x00       0x20       0x00       0x10       0x00       0x00       0x10       0x00

    SetupPayload payload{};

    ASSERT_EQ(CHIP_NO_ERROR, PayloadUtils::GetPayload(kDefaultPayloadQRCode, payload));

    /// why does it fail? isn't version first 3 bits in decoded Onboarding Payload (011'00000)?
    ASSERT_EQ(payload.version, 6);
    /// as stated in 5.1.3. QR Code -> 5.1.3.1. Payload -> Table 39. Packed Binary Data Structure for Onboarding Payload
}

TEST_F(SetUpCodePairerTest, SuccessfulyPopulatesPayloadFromQRSetUpCode2)
{
    constexpr char kDefaultPayloadQRCode[] = "MT:43L90MP500K64J00000";

    std::vector<uint8_t> decoded{};
    ASSERT_EQ(CHIP_NO_ERROR, chip::base38Decode("43L90MP500K64J00000", decoded));

    std::string decoded_base38_str{};

    for (uint8_t i = 0; i < decoded.size(); ++i)
    {
        decoded_base38_str += to_hex_string(decoded[i]);
    }

    ASSERT_EQ(decoded_base38_str, "0200080020001000100000");

    char out_buff[19];
    chip::MutableCharSpan out_span{ out_buff };

    uint8_t input[11] = { 0x02, 0x00, 0x08, 0x00, 0x20, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00 };
    char encodedBuf[64];
    MutableByteSpan inputSpan(input);
    MutableCharSpan encodedSpan(encodedBuf);

    ASSERT_EQ(CHIP_NO_ERROR, base38Encode(inputSpan, encodedSpan));

    std::string encoded_base38_str{};
    for (uint8_t i = 0; i < encodedSpan.size(); ++i)
    {
        encoded_base38_str += encodedSpan[i];
    }

    ASSERT_EQ(encoded_base38_str, "43L90MP500K64J00000");

    // decoded base38 payload:
    // | 01100000 | 00000000 | 00001000 | 00000000 | 00100000 | 00000000 | 00010000 | 00000000 | 00010000 | 00000000 | 00000000 |
    //     0x60       0x00       0x08       0x00       0x20       0x00       0x10       0x00       0x00       0x10       0x00

    SetupPayload payload{};

    ASSERT_EQ(CHIP_NO_ERROR, PayloadUtils::GetPayload(kDefaultPayloadQRCode, payload));

    /// why does it fail? isn't version first 3 bits in decoded Onboarding Payload (011'00000)?
    ASSERT_EQ(payload.version, 6);
    /// as stated in 5.1.3. QR Code -> 5.1.3.1. Payload -> Table 39. Packed Binary Data Structure for Onboarding Payload
}

} // namespace
