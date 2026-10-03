/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <cstdint>

#include <pw_unit_test/framework.h>

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <platform/Darwin/BleConnectionErrorWrapping.h>

using namespace chip;
using namespace chip::DeviceLayer::Internal;

namespace {

// Mirror of WrapCBErrorCodeAsKOS's body, kept in the test so we exercise the SAME guard
// predicate the production code uses (CBErrorCodeFitsInKOSValueRange) plus the kOS encoding it
// gates. WrapCBErrorCodeAsKOS itself is a file-local static in BleConnectionDelegateImpl.mm and
// takes a CoreBluetooth NSInteger, so it is not directly linkable here; the range guard is the
// only part with branching logic and it is fully covered by exercising the shared predicate.
CHIP_ERROR WrapCodeAsKOS(int64_t code, CHIP_ERROR fallback)
{
    if (!CBErrorCodeFitsInKOSValueRange(code))
    {
        return fallback;
    }
    return CHIP_ERROR(chip::ChipError::Range::kOS, static_cast<int32_t>(code));
}

} // namespace

// A negative NSError.code (e.g. -1) must take the guard's fallback branch rather than be masked
// into a bogus kOS value. Without the guard, CHIP_ERROR(kOS, -1) masks to value 0xFFFFFF.
TEST(TestBleConnectionErrorWrapping, NegativeCodeReturnsFallback)
{
    EXPECT_FALSE(CBErrorCodeFitsInKOSValueRange(-1));
    EXPECT_EQ(WrapCodeAsKOS(-1, BLE_ERROR_GATT_WRITE_FAILED), BLE_ERROR_GATT_WRITE_FAILED);
}

// A code just above the 24-bit max (0x1000000) must take the fallback branch. Without the guard,
// CHIP_ERROR(kOS, 0x1000000) masks to value 0 — i.e. it would silently alias to the kOS "no
// value" code and corrupt triage data.
TEST(TestBleConnectionErrorWrapping, AboveMaxCodeReturnsFallback)
{
    EXPECT_FALSE(CBErrorCodeFitsInKOSValueRange(0x1000000));
    EXPECT_EQ(WrapCodeAsKOS(0x1000000, BLE_ERROR_GATT_SUBSCRIBE_FAILED), BLE_ERROR_GATT_SUBSCRIBE_FAILED);
}

// In-range codes (0, an arbitrary mid value, and the 24-bit max) must NOT take the fallback: they
// encode losslessly into the kOS value field and preserve the exact 24 value bits.
//
// Note on the round-trip accessor: ChipError's value field is 24 bits wide but GetValue() returns
// a signed ValueType (int32_t) and sign-extends the field (see ChipError::GetField). So for any
// code whose bit 23 is set (>= 0x800000, e.g. 0xABCDEF and 0xFFFFFF here) GetValue() returns a
// negative int32_t, NOT the raw code. The lossless invariant the wrapping actually provides — and
// the bits triage recovers — is the low 24 bits, so mask with kOSValueMask before comparing.
TEST(TestBleConnectionErrorWrapping, InRangeCodesEncodeAsKOS)
{
    constexpr uint32_t kOSValueMask = 0xFFFFFF; // 24-bit ChipError value field (ChipError::kValueLength)
    const int64_t inRange[]         = { 0, 3 /* CBErrorOperationCancelled-shaped */, 0xABCDEF, 0xFFFFFF };
    for (int64_t code : inRange)
    {
        EXPECT_TRUE(CBErrorCodeFitsInKOSValueRange(code));

        CHIP_ERROR err = WrapCodeAsKOS(code, BLE_ERROR_GATT_INDICATE_FAILED);
        EXPECT_NE(err, BLE_ERROR_GATT_INDICATE_FAILED);
        EXPECT_EQ(err.GetRange(), chip::ChipError::Range::kOS);
        EXPECT_EQ(static_cast<uint32_t>(err.GetValue()) & kOSValueMask, static_cast<uint32_t>(code));
    }
}

// Boundary sanity on the predicate alone: exactly 0 and exactly 0xFFFFFF are in range; the values
// immediately outside (-1 and 0x1000000) are not. This pins the precise interval the kOS value
// field can represent.
TEST(TestBleConnectionErrorWrapping, PredicateBoundaries)
{
    EXPECT_TRUE(CBErrorCodeFitsInKOSValueRange(0));
    EXPECT_TRUE(CBErrorCodeFitsInKOSValueRange(0xFFFFFF));
    EXPECT_FALSE(CBErrorCodeFitsInKOSValueRange(-1));
    EXPECT_FALSE(CBErrorCodeFitsInKOSValueRange(0x1000000));
}
