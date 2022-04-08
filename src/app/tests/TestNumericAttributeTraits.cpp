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

#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

// We are testing the odd-sized-integers.h module
#include <app/util/odd-sized-integers.h>

using namespace chip;
using namespace chip::app;

namespace {

void Test_UINT8(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 1);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 1);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 17);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_SINT8(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 1);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 1);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 17);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

enum class SimpleEnum : uint8_t
{
    kZero = 0,
    kOne  = 1,
};

void Test_SimpleEnum(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 1);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 1);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, &storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == SimpleEnum::kOne);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(&storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
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
void Test_UINT24_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 3);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_UINT24_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 3);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_SINT24_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 3);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 3);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
}

void Test_SINT24_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 3);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 3);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 3);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
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
void Test_UINT40_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 5);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789A);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_UINT40_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 5);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789A);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_SINT40_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 5);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 5);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
}

void Test_SINT40_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 5);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 5);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 5);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
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
void Test_UINT48_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 6);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789ABC);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_UINT48_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 6);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789ABC);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_SINT48_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 6);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 6);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
}

void Test_SINT48_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 6);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 6);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 6);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
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
void Test_UINT56_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 7);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789ABCDE);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_UINT56_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValue) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValue) >= 7);

    // Initialize the Storage Value with the test-buffer
    memcpy(&sValue, storageTestData, sizeof(sValue));

    // Convert the Storage Type to Working Type and
    wValue = IntType::StorageToWorking(sValue);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValue == 0x123456789ABCDE);

    StorageType sNewValue;

    // Convert back to Storage Value
    IntType::WorkingToStorage(wValue, sNewValue);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestData, &sNewValue, sizeof(sNewValue)) == 0);

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestUnsignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
}

void Test_SINT56_LE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 7);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 7);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
}

void Test_SINT56_BE(nlTestSuite * apSuite, void * apContext)
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
    NL_TEST_ASSERT(apSuite, sizeof(sValuePos) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValuePos) >= 7);
    NL_TEST_ASSERT(apSuite, sizeof(sValueNeg) == 7);
    NL_TEST_ASSERT(apSuite, sizeof(wValueNeg) >= 7);

    // Initialize the Storage Values with the test-buffer
    memcpy(&sValuePos, storageTestDataPos, sizeof(sValuePos));
    memcpy(&sValueNeg, storageTestDataNeg, sizeof(sValueNeg));

    // Convert the Storage Types to Working Types and
    wValuePos = IntType::StorageToWorking(sValuePos);
    wValueNeg = IntType::StorageToWorking(sValueNeg);

    // 2) Verify that the correct storage format has been used
    NL_TEST_ASSERT(apSuite, wValuePos == workingDataPos);
    NL_TEST_ASSERT(apSuite, wValueNeg == workingDataNeg);

    StorageType sNewValuePos;
    StorageType sNewValueNeg;

    // Convert back to Storage Values
    IntType::WorkingToStorage(wValuePos, sNewValuePos);
    IntType::WorkingToStorage(wValueNeg, sNewValueNeg);

    // 3) Verify that the bytes are located as intended
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataPos, &sNewValuePos, sizeof(sNewValuePos)) == 0);
    NL_TEST_ASSERT(apSuite, memcmp(storageTestDataNeg, &sNewValueNeg, sizeof(sNewValueNeg)) == 0);

    StorageType sNullValue;
    WorkingType wNullValue;

    // Set Storage value to Null
    IntType::SetNull(sNullValue);
    wNullValue = IntType::StorageToWorking(sNullValue);
    NL_TEST_ASSERT(apSuite, wNullValue == workingTestSignedNullValue);
    NL_TEST_ASSERT(apSuite, (IntType::IsNullValue(sNullValue) == true));

    // Verify that null values can fit into not nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, sNullValue) == true));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(false, wNullValue) == true));

    // Verify that null values can't fit into nullable
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, sNullValue) == false));
    NL_TEST_ASSERT(apSuite, (IntType::CanRepresentValue(true, wNullValue) == false));
}

static int TestSetup(void * inContext)
{
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    return SUCCESS;
}

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test_UINT8", Test_UINT8),
    NL_TEST_DEF("Test_SINT8", Test_SINT8),
    NL_TEST_DEF("Test_SimpleEnum", Test_SimpleEnum),

    NL_TEST_DEF("Test_UINT24_LE",Test_UINT24_LE),
    NL_TEST_DEF("Test_SINT24_LE",Test_SINT24_LE),
    NL_TEST_DEF("Test_UINT24_BE",Test_UINT24_BE),
    NL_TEST_DEF("Test_SINT24_BE",Test_SINT24_BE),

    NL_TEST_DEF("Test_UINT40_LE",Test_UINT40_LE),
    NL_TEST_DEF("Test_SINT40_LE",Test_SINT40_LE),
    NL_TEST_DEF("Test_UINT40_BE",Test_UINT40_BE),
    NL_TEST_DEF("Test_SINT40_BE",Test_SINT40_BE),

    NL_TEST_DEF("Test_UINT48_LE",Test_UINT48_LE),
    NL_TEST_DEF("Test_SINT48_LE",Test_SINT48_LE),
    NL_TEST_DEF("Test_UINT48_BE",Test_UINT48_BE),
    NL_TEST_DEF("Test_SINT48_BE",Test_SINT48_BE),

    NL_TEST_DEF("Test_UINT56_LE",Test_UINT56_LE),
    NL_TEST_DEF("Test_SINT56_LE",Test_SINT56_LE),
    NL_TEST_DEF("Test_UINT56_BE",Test_UINT56_BE),
    NL_TEST_DEF("Test_SINT56_BE",Test_SINT56_BE),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite theSuite =
{
    "TestNumericAttributeTraits",
    &sTests[0],
    TestSetup,
    TestTeardown
};
// clang-format on

} // namespace

int TestNumericAttributeTraits()
{
    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestNumericAttributeTraits)
