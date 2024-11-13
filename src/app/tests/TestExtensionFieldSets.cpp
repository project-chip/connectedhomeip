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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

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

class TestExtensionFieldSets : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestExtensionFieldSets, TestInsertExtensionFieldSet)
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

    EXPECT_TRUE(EFS->IsEmpty());

    // Test creators of single ExtensionFieldSet
    EXPECT_EQ(EFS1.mID, kOnOffClusterId);
    EXPECT_EQ(EFS1.mUsedBytes, kOnOffSize);
    EXPECT_EQ(memcmp(onOffBuffer, EFS1.mBytesBuffer, EFS1.mUsedBytes), 0);

    EXPECT_EQ(EFS2.mID, kLevelControlClusterId);
    EXPECT_EQ(EFS2.mUsedBytes, kLevelControlSize);
    EXPECT_EQ(memcmp(levelControlBuffer, EFS2.mBytesBuffer, EFS2.mUsedBytes), 0);

    EXPECT_EQ(EFS3.mID, kColorControlClusterId);
    EXPECT_EQ(EFS3.mUsedBytes, kColorControlSize);
    EXPECT_EQ(memcmp(colorControlBuffer, EFS3.mBytesBuffer, EFS3.mUsedBytes), 0);

    // operator tests single EFS
    tempEFS = EFS1;
    EXPECT_EQ(tempEFS, EFS1);
    tempEFS = EFS2;
    EXPECT_EQ(tempEFS, EFS2);
    tempEFS = EFS3;
    EXPECT_EQ(tempEFS, EFS3);

    // Test clear EFS
    tempEFS.Clear();
    EXPECT_TRUE(tempEFS.IsEmpty());
    EXPECT_EQ(tempEFS.mID, kInvalidClusterId);
    EXPECT_EQ(tempEFS.mUsedBytes, 0);
    EXPECT_EQ(memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(tempEFS.mBytesBuffer)), 0);

    // Test creation of EFS from Array and ByteSpan that are to big
    tempEFS = scenes::ExtensionFieldSet(kOnOffClusterId, double_size_buffer, sizeof(double_size_buffer));
    EXPECT_EQ(tempEFS.mID, kOnOffClusterId);
    // Confirm EFS empty
    EXPECT_EQ(tempEFS.mUsedBytes, 0);
    EXPECT_EQ(memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(empty_buffer)), 0);

    tempEFS = scenes::ExtensionFieldSet(kLevelControlClusterId, bufferSpan);
    EXPECT_EQ(tempEFS.mID, kLevelControlClusterId);
    // Confirm EFS empty
    EXPECT_EQ(tempEFS.mUsedBytes, 0);
    EXPECT_EQ(memcmp(empty_buffer, tempEFS.mBytesBuffer, sizeof(empty_buffer)), 0);

    // Test creation of EFS from truncating an Array
    tempEFS = scenes::ExtensionFieldSet(kColorControlClusterId, double_size_buffer, sizeof(tempEFS.mBytesBuffer));
    EXPECT_EQ(tempEFS.mID, kColorControlClusterId);
    // Confirm EFS was written
    EXPECT_EQ(tempEFS.mUsedBytes, static_cast<uint8_t>(sizeof(tempEFS.mBytesBuffer)));
    EXPECT_EQ(memcmp(double_size_buffer, tempEFS.mBytesBuffer, sizeof(tempEFS.mBytesBuffer)), 0);

    tempEFS.Clear();
    EXPECT_TRUE(tempEFS.IsEmpty());

    // Test insertion of uninitialized EFS
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, EFS->InsertFieldSet(tempEFS));
    EXPECT_EQ(0, EFS->GetFieldSetCount());

    // Test insertion of empty EFS
    tempEFS.mID = kOnOffClusterId;
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, EFS->InsertFieldSet(tempEFS));
    EXPECT_EQ(0, EFS->GetFieldSetCount());

    // test operators on multiple EFS struct
    EXPECT_EQ(CHIP_NO_ERROR, testEFS1.InsertFieldSet(EFS1));
    EXPECT_EQ(CHIP_NO_ERROR, testEFS1.InsertFieldSet(EFS2));
    EXPECT_EQ(CHIP_NO_ERROR, testEFS1.InsertFieldSet(EFS3));

    EXPECT_EQ(CHIP_NO_ERROR, testEFS2.InsertFieldSet(EFS3));

    EXPECT_EQ(CHIP_NO_ERROR, testEFS3.InsertFieldSet(EFS1));
    EXPECT_EQ(CHIP_NO_ERROR, testEFS3.InsertFieldSet(EFS2));

    tempTestEFS = testEFS1;
    EXPECT_EQ(tempTestEFS, testEFS1);
    EXPECT_FALSE(tempTestEFS == testEFS2);
    EXPECT_FALSE(tempTestEFS == testEFS3);
    tempTestEFS = testEFS2;
    EXPECT_EQ(tempTestEFS, testEFS2);
    EXPECT_FALSE(tempTestEFS == testEFS1);
    EXPECT_FALSE(tempTestEFS == testEFS3);
    tempTestEFS = testEFS3;
    EXPECT_EQ(tempTestEFS, testEFS3);
    EXPECT_FALSE(tempTestEFS == testEFS1);
    EXPECT_FALSE(tempTestEFS == testEFS2);

    // test clear multipler efs struct
    tempTestEFS.Clear();
    EXPECT_TRUE(tempTestEFS.IsEmpty());
    EXPECT_EQ(0, tempTestEFS.GetFieldSetCount());

    // Test insert
    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS1));
    EXPECT_EQ(1, EFS->GetFieldSetCount());

    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS2));
    EXPECT_EQ(2, EFS->GetFieldSetCount());

    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS3));
    EXPECT_EQ(3, EFS->GetFieldSetCount());

    // Test get
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 0));
    EXPECT_EQ(tempEFS, EFS1);

    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 1));
    EXPECT_EQ(tempEFS, EFS2);

    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 2));
    EXPECT_EQ(tempEFS, EFS3);
}

TEST_F(TestExtensionFieldSets, TestSerializeDerializeExtensionFieldSet)
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
    EXPECT_EQ(CHIP_NO_ERROR, EFS1.Serialize(writer));
    EFS1_serialized_length = writer.GetLengthWritten();
    EXPECT_LE(EFS1_serialized_length, scenes::kMaxFieldBytesPerCluster);

    writer.Init(EFS2Buffer);
    EXPECT_EQ(CHIP_NO_ERROR, EFS2.Serialize(writer));
    EFS2_serialized_length = writer.GetLengthWritten();
    EXPECT_LE(EFS2_serialized_length, scenes::kMaxFieldBytesPerCluster);

    writer.Init(EFS3Buffer);
    EXPECT_EQ(CHIP_NO_ERROR, EFS3.Serialize(writer));
    EFS3_serialized_length = writer.GetLengthWritten();
    EXPECT_LE(EFS3_serialized_length, scenes::kMaxFieldBytesPerCluster);

    reader.Init(EFS1Buffer);
    reader.Next(TLV::AnonymousTag());
    EXPECT_EQ(CHIP_NO_ERROR, tempEFS.Deserialize(reader));
    EXPECT_EQ(EFS1, tempEFS);

    reader.Init(EFS2Buffer);
    reader.Next(TLV::AnonymousTag());
    EXPECT_EQ(CHIP_NO_ERROR, tempEFS.Deserialize(reader));
    EXPECT_EQ(EFS2, tempEFS);

    reader.Init(EFS3Buffer);
    reader.Next(TLV::AnonymousTag());
    EXPECT_EQ(CHIP_NO_ERROR, tempEFS.Deserialize(reader));
    EXPECT_EQ(EFS3, tempEFS);

    // All ExtensionFieldSets serialize / deserialize
    writer.Init(sceneEFSBuffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->Serialize(writer));
    writer.EndContainer(outer);
    sceneEFS_serialized_length = writer.GetLengthWritten();
    EXPECT_LE(sceneEFS_serialized_length, kPersistentSceneBufferMax);

    reader.Init(sceneEFSBuffer);
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, reader.EnterContainer(outerRead));
    EXPECT_EQ(CHIP_NO_ERROR, testSceneEFS.Deserialize(reader));

    EXPECT_EQ(CHIP_NO_ERROR, reader.ExitContainer(outerRead));
    EXPECT_EQ(*EFS, testSceneEFS);
}

TEST_F(TestExtensionFieldSets, TestRemoveExtensionFieldSet)
{
    scenes::ExtensionFieldSetsImpl * EFS = &sEFSets;
    scenes::ExtensionFieldSet tempEFS;

    // Order in EFS at this point: [EFS1, EFS2, EFS3]
    // Removal at beginning
    EXPECT_EQ(CHIP_NO_ERROR, EFS->RemoveFieldAtPosition(0));
    EXPECT_EQ(2, EFS->GetFieldSetCount());
    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS1));
    EXPECT_EQ(3, EFS->GetFieldSetCount());

    // Verify order
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 0));
    EXPECT_EQ(tempEFS, EFS2);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 1));
    EXPECT_EQ(tempEFS, EFS3);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 2));
    EXPECT_EQ(tempEFS, EFS1);

    // Order in EFS at this point: [EFS2, EFS3, EFS1]
    // Removal at middle
    EXPECT_EQ(CHIP_NO_ERROR, EFS->RemoveFieldAtPosition(1));
    EXPECT_EQ(2, EFS->GetFieldSetCount());
    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS3));
    EXPECT_EQ(3, EFS->GetFieldSetCount());

    // Verify order
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 0));
    EXPECT_EQ(tempEFS, EFS2);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 1));
    EXPECT_EQ(tempEFS, EFS1);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 2));
    EXPECT_EQ(tempEFS, EFS3);

    // Order in EFS at this point: [EFS2, EFS1, EFS3]
    // Removal at end
    EXPECT_EQ(CHIP_NO_ERROR, EFS->RemoveFieldAtPosition(2));
    EXPECT_EQ(2, EFS->GetFieldSetCount());
    EXPECT_EQ(CHIP_NO_ERROR, EFS->InsertFieldSet(EFS3));
    EXPECT_EQ(3, EFS->GetFieldSetCount());

    // Verify order
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 0));
    EXPECT_EQ(tempEFS, EFS2);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 1));
    EXPECT_EQ(tempEFS, EFS1);
    EXPECT_EQ(CHIP_NO_ERROR, EFS->GetFieldSetAtPosition(tempEFS, 2));
    EXPECT_EQ(tempEFS, EFS3);

    // Emptying the table
    EFS->Clear();
    EXPECT_TRUE(EFS->IsEmpty());
}

} // namespace TestEFS
