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

#include <app/clusters/scenes/SceneTableImpl.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;
using SceneTableEntry    = scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneTableImpl     = scenes::DefaultSceneTableImpl;
using SceneStorageId     = scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData          = scenes::DefaultSceneTableImpl::SceneData;
using ExtensionFieldsSet = scenes::ExtensionFieldsSet;
using TransitionTimeMs   = scenes::TransitionTimeMs;

#define ON_OFF_CID 0x0006
#define LV_CTR_CID 0x0008
#define CC_CTR_CID 0x0300
#define TEST_ENDPOINT1 0x0001
#define TEST_ENDPOINT2 0x0099

// ON OFF ATTRIBUTE IDs
#define ON_OFF_ID 0x0000

// LEVEL CONTROL ATTRIBUTE IDs
#define CURRENT_LVL_ID 0x0000
#define CURRENT_FRQ_ID 0x0004

// COLOR CONTROL ATTRIBUTE IDs
#define CURRENT_SAT_ID 0x0001
#define CURRENT_X_ID 0x0003
#define CURRENT_Y_ID 0x0004
#define COLOR_TEMP_MIR_ID 00007
#define EN_CURRENT_HUE_ID 0x4000
#define C_LOOP_ACTIVE_ID 0x4002
#define C_LOOP_DIR_ID 0x4003
#define C_LOOP_TIME_ID 0x4004

namespace {

// Test fabrics, adding more requires to modify the "ResetSceneTable" function
constexpr chip::FabricIndex kFabric1 = 1;
constexpr chip::FabricIndex kFabric2 = 7;

// Scene storage ID
static const SceneStorageId sceneId1(TEST_ENDPOINT1, 0xAA, 0x101);
static const SceneStorageId sceneId2(TEST_ENDPOINT1, 0xBB, 0x00);
static const SceneStorageId sceneId3(TEST_ENDPOINT2, 0xCC, 0x102);
static const SceneStorageId sceneId4(TEST_ENDPOINT2, 0xBE, 0x00);
static const SceneStorageId sceneId5(TEST_ENDPOINT1, 0x45, 0x103);
static const SceneStorageId sceneId6(TEST_ENDPOINT1, 0x65, 0x00);
static const SceneStorageId sceneId7(TEST_ENDPOINT1, 0x77, 0x101);
static const SceneStorageId sceneId8(TEST_ENDPOINT2, 0xEE, 0x101);
static const SceneStorageId sceneId9(TEST_ENDPOINT2, 0xAB, 0x101);

// Scene data
static const SceneData sceneData1(CharSpan("Scene #1", sizeof("Scene #1")));
static const SceneData sceneData2(CharSpan("Scene #2", sizeof("Scene #2")), 2, 5);
static const SceneData sceneData3(CharSpan("Scene #3", sizeof("Scene #3")), 25);
static const SceneData sceneData4(CharSpan("Scene num4", sizeof("Scene num4")), 5);
static const SceneData sceneData5(CharSpan(), 10);
static const SceneData sceneData6(CharSpan("Scene #6", sizeof("Scene #6")), 3, 15);
static const SceneData sceneData7(CharSpan("Scene #7", sizeof("Scene #7")), 20, 5);
static const SceneData sceneData8(CharSpan("NAME TOO LOOONNG", sizeof("Scene num4")), 10);
static const SceneData sceneData9(CharSpan("Scene #9", sizeof("Scene #9")), 30, 15);
static const SceneData sceneData10(CharSpan("Scene #10", sizeof("Scene #10")), 10, 1);
static const SceneData sceneData11(CharSpan("Scene #11", sizeof("Scene #11")), 20, 10);
static const SceneData sceneData12(CharSpan("Scene #12", sizeof("Scene #12")), 30, 5);

// Scenes
SceneTableEntry scene1(sceneId1, sceneData1);
SceneTableEntry scene2(sceneId2, sceneData2);
SceneTableEntry scene3(sceneId3, sceneData3);
SceneTableEntry scene4(sceneId4, sceneData4);
SceneTableEntry scene5(sceneId5, sceneData5);
SceneTableEntry scene6(sceneId6, sceneData6);
SceneTableEntry scene7(sceneId7, sceneData7);
SceneTableEntry scene8(sceneId8, sceneData8);
SceneTableEntry scene9(sceneId9, sceneData9);
SceneTableEntry scene10(sceneId1, sceneData10);
SceneTableEntry scene11(sceneId5, sceneData11);
SceneTableEntry scene12(sceneId8, sceneData12);

// Clusters EFS data
static app::Clusters::Scenes::Structs::ExtensionFieldSet::Type OOextensionFieldSet;
static app::Clusters::Scenes::Structs::ExtensionFieldSet::Type LCextensionFieldSet;
static app::Clusters::Scenes::Structs::ExtensionFieldSet::Type CCextensionFieldSet;

static app::Clusters::Scenes::Structs::AttributeValuePair::Type OOPairs[1];
static app::Clusters::Scenes::Structs::AttributeValuePair::Type LCPairs[2];
static app::Clusters::Scenes::Structs::AttributeValuePair::Type CCPairs[8];

static uint8_t OO_buffer[scenes::kMaxFieldsPerCluster] = { 0 };
static uint8_t LC_buffer[scenes::kMaxFieldsPerCluster] = { 0 };
static uint8_t CC_buffer[scenes::kMaxFieldsPerCluster] = { 0 };

/// @brief Simulates a Handler where Endpoint 1 supports onoff and level control and Endpoint 2 supports onoff and color control
class TestSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    static constexpr uint8_t kMaxValueSize = 4;
    static constexpr uint8_t kMaxAvPair    = 15;

    TestSceneHandler() = default;
    ~TestSceneHandler() override {}

    // Default function only checks if endpoint and clusters are valid
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        bool ret = false;
        if (endpoint == TEST_ENDPOINT1)
        {
            if (cluster == ON_OFF_CID || cluster == LV_CTR_CID)
            {
                ret = true;
            }
        }

        if (endpoint == TEST_ENDPOINT2)
        {
            if (cluster == ON_OFF_CID || cluster == CC_CTR_CID)
            {
                ret = true;
            }
        }

        return ret;
    }

    /// @brief Simulates save from cluster, data is already in an EFS struct but this isn't mandatory
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serialyzedBytes data to serialize into EFS
    /// @return success if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT if endpoint or cluster not supported
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serialyzedBytes) override
    {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

        if (endpoint == TEST_ENDPOINT1)
        {
            switch (cluster)
            {
            case ON_OFF_CID:
                err = CHIP_NO_ERROR;
                memcpy(serialyzedBytes.data(), OO_buffer, scenes::kMaxFieldsPerCluster);
                serialyzedBytes.reduce_size(15); // Used memory for OnOff TLV
                break;
            case LV_CTR_CID:
                err = CHIP_NO_ERROR;
                memcpy(serialyzedBytes.data(), LC_buffer, scenes::kMaxFieldsPerCluster);
                serialyzedBytes.reduce_size(27); // Used memory for Level Control TLV
                break;
            default:
                break;
            }
        }
        if (endpoint == TEST_ENDPOINT2)
        {
            switch (cluster)
            {
            case ON_OFF_CID:
                err = CHIP_NO_ERROR;
                memcpy(serialyzedBytes.data(), OO_buffer, scenes::kMaxFieldsPerCluster);
                serialyzedBytes.reduce_size(15); // Used memory for Color Control TLV
                break;
            case CC_CTR_CID:
                err = CHIP_NO_ERROR;
                memcpy(serialyzedBytes.data(), CC_buffer, scenes::kMaxFieldsPerCluster);
                serialyzedBytes.reduce_size(99); // Used memory for Color Control TLV
                break;
            default:
                break;
            }
        }
        return err;
    }

    /// @brief Simulates EFS being applied to a scene, here just validates that the data is as expected, no action taken by the
    /// "cluster"
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serialyzedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR
    ApplyScene(EndpointId endpoint, ClusterId cluster, ByteSpan & serialyzedBytes, TransitionTimeMs timeMs) override
    {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

        // Takes values from cluster in Endpoint 1
        if (endpoint == TEST_ENDPOINT1)
        {
            switch (cluster)
            {
            case ON_OFF_CID:
                if (!memcmp(serialyzedBytes.data(), OO_buffer, serialyzedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            case LV_CTR_CID:
                if (!memcmp(serialyzedBytes.data(), LC_buffer, serialyzedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }

        // Takes values from cluster in Endpoint 2
        if (endpoint == TEST_ENDPOINT2)
        {
            switch (cluster)
            {
            case ON_OFF_CID:
                if (!memcmp(serialyzedBytes.data(), OO_buffer, serialyzedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            case CC_CTR_CID:
                if (!memcmp(serialyzedBytes.data(), CC_buffer, serialyzedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }

        return CHIP_NO_ERROR;
    }
};

static chip::TestPersistentStorageDelegate testStorage;
static SceneTableImpl sSceneTable;
static TestSceneHandler sHandler;

void ResetSceneTable(SceneTableImpl * sceneTable)
{
    sceneTable->RemoveFabric(kFabric1);
    sceneTable->RemoveFabric(kFabric2);
}

void TestHandlerFunctions(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    ClusterId tempCluster       = 0;

    app::Clusters::Scenes::Structs::ExtensionFieldSet::Type extensionFieldSetOut;
    app::Clusters::Scenes::Structs::ExtensionFieldSet::DecodableType extensionFieldSetIn;
    app::Clusters::Scenes::Structs::AttributeValuePair::DecodableType aVPair;

    TLV::TLVReader reader;
    TLV::TLVWriter writer;
    TLV::TLVType outer;
    TLV::TLVType outerRead;

    static const uint8_t OO_av_payload[1]    = { 0x01 };
    static const uint8_t LC_av_payload[2][2] = { { 0x40, 0x00 }, { 0x01, 0xF0 } };
    static const uint8_t CC_av_payload[8][2] = { { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 },
                                                 { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 }, { 0x00, 0x00 } };

    OOPairs[0].attributeID.SetValue(ON_OFF_ID);
    OOPairs[0].attributeValue = OO_av_payload;

    LCPairs[0].attributeID.SetValue(CURRENT_LVL_ID);
    LCPairs[0].attributeValue = LC_av_payload[0];
    LCPairs[0].attributeValue.reduce_size(1);
    LCPairs[1].attributeID.SetValue(CURRENT_FRQ_ID);
    LCPairs[1].attributeValue = LC_av_payload[1];

    CCPairs[0].attributeID.SetValue(CURRENT_SAT_ID);
    CCPairs[0].attributeValue = CC_av_payload[0];
    CCPairs[0].attributeValue.reduce_size(1);
    CCPairs[1].attributeID.SetValue(CURRENT_X_ID);
    CCPairs[1].attributeValue = CC_av_payload[1];
    CCPairs[2].attributeID.SetValue(CURRENT_Y_ID);
    CCPairs[2].attributeValue = CC_av_payload[2];
    CCPairs[3].attributeID.SetValue(COLOR_TEMP_MIR_ID);
    CCPairs[3].attributeValue = CC_av_payload[3];
    CCPairs[4].attributeID.SetValue(EN_CURRENT_HUE_ID);
    CCPairs[4].attributeValue = CC_av_payload[4];
    CCPairs[5].attributeID.SetValue(C_LOOP_ACTIVE_ID);
    CCPairs[5].attributeValue = CC_av_payload[5];
    CCPairs[5].attributeValue.reduce_size(1);
    CCPairs[6].attributeID.SetValue(C_LOOP_DIR_ID);
    CCPairs[6].attributeValue = CC_av_payload[6];
    CCPairs[6].attributeValue.reduce_size(1);
    CCPairs[7].attributeID.SetValue(C_LOOP_TIME_ID);
    CCPairs[7].attributeValue = CC_av_payload[7];

    // Initialize Extension Field sets as if they were received by add commands
    OOextensionFieldSet.clusterID          = ON_OFF_CID;
    OOextensionFieldSet.attributeValueList = OOPairs;
    LCextensionFieldSet.clusterID          = LV_CTR_CID;
    LCextensionFieldSet.attributeValueList = LCPairs;
    CCextensionFieldSet.clusterID          = CC_CTR_CID;
    CCextensionFieldSet.attributeValueList = CCPairs;

    ByteSpan OO_list(OO_buffer);
    ByteSpan LC_list(LC_buffer);
    ByteSpan CC_list(CC_buffer);

    uint8_t buffer[scenes::kMaxFieldsPerCluster] = { 0 };
    MutableByteSpan buff_span(buffer);

    // Serialize Extension Field sets as if they were recovered from memory
    writer.Init(OO_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              OOextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    writer.Init(LC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              LCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    writer.Init(CC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              CCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    // Test Registering SceneHandler
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RegisterHandler(&sHandler));
    NL_TEST_ASSERT(aSuite, sceneTable->GetHandlerNum() == 1);

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(OO_list);
    extensionFieldSetIn.clusterID = ON_OFF_CID;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(TEST_ENDPOINT1, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == ON_OFF_CID);
    memset(buffer, 0, buff_span.size());

    // Setup the Level Control Extension field set in the expected state from a command
    reader.Init(LC_list);
    extensionFieldSetIn.clusterID = LV_CTR_CID;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(TEST_ENDPOINT1, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == LV_CTR_CID);
    memset(buffer, 0, buff_span.size());

    // Setup the Color control Extension field set in the expected state from a command
    reader.Init(CC_list);
    extensionFieldSetIn.clusterID = CC_CTR_CID;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(TEST_ENDPOINT2, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == CC_CTR_CID);
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for on off
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.Deserialize(TEST_ENDPOINT1, ON_OFF_CID, OO_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.EndContainer(outer));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for level control
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.Deserialize(TEST_ENDPOINT1, LV_CTR_CID, LC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.EndContainer(outer));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for color control
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.Deserialize(TEST_ENDPOINT2, CC_CTR_CID, CC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == writer.EndContainer(outer));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
};

void TestStoreScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    NL_TEST_ASSERT(aSuite, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    // Populate scene1's EFS (Endpoint1)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene1, ON_OFF_CID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene1, LV_CTR_CID));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == sceneTable->SceneSaveEFS(scene1, CC_CTR_CID));

    // Populate scene2's EFS (Endpoint1)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene2, ON_OFF_CID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene2, LV_CTR_CID));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == sceneTable->SceneSaveEFS(scene2, CC_CTR_CID));

    // Populate scene3's EFS (Endpoint2)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene3, ON_OFF_CID));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == sceneTable->SceneSaveEFS(scene3, LV_CTR_CID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene3, CC_CTR_CID));

    // Populate scene3's EFS (Endpoint2)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene4, ON_OFF_CID));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_ARGUMENT == sceneTable->SceneSaveEFS(scene4, LV_CTR_CID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene4, CC_CTR_CID));

    SceneTableEntry scene;

    // Set test
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene8));

    // Too many scenes 1 fabric
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_LIST_LENGTH == sceneTable->SetSceneTableEntry(kFabric1, scene9));

    // Not Found
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));

    // Get test
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(kFabric1, scene1.mStorageId));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(kFabric1, scene2.mStorageId));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(kFabric1, scene3.mStorageId));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(kFabric1, scene4.mStorageId));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));
    NL_TEST_ASSERT(aSuite, scene == scene8);
}

void TestOverwriteScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    NL_TEST_ASSERT(aSuite, sceneTable);

    SceneTableEntry scene;
    // Overwriting the first entry
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene10));
    // Overwriting in the middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene11));
    // Overwriting the last entry
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene12));

    // Scene 10 has the same sceneId as scene 1, Get->sceneId1 should thus return scene 10, etc.
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene11);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));
    NL_TEST_ASSERT(aSuite, scene == scene12);
}

void TestIterateScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    NL_TEST_ASSERT(aSuite, sceneTable);

    SceneTableEntry scene;
    auto * iterator = sceneTable->IterateSceneEntry(kFabric1);

    NL_TEST_ASSERT(aSuite, iterator != nullptr);

    if (iterator)
    {
        NL_TEST_ASSERT(aSuite, iterator->Count() == 8);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene10);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene2);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene3);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene4);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene11);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene6);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene7);
        NL_TEST_ASSERT(aSuite, iterator->Next(scene));
        NL_TEST_ASSERT(aSuite, scene == scene12);

        NL_TEST_ASSERT(aSuite, iterator->Next(scene) == false);

        iterator->Release();
    }
}

void TestRemoveScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    NL_TEST_ASSERT(aSuite, sceneTable);

    SceneTableEntry scene;

    // Remove middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene5.mStorageId));
    auto * iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Adde scene in middle, a spot should have been freed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene9));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 8);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    NL_TEST_ASSERT(aSuite, scene == scene9);
    iterator->Release();

    // Remove the recently added scene 9
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Remove first
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene1.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 6);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    iterator->Release();

    // Remove Next
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 5);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 4);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 3);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene6.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene7.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 1);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene12);
    iterator->Release();

    // Remove last
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene8.mStorageId));
    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene) == false);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->RemoveSceneTableEntry(kFabric1, scene8.mStorageId));

    iterator = sceneTable->IterateSceneEntry(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    iterator->Release();
}

void TestFabricScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    NL_TEST_ASSERT(aSuite, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    SceneTableEntry scene;

    // Fabric 1 inserts
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));

    // Fabric 2 inserts
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);

    // Remove Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric1));
    // Verify Fabric 1 removed
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->RemoveFabric(kFabric1));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));

    // Verify Fabric 2 still there
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);

    // Remove Fabric 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric2));
    // Verify Fabric 2 removed
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->RemoveFabric(kFabric2));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
}

} // namespace

/**
 *  Tear down the test suite.
 */
int TestSetup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);
    VerifyOrReturnError(CHIP_NO_ERROR == sSceneTable.Init(&testStorage), FAILURE);

    SetSceneTable(&sSceneTable);

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestTeardown(void * inContext)
{
    SceneTableImpl * sceneTable = chip::scenes::GetSceneTable();
    if (nullptr != sceneTable)
    {
        sceneTable->Finish();
    }
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}
int TestSceneTable()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestStoreScenes", TestHandlerFunctions),
                               NL_TEST_DEF("TestStoreScenes", TestStoreScenes),
                               NL_TEST_DEF("TestOverwriteScenes", TestOverwriteScenes),
                               NL_TEST_DEF("TestIterateScenes", TestIterateScenes),
                               NL_TEST_DEF("TestRemoveScenes", TestRemoveScenes),
                               NL_TEST_DEF("TestFabricScenes", TestFabricScenes),
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

CHIP_REGISTER_TEST_SUITE(TestSceneTable)
