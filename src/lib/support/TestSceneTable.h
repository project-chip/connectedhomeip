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

#pragma once

#include <app/clusters/scenes/SceneTableImpl.h>

namespace chip {

namespace SceneTesting {

using FabricIndex        = chip::FabricIndex;
using SceneTableEntry    = chip::scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneTableImpl     = chip::scenes::DefaultSceneTableImpl;
using SceneStorageId     = chip::scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData          = chip::scenes::DefaultSceneTableImpl::SceneData;
using ExtensionFieldsSet = chip::scenes::ExtensionFieldsSet;
using TransitionTimeMs   = chip::scenes::TransitionTimeMs;

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

static TestSceneHandler sHandler;

CHIP_ERROR scene_handler_test(SceneTableImpl * provider)
{
    ClusterId tempCluster = 0;

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
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        OOextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    writer.Init(LC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        LCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    writer.Init(CC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        CCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);

    ReturnErrorOnFailure(provider->RegisterHandler(&sHandler));

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(OO_list);
    extensionFieldSetIn.clusterID = ON_OFF_CID;
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(outerRead));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(extensionFieldSetIn.attributeValueList.Decode(reader));
    ReturnErrorOnFailure(reader.ExitContainer(outerRead));

    ReturnErrorOnFailure(sHandler.SerializeAdd(TEST_ENDPOINT1, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    VerifyOrReturnError(0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_WRITE_FAILED);
    VerifyOrReturnError(tempCluster == ON_OFF_CID, CHIP_ERROR_WRITE_FAILED);
    memset(buffer, 0, buff_span.size());

    // Setup the Level Control Extension field set in the expected state from a command
    reader.Init(LC_list);
    extensionFieldSetIn.clusterID = LV_CTR_CID;
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(outerRead));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(extensionFieldSetIn.attributeValueList.Decode(reader));
    ReturnErrorOnFailure(reader.ExitContainer(outerRead));

    ReturnErrorOnFailure(sHandler.SerializeAdd(TEST_ENDPOINT1, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    VerifyOrReturnError(0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_WRITE_FAILED);
    VerifyOrReturnError(tempCluster == LV_CTR_CID, CHIP_ERROR_WRITE_FAILED);
    memset(buffer, 0, buff_span.size());

    // Setup the Color control Extension field set in the expected state from a command
    reader.Init(CC_list);
    extensionFieldSetIn.clusterID = CC_CTR_CID;
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(outerRead));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(extensionFieldSetIn.attributeValueList.Decode(reader));
    ReturnErrorOnFailure(reader.ExitContainer(outerRead));

    ReturnErrorOnFailure(sHandler.SerializeAdd(TEST_ENDPOINT2, tempCluster, buff_span, extensionFieldSetIn));

    // Verify the handler extracted buffer matches the initial field sets
    VerifyOrReturnError(0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_WRITE_FAILED);
    VerifyOrReturnError(tempCluster == CC_CTR_CID, CHIP_ERROR_WRITE_FAILED);
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for on off
    ReturnErrorOnFailure(sHandler.Deserialize(TEST_ENDPOINT1, ON_OFF_CID, OO_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        extensionFieldSetOut.attributeValueList));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    VerifyOrReturnError(0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_READ_FAILED);
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for level control
    ReturnErrorOnFailure(sHandler.Deserialize(TEST_ENDPOINT1, LV_CTR_CID, LC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        extensionFieldSetOut.attributeValueList));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    VerifyOrReturnError(0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_READ_FAILED);
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for color control
    ReturnErrorOnFailure(sHandler.Deserialize(TEST_ENDPOINT2, CC_CTR_CID, CC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as
    writer.Init(buff_span);
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(app::DataModel::Encode(
        writer, TLV::ContextTag(to_underlying(app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
        extensionFieldSetOut.attributeValueList));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    VerifyOrReturnError(0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()), CHIP_ERROR_READ_FAILED);
    memset(buffer, 0, buff_span.size());

    return CHIP_NO_ERROR;
};

CHIP_ERROR scene_store_test(SceneTableImpl * provider, FabricIndex fabric_index, SceneTableEntry & entry)
{
    SceneTableEntry temp;

    LogErrorOnFailure(provider->SetSceneTableEntry(fabric_index, entry));
    LogErrorOnFailure(provider->GetSceneTableEntry(fabric_index, entry.mStorageId, temp));
    VerifyOrReturnError(temp.mStorageId == entry.mStorageId, CHIP_ERROR_WRITE_FAILED);
    VerifyOrReturnError(temp.mStorageData == entry.mStorageData, CHIP_ERROR_WRITE_FAILED);
    LogErrorOnFailure(provider->SceneApplyEFS(fabric_index, temp.mStorageId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR scene_iterator_test(SceneTableImpl * provider, FabricIndex fabric_index, const SceneTableEntry & entry1,
                               const SceneTableEntry & entry2, const SceneTableEntry & entry3)
{
    SceneTableEntry temp;

    auto * iterator = provider->IterateSceneEntry(fabric_index);
    if (iterator)
    {
        VerifyOrReturnError(iterator->Count() == 3, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry1.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry2.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(iterator->Next(temp), CHIP_ERROR_INVALID_ACCESS_TOKEN);
        VerifyOrReturnError(temp.mStorageId == entry3.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

        // Iterator should return false here
        VerifyOrReturnError(iterator->Next(temp) == false, CHIP_ERROR_INVALID_ACCESS_TOKEN);

        iterator->Release();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR scene_remove_test(SceneTableImpl * provider, FabricIndex fabric_index, SceneTableEntry & entry1,
                             SceneTableEntry & entry2, SceneTableEntry & entry3)
{
    SceneTableEntry temp;

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry2.mStorageId));

    auto * iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 2, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Next(temp);
    VerifyOrReturnError(temp.mStorageId == entry1.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Release();

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry1.mStorageId));
    iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 1, CHIP_ERROR_INVALID_ARGUMENT);
    iterator->Next(temp);
    VerifyOrReturnError(temp.mStorageId == entry3.mStorageId, CHIP_ERROR_INVALID_ARGUMENT);

    LogErrorOnFailure(provider->RemoveSceneTableEntry(fabric_index, entry3.mStorageId));
    iterator = provider->IterateSceneEntry(fabric_index);
    VerifyOrReturnError(iterator->Count() == 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Iterator should return false here
    VerifyOrReturnError(iterator->Next(temp) == false, CHIP_ERROR_INVALID_ACCESS_TOKEN);
    iterator->Release();

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestSceneData(SceneTableImpl * provider, FabricIndex fabric_index)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Scene storage ID
    static const SceneStorageId sceneId1(TEST_ENDPOINT1, 0xAA, 0x101);
    static const SceneStorageId sceneId2(TEST_ENDPOINT1, 0xBB, 0x00);
    static const SceneStorageId sceneId3(TEST_ENDPOINT2, 0xCC, 0x102);

    // Scene data
    static const SceneData sceneData1(CharSpan("Scene #1", sizeof("Scene #1")));
    static const SceneData sceneData2(CharSpan("Scene #2", sizeof("Scene #2")), 2, 5);
    static const SceneData sceneData3(CharSpan(), 25);

    // Scenes
    SceneTableEntry scene1(sceneId1, sceneData1);
    SceneTableEntry scene2(sceneId2, sceneData2);
    SceneTableEntry scene3(sceneId3, sceneData3);

    LogErrorOnFailure(scene_handler_test(provider));

    err = provider->SceneSaveEFS(scene1, ON_OFF_CID);
    LogErrorOnFailure(err);
    err = provider->SceneSaveEFS(scene1, LV_CTR_CID);
    LogErrorOnFailure(err);

    err = provider->SceneSaveEFS(scene3, ON_OFF_CID);
    LogErrorOnFailure(err);
    err = provider->SceneSaveEFS(scene3, CC_CTR_CID);
    LogErrorOnFailure(err);

    // Tests
    err = scene_store_test(provider, fabric_index, scene1);
    LogErrorOnFailure(err);
    err = scene_store_test(provider, fabric_index, scene2);
    LogErrorOnFailure(err);
    err = scene_store_test(provider, fabric_index, scene3);
    LogErrorOnFailure(err);

    err = scene_iterator_test(provider, fabric_index, scene1, scene2, scene3);
    LogErrorOnFailure(err);
    err = scene_remove_test(provider, fabric_index, scene1, scene2, scene3);
    LogErrorOnFailure(err);

    return err;
}

} // namespace SceneTesting

} // namespace chip
