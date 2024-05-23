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

#include <app/clusters/scenes-server/SceneTableImpl.h>
#include <app/util/mock/Constants.h>
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

namespace TestScenes {

// TestTableSize
constexpr uint8_t defaultTestTableSize      = 16;
constexpr uint8_t defaultTestFabricCapacity = (defaultTestTableSize - 1) / 2;

// Test Cluster ID
constexpr chip::ClusterId kOnOffClusterId        = 0x0006;
constexpr chip::ClusterId kLevelControlClusterId = 0x0008;
constexpr chip::ClusterId kColorControlClusterId = 0x0300;

// Test Endpoint ID
constexpr chip::EndpointId kTestEndpoint1 = chip::Test::kMockEndpoint1;
constexpr chip::EndpointId kTestEndpoint2 = chip::Test::kMockEndpoint2;
constexpr chip::EndpointId kTestEndpoint3 = chip::Test::kMockEndpoint3;

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

// Test Group ID
constexpr chip::GroupId kGroup1 = 0x101;
constexpr chip::GroupId kGroup2 = 0x102;
constexpr chip::GroupId kGroup3 = 0x103;
constexpr chip::GroupId kGroup4 = 0x00;

// Test Scene ID
constexpr chip::SceneId kScene1 = 0xAA;
constexpr chip::SceneId kScene2 = 0x45;
constexpr chip::SceneId kScene3 = 0x77;
constexpr chip::SceneId kScene4 = 0xED;
constexpr chip::SceneId kScene5 = 0xDE;
constexpr chip::SceneId kScene6 = 0xAB;
constexpr chip::SceneId kScene7 = 0xBB;
constexpr chip::SceneId kScene8 = 0x22;

// Test fabrics, adding more requires to modify the "ResetSceneTable" function
constexpr chip::FabricIndex kFabric1 = 1;
constexpr chip::FabricIndex kFabric2 = 7;
constexpr chip::FabricIndex kFabric3 = 77;

// Scene storage ID
static const SceneStorageId sceneId1(kScene1, kGroup1);
static const SceneStorageId sceneId2(kScene2, kGroup1);
static const SceneStorageId sceneId3(kScene3, kGroup1);
static const SceneStorageId sceneId4(kScene4, kGroup1);
static const SceneStorageId sceneId5(kScene5, kGroup2);
static const SceneStorageId sceneId6(kScene6, kGroup2);
static const SceneStorageId sceneId7(kScene7, kGroup3);
static const SceneStorageId sceneId8(kScene8, kGroup4);
static const SceneStorageId sceneId9(kScene1, kGroup4);
static const SceneStorageId sceneId10(kScene4, kGroup4);
static const SceneStorageId sceneId11(kScene5, kGroup4);
static const SceneStorageId sceneId12(kScene6, kGroup4);

CharSpan empty;

// Scene data
static const SceneData sceneData1("Scene #1"_span);
static const SceneData sceneData2("Scene #2"_span, 2000);
static const SceneData sceneData3("Scene #3"_span, 250);
static const SceneData sceneData4("Scene num4"_span, 5000);
static const SceneData sceneData5(empty);
static const SceneData sceneData6("Scene #6"_span, 3000);
static const SceneData sceneData7("Scene #7"_span, 20000);
static const SceneData sceneData8("NAME TOO LOOONNG!"_span, 15000);
static const SceneData sceneData9("Scene #9"_span, 3000);
static const SceneData sceneData10("Scene #10"_span, 1000);
static const SceneData sceneData11("Scene #11"_span, 50);
static const SceneData sceneData12("Scene #12"_span, 100);
static const SceneData sceneData13("Scene #13"_span, 100);
static const SceneData sceneData14("Scene #14"_span, 100);
static const SceneData sceneData15("Scene #15"_span, 100);

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
SceneTableEntry scene12(sceneId7, sceneData12);
SceneTableEntry scene13(sceneId10, sceneData13);
SceneTableEntry scene14(sceneId11, sceneData14);
SceneTableEntry scene15(sceneId12, sceneData15);

// Clusters EFS data
static app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type OOextensionFieldSet;
static app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type LCextensionFieldSet;
static app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type CCextensionFieldSet;

static app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type OOPairs[1];
static app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type LCPairs[2];
static app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type CCPairs[8];

static uint8_t OO_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t LC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t CC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };

static uint32_t OO_buffer_serialized_length = 0;
static uint32_t LC_buffer_serialized_length = 0;
static uint32_t CC_buffer_serialized_length = 0;

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
        else
        {
            clusterBuffer.reduce_size(0);
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

        if (endpoint == kTestEndpoint1)
        {
            if (cluster == kOnOffClusterId || cluster == kColorControlClusterId)
            {
                return true;
            }
        }

        if (endpoint == kTestEndpoint1)
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

class TestSceneTableImpl : public SceneTableImpl
{
public:
    TestSceneTableImpl(uint16_t maxScenesPerFabric = defaultTestFabricCapacity, uint16_t maxScenesGlobal = defaultTestTableSize) :
        SceneTableImpl(maxScenesPerFabric, maxScenesGlobal)
    {}
    ~TestSceneTableImpl() override {}

protected:
    uint8_t GetClustersFromEndpoint(ClusterId * clusterList, uint8_t listLen) override
    {
        if (listLen >= 3)
        {
            clusterList[0] = kOnOffClusterId;
            clusterList[1] = kLevelControlClusterId;
            clusterList[2] = kColorControlClusterId;
            return 3;
        }

        return 0;
    }

    uint8_t GetClusterCountFromEndpoint() override { return 3; }
};

// Storage
static chip::TestPersistentStorageDelegate testStorage;
// Scene
static TestSceneHandler sHandler;

void ResetSceneTable(SceneTable * sceneTable)
{
    sceneTable->RemoveFabric(kFabric1);
    sceneTable->RemoveFabric(kFabric2);
    sceneTable->RemoveFabric(kFabric3);
}

void TestHandlerRegistration(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    TestSceneHandler tmpHandler[scenes::kMaxClustersPerScene];

    for (uint8_t i = 0; i < scenes::kMaxClustersPerScene; i++)
    {
        sceneTable->RegisterHandler(&tmpHandler[i]);
    }

    // Emptying Handler array
    sceneTable->UnregisterAllHandlers();

    // Verify the handler num has been updated properly
    NL_TEST_ASSERT(aSuite, sceneTable->HandlerListEmpty());

    for (uint8_t i = 0; i < scenes::kMaxClustersPerScene; i++)
    {
        sceneTable->RegisterHandler(&tmpHandler[i]);
    }

    // Hanlder order in table : [H0, H1, H2]

    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());
    // Removal at beginning
    sceneTable->UnregisterHandler(&tmpHandler[0]);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());
    // Re-insert
    sceneTable->RegisterHandler(&tmpHandler[0]);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());
    // Hanlder order in table : [H0, H1, H2]

    // Removal at the middle
    sceneTable->UnregisterHandler(&tmpHandler[2]);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());
    // Re-insert
    sceneTable->RegisterHandler(&tmpHandler[2]);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());
    // Hanlder order in table : [H1, H0, H2]

    // Removal at the end
    sceneTable->UnregisterHandler(&tmpHandler[2]);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());

    // Emptying Handler array
    sceneTable->UnregisterAllHandlers();

    // Verify the handler num has been updated properly
    NL_TEST_ASSERT(aSuite, sceneTable->HandlerListEmpty());
}

void TestHandlerFunctions(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type extensionFieldSetOut;
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType extensionFieldSetIn;

    TLV::TLVReader reader;
    TLV::TLVWriter writer;

    static const uint8_t OO_av_payload     = 0x01;
    static const uint16_t LC_av_payload[2] = { 0x64, 0x01F0 };
    static const uint16_t CC_av_payload[8] = { 0 };

    OOPairs[0].attributeID    = kOnOffAttId;
    OOPairs[0].attributeValue = OO_av_payload;

    LCPairs[0].attributeID    = kCurrentLevelId;
    LCPairs[0].attributeValue = LC_av_payload[0];
    LCPairs[1].attributeID    = kCurrentFrequencyId;
    LCPairs[1].attributeValue = LC_av_payload[1];

    CCPairs[0].attributeID    = kCurrentSaturationId;
    CCPairs[0].attributeValue = CC_av_payload[0];
    CCPairs[1].attributeID    = kCurrentXId;
    CCPairs[1].attributeValue = CC_av_payload[1];
    CCPairs[2].attributeID    = kCurrentYId;
    CCPairs[2].attributeValue = CC_av_payload[2];
    CCPairs[3].attributeID    = kColorTemperatureMiredsId;
    CCPairs[3].attributeValue = CC_av_payload[3];
    CCPairs[4].attributeID    = kEnhancedCurrentHueId;
    CCPairs[4].attributeValue = CC_av_payload[4];
    CCPairs[5].attributeID    = kColorLoopActiveId;
    CCPairs[5].attributeValue = CC_av_payload[5];
    CCPairs[6].attributeID    = kColorLoopDirectionId;
    CCPairs[6].attributeValue = CC_av_payload[6];
    CCPairs[7].attributeID    = kColorLoopTimeId;
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
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), OOextensionFieldSet.attributeValueList));
    OO_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(LC_buffer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), LCextensionFieldSet.attributeValueList));
    LC_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(CC_buffer);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), CCextensionFieldSet.attributeValueList));
    CC_buffer_serialized_length = writer.GetLengthWritten();

    // Test Registering SceneHandler
    sceneTable->RegisterHandler(&sHandler);
    NL_TEST_ASSERT(aSuite, !sceneTable->HandlerListEmpty());

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(OO_list);
    extensionFieldSetIn.clusterID = kOnOffClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));

    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, extensionFieldSetIn.clusterID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Setup the Level Control Extension field set in the expected state from a command
    reader.Init(LC_list);
    extensionFieldSetIn.clusterID = kLevelControlClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));

    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, extensionFieldSetIn.clusterID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Setup the Color control Extension field set in the expected state from a command
    reader.Init(CC_list);
    extensionFieldSetIn.clusterID = kColorControlClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldSetIn.attributeValueList.Decode(reader));

    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, extensionFieldSetIn.clusterID));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    NL_TEST_ASSERT(aSuite, 0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Verify Deserializing is properly filling out output extension field set for on off
    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, kOnOffClusterId));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint1, kOnOffClusterId, OO_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for on off previously
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for level control
    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, kLevelControlClusterId));
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint1, kLevelControlClusterId, LC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for level control previously
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for color control
    NL_TEST_ASSERT(aSuite, sHandler.SupportsCluster(kTestEndpoint1, kColorControlClusterId));
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == sHandler.Deserialize(kTestEndpoint1, kColorControlClusterId, CC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for color control previously
    writer.Init(buff_span);
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    NL_TEST_ASSERT(aSuite, 0 == memcmp(CC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());

    // To test failure on serialize and deserialize when too many pairs are in the field sets
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type extensionFieldFailTestOut;
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType extensionFieldFailTestIn;
    app::Clusters::ScenesManagement::Structs::AttributeValuePair::Type TooManyPairs[16];

    uint8_t payloadOk = 0;
    for (uint8_t i = 0; i < 16; i++)
    {
        TooManyPairs[i].attributeValue = payloadOk;
    }

    extensionFieldFailTestOut.clusterID          = kColorControlClusterId;
    extensionFieldFailTestOut.attributeValueList = TooManyPairs;

    // Give a bigger buffer given we are using too many pairs on purpose
    uint8_t failBuffer[2 * scenes::kMaxFieldBytesPerCluster] = { 0 };
    ByteSpan fail_list(failBuffer);

    // Serialize Extension Field sets as if they were recovered from memory
    writer.Init(failBuffer);
    NL_TEST_ASSERT(
        aSuite, CHIP_NO_ERROR == app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldFailTestOut.attributeValueList));

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(fail_list);
    extensionFieldFailTestIn.clusterID = kColorControlClusterId;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == reader.Next());
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == extensionFieldFailTestIn.attributeValueList.Decode(reader));

    // Verify failure on both serialize and deserialize
    NL_TEST_ASSERT(aSuite,
                   CHIP_ERROR_BUFFER_TOO_SMALL == sHandler.SerializeAdd(kTestEndpoint1, extensionFieldFailTestIn, buff_span));
    NL_TEST_ASSERT(aSuite,
                   CHIP_ERROR_BUFFER_TOO_SMALL ==
                       sHandler.Deserialize(kTestEndpoint1, kColorControlClusterId, fail_list, extensionFieldFailTestOut));

    memset(failBuffer, 0, fail_list.size());
    memset(buffer, 0, buff_span.size());
};

void TestHandlerHelpers(nlTestSuite * aSuite, void * aContext) {}

void TestStoreScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    SceneId sceneList[defaultTestFabricCapacity];

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
    Span<SceneId> sceneListSpan = Span<SceneId>(sceneList);
    Span<SceneId> emptyListSpan = Span<SceneId>(sceneList, 0);
    Span<SceneId> smallListSpan = Span<SceneId>(sceneList, 1);

    // Test Get All scenes in Group in empty scene table
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    NL_TEST_ASSERT(aSuite, 0 == emptyListSpan.size());

    // Set test
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));

    // Test single scene in table with 0 size span
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_BUFFER_TOO_SMALL == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, smallListSpan));
    NL_TEST_ASSERT(aSuite, 1 == smallListSpan.size());

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Too many scenes for 1 fabric
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NO_MEMORY == sceneTable->SetSceneTableEntry(kFabric1, scene9));

    // Not Found
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_BUFFER_TOO_SMALL == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));

    // Get test
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SceneApplyEFS(scene));

    // Test error when list too small in a full table
    // Test failure for 3 spaces in 4 scenes list
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_BUFFER_TOO_SMALL == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, smallListSpan));
    // Test failure for no space in a 4 scenes list
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_BUFFER_TOO_SMALL == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    // Test failure for no space in a 1 scene list
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_BUFFER_TOO_SMALL == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, emptyListSpan));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, smallListSpan));
    NL_TEST_ASSERT(aSuite, 1 == smallListSpan.size());

    // Test successfully getting Ids from various groups
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, sceneListSpan));
    NL_TEST_ASSERT(aSuite, 4 == sceneListSpan.size());
    NL_TEST_ASSERT(aSuite, kScene1 == sceneList[0]);
    NL_TEST_ASSERT(aSuite, kScene2 == sceneList[1]);
    NL_TEST_ASSERT(aSuite, kScene3 == sceneList[2]);
    NL_TEST_ASSERT(aSuite, kScene4 == sceneList[3]);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup2, sceneListSpan));
    NL_TEST_ASSERT(aSuite, 2 == sceneListSpan.size());
    NL_TEST_ASSERT(aSuite, kScene5 == sceneList[0]);
    NL_TEST_ASSERT(aSuite, kScene6 == sceneList[1]);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, sceneListSpan));
    NL_TEST_ASSERT(aSuite, 1 == sceneListSpan.size());
    NL_TEST_ASSERT(aSuite, kScene7 == sceneList[0]);

    uint8_t sceneCount = 0;
    sceneTable->GetEndpointSceneCount(sceneCount);
    sceneTable->GetFabricSceneCount(kFabric1, sceneCount);
}

void TestOverwriteScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    uint8_t sceneCount = 0;
    sceneTable->GetEndpointSceneCount(sceneCount);
    sceneTable->GetFabricSceneCount(kFabric1, sceneCount);
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

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, scene == scene12);
}

void TestIterateScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    SceneTableEntry scene;
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);

    NL_TEST_ASSERT(aSuite, iterator != nullptr);

    if (iterator)
    {
        NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
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
        NL_TEST_ASSERT(aSuite, scene == scene12);

        NL_TEST_ASSERT(aSuite, iterator->Next(scene) == false);

        iterator->Release();
    }
}

void TestRemoveScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTableImpl * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    SceneTableEntry scene;

    // Removing non-existing entry should not return errors
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));

    // Remove middle
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene5.mStorageId));
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 6);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Add scene in middle, a spot should have been freed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene9));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 7);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    NL_TEST_ASSERT(aSuite, scene == scene9);
    iterator->Release();

    // Remove the recently added scene 9
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 6);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene10);
    iterator->Release();

    // Remove first
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntryAtPosition(kTestEndpoint1, kFabric1, 0));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 5);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    iterator->Release();

    // Remove Next
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 4);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 3);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene6.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 1);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene12);
    iterator->Release();

    // Remove last
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, scene7.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene) == false);
    iterator->Release();

    // Remove at empty position, shouldn't trigger error
    NL_TEST_ASSERT(aSuite,
                   CHIP_NO_ERROR ==
                       sceneTable->RemoveSceneTableEntryAtPosition(kTestEndpoint1, kFabric1, defaultTestFabricCapacity - 1));

    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    iterator->Release();

    // Test Remove all scenes in Group
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 6);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->DeleteAllScenesInGroup(kFabric1, kGroup1));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 2);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);
    NL_TEST_ASSERT(aSuite, iterator->Next(scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    iterator->Release();

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->DeleteAllScenesInGroup(kFabric1, kGroup2));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, iterator->Count() == 0);
    iterator->Release();
}

void TestFabricScenes(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    SceneTableEntry scene;
    uint8_t fabric_capacity = 0;

    // Verify capacities are at max
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);

    // Fabric 1 inserts
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    uint8_t scene_count = 0;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == scene_count);

    // Fabric 2 inserts
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, (defaultTestFabricCapacity - 4) == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, 11 == scene_count);

    // Fabric 3 inserts, should only be 4 spaces left at this point since 12 got taken
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestTableSize - 11 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric3, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric3, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric3, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric3, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric3, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    NL_TEST_ASSERT(aSuite, 5 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, defaultTestTableSize == scene_count);

    // Checks capacity is now 0 accross all fabrics
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // To many scenes accross fabrics (Max scenes accross fabrics == 16)
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NO_MEMORY == sceneTable->SetSceneTableEntry(kFabric3, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NO_MEMORY == sceneTable->SetSceneTableEntry(kFabric2, scene5));

    // Verifying all inserted scenes are accessible
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);

    // Remove Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric1));
    // Verify Fabric 1 removed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, 9 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));

    // Verify Fabric 2 still there
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, 4 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);

    // Verify capacity updated for all fabrics
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 4 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 5 == fabric_capacity);

    // Verify we can now write more scenes in scene fabric 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene7));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, scene == scene6);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, scene == scene7);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, 7 == scene_count);

    // Verify capacity updated properly
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 4 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 5 == fabric_capacity);

    // Verify Fabric 3 still there
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);

    // Remove Fabric 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric2));
    // Verify Fabric 2 removed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, 5 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Verify Fabric 3 still there
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    NL_TEST_ASSERT(aSuite, 5 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, scene == scene2);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, scene == scene3);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, scene == scene4);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, scene == scene5);

    // Remove Fabric 3
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveFabric(kFabric3));
    // Verify Fabric 3 removed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));

    // Confirm all counts are at 0
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    NL_TEST_ASSERT(aSuite, 0 == scene_count);

    // Verify capacity updated for all fabrics
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
}

void TestEndpointScenes(nlTestSuite * aSuite, void * aContext)
{
    // Get Count for Endpoint 1
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);
    SceneTableEntry scene;

    // Verify all endpoints are empty
    uint8_t endpoint_scene_count = 0;

    // Get Count for Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    // Get Count for Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    // Get Count for Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);

    // Test Scenes insertion not accessible accross all endpoints
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 2 == endpoint_scene_count);

    uint8_t fabric_capacity = 0;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 1 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 1 == fabric_capacity);

    // Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    // Endpoint3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);

    // Check if scene present in Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);

    // Check if scene present in Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    // Check if scene present in Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));

    // Test removal on different endpoints do not affect each endpoints
    // Insertion on Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);

    // Removal on Endpoint1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, sceneId1));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));

    // Scene present on Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);

    // Removal on Endpoint 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric1, sceneId1));

    // Removal on Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveSceneTableEntry(kFabric2, sceneId1));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    // Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    // Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);

    // Test the fabric capacity accross endpoint
    // Fill fabric 1 endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Fill fabric 2 endpoint 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene7));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // Endpoints 2 and 3 should be unaffected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);

    // Verify filling Fabric on endpoint 2 does not affect on endpoint 3 despite Max per fabric being reached by adding Endpoint1
    // and Endpoint2
    // Fill fabric 1 endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Fill fabric 2 endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene7));
    // scene count to Endpoint

    // Endpoint 3 still unafected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);

    // Fill fabric 1 endpoint 3
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Test removal of Endpoint clears scene on all fabrics for that endpoint
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->RemoveEndpoint());

    // Check Fabric1 on Endpoint 2
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Check Fabric 1 and 2 on Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Check Fabric 1 on Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    // Test removal of fabric clears scene fabric on all endpoints
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    sceneTable->RemoveFabric(kFabric1);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable->RemoveFabric(kFabric2);

    // Validate endpoints are empty
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    NL_TEST_ASSERT(aSuite, 0 == endpoint_scene_count);

    // Validate Fabric capacities at maximum accross all endpoints

    // Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    // Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    // Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);

    // Test of Get with changes to Endpoint capacity
    // Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize - 2);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 1 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 1 == fabric_capacity);

    // Test Endpoint 2's capacity remains unaffected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == fabric_capacity);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    // Test Insertion then change of capacity
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 4 == fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 2);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 6 == fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 4);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // Test making the endpoint scene table smaller than the actual number of scenes on it
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 5);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
}

void TestOTAChanges(nlTestSuite * aSuite, void * aContext)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    NL_TEST_ASSERT(aSuite, nullptr != sceneTable);
    VerifyOrReturn(nullptr != sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    SceneTableEntry scene;
    uint8_t fabric_capacity  = 0;
    uint8_t fabricsFullCount = defaultTestTableSize - 2;

    // Fill scene table
    // Fill fabric 1 to capacity
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric1, scene7));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    uint8_t scene_table_fabric1_capacity = fabric_capacity;
    auto * iterator                      = sceneTable->IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == iterator->Count());
    iterator->Release();

    // Fill fabric 2 to capacity
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene1));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene2));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene3));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene4));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene5));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene6));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->SetSceneTableEntry(kFabric2, scene7));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    uint8_t scene_table_fabric2_capacity = fabric_capacity;
    iterator                             = sceneTable->IterateSceneEntries(kFabric2);
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity == iterator->Count());
    iterator->Release();
    // SceneTable should be full at this point
    uint8_t scene_count;
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetEndpointSceneCount(scene_count));
    // Global count should not have been modified
    NL_TEST_ASSERT(aSuite, fabricsFullCount == scene_count);

    // Create a scene table with a greater capacity than the original one (Max allowed capacity from gen_config.h)
    TestSceneTableImpl ExpandedSceneTable(scenes::kMaxScenesPerFabric, scenes::kMaxScenesPerEndpoint);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.Init(&testStorage));
    ExpandedSceneTable.SetEndpoint(kTestEndpoint1);

    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite,
                   scene_table_fabric1_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity) == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite,
                   scene_table_fabric2_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity) == fabric_capacity);

    // We should be able to insert 4 scenes into fabric 2
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene9));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene13));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene14));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene15));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // Fabric 1's capacity should have remain unchanged
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite,
                   scene_table_fabric1_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity) == fabric_capacity);

    // Global count should have increased by (scenes::kMaxScenesPerFarbic - defaultTestFabricCapacity)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, fabricsFullCount + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity) == scene_count);

    // Same test for 4 insertion in fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene9));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene13));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene14));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene15));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // Global count should be at defaultTestTableSize + (scenes::kMaxScenesPerEndpoint - defaultTestTableSize)
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ExpandedSceneTable.GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, fabricsFullCount + (scenes::kMaxScenesPerEndpoint - defaultTestTableSize) == scene_count);

    // Test failure to init a SceneTable with sizes above the defined max scenes per fabric or globaly
    TestSceneTableImpl SceneTableTooManyPerFabric(scenes::kMaxScenesPerFabric + 1, scenes::kMaxScenesPerEndpoint);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_INTEGER_VALUE == SceneTableTooManyPerFabric.Init(&testStorage));
    SceneTableTooManyPerFabric.Finish();

    TestSceneTableImpl SceneTableTooManyGlobal(scenes::kMaxScenesPerFabric, scenes::kMaxScenesPerEndpoint + 1);
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_INVALID_INTEGER_VALUE == SceneTableTooManyGlobal.Init(&testStorage));
    SceneTableTooManyGlobal.Finish();

    // Create a new table with a lower limit of scenes per fabric
    uint8_t newCapacity        = defaultTestFabricCapacity - 1;
    uint8_t newTableSize       = defaultTestTableSize - 2;
    uint8_t capacityDifference = static_cast<uint8_t>(scenes::kMaxScenesPerFabric - newCapacity);
    TestSceneTableImpl ReducedSceneTable(newCapacity, newTableSize);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.Init(&testStorage));
    ReducedSceneTable.SetEndpoint(kTestEndpoint1);

    // Global count should not have been modified
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, scenes::kMaxScenesPerEndpoint - 2 == scene_count);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    // Load a scene from fabric 1, this should adjust fabric 1 scene count in flash
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetSceneTableEntry(kFabric1, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);

    // The number count of scenes in Fabric 1 should have been adjusted here
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, newCapacity == iterator->Count());
    iterator->Release();
    // Capacity should still be 0 in fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetEndpointSceneCount(scene_count));
    // Global count should have been reduced by the difference between the max fabric capacity of a fabric and the
    // new fabric capacity since we haven't loaded from fabric 2 yet
    NL_TEST_ASSERT(aSuite, scenes::kMaxScenesPerEndpoint - 2 - capacityDifference == scene_count);

    // Remove a Scene from the Fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene1.mStorageId));
    // Check count updated for fabric
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, static_cast<uint8_t>(newCapacity - 1) == iterator->Count());
    iterator->Release();
    // Check fabric still doesn't have capacity because fabric 2 still have a higher number of scene than allowed
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);
    // Remove another scene from fabric 1
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    // Check count updated for fabric
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    NL_TEST_ASSERT(aSuite, 2 == iterator->Count());
    iterator->Release();

    // Confirm global count has been updated
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetEndpointSceneCount(scene_count));
    NL_TEST_ASSERT(aSuite, 13 == scene_count);
    // Confirm we now have capacity in fabric one
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 1 == fabric_capacity);

    // Load a scene from fabric 2, this should adjust fabric 2 scene count in flash
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetSceneTableEntry(kFabric2, sceneId1, scene));
    NL_TEST_ASSERT(aSuite, scene == scene1);

    // The number count of scenes in Fabric 2 should have been adjusted here
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric2);
    NL_TEST_ASSERT(aSuite, defaultTestFabricCapacity - 1 == iterator->Count());
    iterator->Release();
    // Global count should also have been adjusted
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetEndpointSceneCount(scene_count));
    // had 22 scenes, truncated 5 from both (10) and deleted 4 from fabric 1: 8 scenes left
    NL_TEST_ASSERT(aSuite, 8 == scene_count);
    // Confirm we now have capacity of 6 in the first fabric since we previously removed 6 scenes form there
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 4 == fabric_capacity);
    // Fabric 2 should still be at capacity
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == ReducedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 0 == fabric_capacity);

    ReducedSceneTable.Finish();

    // The Scene 8 should now have been truncated from the memory and thus not be accessible from both fabrics in the
    // original scene table
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));
    NL_TEST_ASSERT(aSuite, CHIP_ERROR_NOT_FOUND == sceneTable->GetSceneTableEntry(kFabric2, sceneId8, scene));
    // The Remaining capacity in the original scene table therefore have been modified as well
    // Fabric 2 should still be almost at capacity
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    NL_TEST_ASSERT(aSuite, 5 == fabric_capacity);
    NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));

    NL_TEST_ASSERT(aSuite, 1 == fabric_capacity);
}

} // namespace TestScenes

namespace {
/**
 *  Setup the test suite.
 */
int TestSetup(void * inContext)
{
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);

    // Initialize Scene Table
    SceneTable * sceneTable = scenes::GetSceneTableImpl();
    VerifyOrReturnError(nullptr != sceneTable, FAILURE);
    VerifyOrReturnError(CHIP_NO_ERROR == sceneTable->Init(&TestScenes::testStorage), FAILURE);

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int TestTeardown(void * inContext)
{
    // Terminate Scene Table
    SceneTable * sceneTable = scenes::GetSceneTableImpl();
    VerifyOrReturnError(nullptr != sceneTable, FAILURE);
    sceneTable->Finish();
    chip::Platform::MemoryShutdown();

    return SUCCESS;
}
} // namespace

int TestSceneTable()
{
    static nlTest sTests[] = { NL_TEST_DEF("TestHandlerRegistration", TestScenes::TestHandlerRegistration),
                               NL_TEST_DEF("TestHandlerFunctions", TestScenes::TestHandlerFunctions),
                               NL_TEST_DEF("TestStoreScenes", TestScenes::TestStoreScenes),
                               NL_TEST_DEF("TestOverwriteScenes", TestScenes::TestOverwriteScenes),
                               NL_TEST_DEF("TestIterateScenes", TestScenes::TestIterateScenes),
                               NL_TEST_DEF("TestRemoveScenes", TestScenes::TestRemoveScenes),
                               NL_TEST_DEF("TestFabricScenes", TestScenes::TestFabricScenes),
                               NL_TEST_DEF("TestEndpointScenes", TestScenes::TestEndpointScenes),
                               NL_TEST_DEF("TestOTAChanges", TestScenes::TestOTAChanges),

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
