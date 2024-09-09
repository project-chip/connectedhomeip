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

/**
 *    @file
 *      This file implements unit tests for the NumericAttributeTraits object
 *      which includes handling of odd sized intergers in the attribute store.
 *
 *      StorageType is the type used in the attribute store. Ie. a 24-bit attribute
 *      must occupy exactly 3 bytes in the attribute store. But for the 24-bit
 *      WorkingType the best suitable type must be used, as long as it is >= 3 bytes.
 *
 */

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

// We are testing the odd-sized-integers.h module
#include <app/util/odd-sized-integers.h>

using namespace chip;
using namespace chip::app;

namespace {

TEST(TestNumericAttributeTraits, Test_UINT8)
{
    // Unsigned 8-bit Integer : 1 byte, endianness does not matter.
    using IntType     = NumericAttributeTraits<uint8_t>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = 17;
    const WorkingType workingTestUnsignedNullValue = 0xFF;

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 1u);
    EXPECT_GE(sizeof(wValue), 1u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 17u);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT8)
{
    // Signed 8-bit Integer : 1 byte, endianness does not matter.
    using IntType     = NumericAttributeTraits<int8_t>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = 17;
    const WorkingType workingTestUnsignedNullValue = -128; // 0x80

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 1u);
    EXPECT_GE(sizeof(wValue), 1u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 17);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

enum class SimpleEnum : uint8_t
{
    kZero = 0,
    kOne  = 1,
};

TEST(TestNumericAttributeTraits, Test_SimpleEnum)
{
    // Unsigned 8-bit Integer : 1 byte, endianness does not matter.
    using IntType     = NumericAttributeTraits<SimpleEnum>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = SimpleEnum::kOne;
    const WorkingType workingTestUnsignedNullValue = static_cast<SimpleEnum>(0xFF);

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 1u);
    EXPECT_GE(sizeof(wValue), 1u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, SimpleEnum::kOne);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

////////////////////////////////////////////////////////////
//   ______   __    __        _______   ______  ________  //
//  /      \ /  |  /  |      /       \ /      |/        | //
// /$$$$$$  |$$ |  $$ |      $$$$$$$  |$$$$$$/ $$$$$$$$/  //
// $$____$$ |$$ |__$$ |      $$ |__$$ |  $$ |     $$ |    //
//  /    $$/ $$    $$ |      $$    $$<   $$ |     $$ |    //
// /$$$$$$/  $$$$$$$$ |      $$$$$$$  |  $$ |     $$ |    //
// $$ |_____       $$ |      $$ |__$$ | _$$ |_    $$ |    //
// $$       |      $$ |      $$    $$/ / $$   |   $$ |    //
// $$$$$$$$/       $$/       $$$$$$$/  $$$$$$/    $$/     //
//                                                        //
////////////////////////////////////////////////////////////
TEST(TestNumericAttributeTraits, Test_UINT24_LE)
{
    // Unsigned 24-bit Integer : 3 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<3, false>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x56, 0x34, 0x12 };
    const WorkingType workingTestUnsignedNullValue = 16777215; // 0xFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 3u);
    EXPECT_GE(sizeof(wValue), 3u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456u);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_UINT24_BE)
{
    // Unsigned 24-bit Integer : 3 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<3, false>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x12, 0x34, 0x56 };
    const WorkingType workingTestUnsignedNullValue = 16777215; // 0xFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 3u);
    EXPECT_GE(sizeof(wValue), 3u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456u);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT24_LE)
{
    // Signed 24-bit Integer : 3 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<3, true>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0xEF, 0xFE, 0x7F }; //  8388335
    const StorageType storageTestDataNeg         = { 0x11, 0x22, 0x80 }; // -8379887
    const WorkingType workingDataPos             = 8388335;              // 0x7FFEEF
    const WorkingType workingDataNeg             = -8379887;             // INV(0x802211) = 0x7FDDEE + 1 => -0x7FDDEF
    const WorkingType workingTestSignedNullValue = -8388608;             // -0x800000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 3u);
    EXPECT_GE(sizeof(wValuePos), 3u);
    EXPECT_EQ(sizeof(sValueNeg), 3u);
    EXPECT_GE(sizeof(wValueNeg), 3u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT24_BE)
{
    // Signed 24-bit Integer : 3 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<3, true>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0x7F, 0xFE, 0xEF }; //  8388335
    const StorageType storageTestDataNeg         = { 0x80, 0x22, 0x11 }; // -8379887
    const WorkingType workingDataPos             = 8388335;              // 0x7FFEEF
    const WorkingType workingDataNeg             = -8379887;             // INV(0x802211) = 0x7FDDEE + 1 => -0x7FDDEF
    const WorkingType workingTestSignedNullValue = -8388608;             // -0x800000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 3u);
    EXPECT_GE(sizeof(wValuePos), 3u);
    EXPECT_EQ(sizeof(sValueNeg), 3u);
    EXPECT_GE(sizeof(wValueNeg), 3u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

////////////////////////////////////////////////////////////
//  __    __   ______         _______   ______  ________  //
// /  |  /  | /      \       /       \ /      |/        | //
// $$ |  $$ |/$$$$$$  |      $$$$$$$  |$$$$$$/ $$$$$$$$/  //
// $$ |__$$ |$$$  \$$ |      $$ |__$$ |  $$ |     $$ |    //
// $$    $$ |$$$$  $$ |      $$    $$<   $$ |     $$ |    //
// $$$$$$$$ |$$ $$ $$ |      $$$$$$$  |  $$ |     $$ |    //
//       $$ |$$ \$$$$ |      $$ |__$$ | _$$ |_    $$ |    //
//       $$ |$$   $$$/       $$    $$/ / $$   |   $$ |    //
//       $$/  $$$$$$/        $$$$$$$/  $$$$$$/    $$/     //
//                                                        //
////////////////////////////////////////////////////////////
TEST(TestNumericAttributeTraits, Test_UINT40_LE)
{
    // Unsigned 40-bit Integer : 5 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<5, false>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x9A, 0x78, 0x56, 0x34, 0x12 };
    const WorkingType workingTestUnsignedNullValue = 1099511627775; // 0xFFFFFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 5u);
    EXPECT_GE(sizeof(wValue), 5u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789Au);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_UINT40_BE)
{
    // Unsigned 40-bit Integer : 5 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<5, false>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x12, 0x34, 0x56, 0x78, 0x9A };
    const WorkingType workingTestUnsignedNullValue = 1099511627775; // 0xFFFFFFFFFF
    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 5u);
    EXPECT_GE(sizeof(wValue), 5u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789Au);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT40_LE)
{
    // Signed 40-bit Integer : 5 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<5, true>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0xEF, 0xFE, 0xEE, 0xFF, 0x7F }; //  549754699503
    const StorageType storageTestDataNeg         = { 0x11, 0x22, 0x33, 0x44, 0x80 }; // -548611612143
    const WorkingType workingDataPos             = 549754699503;                     // 0x7FFFEEFEEF
    const WorkingType workingDataNeg             = -548611612143; // INV(0x8044332211) = 0x7FBBCCDDEE + 1 => -0x7FBBCCDDEF
    const WorkingType workingTestSignedNullValue = -549755813888; // -0x8000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 5u);
    EXPECT_GE(sizeof(wValuePos), 5u);
    EXPECT_EQ(sizeof(sValueNeg), 5u);
    EXPECT_GE(sizeof(wValueNeg), 5u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT40_BE)
{
    // Signed 40-bit Integer : 5 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<5, true>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0x7F, 0xFF, 0xEE, 0xFE, 0xEF }; //  549754699503
    const StorageType storageTestDataNeg         = { 0x80, 0x44, 0x33, 0x22, 0x11 }; // -548611612143
    const WorkingType workingDataPos             = 549754699503;                     // 0x7FFFEEFEEF
    const WorkingType workingDataNeg             = -548611612143; // INV(0x8044332211) = 0x7FBBCCDDEE + 1 => -0x7FBBCCDDEF
    const WorkingType workingTestSignedNullValue = -549755813888; // -0x8000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 5u);
    EXPECT_GE(sizeof(wValuePos), 5u);
    EXPECT_EQ(sizeof(sValueNeg), 5u);
    EXPECT_GE(sizeof(wValueNeg), 5u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

////////////////////////////////////////////////////////////
//  __    __   ______         _______   ______  ________  //
// /  |  /  | /      \       /       \ /      |/        | //
// $$ |  $$ |/$$$$$$  |      $$$$$$$  |$$$$$$/ $$$$$$$$/  //
// $$ |__$$ |$$ \__$$ |      $$ |__$$ |  $$ |     $$ |    //
// $$    $$ |$$    $$<       $$    $$<   $$ |     $$ |    //
// $$$$$$$$ | $$$$$$  |      $$$$$$$  |  $$ |     $$ |    //
//       $$ |$$ \__$$ |      $$ |__$$ | _$$ |_    $$ |    //
//       $$ |$$    $$/       $$    $$/ / $$   |   $$ |    //
//       $$/  $$$$$$/        $$$$$$$/  $$$$$$/    $$/     //
//                                                        //
////////////////////////////////////////////////////////////
TEST(TestNumericAttributeTraits, Test_UINT48_LE)
{
    // Unsigned 48-bit Integer : 6 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<6, false>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12 };
    const WorkingType workingTestUnsignedNullValue = 281474976710655; // 0xFFFFFFFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 6u);
    EXPECT_GE(sizeof(wValue), 6u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789ABCu);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_UINT48_BE)
{
    // Unsigned 48-bit Integer : 6 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<6, false>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC };
    const WorkingType workingTestUnsignedNullValue = 281474976710655; // 0xFFFFFFFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 6u);
    EXPECT_GE(sizeof(wValue), 6u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789ABCu);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT48_LE)
{
    // Signed 48-bit Integer : 6 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<6, true>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0xEF, 0xFE, 0xEE, 0xFF, 0x00, 0x7F }; //  139642270580463
    const StorageType storageTestDataNeg         = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x80 }; // -140371271933423
    const WorkingType workingDataPos             = 139642270580463;                        // 0x7F00FFEEFEEF
    const WorkingType workingDataNeg             = -140371271933423; // INV(0x805544332211) = 0x7FAABBCCDDEE + 1 => -0x7FAABBCCDDEF
    const WorkingType workingTestSignedNullValue = -140737488355328; // -0x800000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 6u);
    EXPECT_GE(sizeof(wValuePos), 6u);
    EXPECT_EQ(sizeof(sValueNeg), 6u);
    EXPECT_GE(sizeof(wValueNeg), 6u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT48_BE)
{
    // Signed 48-bit Integer : 6 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<6, true>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos         = { 0x7F, 0x00, 0xFF, 0xEE, 0xFE, 0xEF }; //  139642270580463
    const StorageType storageTestDataNeg         = { 0x80, 0x55, 0x44, 0x33, 0x22, 0x11 }; // -140371271933423
    const WorkingType workingDataPos             = 139642270580463;                        // 0x7F00FFEEFEEF
    const WorkingType workingDataNeg             = -140371271933423; // INV(0x805544332211) = 0x7FAABBCCDDEE + 1 => -0x7FAABBCCDDEF
    const WorkingType workingTestSignedNullValue = -140737488355328; // -0x800000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 6u);
    EXPECT_GE(sizeof(wValuePos), 6u);
    EXPECT_EQ(sizeof(sValueNeg), 6u);
    EXPECT_GE(sizeof(wValueNeg), 6u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

////////////////////////////////////////////////////////////
//  _______    ______         _______   ______  ________  //
// /       |  /      \       /       \ /      |/        | //
// $$$$$$$/  /$$$$$$  |      $$$$$$$  |$$$$$$/ $$$$$$$$/  //
// $$ |____  $$ \__$$/       $$ |__$$ |  $$ |     $$ |    //
// $$      \ $$      \       $$    $$<   $$ |     $$ |    //
// $$$$$$$  |$$$$$$$  |      $$$$$$$  |  $$ |     $$ |    //
// /  \__$$ |$$ \__$$ |      $$ |__$$ | _$$ |_    $$ |    //
// $$    $$/ $$    $$/       $$    $$/ / $$   |   $$ |    //
//  $$$$$$/   $$$$$$/        $$$$$$$/  $$$$$$/    $$/     //
//                                                        //
////////////////////////////////////////////////////////////
TEST(TestNumericAttributeTraits, Test_UINT56_LE)
{
    // Unsigned 56-bit Integer : 7 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<7, false>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12 };
    const WorkingType workingTestUnsignedNullValue = 72057594037927935; // 0xFFFFFFFFFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 7u);
    EXPECT_GE(sizeof(wValue), 7u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789ABCDEu);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_UINT56_BE)
{
    // Unsigned 56-bit Integer : 7 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<7, false>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValue;
    WorkingType wValue;
    StorageType sNullValue;
    WorkingType wNullValue;
    const StorageType storageTestData              = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE };
    const WorkingType workingTestUnsignedNullValue = 72057594037927935; // 0xFFFFFFFFFFFFFF

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValue), 7u);
    EXPECT_GE(sizeof(wValue), 7u);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValue, 0x123456789ABCDEu);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestData, &sNewValue, sizeof(sNewValue)), 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestUnsignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT56_LE)
{
    // Signed 56-bit Integer : 6 bytes - little-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<7, true>, false>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos = { 0xEF, 0xFE, 0xEE, 0xFF, 0x00, 0x11, 0x7F }; //  35766018033778415
    const StorageType storageTestDataNeg = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x80 }; // -35916280616508911
    const WorkingType workingDataPos     = 35766018033778415;                            // 0x7F1100FFEEFEEF
    const WorkingType workingDataNeg     = -35916280616508911; // INV(0x80665544332211) = 0x7F99AABBCCDDEE + 1 => 0x7F99AABBCCDDEF
    const WorkingType workingTestSignedNullValue = -36028797018963968; // -0x80000000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 7u);
    EXPECT_GE(sizeof(wValuePos), 7u);
    EXPECT_EQ(sizeof(sValueNeg), 7u);
    EXPECT_GE(sizeof(wValueNeg), 7u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

TEST(TestNumericAttributeTraits, Test_SINT56_BE)
{
    // Signed 56-bit Integer : 7 bytes - big-endian
    using IntType     = NumericAttributeTraits<OddSizedInteger<7, true>, true>;
    using StorageType = typename IntType::StorageType;
    using WorkingType = typename IntType::WorkingType;

    StorageType sValuePos;
    WorkingType wValuePos;
    StorageType sValueNeg;
    WorkingType wValueNeg;

    const StorageType storageTestDataPos = { 0x7F, 0x11, 0x00, 0xFF, 0xEE, 0xFE, 0xEF }; //  35766018033778415
    const StorageType storageTestDataNeg = { 0x80, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 }; // -35916280616508911
    const WorkingType workingDataPos     = 35766018033778415;                            // 0x7F1100FFEEFEEF
    const WorkingType workingDataNeg     = -35916280616508911; // INV(0x80665544332211) = 0x7F99AABBCCDDEE + 1 => 0x7F99AABBCCDDEF
    const WorkingType workingTestSignedNullValue = -36028797018963968; // -0x80000000000000

    // 1) Verify the size of the types
    EXPECT_EQ(sizeof(sValuePos), 7u);
    EXPECT_GE(sizeof(wValuePos), 7u);
    EXPECT_EQ(sizeof(sValueNeg), 7u);
    EXPECT_GE(sizeof(wValueNeg), 7u);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    EXPECT_EQ(wValuePos, workingDataPos);
    EXPECT_EQ(wValueNeg, workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    EXPECT_EQ(memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)), 0);
    EXPECT_EQ(memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)), 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    EXPECT_EQ(wNullValue, workingTestSignedNullValue);
    EXPECT_TRUE(IntType::IsNullValue(sNullValue));

    // Verify that null values can fit into not nullable
    EXPECT_TRUE(IntType::CanRepresentValue(false, sNullValue));
    EXPECT_TRUE(IntType::CanRepresentValue(false, wNullValue));

    // Verify that null values can't fit into nullable
    EXPECT_FALSE(IntType::CanRepresentValue(true, sNullValue));
    EXPECT_FALSE(IntType::CanRepresentValue(true, wNullValue));
}

} // namespace
