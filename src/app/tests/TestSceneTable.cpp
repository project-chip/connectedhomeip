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
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>

using namespace chip;

using SceneTable        = scenes::SceneTable<scenes::ExtensionFieldSetsImpl>;
using SceneTableEntry   = scenes::DefaultSceneTableImpl::SceneTableEntry;
using SceneTableImpl    = scenes::DefaultSceneTableImpl;
using SceneStorageId    = scenes::DefaultSceneTableImpl::SceneStorageId;
using SceneData         = scenes::DefaultSceneTableImpl::SceneData;
using ExtensionFieldSet = scenes::ExtensionFieldSet;
using TransitionTimeMs  = scenes::TransitionTimeMs;

namespace {

// Group constants
constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 4;

// Test Cluster ID
constexpr chip::ClusterId kOnOffClusterId        = 0x0006;
constexpr chip::ClusterId kLevelControlClusterId = 0x0008;
constexpr chip::ClusterId kColorControlClusterId = 0x0300;

// Test Endpoint ID
constexpr chip::EndpointId kTestEndpoint1 = 0x0001;
constexpr chip::EndpointId kTestEndpoint2 = 0x0099;
constexpr chip::EndpointId kTestEndpoint3 = 0x0010;

// Test Attribute ID
constexpr uint32_t kOnOffAttId               = 0x0000;
constexpr uint32_t kCurrentLevelId           = 0x0000;
constexpr uint32_t kCurrentFrequencyId       = 0x0004;
constexpr uint32_t kCurrentSaturationId      = 0x0001;
constexpr uint32_t kCurrentXId               = 0x0003;
constexpr uint32_t kCurrentYId               = 0x0004;
constexpr uint32_t kColorTemperatureMiredsId = 0x0007;
constexpr uint32_t kEnhancedCurrentHueId     = 0x4000;
constexpr uint32_t kColorLoopActiveId        = 0x4002;
constexpr uint32_t kColorLoopDirectionId     = 0x4003;
constexpr uint32_t kColorLoopTimeId          = 0x4004;

// Test fabrics, adding more requires to modify the "ResetSceneTable" function
constexpr chip::FabricIndex kFabric1 = 1;
constexpr chip::FabricIndex kFabric2 = 7;

// Scene storage ID
static const SceneStorageId sceneId1(kTestEndpoint1, 0xAA, 0x101);
static const SceneStorageId sceneId2(kTestEndpoint1, 0xBB, 0x00);
static const SceneStorageId sceneId3(kTestEndpoint2, 0xCC, 0x102);
static const SceneStorageId sceneId4(kTestEndpoint2, 0xBE, 0x00);
static const SceneStorageId sceneId5(kTestEndpoint1, 0x45, 0x103);
static const SceneStorageId sceneId6(kTestEndpoint1, 0x65, 0x00);
static const SceneStorageId sceneId7(kTestEndpoint1, 0x77, 0x101);
static const SceneStorageId sceneId8(kTestEndpoint3, 0xEE, 0x101);
static const SceneStorageId sceneId9(kTestEndpoint2, 0xAB, 0x101);

CharSpan empty;

// Scene data
static const SceneData sceneData1(CharSpan("Scene #1"));
static const SceneData sceneData2(CharSpan("Scene #2"), 2, 5);
static const SceneData sceneData3(CharSpan("Scene #3"), 25);
static const SceneData sceneData4(CharSpan("Scene num4"), 5);
static const SceneData sceneData5(empty);
static const SceneData sceneData6(CharSpan("Scene #6"), 3, 15);
static const SceneData sceneData7(CharSpan("Scene #7"), 20, 5);
static const SceneData sceneData8(CharSpan("NAME TOO LOOONNG!"), 1, 10);
static const SceneData sceneData9(CharSpan("Scene #9"), 30, 15);
static const SceneData sceneData10(CharSpan("Scene #10"), 10, 1);
static const SceneData sceneData11(CharSpan("Scene #11"), 20, 10);
static const SceneData sceneData12(CharSpan("Scene #12"), 30, 5);

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

static uint8_t OO_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t LC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t CC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };

static uint32_t OO_buffer_serialized_length = 0;
static uint32_t LC_buffer_serialized_length = 0;
static uint32_t CC_buffer_serialized_length = 0;

// Group related data
constexpr chip::GroupId kGroup1 = kMinFabricGroupId;
constexpr chip::GroupId kGroup2 = 0x2222;
constexpr chip::GroupId kGroup3 = 0x1111;

static const chip::Credentials::GroupDataProvider::GroupInfo kGroupInfo1_1(kGroup1, "Group-1.1");
static const chip::Credentials::GroupDataProvider::GroupInfo kGroupInfo1_2(kGroup2, "Group-1.2");
static const chip::Credentials::GroupDataProvider::GroupInfo kGroupInfo1_3(kGroup3, "Group-1.3");

/// @brief Simulates a Handler where Endpoint 1 supports onoff and level control and Endpoint 2 supports onoff and color control
class TestSceneHandler : public scenes::DefaultSceneHandlerImpl
{
public:
    TestSceneHandler() = default;
    ~TestSceneHandler() override {}

    // Fills in cluster buffer and adjusts its size to lower than the maximum number of cluster per scenes
    virtual void GetSupportedClusters(EndpointId endpoint, Span<ClusterId> & clusterBuffer) override
    {
        ClusterId * buffer = clusterBuffer.data();
        if (endpoint == kTestEndpoint1)
        {
            if (clusterBuffer.size() >= 2)
            {
                buffer[0] = kOnOffClusterId;
                buffer[1] = kLevelControlClusterId;
                clusterBuffer.reduce_size(2);
            }
        }
        else if (endpoint == kTestEndpoint2)
        {
            if (clusterBuffer.size() >= 2)
            {
                buffer[0] = kOnOffClusterId;
                buffer[1] = kColorControlClusterId;
                clusterBuffer.reduce_size(2);
            }
        }
        else if (endpoint == kTestEndpoint3)
        {
            if (clusterBuffer.size() >= 3)
            {
                buffer[0] = kOnOffClusterId;
                buffer[1] = kLevelControlClusterId;
                buffer[2] = kColorControlClusterId;
                clusterBuffer.reduce_size(3);
            }
        }
    }

    // Default function only checks if endpoint and clusters are valid
    bool SupportsCluster(EndpointId endpoint, ClusterId cluster) override
    {
        if (endpoint == kTestEndpoint1)
        {
            if (cluster == kOnOffClusterId || cluster == kLevelControlClusterId)
            {
                return true;
            }
        }

        if (endpoint == kTestEndpoint2)
        {
            if (cluster == kOnOffClusterId || cluster == kColorControlClusterId)
            {
                return true;
            }
        }

        if (endpoint == kTestEndpoint3)
        {
            if (cluster == kOnOffClusterId || cluster == kLevelControlClusterId || cluster == kColorControlClusterId)
            {
                return true;
            }
        }

        return false;
    }

    /// @brief Simulates save from cluster, data is already in an EFS struct but this isn't mandatory
    /// @param endpoint target endpoint
    /// @param cluster  target cluster
    /// @param serialisedBytes data to serialize into EFS
    /// @return success if successfully serialized the data, CHIP_ERROR_INVALID_ARGUMENT if endpoint or cluster not supported
    CHIP_ERROR SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serialisedBytes) override
    {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

        if (endpoint == kTestEndpoint1)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                err = CHIP_NO_ERROR;
                // Warning: OO_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), OO_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(OO_buffer_serialized_length); // Used memory for OnOff TLV
                break;
            case kLevelControlClusterId:
                err = CHIP_NO_ERROR;
                // Warning: LC_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), LC_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(LC_buffer_serialized_length); // Used memory for Level Control TLV
                break;
            default:
                break;
            }
        }
        if (endpoint == kTestEndpoint2)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                err = CHIP_NO_ERROR;
                // Warning: OO_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), OO_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(OO_buffer_serialized_length); // Used memory for OnOff TLV
                break;
            case kColorControlClusterId:
                err = CHIP_NO_ERROR;
                // Warning: CC_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), CC_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(CC_buffer_serialized_length); // Used memory for Color Control TLV
                break;
            default:
                break;
            }
        }
        if (endpoint == kTestEndpoint3)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                err = CHIP_NO_ERROR;
                // Warning: OO_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), OO_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(OO_buffer_serialized_length); // Used memory for OnOff TLV
                break;
            case kLevelControlClusterId:
                err = CHIP_NO_ERROR;
                // Warning: LC_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), LC_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(LC_buffer_serialized_length); // Used memory for Level Control TLV
                break;
            case kColorControlClusterId:
                err = CHIP_NO_ERROR;
                // Warning: CC_buffer needs to be populated before calling this function
                memcpy(serialisedBytes.data(), CC_buffer, scenes::kMaxFieldBytesPerCluster);
                // Warning: serialized size of the buffer must also be computed before calling this function
                serialisedBytes.reduce_size(CC_buffer_serialized_length); // Used memory for Color Control TLV
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
    /// @param serialisedBytes Data from nvm
    /// @param timeMs transition time in ms
    /// @return CHIP_NO_ERROR if value as expected, CHIP_ERROR_INVALID_ARGUMENT otherwise
    CHIP_ERROR
    ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serialisedBytes, TransitionTimeMs timeMs) override
    {
        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

        // Takes values from cluster in Endpoint 1
        if (endpoint == kTestEndpoint1)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                if (!memcmp(serialisedBytes.data(), OO_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            case kLevelControlClusterId:
                if (!memcmp(serialisedBytes.data(), LC_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }

        // Takes values from cluster in Endpoint 2
        if (endpoint == kTestEndpoint2)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                if (!memcmp(serialisedBytes.data(), OO_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            case kColorControlClusterId:
                if (!memcmp(serialisedBytes.data(), CC_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }

        // Takes values from cluster in Endpoint 3
        if (endpoint == kTestEndpoint3)
        {
            switch (cluster)
            {
            case kOnOffClusterId:
                if (!memcmp(serialisedBytes.data(), OO_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            case kLevelControlClusterId:
                if (!memcmp(serialisedBytes.data(), LC_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
            case kColorControlClusterId:
                if (!memcmp(serialisedBytes.data(), CC_buffer, serialisedBytes.size()))
                {
                    err = CHIP_NO_ERROR;
                }
                break;
            default:
                break;
            }
        }

        return err;
    }
};

// Storage
static chip::TestPersistentStorageDelegate testStorage;

// Groups
static chip::Crypto::DefaultSessionKeystore sSessionKeystore;
static chip::Credentials::GroupDataProviderImpl sProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);
// Scene
static SceneTableImpl sSceneTable;
static TestSceneHandler sHandler;

void ResetSceneTable(SceneTable * sceneTable)
{
    sceneTable->RemoveFabric(kFabric1);
    sceneTable->RemoveFabric(kFabric2);
}

void ResetProvider(chip::Credentials::GroupDataProvider * provider)
{
    provider->RemoveFabric(kFabric1);
    provider->RemoveFabric(kFabric2);
}

void TestHandlerRegistration(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = &sSceneTable;
    TestSceneHandler tmpHandler[scenes::kMaxSceneHandlers];

    for (uint8_t i = 0; i < scenes::kMaxSceneHandlers; i++)
    {
        NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == i);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RegisterHandler(&tmpHandler[i]));
    }
    // Hanlder order in table : [H0, H1, H2]

    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == scenes::kMaxSceneHandlers);
    // Removal at beginning
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->UnregisterHandler(&tmpHandler[0]));
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == static_cast<uint8_t>(scenes::kMaxSceneHandlers - 1));
    // Confirm array was compressed and last position is now null
    NL_TEST_ASSERT(aSuite, nullptr == sceneTable->mHandlers[scenes::kMaxSceneHandlers - 1]);
    // Re-insert
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RegisterHandler(&tmpHandler[0]));
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == static_cast<uint8_t>(scenes::kMaxSceneHandlers));
    // Hanlder order in table : [H1, H2, H0]

    // Removal at the middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->UnregisterHandler(&tmpHandler[2]));
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == static_cast<uint8_t>(scenes::kMaxSceneHandlers - 1));
    // Confirm array was compressed and last position is now null
    NL_TEST_ASSERT(aSuite, nullptr == sceneTable->mHandlers[scenes::kMaxSceneHandlers - 1]);
    // Re-insert
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RegisterHandler(&tmpHandler[2]));
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == static_cast<uint8_t>(scenes::kMaxSceneHandlers));
    // Hanlder order in table : [H1, H0, H2]

    // Removal at the end
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->UnregisterHandler(&tmpHandler[2]));
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == static_cast<uint8_t>(scenes::kMaxSceneHandlers - 1));
    NL_TEST_ASSERT(aSuite, nullptr == sceneTable->mHandlers[scenes::kMaxSceneHandlers - 1]);

    // Emptying Handler array
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->UnregisterAllHandlers());
    for (uint8_t i = 0; i < scenes::kMaxSceneHandlers; i++)
    {
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->UnregisterHandler(&tmpHandler[i]));
    }

    // Verify the handler num has been updated properly
    NL_TEST_ASSERT(aSuite, sceneTable->mNumHandlers == 0);

    // Verify all array is empty
    for (uint8_t i = 0; i < scenes::kMaxSceneHandlers; i++)
    {
        NL_TEST_ASSERT(aSuite, nullptr == sceneTable->mHandlers[i]);
    }
}

void TestHandlerFunctions(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = &sSceneTable;
    ClusterId tempCluster   = 0;

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

    OOPairs[0].attributeID.SetValue(kOnOffAttId);
    OOPairs[0].attributeValue = OO_av_payload;

    LCPairs[0].attributeID.SetValue(kCurrentLevelId);
    LCPairs[0].attributeValue = LC_av_payload[0];
    LCPairs[0].attributeValue.reduce_size(1);
    LCPairs[1].attributeID.SetValue(kCurrentFrequencyId);
    LCPairs[1].attributeValue = LC_av_payload[1];

    CCPairs[0].attributeID.SetValue(kCurrentSaturationId);
    CCPairs[0].attributeValue = CC_av_payload[0];
    CCPairs[0].attributeValue.reduce_size(1);
    CCPairs[1].attributeID.SetValue(kCurrentXId);
    CCPairs[1].attributeValue = CC_av_payload[1];
    CCPairs[2].attributeID.SetValue(kCurrentYId);
    CCPairs[2].attributeValue = CC_av_payload[2];
    CCPairs[3].attributeID.SetValue(kColorTemperatureMiredsId);
    CCPairs[3].attributeValue = CC_av_payload[3];
    CCPairs[4].attributeID.SetValue(kEnhancedCurrentHueId);
    CCPairs[4].attributeValue = CC_av_payload[4];
    CCPairs[5].attributeID.SetValue(kColorLoopActiveId);
    CCPairs[5].attributeValue = CC_av_payload[5];
    CCPairs[5].attributeValue.reduce_size(1);
    CCPairs[6].attributeID.SetValue(kColorLoopDirectionId);
    CCPairs[6].attributeValue = CC_av_payload[6];
    CCPairs[6].attributeValue.reduce_size(1);
    CCPairs[7].attributeID.SetValue(kColorLoopTimeId);
    CCPairs[7].attributeValue = CC_av_payload[7];

    // Initialize Extension Field sets as if they were received by add commands
    OOextensionFieldSet.clusterID          = kOnOffClusterId;
    OOextensionFieldSet.attributeValueList = OOPairs;
    LCextensionFieldSet.clusterID          = kLevelControlClusterId;
    LCextensionFieldSet.attributeValueList = LCPairs;
    CCextensionFieldSet.clusterID          = kColorControlClusterId;
    CCextensionFieldSet.attributeValueList = CCPairs;

    ByteSpan OO_list(OO_buffer);
    ByteSpan LC_list(LC_buffer);
    ByteSpan CC_list(CC_buffer);

    uint8_t buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
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
    OO_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(LC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              LCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);
    LC_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(CC_buffer);
    writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       app::DataModel::Encode(writer,
                                              TLV::ContextTag(to_underlying(
                                                  app::Clusters::Scenes::Structs::ExtensionFieldSet::Fields::kAttributeValueList)),
                                              CCextensionFieldSet.attributeValueList));
    writer.EndContainer(outer);
    CC_buffer_serialized_length = writer.GetLengthWritten();

    // Test Registering SceneHandler
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RegisterHandler(&sHandler));
    NL_TEST_ASSERT(aSuite, sceneTable->GetHandlerNum() == 1);

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(OO_list);
    extensionFieldSetIn.clusterID = kOnOffClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldSetIn, tempCluster, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == kOnOffClusterId);
    memset(buffer, 0, buff_span.size());

    // Setup the Level Control Extension field set in the expected state from a command
    reader.Init(LC_list);
    extensionFieldSetIn.clusterID = kLevelControlClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldSetIn, tempCluster, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == kLevelControlClusterId);
    memset(buffer, 0, buff_span.size());

    // Setup the Color control Extension field set in the expected state from a command
    reader.Init(CC_list);
    extensionFieldSetIn.clusterID = kColorControlClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.EnterContainer(outerRead));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.ExitContainer(outerRead));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint2, extensionFieldSetIn, tempCluster, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()));
    NL_TEST_ASSERT(aSuite, tempCluster == kColorControlClusterId);
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for on off
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint1, kOnOffClusterId, OO_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for on off previously
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
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint1, kLevelControlClusterId, LC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for level control previously
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
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint2, kColorControlClusterId, CC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for color control previously
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
    SceneTable * sceneTable = &sSceneTable;
    NL_TEST_ASSERT(aSuite, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    // Populate scene1's EFS (Endpoint1)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene1));

    // Populate scene2's EFS (Endpoint1)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene2));

    // Populate scene3's EFS (Endpoint2)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene3));

    // Populate scene4's EFS (Endpoint2)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene4));

    // Populate scene8's EFS (Endpoint3)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneSaveEFS(scene8));

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

    // Too many scenes for 1 fabric
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
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(kFabric1, scene8.mStorageId));
}

void TestOverwriteScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = &sSceneTable;
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
    SceneTable * sceneTable = &sSceneTable;
    NL_TEST_ASSERT(aSuite, sceneTable);

    SceneTableEntry scene;
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);

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
    SceneTable * sceneTable = &sSceneTable;
    NL_TEST_ASSERT(aSuite, sceneTable);

    SceneTableEntry scene;

    // Removing non-existing entry should not return errors
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));

    // Remove middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene5.mStorageId));
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Add scene in middle, a spot should have been freed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene9));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 8);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    NL_TEST_ASSERT(aSuite, scene == scene9);
    iterator->Release();

    // Remove the recently added scene 9
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Remove first
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntryAtPosition(kFabric1, 0));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 6);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    iterator->Release();

    // Remove Next
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 5);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 4);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 3);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene6.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene7.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 1);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene12);
    iterator->Release();

    // Remove last
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene8.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene) == false);
    iterator->Release();

    // Remove at empty position, shouldn't trigger error
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntryAtPosition(kFabric1, chip::scenes::kMaxScenePerFabric - 1));

    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    iterator->Release();
}

void TestFabricScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = &sSceneTable;
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

void TestGroupScenesInteraction(nlTestSuite * aSuite, void * aContext)
{
    chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
    NL_TEST_ASSERT(aSuite, provider);
    SceneTable * sceneTable = &sSceneTable;
    NL_TEST_ASSERT(aSuite, sceneTable);

    // Scene Entry
    SceneTableEntry scene;

    // Reset test
    ResetSceneTable(sceneTable);
    ResetProvider(provider);

    // Group Info
    chip::Credentials::GroupDataProvider::GroupInfo group;

    // Setup Group Data in Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_3));

    // Validate data written properly
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 0, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 1, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 2, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_3);

    // Setup Scene Data in Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));

    // Validate data written properly
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);

    // Setup Group Data in Fabric 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo1_3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo1_1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->SetGroupInfoAt(kFabric2, 2, kGroupInfo1_2));

    // Validate data written properly
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 0, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 1, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 2, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_2);

    // Setup Scene Data in Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));

    // Validate data written properly
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);

    // Verify removing a Fabric Scene Data doesn't impact Fabric Group Data
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric1));

    // Scene Entry
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));

    // Group Info
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 0, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 1, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric2, 2, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 0, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 1, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->GetGroupInfoAt(kFabric1, 2, group));
    NL_TEST_ASSERT(aSuite, group == kGroupInfo1_3);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->RemoveFabric(kFabric1));

    // Group Info
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric1, 0, group));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric1, 1, group));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric1, 2, group));

    // Verify removing a Fabric Group Data doesn't impact Fabric Scene Data
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == provider->RemoveFabric(kFabric2));

    // Group Info
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric2, 0, group));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric2, 1, group));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == provider->GetGroupInfoAt(kFabric2, 2, group));

    // Scene Entry
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric2));

    // Scene Entry
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
    printf("1\n");
    // Initialize Group Data Provider
    sProvider.SetStorageDelegate(&testStorage);
    sProvider.SetSessionKeystore(&sSessionKeystore);
    VerifyOrReturnError(CHIP_NO_ERROR == sProvider.Init(), FAILURE);
    SetGroupDataProvider(&sProvider);
    printf("2\n");

    // Initialize Scene Table
    VerifyOrReturnError(CHIP_NO_ERROR == sSceneTable.Init(&testStorage), FAILURE);
    printf("3\n");

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestTeardown(void * inContext)
{
    sSceneTable.Finish();
    chip::Credentials::GroupDataProvider * provider = chip::Credentials::GetGroupDataProvider();
    if (nullptr != provider)
    {
        provider->Finish();
    }
    chip::Platform::MemoryShutdown();

    return SUCCESS;
}

int TestSceneTable()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestHandlerRegistration", TestHandlerRegistration),
                               NL_TEST_DEF("TestHandlerFunctions", TestHandlerFunctions),
                               NL_TEST_DEF("TestStoreScenes", TestStoreScenes),
                               NL_TEST_DEF("TestOverwriteScenes", TestOverwriteScenes),
                               NL_TEST_DEF("TestIterateScenes", TestIterateScenes),
                               NL_TEST_DEF("TestRemoveScenes", TestRemoveScenes),
                               NL_TEST_DEF("TestFabricScenes", TestFabricScenes),
                               NL_TEST_DEF("TestGroupScenesInteraction", TestGroupScenesInteraction),

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
