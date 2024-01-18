/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/clusters/scenes-server/ExtensionFieldSetsImpl.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;

namespace TestEFS {

enum class TagTestEFS : uint8_t
{
    kEFS = 1,
    kTestArray,
};

static constexpr size_t kPersistentSceneBufferMax = 256;

// Test Cluster ID
constexpr chip::ClusterId kOnOffClusterId        = 0x0006;
constexpr chip::ClusterId kLevelControlClusterId = 0x0008;
constexpr chip::ClusterId kColorControlClusterId = 0x0300;

constexpr uint8_t kOnOffSize        = 1;
constexpr uint8_t kLevelControlSize = 3;
constexpr uint8_t kColorControlSize = 14;

static uint8_t onOffBuffer[scenes::kMaxFieldBytesPerCluster]        = "0";
static uint8_t levelControlBuffer[scenes::kMaxFieldBytesPerCluster] = "123";
static uint8_t colorControlBuffer[scenes::kMaxFieldBytesPerCluster] = "abcdefghijklmn";

static const scenes::ExtensionFieldSet EFS1(kOnOffClusterId, onOffBuffer, kOnOffSize);
static const scenes::ExtensionFieldSet EFS2(kLevelControlClusterId, levelControlBuffer, kLevelControlSize);
static const scenes::ExtensionFieldSet EFS3(kColorControlClusterId, colorControlBuffer, kColorControlSize);

static scenes::ExtensionFieldSetsImpl sEFSets;

void TestInsertExtensionFieldSet(nlTestSuite * aSuite, void * aContext)
{
    scenes::ExtensionFieldSetsImpl * EFS = &sEFSets;
    scenes::ExtensionFieldSetsImpl testEFS1;
    scenes::ExtensionFieldSetsImpl testEFS2;
    scenes::ExtensionFieldSetsImpl testEFS3;
    scenes::ExtensionFieldSetsImpl tempTestEFS;
    scenes::ExtensionFieldSet tempEFS;

    uint8_t empty_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
    uint8_t double_size_buffer[scenes::kMaxFieldBytesPerCluster + 1];
    ByteSpan bufferSpan(double_size_buffer);

    memset(double_size_buffer, static_cast<uint8_t>(1), sizeof(double_size_buffer));

    NL_TEST_ASSERT(aSuite, true == EFS->IsEmpty());

    // Test creators of single ExtensionFieldSet
    NL_TEST_ASSERT(aSuite, EFS1.mID == kOnOffClusterId);
    NL_TEST_ASSERT(aSuite, EFS1.mUsedBytes == kOnOffSize);
    NL_TEST_ASSERT(aSuite, !memcmp(onOffBuffer, EFS1.mBytesBuffer, EFS1.mUsedBytes));

    NL_TEST_ASSERT(aSuite, EFS2.mID == kLevelControlClusterId);
    NL_TEST_ASSERT(aSuite, EFS2.mUsedBytes == kLevelControlSize);
    NL_TEST_ASSERT(aSuite, !memcmp(levelControlBuffer, EFS2.mBytesBuffer, EFS2.mUsedBytes));

    NL_TEST_ASSERT(aSuite, EFS3.mID == kColorControlClusterId);
    NL_TEST_ASSERT(aSuite, EFS3.mUsedBytes == kColorControlSize);
    NL_TEST_ASSERT(aSuite, !memcmp(colorControlBuffer, EFS3.mBytesBuffer, EFS3.mUsedBytes));

    // operator tests single EFS
    tempEFS = EFS1;
    NL_TEST_ASSERT(aSuite, tempEFS == EFS1);
    tempEFS = EFS2;
    NL_TEST_ASSERT(aSuite, tempEFS == EFS2);
    tempEFS = EFS3;
    NL_TEST_ASSERT(aSuite, tempEFS == EFS3);

    // Test clear EFS
    tempEFS.Clear();
    NL_TEST_ASSERT(aSuite, tempEFS.IsEmpty());
    NL_TEST_ASSERT(aSuite, tempEFS.mID == kInvalidClusterId);
    NL_TEST_ASSERT(aSuite, tempEFS.mUsedBytes == 0);
    NL_TEST_ASSERT(aSuite, !memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(tempEFS.mBytesBuffer)));

    // Test creation of EFS from Array and ByteSpan that are to big
    tempEFS = scenes::ExtensionFieldSet(kOnOffClusterId, double_size_buffer, sizeof(double_size_buffer));
    NL_TEST_ASSERT(aSuite, tempEFS.mID == kOnOffClusterId);
    // Confirm EFS empty
    NL_TEST_ASSERT(aSuite, tempEFS.mUsedBytes == 0);
    NL_TEST_ASSERT(aSuite, !memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(empty_buffer)));

    tempEFS = scenes::ExtensionFieldSet(kLevelControlClusterId, bufferSpan);
    NL_TEST_ASSERT(aSuite, tempEFS.mID == kLevelControlClusterId);
    // Confirm EFS empty
    NL_TEST_ASSERT(aSuite, tempEFS.mUsedBytes == 0);
    NL_TEST_ASSERT(aSuite, !memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(empty_buffer)));

    // Test creation of EFS from truncating an Array
    tempEFS = scenes::ExtensionFieldSet(kColorControlClusterId, double_size_buffer, sizeof(tempEFS.mBytesBuffer));
    NL_TEST_ASSERT(aSuite, tempEFS.mID == kColorControlClusterId);
    // Confirm EFS was written
    NL_TEST_ASSERT(aSuite, tempEFS.mUsedBytes == static_cast<uint8_t>(sizeof(tempEFS.mBytesBuffer)));
    NL_TEST_ASSERT(aSuite, !memcmp(double_size_buffer, tempEFS.mBytesBuffer, sizeof(tempEFS.mBytesBuffer)));

    tempEFS.Clear();
    NL_TEST_ASSERT(aSuite, tempEFS.IsEmpty());

    // Test insertion of uninitialized EFS
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == EFS->InsertFieldSet(tempEFS));
    NL_TEST_ASSERT(aSuite, 0 == EFS->GetFieldSetCount());

    // Test insertion of empty EFS
    tempEFS.mID = kOnOffClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == EFS->InsertFieldSet(tempEFS));
    NL_TEST_ASSERT(aSuite, 0 == EFS->GetFieldSetCount());

    // test operators on multiple EFS struct
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS1.InsertFieldSet(EFS1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS1.InsertFieldSet(EFS2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS1.InsertFieldSet(EFS3));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS2.InsertFieldSet(EFS3));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS3.InsertFieldSet(EFS1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testEFS3.InsertFieldSet(EFS2));

    tempTestEFS = testEFS1;
    NL_TEST_ASSERT(aSuite, tempTestEFS == testEFS1);
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS2));
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS3));
    tempTestEFS = testEFS2;
    NL_TEST_ASSERT(aSuite, tempTestEFS == testEFS2);
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS1));
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS3));
    tempTestEFS = testEFS3;
    NL_TEST_ASSERT(aSuite, tempTestEFS == testEFS3);
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS1));
    NL_TEST_ASSERT(aSuite, !(tempTestEFS == testEFS2));

    // test clear multipler efs struct
    tempTestEFS.Clear();
    NL_TEST_ASSERT(aSuite, tempTestEFS.IsEmpty());
    NL_TEST_ASSERT(aSuite, 0 == tempTestEFS.GetFieldSetCount());

    // Test insert
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS1));
    NL_TEST_ASSERT(aSuite, 1 == EFS->GetFieldSetCount());

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS2));
    NL_TEST_ASSERT(aSuite, 2 == EFS->GetFieldSetCount());

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS3));
    NL_TEST_ASSERT(aSuite, 3 == EFS->GetFieldSetCount());

    // Test get
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 0));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS1);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 1));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS2);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 2));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS3);
}

void TestSerializeDerializeExtensionFieldSet(nlTestSuite * aSuite, void * aContext)
{
    scenes::ExtensionFieldSetsImpl * EFS = &sEFSets;
    scenes::ExtensionFieldSetsImpl testSceneEFS;

    scenes::ExtensionFieldSet tempEFS;

    uint8_t EFS1Buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
    uint8_t EFS2Buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
    uint8_t EFS3Buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
    uint8_t sceneEFSBuffer[kPersistentSceneBufferMax]    = { 0 };

    uint32_t EFS1_serialized_length     = 0;
    uint32_t EFS2_serialized_length     = 0;
    uint32_t EFS3_serialized_length     = 0;
    uint32_t sceneEFS_serialized_length = 0;

    TLV::TLVReader reader;
    TLV::TLVWriter writer;
    TLV::TLVType outer;
    TLV::TLVType outerRead;

    // Individual Field Sets serialize / deserialize
    writer.Init(EFS1Buffer);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS1.Serialize(writer));
    EFS1_serialized_length = writer.GetLengthWritten();
    NL_TEST_ASSERT(aSuite, EFS1_serialized_length <= scenes::kMaxFieldBytesPerCluster);

    writer.Init(EFS2Buffer);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS2.Serialize(writer));
    EFS2_serialized_length = writer.GetLengthWritten();
    NL_TEST_ASSERT(aSuite, EFS2_serialized_length <= scenes::kMaxFieldBytesPerCluster);

    writer.Init(EFS3Buffer);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS3.Serialize(writer));
    EFS3_serialized_length = writer.GetLengthWritten();
    NL_TEST_ASSERT(aSuite, EFS3_serialized_length <= scenes::kMaxFieldBytesPerCluster);

    reader.Init(EFS1Buffer);
    reader.Next(TLV::AnonymousTag());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == tempEFS.Deserialize(reader));
    NL_TEST_ASSERT(aSuite, EFS1 == tempEFS);

    reader.Init(EFS2Buffer);
    reader.Next(TLV::AnonymousTag());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == tempEFS.Deserialize(reader));
    NL_TEST_ASSERT(aSuite, EFS2 == tempEFS);

    reader.Init(EFS3Buffer);
    reader.Next(TLV::AnonymousTag());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == tempEFS.Deserialize(reader));
    NL_TEST_ASSERT(aSuite, EFS3 == tempEFS);

    // All ExtensionFieldSets serialize / deserialize
    writer.Init(sceneEFSBuffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->Serialize(writer));
    writer.EndContainer(outer);
    sceneEFS_serialized_length = writer.GetLengthWritten();
    NL_TEST_ASSERT(aSuite, sceneEFS_serialized_length <= kPersistentSceneBufferMax);

    reader.Init(sceneEFSBuffer);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == testSceneEFS.Deserialize(reader));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));
    NL_TEST_ASSERT(aSuite, *EFS == testSceneEFS);
}

void TestRemoveExtensionFieldSet(nlTestSuite * aSuite, void * aContext)
{
    scenes::ExtensionFieldSetsImpl * EFS = &sEFSets;
    scenes::ExtensionFieldSet tempEFS;

    // Order in EFS at this point: [EFS1, EFS2, EFS3]
    // Removal at beginning
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->RemoveFieldAtPosition(0));
    NL_TEST_ASSERT(aSuite, 2 == EFS->GetFieldSetCount());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS1));
    NL_TEST_ASSERT(aSuite, 3 == EFS->GetFieldSetCount());

    // Verify order
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 0));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 1));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 2));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS1);

    // Order in EFS at this point: [EFS2, EFS3, EFS1]
    // Removal at middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->RemoveFieldAtPosition(1));
    NL_TEST_ASSERT(aSuite, 2 == EFS->GetFieldSetCount());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS3));
    NL_TEST_ASSERT(aSuite, 3 == EFS->GetFieldSetCount());

    // Verify order
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 0));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 1));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 2));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS3);

    // Order in EFS at this point: [EFS2, EFS1, EFS3]
    // Removal at end
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->RemoveFieldAtPosition(2));
    NL_TEST_ASSERT(aSuite, 2 == EFS->GetFieldSetCount());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->InsertFieldSet(EFS3));
    NL_TEST_ASSERT(aSuite, 3 == EFS->GetFieldSetCount());

    // Verify order
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 0));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 1));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == EFS->GetFieldSetAtPosition(tempEFS, 2));
    NL_TEST_ASSERT(aSuite, tempEFS == EFS3);

    // Emptying the table
    EFS->Clear();
    NL_TEST_ASSERT(aSuite, true == EFS->IsEmpty());
}

} // namespace TestEFS
/**
 *  Tear down the test suite.
 */
int TestSetup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);

    return SUCCESS;
}

namespace {
/**
 *  Setup the test suite.
 */
int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();

    return SUCCESS;
}
} // namespace

int TestExtensionFieldSets()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestInsertExtensionFieldSet", TestEFS::TestInsertExtensionFieldSet),
                               NL_TEST_DEF("TestSerializeDerializeExtensionFieldSet",
                                           TestEFS::TestSerializeDerializeExtensionFieldSet),
                               NL_TEST_DEF("TestRemoveExtensionFieldSet", TestEFS::TestRemoveExtensionFieldSet),

                               NL_TEST_SENTINEL() };

    nlTestSuite theSuite = {
        "SceneTable",
        &sTests[0],
        TestSetup,
        TestTeardown,
    };

    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestExtensionFieldSets)
