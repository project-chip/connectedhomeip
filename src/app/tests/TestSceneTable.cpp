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
#include <app/util/attribute-metadata.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <app/util/mock/MockNodeConfig.h>
#include <app/util/odd-sized-integers.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>
using namespace chip;
using namespace chip::Test;
using namespace chip::app::Clusters::Globals::Attributes;

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
constexpr chip::ClusterId kFakeClusterId         = 0x0007;
constexpr chip::ClusterId kColorControlClusterId = 0x0300;
constexpr chip::ClusterId kScenesClusterId       = 0x0062;

// Test Endpoint ID
constexpr chip::EndpointId kTestEndpoint1 = chip::Test::kMockEndpoint1;
constexpr chip::EndpointId kTestEndpoint2 = chip::Test::kMockEndpoint2;
constexpr chip::EndpointId kTestEndpoint3 = chip::Test::kMockEndpoint3;
constexpr chip::EndpointId kTestEndpoint4 = kMockEndpointMin;

// Test Attribute ID
constexpr uint32_t kOnOffAttId               = app::Clusters::OnOff::Attributes::OnOff::Id;
constexpr uint32_t kCurrentLevelId           = app::Clusters::LevelControl::Attributes::CurrentLevel::Id;
constexpr uint32_t kCurrentFrequencyId       = app::Clusters::LevelControl::Attributes::CurrentFrequency::Id;
constexpr uint32_t kCurrentSaturationId      = app::Clusters::ColorControl::Attributes::CurrentSaturation::Id;
constexpr uint32_t kCurrentXId               = app::Clusters::ColorControl::Attributes::CurrentX::Id;
constexpr uint32_t kCurrentYId               = app::Clusters::ColorControl::Attributes::CurrentY::Id;
constexpr uint32_t kColorTemperatureMiredsId = app::Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id;
constexpr uint32_t kEnhancedCurrentHueId     = app::Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id;
constexpr uint32_t kEnhancedColorMode        = app::Clusters::ColorControl::Attributes::EnhancedColorMode::Id;
constexpr uint32_t kColorLoopActiveId        = app::Clusters::ColorControl::Attributes::ColorLoopActive::Id;
constexpr uint32_t kColorLoopDirectionId     = app::Clusters::ColorControl::Attributes::ColorLoopDirection::Id;
constexpr uint32_t kColorLoopTimeId          = app::Clusters::ColorControl::Attributes::ColorLoopTime::Id;

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

static app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type OOPairs[1];
static app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type LCPairs[2];
static app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type CCPairs[9];

static uint8_t OO_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t LC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };
static uint8_t CC_buffer[scenes::kMaxFieldBytesPerCluster] = { 0 };

static uint32_t OO_buffer_serialized_length = 0;
static uint32_t LC_buffer_serialized_length = 0;
static uint32_t CC_buffer_serialized_length = 0;

static const uint8_t defaultValueData64[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static EmberAfAttributeMinMaxValue minMaxValueBool = { false, false, true };
static EmberAfAttributeMinMaxValue minMaxValue8  = { static_cast<uint8_t>(0), static_cast<uint8_t>(1), static_cast<uint8_t>(0xFE) };
static EmberAfAttributeMinMaxValue minMaxValue8S = { static_cast<uint8_t>(0), static_cast<uint8_t>(-1),
                                                     static_cast<uint8_t>(0x7F) };
static EmberAfAttributeMinMaxValue minMaxValue16S = { static_cast<uint16_t>(0), static_cast<uint16_t>(-1),
                                                      static_cast<uint16_t>(0x7FFD) };

static EmberAfAttributeMetadata mockMetadataBool = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(&minMaxValueBool),
    .attributeId   = 0,
    .size          = 1,
    .attributeType = ZCL_BOOLEAN_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint8 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint8_t>(0)),
    .attributeId   = 0,
    .size          = 1,
    .attributeType = ZCL_INT8U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE,
};

static EmberAfAttributeMetadata mockMetadataUint8Max = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(&minMaxValue8),
    .attributeId   = 0,
    .size          = 1,
    .attributeType = ZCL_INT8U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_NULLABLE | MATTER_ATTRIBUTE_FLAG_MIN_MAX,
};

static EmberAfAttributeMetadata mockMetadataUint16 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0,
    .size          = 2,
    .attributeType = ZCL_INT16U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint24 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0,
    .size          = 3,
    .attributeType = ZCL_INT24U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint32 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0,
    .size          = 4,
    .attributeType = ZCL_INT32U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint40 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 5,
    .attributeType = ZCL_INT40U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint48 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 6,
    .attributeType = ZCL_INT48U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint56 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 7,
    .attributeType = ZCL_INT56U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataUint64 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 8,
    .attributeType = ZCL_INT64U_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt8 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(&minMaxValue8S),
    .attributeId   = 0,
    .size          = 1,
    .attributeType = ZCL_INT8S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_MIN_MAX,
};

static EmberAfAttributeMetadata mockMetadataInt16 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(&minMaxValue16S),
    .attributeId   = 0,
    .size          = 2,
    .attributeType = ZCL_INT16S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE | MATTER_ATTRIBUTE_FLAG_MIN_MAX,
};

static EmberAfAttributeMetadata mockMetadataInt24 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0,
    .size          = 3,
    .attributeType = ZCL_INT24S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt32 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint32_t>(0)),
    .attributeId   = 0,
    .size          = 4,
    .attributeType = ZCL_INT32S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt40 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 5,
    .attributeType = ZCL_INT40S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt48 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 6,
    .attributeType = ZCL_INT48S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt56 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 7,
    .attributeType = ZCL_INT56S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

static EmberAfAttributeMetadata mockMetadataInt64 = {
    .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(defaultValueData64),
    .attributeId   = 0,
    .size          = 8,
    .attributeType = ZCL_INT64S_ATTRIBUTE_TYPE,
    .mask          = MATTER_ATTRIBUTE_FLAG_WRITABLE,
};

// clang-format off
static const MockNodeConfig SceneMockNodeConfig({
    MockEndpointConfig(kTestEndpoint1, {
        MockClusterConfig(kScenesClusterId, {}),
        MockClusterConfig(kOnOffClusterId, {
            MockAttributeConfig(kOnOffAttId, mockMetadataBool)
        }),
        MockClusterConfig(kLevelControlClusterId, {
            MockAttributeConfig(kCurrentLevelId, mockMetadataUint8Max), MockAttributeConfig(kCurrentFrequencyId, mockMetadataUint16)
        }),
    }),
    MockEndpointConfig(kTestEndpoint2, {
        MockClusterConfig(kScenesClusterId, {}),
        MockClusterConfig(kOnOffClusterId, {
            MockAttributeConfig(kOnOffAttId, mockMetadataBool)
        }),
        MockClusterConfig(kColorControlClusterId, {
            MockAttributeConfig(kCurrentSaturationId, mockMetadataUint8), MockAttributeConfig(kCurrentXId, mockMetadataUint16),
            MockAttributeConfig(kCurrentYId, mockMetadataUint16), MockAttributeConfig(kColorTemperatureMiredsId, mockMetadataUint16),
            MockAttributeConfig(kEnhancedCurrentHueId, mockMetadataUint16), MockAttributeConfig(kEnhancedColorMode, mockMetadataUint8),
            MockAttributeConfig(kColorLoopActiveId, mockMetadataUint8), MockAttributeConfig(kColorLoopDirectionId, mockMetadataUint8),
            MockAttributeConfig(kColorLoopTimeId, mockMetadataUint16)
        }),
    }),
    MockEndpointConfig(kTestEndpoint3, {
        MockClusterConfig(kScenesClusterId, {}),
        MockClusterConfig(kOnOffClusterId, {
            MockAttributeConfig(kOnOffAttId, mockMetadataBool)
        }),
        MockClusterConfig(kLevelControlClusterId, {
            MockAttributeConfig(kCurrentLevelId, mockMetadataUint8Max), MockAttributeConfig(kCurrentFrequencyId, mockMetadataUint16)
        }),
        MockClusterConfig(kColorControlClusterId, {
            MockAttributeConfig(kCurrentSaturationId, mockMetadataUint8), MockAttributeConfig(kCurrentXId, mockMetadataUint16),
            MockAttributeConfig(kCurrentYId, mockMetadataUint16), MockAttributeConfig(kColorTemperatureMiredsId, mockMetadataUint16),
            MockAttributeConfig(kEnhancedCurrentHueId, mockMetadataUint16), MockAttributeConfig(kEnhancedColorMode, mockMetadataUint8),
            MockAttributeConfig(kColorLoopActiveId, mockMetadataUint8), MockAttributeConfig(kColorLoopDirectionId, mockMetadataUint8),
            MockAttributeConfig(kColorLoopTimeId, mockMetadataUint8)
        }),
    }),

    MockEndpointConfig(kTestEndpoint4, {
        MockClusterConfig(kScenesClusterId, {}),
        MockClusterConfig(MockClusterId(kColorControlClusterId), {
            MockAttributeConfig(MockAttributeId(kCurrentSaturationId), mockMetadataUint24), MockAttributeConfig(MockAttributeId(kCurrentXId), mockMetadataUint32),
            MockAttributeConfig(MockAttributeId(kCurrentYId), mockMetadataUint48), MockAttributeConfig(MockAttributeId(kColorTemperatureMiredsId), mockMetadataUint56),
            MockAttributeConfig(MockAttributeId(kEnhancedCurrentHueId), mockMetadataUint64), MockAttributeConfig(MockAttributeId(kEnhancedColorMode), mockMetadataInt8),
            MockAttributeConfig(MockAttributeId(kColorLoopActiveId), mockMetadataInt16), MockAttributeConfig(MockAttributeId(kColorLoopDirectionId), mockMetadataInt24),
            MockAttributeConfig(MockAttributeId(kColorLoopTimeId), mockMetadataInt32)
        }),
        MockClusterConfig(MockClusterId(kOnOffClusterId), {
            MockAttributeConfig(MockAttributeId(kOnOffAttId), mockMetadataInt48)
        }),
        MockClusterConfig(MockClusterId(kLevelControlClusterId), {
            MockAttributeConfig(MockAttributeId(kCurrentLevelId), mockMetadataInt56), MockAttributeConfig(MockAttributeId(kCurrentFrequencyId), mockMetadataInt64)
        }),
        MockClusterConfig(MockClusterId(kFakeClusterId), {
            MockAttributeConfig(MockAttributeId(kCurrentLevelId), mockMetadataUint40), MockAttributeConfig(MockAttributeId(kCurrentFrequencyId), mockMetadataInt40)
        }),
    }),
});
// clang-format on

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
        else if (endpoint == kTestEndpoint4)
        {
            if (clusterBuffer.size() >= 3)
            {
                buffer[0] = MockClusterId(kOnOffClusterId);
                buffer[1] = MockClusterId(kLevelControlClusterId);
                buffer[2] = MockClusterId(kColorControlClusterId);
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

        if (endpoint == kTestEndpoint4)
        {
            if (cluster == MockClusterId(kColorControlClusterId) || cluster == MockClusterId(kLevelControlClusterId) ||
                cluster == MockClusterId(kColorControlClusterId) || cluster == MockClusterId(kFakeClusterId))
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

// Test Fixture Class
class TestSceneTable : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        mpTestStorage  = new chip::TestPersistentStorageDelegate;
        mpSceneHandler = new TestSceneHandler;

        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

        // Initialize Scene Table
        SceneTable * sceneTable = scenes::GetSceneTableImpl();
        ASSERT_NE(sceneTable, nullptr);
        ASSERT_EQ(sceneTable->Init(mpTestStorage), CHIP_NO_ERROR);
        SetMockNodeConfig(SceneMockNodeConfig);
    }

    static void TearDownTestSuite()
    {
        // Terminate Scene Table
        SceneTable * sceneTable = scenes::GetSceneTableImpl();
        ASSERT_NE(sceneTable, nullptr);
        sceneTable->Finish();
        delete mpTestStorage;
        delete mpSceneHandler;
        chip::Platform::MemoryShutdown();
    }

    // Storage
    static chip::TestPersistentStorageDelegate * mpTestStorage;
    // Scene
    static TestSceneHandler * mpSceneHandler;
};

chip::TestPersistentStorageDelegate * TestSceneTable::mpTestStorage = nullptr;
TestSceneHandler * TestSceneTable::mpSceneHandler                   = nullptr;

void ResetSceneTable(SceneTable * sceneTable)
{
    sceneTable->RemoveFabric(kFabric1);
    sceneTable->RemoveFabric(kFabric2);
    sceneTable->RemoveFabric(kFabric3);
}

TEST_F(TestSceneTable, TestHandlerRegistration)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    TestSceneHandler tmpHandler[scenes::kMaxClustersPerScene];

    for (uint8_t i = 0; i < scenes::kMaxClustersPerScene; i++)
    {
        sceneTable->RegisterHandler(&tmpHandler[i]);
    }

    // Emptying Handler array
    sceneTable->UnregisterAllHandlers();

    // Verify the handler num has been updated properly
    EXPECT_TRUE(sceneTable->HandlerListEmpty());

    for (uint8_t i = 0; i < scenes::kMaxClustersPerScene; i++)
    {
        sceneTable->RegisterHandler(&tmpHandler[i]);
    }

    // Hanlder order in table : [H0, H1, H2]

    EXPECT_FALSE(sceneTable->HandlerListEmpty());
    // Removal at beginning
    sceneTable->UnregisterHandler(&tmpHandler[0]);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());
    // Re-insert
    sceneTable->RegisterHandler(&tmpHandler[0]);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());
    // Hanlder order in table : [H0, H1, H2]

    // Removal at the middle
    sceneTable->UnregisterHandler(&tmpHandler[2]);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());
    // Re-insert
    sceneTable->RegisterHandler(&tmpHandler[2]);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());
    // Hanlder order in table : [H1, H0, H2]

    // Removal at the end
    sceneTable->UnregisterHandler(&tmpHandler[2]);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());

    // Emptying Handler array
    sceneTable->UnregisterAllHandlers();

    // Verify the handler num has been updated properly
    EXPECT_TRUE(sceneTable->HandlerListEmpty());
}

TEST_F(TestSceneTable, TestHandlerFunctions)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type extensionFieldSetOut;
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType extensionFieldSetIn;

    TLV::TLVReader reader;
    TLV::TLVWriter writer;

    OOPairs[0].attributeID = kOnOffAttId;
    OOPairs[0].valueUnsigned8.SetValue(0x01);

    LCPairs[0].attributeID = kCurrentLevelId;
    LCPairs[0].valueUnsigned8.SetValue(0x64);
    LCPairs[1].attributeID = kCurrentFrequencyId;
    LCPairs[1].valueUnsigned16.SetValue(0x01F0);

    CCPairs[0].attributeID = kCurrentSaturationId;
    CCPairs[0].valueUnsigned8.SetValue(0);
    CCPairs[1].attributeID = kCurrentXId;
    CCPairs[1].valueUnsigned16.SetValue(0);
    CCPairs[2].attributeID = kCurrentYId;
    CCPairs[2].valueUnsigned16.SetValue(0);
    CCPairs[3].attributeID = kColorTemperatureMiredsId;
    CCPairs[3].valueUnsigned16.SetValue(0);
    CCPairs[4].attributeID = kEnhancedCurrentHueId;
    CCPairs[4].valueUnsigned16.SetValue(0);
    CCPairs[5].attributeID = kEnhancedColorMode;
    CCPairs[5].valueUnsigned8.SetValue(0);
    CCPairs[6].attributeID = kColorLoopActiveId;
    CCPairs[6].valueUnsigned8.SetValue(0);
    CCPairs[7].attributeID = kColorLoopDirectionId;
    CCPairs[7].valueUnsigned8.SetValue(0);
    CCPairs[8].attributeID = kColorLoopTimeId;
    CCPairs[8].valueUnsigned16.SetValue(0);

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

    constexpr uint16_t bufferSize = 1024;
    uint8_t buffer[bufferSize]    = { 0 };
    MutableByteSpan buff_span(buffer);

    // Serialize Extension Field sets as if they were recovered from memory
    writer.Init(OO_buffer);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), OOextensionFieldSet.attributeValueList));
    OO_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(LC_buffer);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), LCextensionFieldSet.attributeValueList));
    LC_buffer_serialized_length = writer.GetLengthWritten();

    writer.Init(CC_buffer);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), CCextensionFieldSet.attributeValueList));
    CC_buffer_serialized_length = writer.GetLengthWritten();

    // Test Registering SceneHandler
    sceneTable->RegisterHandler(mpSceneHandler);
    EXPECT_FALSE(sceneTable->HandlerListEmpty());

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(OO_list);
    extensionFieldSetIn.clusterID = kOnOffClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldSetIn.attributeValueList.Decode(reader));

    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint1, extensionFieldSetIn.clusterID));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    EXPECT_EQ(0, memcmp(OO_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Setup the Level Control Extension field set in the expected state from a command
    reader.Init(LC_list);
    extensionFieldSetIn.clusterID = kLevelControlClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldSetIn.attributeValueList.Decode(reader));

    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint1, extensionFieldSetIn.clusterID));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    EXPECT_EQ(0, memcmp(LC_list.data(), buff_span.data(), buff_span.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Setup the Color control Extension field set in the expected state from a command
    reader.Init(CC_list);
    extensionFieldSetIn.clusterID = kColorControlClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldSetIn.attributeValueList.Decode(reader));

    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint2, extensionFieldSetIn.clusterID));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint2, extensionFieldSetIn, buff_span));

    // Verify the handler extracted buffer matches the initial field sets
    EXPECT_EQ(0, memcmp(CC_list.data(), buff_span.data(), CC_list.size()));
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Verify Deserializing is properly filling out output extension field set for on off
    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint1, kOnOffClusterId));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->Deserialize(kTestEndpoint1, kOnOffClusterId, OO_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for on off previously
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    EXPECT_EQ(0, memcmp(OO_list.data(), buff_span.data(), OO_list.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for level control
    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint1, kLevelControlClusterId));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->Deserialize(kTestEndpoint1, kLevelControlClusterId, LC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for level control previously
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    EXPECT_EQ(0, memcmp(LC_list.data(), buff_span.data(), LC_list.size()));
    memset(buffer, 0, buff_span.size());

    // Verify Deserializing is properly filling out output extension field set for color control
    EXPECT_TRUE(mpSceneHandler->SupportsCluster(kTestEndpoint2, kColorControlClusterId));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->Deserialize(kTestEndpoint2, kColorControlClusterId, CC_list, extensionFieldSetOut));

    // Verify Encoding the Extension field set returns the same data as the one serialized for color control previously
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldSetOut.attributeValueList));
    EXPECT_EQ(0, memcmp(CC_list.data(), buff_span.data(), CC_list.size()));
    memset(buffer, 0, buff_span.size());

    // To test failure on serialize and deserialize when too many pairs are in the field sets
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type extensionFieldFailTestOut;
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType extensionFieldFailTestIn;
    app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type TooManyPairs[16];

    for (uint8_t i = 0; i < 16; i++)
    {
        TooManyPairs[i].valueUnsigned8.SetValue(0);
    }

    extensionFieldFailTestOut.clusterID          = kColorControlClusterId;
    extensionFieldFailTestOut.attributeValueList = TooManyPairs;

    // Give a bigger buffer given we are using too many pairs on purpose
    uint8_t failBuffer[2 * scenes::kMaxFieldBytesPerCluster] = { 0 };
    ByteSpan fail_list(failBuffer);

    // Serialize Extension Field sets as if they were recovered from memory
    writer.Init(failBuffer);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldFailTestOut.attributeValueList));

    // Setup the On Off Extension field set in the expected state from a command
    reader.Init(fail_list);
    extensionFieldFailTestIn.clusterID = kColorControlClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldFailTestIn.attributeValueList.Decode(reader));

    // Verify failure on both serialize and deserialize
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldFailTestIn, buff_span));
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL,
              mpSceneHandler->Deserialize(kTestEndpoint1, kColorControlClusterId, fail_list, extensionFieldFailTestOut));

    memset(failBuffer, 0, fail_list.size());
    memset(buffer, 0, buff_span.size());

    // Test Serialize Add of an attribute value that is greater than the mock attribute max (Max bool value)
    OOPairs[0].valueUnsigned8.SetValue(0xFF);

    // EFS to test caping of value once a variable above the mock attribute size is serialized
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::Type extensionFieldValueCapOut;
    app::Clusters::ScenesManagement::Structs::ExtensionFieldSet::DecodableType extensionFieldValueCapIn;

    extensionFieldValueCapOut.clusterID          = kOnOffClusterId;
    extensionFieldValueCapOut.attributeValueList = OOPairs;

    /// Setup of input EFS (by temporary using the output one)
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldValueCapOut.attributeValueList));

    reader.Init(buffer);
    extensionFieldValueCapIn.clusterID = kOnOffClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

    // Verify that the initial value is not capped
    auto pair_iterator = extensionFieldValueCapIn.attributeValueList.begin();
    pair_iterator.Next();
    app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type pair = pair_iterator.GetValue();
    EXPECT_EQ(pair.valueUnsigned8.Value(), OOPairs[0].valueUnsigned8.Value());

    // Verify that we cap the value to the mock attribute size when serializing
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldValueCapIn, buff_span));
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->Deserialize(kTestEndpoint1, kOnOffClusterId, buff_span, extensionFieldValueCapOut));

    // Verify that the output value is capped to 1
    EXPECT_EQ(1, extensionFieldValueCapOut.attributeValueList[0].valueUnsigned8.Value());

    // Clear buffer
    memset(buffer, 0, buff_span.size());

    // Test Serialize Add of an attribute value that is smaller than the mock attribute min (1) for LC current level
    LCPairs[0].valueUnsigned8.SetValue(0);

    extensionFieldValueCapOut.clusterID          = kLevelControlClusterId;
    extensionFieldValueCapOut.attributeValueList = LCPairs;

    /// Setup of input EFS (by temporary using the output one)
    buff_span = MutableByteSpan(buffer);
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldValueCapOut.attributeValueList));

    reader.Init(buffer);
    extensionFieldValueCapIn.clusterID = kLevelControlClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

    // Verify that the initial value is not capped
    auto iteratorMin = extensionFieldValueCapIn.attributeValueList.begin();
    iteratorMin.Next();
    pair = iteratorMin.GetValue();
    EXPECT_EQ(pair.valueUnsigned8.Value(), LCPairs[0].valueUnsigned8.Value());

    // Verify that we cap the value to the mock attribute size when serializing
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldValueCapIn, buff_span));
    EXPECT_EQ(CHIP_NO_ERROR,
              mpSceneHandler->Deserialize(kTestEndpoint1, kLevelControlClusterId, buff_span, extensionFieldValueCapOut));

    // Verify that the output value is capped to 255 (NULL) as Level Control Current Level is a nullable uint8_t
    EXPECT_EQ(255, extensionFieldValueCapOut.attributeValueList[0].valueUnsigned8.Value());

    // Clear buffer
    memset(buffer, 0, buff_span.size());

    // Test Serialize Add of an attribute value that is higher than the mock attribute max (0xFE) for LC current level
    LCPairs[0].valueUnsigned8.SetValue(0xFF);

    extensionFieldValueCapOut.clusterID          = kLevelControlClusterId;
    extensionFieldValueCapOut.attributeValueList = LCPairs;

    /// Setup of input EFS (by temporary using the output one)
    buff_span = MutableByteSpan(buffer);
    writer.Init(buff_span);
    EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), extensionFieldValueCapOut.attributeValueList));

    reader.Init(buffer);
    extensionFieldValueCapIn.clusterID = kLevelControlClusterId;
    EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
    EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

    // Verify that the initial value is not capped
    auto iteratorMax = extensionFieldValueCapIn.attributeValueList.begin();
    iteratorMax.Next();
    pair = iteratorMax.GetValue();
    EXPECT_EQ(pair.valueUnsigned8.Value(), LCPairs[0].valueUnsigned8.Value());
    EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint1, extensionFieldValueCapIn, buff_span));
    EXPECT_EQ(CHIP_NO_ERROR,
              mpSceneHandler->Deserialize(kTestEndpoint1, kLevelControlClusterId, buff_span, extensionFieldValueCapOut));

    // Verify that the output value is 0xFF (NULL) as Level Control Current Level is a nullable uint8_t
    EXPECT_EQ(0xFF, extensionFieldValueCapOut.attributeValueList[0].valueUnsigned8.Value());

    // Clear buffer
    memset(buffer, 0, buff_span.size());
    buff_span = MutableByteSpan(buffer);

    // Test for attribtues types that are in no Real clusters yet but are supported in scenes
    {
        // Setup EFS for mock cluster testing all attributes types
        app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type MockOOPairs[1];
        app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type MockLCPairs[2];
        app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type MockCCPairs[9];
        app::Clusters::ScenesManagement::Structs::AttributeValuePairStruct::Type MockFKPairs[2];
        // Mock CC
        MockCCPairs[0].attributeID = MockAttributeId(kCurrentSaturationId);
        MockCCPairs[0].valueUnsigned32.SetValue(UINT32_MAX); // will cap to 0x00FFFFFF (uint24)
        MockCCPairs[1].attributeID = MockAttributeId(kCurrentXId);
        MockCCPairs[1].valueUnsigned32.SetValue(UINT32_MAX); // not capped
        MockCCPairs[2].attributeID = MockAttributeId(kCurrentYId);
        MockCCPairs[2].valueUnsigned64.SetValue(UINT64_MAX); // will cap to 0x0000FFFFFFFFFFFF (uint48)
        MockCCPairs[3].attributeID = MockAttributeId(kColorTemperatureMiredsId);
        MockCCPairs[3].valueUnsigned64.SetValue(UINT64_MAX); // will cap to 0x00FFFFFFFFFFFFFF (uint56)
        MockCCPairs[4].attributeID = MockAttributeId(kEnhancedCurrentHueId);
        MockCCPairs[4].valueUnsigned64.SetValue(UINT64_MAX); // not capped
        MockCCPairs[5].attributeID = MockAttributeId(kEnhancedColorMode);
        MockCCPairs[5].valueSigned8.SetValue(static_cast<int8_t>(-2)); // will cap to -1
        MockCCPairs[6].attributeID = MockAttributeId(kColorLoopActiveId);
        MockCCPairs[6].valueSigned16.SetValue(
            static_cast<int16_t>(0x7FFE)); // will cap to 0x7FFD in int16 due to declared maximum in the attribute's mock metadata
        MockCCPairs[7].attributeID = MockAttributeId(kColorLoopDirectionId);
        MockCCPairs[7].valueSigned32.SetValue(-1); // will cap to -1 in int24
        MockCCPairs[8].attributeID = MockAttributeId(kColorLoopTimeId);
        MockCCPairs[8].valueSigned32.SetValue(-1); // not capped
        // Mock OO
        MockOOPairs[0].attributeID = MockAttributeId(kOnOffAttId);
        MockOOPairs[0].valueSigned64.SetValue(INT64_MAX); // will cap to 0x00007FFFFFFFFFFF (int48)
        // Mock LC
        MockLCPairs[0].attributeID = MockAttributeId(kCurrentLevelId);
        MockLCPairs[0].valueSigned64.SetValue(INT64_MIN); // will cap to 0x0080000000000000 (int56 min)
        MockLCPairs[1].attributeID = MockAttributeId(kCurrentFrequencyId);
        MockLCPairs[1].valueSigned64.SetValue(INT64_MIN); // not capped
        // Mock Fake
        MockFKPairs[0].attributeID = MockAttributeId(kCurrentLevelId);
        MockFKPairs[0].valueUnsigned64.SetValue(UINT64_MAX); // will cap to UINT40_MAX
        MockFKPairs[1].attributeID = MockAttributeId(kCurrentFrequencyId);
        MockFKPairs[1].valueSigned64.SetValue(INT64_MAX); // will cap to INT40_MIN

        // Initialize Extension Field sets as if they were received by add commands
        OOextensionFieldSet.clusterID          = MockClusterId(kOnOffClusterId);
        OOextensionFieldSet.attributeValueList = MockOOPairs;
        LCextensionFieldSet.clusterID          = MockClusterId(kLevelControlClusterId);
        LCextensionFieldSet.attributeValueList = MockLCPairs;
        CCextensionFieldSet.clusterID          = MockClusterId(kColorControlClusterId);
        CCextensionFieldSet.attributeValueList = MockCCPairs;

        uint8_t mock_OO_buffer[scenes::kMaxFieldBytesPerCluster]     = { 0 };
        uint8_t mock_LC_buffer[scenes::kMaxFieldBytesPerCluster]     = { 0 };
        uint8_t mock_CC_buffer[scenes::kMaxFieldBytesPerCluster * 2] = {
            0
        }; // Using mock attributes way bigger than the real ones so we increase the buffer size for this test
        ByteSpan Mock_OO_list(mock_OO_buffer);
        ByteSpan Mock_LC_list(mock_LC_buffer);
        ByteSpan Mock_CC_list(mock_CC_buffer);

        // Serialize Extension Field sets as if they were recovered from memory
        writer.Init(mock_OO_buffer);
        EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), OOextensionFieldSet.attributeValueList));
        OO_buffer_serialized_length = writer.GetLengthWritten();

        writer.Init(mock_LC_buffer);
        EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), LCextensionFieldSet.attributeValueList));
        LC_buffer_serialized_length = writer.GetLengthWritten();

        writer.Init(mock_CC_buffer);
        EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), CCextensionFieldSet.attributeValueList));
        CC_buffer_serialized_length = writer.GetLengthWritten();

        // Setup the On Off Extension field set in the expected state from a command
        reader.Init(Mock_OO_list);
        extensionFieldValueCapIn.clusterID = MockClusterId(kOnOffClusterId);
        EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
        EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

        // Verify that the initial value is not capped
        auto iteratorOO = extensionFieldValueCapIn.attributeValueList.begin();
        iteratorOO.Next();
        pair = iteratorOO.GetValue();
        EXPECT_EQ(pair.valueSigned64.Value(), MockOOPairs[0].valueSigned64.Value());

        // Verify that we cap the value to the mock attribute size when serializing
        EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint4, extensionFieldValueCapIn, buff_span));
        EXPECT_EQ(
            CHIP_NO_ERROR,
            mpSceneHandler->Deserialize(kTestEndpoint4, MockClusterId(kOnOffClusterId), buff_span, extensionFieldValueCapOut));

        // Verify that the output value is capped to int48 max value
        int64_t int48Max = static_cast<int64_t>(0x00007FFFFFFFFFFF);
        EXPECT_EQ(int48Max, extensionFieldValueCapOut.attributeValueList[0].valueSigned64.Value());

        // Clear buffer
        memset(buffer, 0, buff_span.size());
        // Reinit buffer
        buff_span = MutableByteSpan(buffer);

        reader.Init(Mock_LC_list);
        extensionFieldValueCapIn.clusterID = MockClusterId(kLevelControlClusterId);
        EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
        EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

        // Verify that the initial values are not capped
        auto iteratorLC = extensionFieldValueCapIn.attributeValueList.begin();
        iteratorLC.Next();
        pair = iteratorLC.GetValue();
        EXPECT_EQ(pair.valueSigned64.Value(), MockLCPairs[0].valueSigned64.Value());
        iteratorLC.Next();
        pair = iteratorLC.GetValue();
        EXPECT_EQ(pair.valueSigned64.Value(), MockLCPairs[1].valueSigned64.Value());

        // Verify that we cap the value to the mock attribute size when serializing
        EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint4, extensionFieldValueCapIn, buff_span));
        EXPECT_EQ(CHIP_NO_ERROR,
                  mpSceneHandler->Deserialize(kTestEndpoint4, MockClusterId(kLevelControlClusterId), buff_span,
                                              extensionFieldValueCapOut));

        // Verify that the output value is capped to int56 min value
        int64_t int56Min = static_cast<int64_t>(0xFF80000000000000);
        EXPECT_EQ(int56Min, static_cast<int64_t>(extensionFieldValueCapOut.attributeValueList[0].valueSigned64.Value()));

        // Verify that the output value is not capped
        EXPECT_EQ(INT64_MIN, extensionFieldValueCapOut.attributeValueList[1].valueSigned64.Value());

        // Clear buffer
        memset(buffer, 0, buff_span.size());
        // Reinit buffer
        buff_span = MutableByteSpan(buffer);

        reader.Init(Mock_CC_list);
        extensionFieldValueCapIn.clusterID = MockClusterId(kColorControlClusterId);
        EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
        EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

        // Verify that the initial values are not capped
        auto iteratorCC = extensionFieldValueCapIn.attributeValueList.begin();
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueUnsigned32.Value(), MockCCPairs[0].valueUnsigned32.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueUnsigned32.Value(), MockCCPairs[1].valueUnsigned32.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueUnsigned64.Value(), MockCCPairs[2].valueUnsigned64.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueUnsigned64.Value(), MockCCPairs[3].valueUnsigned64.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueUnsigned64.Value(), MockCCPairs[4].valueUnsigned64.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueSigned8.Value(), MockCCPairs[5].valueSigned8.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueSigned16.Value(), MockCCPairs[6].valueSigned16.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueSigned32.Value(), MockCCPairs[7].valueSigned32.Value());
        iteratorCC.Next();
        pair = iteratorCC.GetValue();
        EXPECT_EQ(pair.valueSigned32.Value(), MockCCPairs[8].valueSigned32.Value());

        // Verify that we cap the value to the mock attribute size when serializing
        EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint4, extensionFieldValueCapIn, buff_span));
        EXPECT_EQ(CHIP_NO_ERROR,
                  mpSceneHandler->Deserialize(kTestEndpoint4, MockClusterId(kColorControlClusterId), buff_span,
                                              extensionFieldValueCapOut));

        // Verify that the output value is capped to uint24t max value
        uint32_t uint24Max = static_cast<uint32_t>(0x00FFFFFF);
        EXPECT_EQ(uint24Max, extensionFieldValueCapOut.attributeValueList[0].valueUnsigned32.Value());

        // Verify that the output value is not capped
        EXPECT_EQ(UINT32_MAX, extensionFieldValueCapOut.attributeValueList[1].valueUnsigned32.Value());

        // Verify that the output value is capped to int48_t max value
        uint64_t uint48Max = static_cast<uint64_t>(0x0000FFFFFFFFFFFF);
        EXPECT_EQ(uint48Max, extensionFieldValueCapOut.attributeValueList[2].valueUnsigned64.Value());

        // Verify that the output value is capped to int56_t max value
        uint64_t uint56Max = static_cast<uint64_t>(0x00FFFFFFFFFFFFFF);
        EXPECT_EQ(uint56Max, extensionFieldValueCapOut.attributeValueList[3].valueUnsigned64.Value());

        // Verify that the output value is not capped
        EXPECT_EQ(UINT64_MAX, extensionFieldValueCapOut.attributeValueList[4].valueUnsigned64.Value());

        // Verify that the output value is capped to the defined min for this attribute
        EXPECT_EQ(static_cast<int8_t>(-1), extensionFieldValueCapOut.attributeValueList[5].valueSigned8.Value());

        // Verify that the output value is capped to the defined max for this attribute
        EXPECT_EQ(0x7FFD, extensionFieldValueCapOut.attributeValueList[6].valueSigned16.Value());

        // Verify that the output value is not capped to -1 in int24t
        using Int24Type = app::NumericAttributeTraits<app::OddSizedInteger<3, true>>::WorkingType;
        EXPECT_EQ(static_cast<Int24Type>(-1), extensionFieldValueCapOut.attributeValueList[7].valueSigned32.Value());

        // Verify that the output value will not cap
        EXPECT_EQ(-1, extensionFieldValueCapOut.attributeValueList[8].valueSigned32.Value());

        // Clear buffer
        memset(buffer, 0, buff_span.size());

        LCextensionFieldSet.clusterID          = MockClusterId(kFakeClusterId);
        LCextensionFieldSet.attributeValueList = MockFKPairs;

        writer.Init(mock_LC_buffer);
        EXPECT_EQ(CHIP_NO_ERROR, app::DataModel::Encode(writer, TLV::AnonymousTag(), LCextensionFieldSet.attributeValueList));
        LC_buffer_serialized_length = writer.GetLengthWritten();

        // Reinit buffer
        buff_span = MutableByteSpan(buffer);

        reader.Init(Mock_LC_list);
        extensionFieldValueCapIn.clusterID = MockClusterId(kFakeClusterId);
        EXPECT_EQ(CHIP_NO_ERROR, reader.Next());
        EXPECT_EQ(CHIP_NO_ERROR, extensionFieldValueCapIn.attributeValueList.Decode(reader));

        // Verify that the initial values are not capped
        auto iteratorFK = extensionFieldValueCapIn.attributeValueList.begin();
        iteratorFK.Next();
        pair = iteratorFK.GetValue();
        EXPECT_EQ(pair.valueUnsigned64.Value(), MockFKPairs[0].valueUnsigned64.Value());
        iteratorFK.Next();
        pair = iteratorFK.GetValue();
        EXPECT_EQ(pair.valueSigned64.Value(), MockFKPairs[1].valueSigned64.Value());

        // Verify that we cap the value to the mock attribute size when serializing
        EXPECT_EQ(CHIP_NO_ERROR, mpSceneHandler->SerializeAdd(kTestEndpoint4, extensionFieldValueCapIn, buff_span));
        EXPECT_EQ(CHIP_NO_ERROR,
                  mpSceneHandler->Deserialize(kTestEndpoint4, MockClusterId(kFakeClusterId), buff_span, extensionFieldValueCapOut));

        // Verify that the output value is capped to uint40 max value
        uint64_t uint40Max = static_cast<uint64_t>(0x000000FFFFFFFFFF);
        EXPECT_EQ(uint40Max, extensionFieldValueCapOut.attributeValueList[0].valueUnsigned64.Value());

        // Verify that the output value is capped to int40 max value
        int64_t int40Max = static_cast<int64_t>(0x0000007FFFFFFFFF);
        EXPECT_EQ(int40Max, extensionFieldValueCapOut.attributeValueList[1].valueSigned64.Value());

        // Clear buffer
        memset(buffer, 0, buff_span.size());
        // Reinit buffer
    }
};

TEST_F(TestSceneTable, TestStoreScenes)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    SceneId sceneList[defaultTestFabricCapacity];

    // Reset test
    ResetSceneTable(sceneTable);

    // Populate scene1's EFS (Endpoint1)
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneSaveEFS(scene1));

    // Populate scene2's EFS (Endpoint1)
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneSaveEFS(scene2));

    // Populate scene3's EFS (Endpoint2)
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneSaveEFS(scene3));

    // Populate scene4's EFS (Endpoint2)
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneSaveEFS(scene4));

    // Populate scene8's EFS (Endpoint3)
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneSaveEFS(scene8));

    SceneTableEntry scene;
    Span<SceneId> sceneListSpan = Span<SceneId>(sceneList);
    Span<SceneId> emptyListSpan = Span<SceneId>(sceneList, 0);
    Span<SceneId> smallListSpan = Span<SceneId>(sceneList, 1);

    // Test Get All scenes in Group in empty scene table
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    EXPECT_EQ(0u, emptyListSpan.size());

    // Set test
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));

    // Test single scene in table with 0 size span
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, smallListSpan));
    EXPECT_EQ(1u, smallListSpan.size());

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Too many scenes for 1 fabric
    EXPECT_EQ(CHIP_ERROR_NO_MEMORY, sceneTable->SetSceneTableEntry(kFabric1, scene9));

    // Not Found
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));

    // Get test
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneApplyEFS(scene));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneApplyEFS(scene));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneApplyEFS(scene));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(scene, scene4);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneApplyEFS(scene));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(scene, scene5);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(scene, scene6);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    EXPECT_EQ(scene, scene7);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SceneApplyEFS(scene));

    // Test error when list too small in a full table
    // Test failure for 3 spaces in 4 scenes list
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, smallListSpan));
    // Test failure for no space in a 4 scenes list
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, emptyListSpan));
    // Test failure for no space in a 1 scene list
    EXPECT_EQ(CHIP_ERROR_BUFFER_TOO_SMALL, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, emptyListSpan));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, smallListSpan));
    EXPECT_EQ(1u, smallListSpan.size());

    // Test successfully getting Ids from various groups
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup1, sceneListSpan));
    EXPECT_EQ(4u, sceneListSpan.size());
    EXPECT_EQ(kScene1, sceneList[0]);
    EXPECT_EQ(kScene2, sceneList[1]);
    EXPECT_EQ(kScene3, sceneList[2]);
    EXPECT_EQ(kScene4, sceneList[3]);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup2, sceneListSpan));
    EXPECT_EQ(2u, sceneListSpan.size());
    EXPECT_EQ(kScene5, sceneList[0]);
    EXPECT_EQ(kScene6, sceneList[1]);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetAllSceneIdsInGroup(kFabric1, kGroup3, sceneListSpan));
    EXPECT_EQ(1u, sceneListSpan.size());
    EXPECT_EQ(kScene7, sceneList[0]);

    uint8_t sceneCount = 0;
    sceneTable->GetEndpointSceneCount(sceneCount);
    sceneTable->GetFabricSceneCount(kFabric1, sceneCount);
}

TEST_F(TestSceneTable, TestOverwriteScenes)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    uint8_t sceneCount = 0;
    sceneTable->GetEndpointSceneCount(sceneCount);
    sceneTable->GetFabricSceneCount(kFabric1, sceneCount);
    SceneTableEntry scene;
    // Overwriting the first entry
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene10));
    // Overwriting in the middle
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene11));
    // Overwriting the last entry
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene12));

    // Scene 10 has the same sceneId as scene 1, Get->sceneId1 should thus return scene 10, etc.
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene10);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(scene, scene11);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    EXPECT_EQ(scene, scene12);
}

TEST_F(TestSceneTable, TestIterateScenes)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    SceneTableEntry scene;
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);

    ASSERT_NE(iterator, nullptr);

    if (iterator)
    {
        EXPECT_EQ(iterator->Count(), 7u);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene10);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene2);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene3);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene4);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene11);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene6);
        EXPECT_TRUE(iterator->Next(scene));
        EXPECT_EQ(scene, scene12);

        EXPECT_FALSE(iterator->Next(scene));

        iterator->Release();
    }
}

TEST_F(TestSceneTable, TestRemoveScenes)
{
    SceneTableImpl * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    SceneTableEntry scene;

    // Removing non-existing entry should not return errors
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));

    // Remove middle
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene5.mStorageId));
    auto * iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 6u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene10);
    iterator->Release();

    // Add scene in middle, a spot should have been freed
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene9));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 7u);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId9, scene));
    EXPECT_EQ(scene, scene9);
    iterator->Release();

    // Remove the recently added scene 9
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene9.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 6u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene10);
    iterator->Release();

    // Remove first
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntryAtPosition(kTestEndpoint1, kFabric1, 0));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 5u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene2);
    iterator->Release();

    // Remove Next
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 4u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene4);
    iterator->Release();

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 3u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene4);
    iterator->Release();

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 2u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene6);
    iterator->Release();

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene6.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 1u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene12);
    iterator->Release();

    // Remove last
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, scene7.mStorageId));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 0u);
    EXPECT_FALSE(iterator->Next(scene));
    iterator->Release();

    // Remove at empty position, shouldn't trigger error
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntryAtPosition(kTestEndpoint1, kFabric1, defaultTestFabricCapacity - 1));

    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 0u);
    iterator->Release();

    // Test Remove all scenes in Group
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 6u);
    iterator->Release();

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->DeleteAllScenesInGroup(kFabric1, kGroup1));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 2u);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene5);
    EXPECT_TRUE(iterator->Next(scene));
    EXPECT_EQ(scene, scene6);
    iterator->Release();

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->DeleteAllScenesInGroup(kFabric1, kGroup2));
    iterator = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(iterator->Count(), 0u);
    iterator->Release();
}

TEST_F(TestSceneTable, TestFabricScenes)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    SceneTableEntry scene;
    uint8_t fabric_capacity = 0;

    // Verify capacities are at max
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);

    // Fabric 1 inserts
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    uint8_t scene_count = 0;
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    EXPECT_EQ(defaultTestFabricCapacity, scene_count);

    // Fabric 2 inserts
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ((defaultTestFabricCapacity - 4), fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    EXPECT_EQ(11, scene_count);

    // Fabric 3 inserts, should only be 4 spaces left at this point since 12 got taken
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(defaultTestTableSize - 11, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric3, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric3, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric3, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric3, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric3, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    EXPECT_EQ(5, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    EXPECT_EQ(defaultTestTableSize, scene_count);

    // Checks capacity is now 0 accross all fabrics
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // To many scenes accross fabrics (Max scenes accross fabrics == 16)
    EXPECT_EQ(CHIP_ERROR_NO_MEMORY, sceneTable->SetSceneTableEntry(kFabric3, scene6));
    EXPECT_EQ(CHIP_ERROR_NO_MEMORY, sceneTable->SetSceneTableEntry(kFabric2, scene5));

    // Verifying all inserted scenes are accessible
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(scene, scene4);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(scene, scene5);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(scene, scene6);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    EXPECT_EQ(scene, scene7);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    EXPECT_EQ(scene, scene4);

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    EXPECT_EQ(scene, scene4);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    EXPECT_EQ(scene, scene5);

    // Remove Fabric 1
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveFabric(kFabric1));
    // Verify Fabric 1 removed
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    EXPECT_EQ(9, scene_count);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));

    // Verify Fabric 2 still there
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(4, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    EXPECT_EQ(scene, scene4);

    // Verify capacity updated for all fabrics
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 4, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 5, fabric_capacity);

    // Verify we can now write more scenes in scene fabric 2
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene7));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    EXPECT_EQ(scene, scene5);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    EXPECT_EQ(scene, scene6);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));
    EXPECT_EQ(scene, scene7);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(7, scene_count);

    // Verify capacity updated properly
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(4, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 5, fabric_capacity);

    // Verify Fabric 3 still there
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    EXPECT_EQ(scene, scene4);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    EXPECT_EQ(scene, scene5);

    // Remove Fabric 2
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveFabric(kFabric2));
    // Verify Fabric 2 removed
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    EXPECT_EQ(5, scene_count);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Verify Fabric 3 still there
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    EXPECT_EQ(5, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    EXPECT_EQ(scene, scene2);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    EXPECT_EQ(scene, scene3);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    EXPECT_EQ(scene, scene4);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));
    EXPECT_EQ(scene, scene5);

    // Remove Fabric 3
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveFabric(kFabric3));
    // Verify Fabric 3 removed
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric3, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric3, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric3, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric3, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric3, sceneId5, scene));

    // Confirm all counts are at 0
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric1, scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric2, scene_count));
    EXPECT_EQ(0, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetFabricSceneCount(kFabric3, scene_count));
    EXPECT_EQ(0, scene_count);

    // Verify capacity updated for all fabrics
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric3, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
}

TEST_F(TestSceneTable, TestEndpointScenes)
{
    // Get Count for Endpoint 1
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);
    SceneTableEntry scene;

    // Verify all endpoints are empty
    uint8_t endpoint_scene_count = 0;

    // Get Count for Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    // Get Count for Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    // Get Count for Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);

    // Test Scenes insertion not accessible accross all endpoints
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(2, endpoint_scene_count);

    uint8_t fabric_capacity = 0;
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 1, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 1, fabric_capacity);

    // Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    // Endpoint3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);

    // Check if scene present in Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(scene, scene1);

    // Check if scene present in Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    // Check if scene present in Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));

    // Test removal on different endpoints do not affect each endpoints
    // Insertion on Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);

    // Removal on Endpoint1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, sceneId1));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));

    // Scene present on Endpoint2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);

    // Removal on Endpoint 2
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric1, sceneId1));

    // Removal on Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveSceneTableEntry(kFabric2, sceneId1));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    // Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    // Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);

    // Test the fabric capacity accross endpoint
    // Fill fabric 1 endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Fill fabric 2 endpoint 1
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene7));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // Endpoints 2 and 3 should be unaffected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);

    // Verify filling Fabric on endpoint 2 does not affect on endpoint 3 despite Max per fabric being reached by adding Endpoint1
    // and Endpoint2
    // Fill fabric 1 endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Fill fabric 2 endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene7));
    // scene count to Endpoint

    // Endpoint 3 still unafected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);

    // Fill fabric 1 endpoint 3
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));

    // Test removal of Endpoint clears scene on all fabrics for that endpoint
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->RemoveEndpoint());

    // Check Fabric1 on Endpoint 2
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Check Fabric 1 and 2 on Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId2, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId3, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId4, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId5, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId6, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric2, sceneId7, scene));

    // Check Fabric 1 on Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    // Test removal of fabric clears scene fabric on all endpoints
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    sceneTable->RemoveFabric(kFabric1);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId2, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId3, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId4, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId5, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId6, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId7, scene));

    sceneTable->RemoveFabric(kFabric2);

    // Validate endpoints are empty
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(endpoint_scene_count));
    EXPECT_EQ(0, endpoint_scene_count);

    // Validate Fabric capacities at maximum accross all endpoints

    // Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    // Endpoint 2
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    // Endpoint 3
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint3, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);

    // Test of Get with changes to Endpoint capacity
    // Endpoint 1
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize - 2);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 1, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 1, fabric_capacity);

    // Test Endpoint 2's capacity remains unaffected
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint2, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity, fabric_capacity);
    ASSERT_NE(nullptr, sceneTable);

    // Test Insertion then change of capacity
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 4, fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 2);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(defaultTestFabricCapacity - 6, fabric_capacity);

    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 4);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // Test making the endpoint scene table smaller than the actual number of scenes on it
    sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestFabricCapacity - 5);
    ASSERT_NE(nullptr, sceneTable);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
}

TEST_F(TestSceneTable, TestOTAChanges)
{
    SceneTable * sceneTable = scenes::GetSceneTableImpl(kTestEndpoint1, defaultTestTableSize);
    ASSERT_NE(nullptr, sceneTable);

    // Reset test
    ResetSceneTable(sceneTable);

    SceneTableEntry scene;
    uint8_t fabric_capacity  = 0;
    uint8_t fabricsFullCount = defaultTestTableSize - 2;

    // Fill scene table
    // Fill fabric 1 to capacity
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric1, scene7));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    uint8_t scene_table_fabric1_capacity = fabric_capacity;
    auto * iterator                      = sceneTable->IterateSceneEntries(kFabric1);
    EXPECT_EQ(defaultTestFabricCapacity, iterator->Count());
    iterator->Release();

    // Fill fabric 2 to capacity
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene1));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene2));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene3));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene4));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene5));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene6));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->SetSceneTableEntry(kFabric2, scene7));
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    uint8_t scene_table_fabric2_capacity = fabric_capacity;
    iterator                             = sceneTable->IterateSceneEntries(kFabric2);
    EXPECT_EQ(defaultTestFabricCapacity, iterator->Count());
    iterator->Release();
    // SceneTable should be full at this point
    uint8_t scene_count;
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetEndpointSceneCount(scene_count));
    // Global count should not have been modified
    EXPECT_EQ(fabricsFullCount, scene_count);

    // Create a scene table with a greater capacity than the original one (Max allowed capacity from gen_config.h)
    TestSceneTableImpl ExpandedSceneTable(scenes::kMaxScenesPerFabric, scenes::kMaxScenesPerEndpoint);
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.Init(mpTestStorage));
    ExpandedSceneTable.SetEndpoint(kTestEndpoint1);

    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(scene_table_fabric1_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity), fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(scene_table_fabric2_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity), fabric_capacity);

    // We should be able to insert 4 scenes into fabric 2
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene9));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene13));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene14));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric2, scene15));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // Fabric 1's capacity should have remain unchanged
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(scene_table_fabric1_capacity + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity), fabric_capacity);

    // Global count should have increased by (scenes::kMaxScenesPerFarbic - defaultTestFabricCapacity)
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetEndpointSceneCount(scene_count));
    EXPECT_EQ(fabricsFullCount + (scenes::kMaxScenesPerFabric - defaultTestFabricCapacity), scene_count);

    // Same test for 4 insertion in fabric 1
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene9));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene13));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene14));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.SetSceneTableEntry(kFabric1, scene15));
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // Global count should be at defaultTestTableSize + (scenes::kMaxScenesPerEndpoint - defaultTestTableSize)
    EXPECT_EQ(CHIP_NO_ERROR, ExpandedSceneTable.GetEndpointSceneCount(scene_count));
    EXPECT_EQ(fabricsFullCount + (scenes::kMaxScenesPerEndpoint - defaultTestTableSize), scene_count);

    // Test failure to init a SceneTable with sizes above the defined max scenes per fabric or globaly
    TestSceneTableImpl SceneTableTooManyPerFabric(scenes::kMaxScenesPerFabric + 1, scenes::kMaxScenesPerEndpoint);
    EXPECT_EQ(CHIP_ERROR_INVALID_INTEGER_VALUE, SceneTableTooManyPerFabric.Init(mpTestStorage));
    SceneTableTooManyPerFabric.Finish();

    TestSceneTableImpl SceneTableTooManyGlobal(scenes::kMaxScenesPerFabric, scenes::kMaxScenesPerEndpoint + 1);
    EXPECT_EQ(CHIP_ERROR_INVALID_INTEGER_VALUE, SceneTableTooManyGlobal.Init(mpTestStorage));
    SceneTableTooManyGlobal.Finish();

    // Create a new table with a lower limit of scenes per fabric
    uint8_t newCapacity        = defaultTestFabricCapacity - 1;
    uint8_t newTableSize       = defaultTestTableSize - 2;
    uint8_t capacityDifference = static_cast<uint8_t>(scenes::kMaxScenesPerFabric - newCapacity);
    TestSceneTableImpl ReducedSceneTable(newCapacity, newTableSize);
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.Init(mpTestStorage));
    ReducedSceneTable.SetEndpoint(kTestEndpoint1);

    // Global count should not have been modified
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetEndpointSceneCount(scene_count));
    EXPECT_EQ(scenes::kMaxScenesPerEndpoint - 2, scene_count);
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    // Load a scene from fabric 1, this should adjust fabric 1 scene count in flash
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetSceneTableEntry(kFabric1, sceneId1, scene));
    EXPECT_EQ(scene, scene1);

    // The number count of scenes in Fabric 1 should have been adjusted here
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    EXPECT_EQ(newCapacity, iterator->Count());
    iterator->Release();
    // Capacity should still be 0 in fabric 1
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetEndpointSceneCount(scene_count));
    // Global count should have been reduced by the difference between the max fabric capacity of a fabric and the
    // new fabric capacity since we haven't loaded from fabric 2 yet
    EXPECT_EQ(scenes::kMaxScenesPerEndpoint - 2 - capacityDifference, scene_count);

    // Remove a Scene from the Fabric 1
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene1.mStorageId));
    // Check count updated for fabric
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    EXPECT_EQ(static_cast<uint8_t>(newCapacity - 1), iterator->Count());
    iterator->Release();
    // Check fabric still doesn't have capacity because fabric 2 still have a higher number of scene than allowed
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);
    // Remove another scene from fabric 1
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene2.mStorageId));
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene3.mStorageId));
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.RemoveSceneTableEntry(kFabric1, scene4.mStorageId));
    // Check count updated for fabric
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric1);
    EXPECT_EQ(2u, iterator->Count());
    iterator->Release();

    // Confirm global count has been updated
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetEndpointSceneCount(scene_count));
    EXPECT_EQ(13, scene_count);
    // Confirm we now have capacity in fabric one
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(1, fabric_capacity);

    // Load a scene from fabric 2, this should adjust fabric 2 scene count in flash
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetSceneTableEntry(kFabric2, sceneId1, scene));
    EXPECT_EQ(scene, scene1);

    // The number count of scenes in Fabric 2 should have been adjusted here
    iterator = ReducedSceneTable.IterateSceneEntries(kFabric2);
    EXPECT_EQ(defaultTestFabricCapacity - 1u, iterator->Count());
    iterator->Release();
    // Global count should also have been adjusted
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetEndpointSceneCount(scene_count));
    // had 22 scenes, truncated 5 from both (10) and deleted 4 from fabric 1: 8 scenes left
    EXPECT_EQ(8, scene_count);
    // Confirm we now have capacity of 6 in the first fabric since we previously removed 6 scenes form there
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(4, fabric_capacity);
    // Fabric 2 should still be at capacity
    EXPECT_EQ(CHIP_NO_ERROR, ReducedSceneTable.GetRemainingCapacity(kFabric2, fabric_capacity));
    EXPECT_EQ(0, fabric_capacity);

    ReducedSceneTable.Finish();

    // The Scene 8 should now have been truncated from the memory and thus not be accessible from both fabrics in the
    // original scene table
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric1, sceneId8, scene));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, sceneTable->GetSceneTableEntry(kFabric2, sceneId8, scene));
    // The Remaining capacity in the original scene table therefore have been modified as well
    // Fabric 2 should still be almost at capacity
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric1, fabric_capacity));
    EXPECT_EQ(5, fabric_capacity);
    EXPECT_EQ(CHIP_NO_ERROR, sceneTable->GetRemainingCapacity(kFabric2, fabric_capacity));

    EXPECT_EQ(1, fabric_capacity);
}

} // namespace TestScenes
