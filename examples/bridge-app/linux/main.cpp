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

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app-common/zap-generated/af-structs.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/EventLogging.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <pthread.h>
#include <sys/ioctl.h>

#include "CommissionableInit.h"
#include "Device.h"
#include "main.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace {

const int kNodeLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
const int kDescriptorAttributeArraySize = 254;

EndpointId gCurrentEndpointId;
EndpointId gFirstDynamicEndpointId;
Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
std::vector<Room *> gRooms;
std::vector<Action *> gActions;

// ENDPOINT DEFINITIONS:
// =================================================================================
//
// Endpoint definitions will be reused across multiple endpoints for every instance of the
// endpoint type.
// There will be no intrinsic storage for the endpoint attributes declared here.
// Instead, all attributes will be treated as EXTERNAL, and therefore all reads
// or writes to the attributes must be handled within the emberAfExternalAttributeWriteCallback
// and emberAfExternalAttributeReadCallback functions declared herein. This fits
// the typical model of a bridge, since a bridge typically maintains its own
// state database representing the devices connected to it.

// Device types for dynamic endpoints: TODO Need a generated file from ZAP to define these!
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT_SWITCH 0x0103
// (taken from chip-devices.xml)
#define DEVICE_TYPE_POWER_SOURCE 0x0011

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

// ---------------------------------------------------------------------------
//
// LIGHT ENDPOINT: contains the following clusters:
//   - On/Off
//   - Descriptor
//   - Bridged Device Basic

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_ON_OFF_ATTRIBUTE_ID, BOOLEAN, 1, 0), /* on/off */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_NODE_LABEL_ATTRIBUTE_ID, CHAR_STRING, kNodeLabelSize, 0), /* NodeLabel */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_REACHABLE_ATTRIBUTE_ID, BOOLEAN, 1, 0),               /* Reachable */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Cluster List for Bridged Light endpoint
// TODO: It's not clear whether it would be better to get the command lists from
// the ZAP config on our last fixed endpoint instead.
constexpr CommandId onOffIncomingCommands[] = {
    app::Clusters::OnOff::Commands::Off::Id,
    app::Clusters::OnOff::Commands::On::Id,
    app::Clusters::OnOff::Commands::Toggle::Id,
    app::Clusters::OnOff::Commands::OffWithEffect::Id,
    app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
    app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
    kInvalidCommandId,
};

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedLightClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_ON_OFF_CLUSTER_ID, onOffAttrs, onOffIncomingCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, bridgedDeviceBasicAttrs, nullptr,
                            nullptr) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);
DataVersion gLight1DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight2DataVersions[ArraySize(bridgedLightClusters)];

DeviceOnOff Light1("Light 1", "Office");
DeviceOnOff Light2("Light 2", "Office");

DeviceSwitch Switch1("Switch 1", "Office", EMBER_AF_SWITCH_FEATURE_LATCHING_SWITCH);
DeviceSwitch Switch2("Switch 2", "Office",
                     EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH | EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_RELEASE |
                         EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_LONG_PRESS |
                         EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_MULTI_PRESS);

// Declare Bridged endpoints used for Action clusters
DataVersion gActionLight1DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gActionLight2DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gActionLight3DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gActionLight4DataVersions[ArraySize(bridgedLightClusters)];

DeviceOnOff ActionLight1("Action Light 1", "Room 1");
DeviceOnOff ActionLight2("Action Light 2", "Room 1");
DeviceOnOff ActionLight3("Action Light 3", "Room 2");
DeviceOnOff ActionLight4("Action Light 4", "Room 2");

Room room1("Room 1", 0xE001, BridgedActions::EndpointListTypeEnum::kRoom, true);
Room room2("Room 2", 0xE002, BridgedActions::EndpointListTypeEnum::kRoom, true);
Room room3("Zone 3", 0xE003, BridgedActions::EndpointListTypeEnum::kZone, false);

Action action1(0x1001, "Room 1 On", BridgedActions::ActionTypeEnum::kAutomation, 0xE001, 0x1,
               BridgedActions::ActionStateEnum::kInactive, true);
Action action2(0x1002, "Turn On Room 2", BridgedActions::ActionTypeEnum::kAutomation, 0xE002, 0x01,
               BridgedActions::ActionStateEnum::kInactive, true);
Action action3(0x1003, "Turn Off Room 1", BridgedActions::ActionTypeEnum::kAutomation, 0xE003, 0x01,
               BridgedActions::ActionStateEnum::kInactive, false);

// ---------------------------------------------------------------------------
//
// SWITCH ENDPOINT: contains the following clusters:
//   - Switch
//   - Descriptor
//   - Bridged Device Basic

// Declare Switch cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(switchAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_NUMBER_OF_POSITIONS_ATTRIBUTE_ID, INT8U, 1, 0),       /* NumberOfPositions */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CURRENT_POSITION_ATTRIBUTE_ID, INT8U, 1, 0),      /* CurrentPosition */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_MULTI_PRESS_MAX_ATTRIBUTE_ID, INT8U, 1, 0),       /* MultiPressMax */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID, BITMAP32, 4, 0), /* FeatureMap */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(switchDescriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_DEVICE_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),     /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_SERVER_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_CLIENT_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_PARTS_LIST_ATTRIBUTE_ID, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(switchBridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_NODE_LABEL_ATTRIBUTE_ID, CHAR_STRING, kNodeLabelSize, 0), /* NodeLabel */
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_REACHABLE_ATTRIBUTE_ID, BOOLEAN, 1, 0),               /* Reachable */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Cluster List for Bridged Switch endpoint
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedSwitchClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_SWITCH_CLUSTER_ID, switchAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, switchDescriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, switchBridgedDeviceBasicAttrs, nullptr,
                            nullptr) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Switch endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedSwitchEndpoint, bridgedSwitchClusters);
DataVersion gSwitch1DataVersions[ArraySize(bridgedSwitchClusters)];
DataVersion gSwitch2DataVersions[ArraySize(bridgedSwitchClusters)];

// ---------------------------------------------------------------------------
//
// POWER SOURCE ENDPOINT: contains the following clusters:
//   - Power Source
//   - Descriptor
//   - Bridged Device Basic

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(powerSourceAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(ZCL_POWER_SOURCE_BAT_CHARGE_LEVEL_ATTRIBUTE_ID, ENUM8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_POWER_SOURCE_ORDER_ATTRIBUTE_ID, INT8U, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_POWER_SOURCE_STATUS_ATTRIBUTE_ID, ENUM8, 1, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(ZCL_POWER_SOURCE_DESCRIPTION_ATTRIBUTE_ID, CHAR_STRING, 32, 0), DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedPowerSourceClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, bridgedDeviceBasicAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_POWER_SOURCE_CLUSTER_ID, powerSourceAttrs, nullptr, nullptr), DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(bridgedPowerSourceEndpoint, bridgedPowerSourceClusters);

// ---------------------------------------------------------------------------
//
// COMPOSED DEVICE ENDPOINT: contains the following clusters:
//   - Descriptor
//   - Bridged Device Basic

// Composed Device Configuration
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedComposedDeviceClusters)
DECLARE_DYNAMIC_CLUSTER(ZCL_DESCRIPTOR_CLUSTER_ID, descriptorAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID, bridgedDeviceBasicAttrs, nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_ENDPOINT(bridgedComposedDeviceEndpoint, bridgedComposedDeviceClusters);
DataVersion gComposedDeviceDataVersions[ArraySize(bridgedComposedDeviceClusters)];
DataVersion gComposedSwitch1DataVersions[ArraySize(bridgedSwitchClusters)];
DataVersion gComposedSwitch2DataVersions[ArraySize(bridgedSwitchClusters)];
DataVersion gComposedPowerSourceDataVersions[ArraySize(bridgedPowerSourceClusters)];

} // namespace

// REVISION DEFINITIONS:
// =================================================================================

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION (1u)
#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)
#define ZCL_SWITCH_CLUSTER_REVISION (1u)
#define ZCL_POWER_SOURCE_CLUSTER_REVISION (1u)

// ---------------------------------------------------------------------------

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId = chip::kInvalidEndpointId)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            while (1)
            {
                dev->SetEndpointId(gCurrentEndpointId);
                dev->SetParentEndpointId(parentEndpointId);
                ret =
                    emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    return -1;
                }
                // Handle wrap condition
                if (++gCurrentEndpointId < gFirstDynamicEndpointId)
                {
                    gCurrentEndpointId = gFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int RemoveDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (gDevices[index] == dev)
        {
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            gDevices[index] = nullptr;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId)
{
    std::vector<EndpointListInfo> infoList;

    for (auto room : gRooms)
    {
        if (room->getIsVisible())
        {
            EndpointListInfo info(room->getEndpointListId(), room->getName(), room->getType());
            int index = 0;
            while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
            {
                if ((gDevices[index] != nullptr) && (gDevices[index]->GetParentEndpointId() == parentId))
                {
                    std::string location;
                    if (room->getType() == BridgedActions::EndpointListTypeEnum::kZone)
                    {
                        location = gDevices[index]->GetZone();
                    }
                    else
                    {
                        location = gDevices[index]->GetLocation();
                    }
                    if (room->getName().compare(location) == 0)
                    {
                        info.AddEndpointId(gDevices[index]->GetEndpointId());
                    }
                }
                index++;
            }
            if (info.GetEndpointListSize() > 0)
            {
                infoList.push_back(info);
            }
        }
    }

    return infoList;
}

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId)
{
    return gActions;
}

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    if (itemChangedMask & Device::kChanged_Reachable)
    {
        uint8_t reachable = dev->IsReachable() ? 1 : 0;
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID,
                                               ZCL_REACHABLE_ATTRIBUTE_ID, ZCL_BOOLEAN_ATTRIBUTE_TYPE, &reachable);
    }

    if (itemChangedMask & Device::kChanged_Name)
    {
        uint8_t zclName[kNodeLabelSize];
        MutableByteSpan zclNameSpan(zclName);
        MakeZclCharString(zclNameSpan, dev->GetName());
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID,
                                               ZCL_NODE_LABEL_ATTRIBUTE_ID, ZCL_CHAR_STRING_ATTRIBUTE_TYPE, zclNameSpan.data());
    }
}

void HandleDeviceOnOffStatusChanged(DeviceOnOff * dev, DeviceOnOff::Changed_t itemChangedMask)
{
    if (itemChangedMask & (DeviceOnOff::kChanged_Reachable | DeviceOnOff::kChanged_Name | DeviceOnOff::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }

    if (itemChangedMask & DeviceOnOff::kChanged_OnOff)
    {
        uint8_t isOn = dev->IsOn() ? 1 : 0;
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID,
                                               ZCL_BOOLEAN_ATTRIBUTE_TYPE, &isOn);
    }
}

void HandleDeviceSwitchStatusChanged(DeviceSwitch * dev, DeviceSwitch::Changed_t itemChangedMask)
{
    if (itemChangedMask & (DeviceSwitch::kChanged_Reachable | DeviceSwitch::kChanged_Name | DeviceSwitch::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }

    if (itemChangedMask & DeviceSwitch::kChanged_NumberOfPositions)
    {
        uint8_t numberOfPositions = dev->GetNumberOfPositions();
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_SWITCH_CLUSTER_ID, ZCL_NUMBER_OF_POSITIONS_ATTRIBUTE_ID,
                                               ZCL_INT8U_ATTRIBUTE_TYPE, &numberOfPositions);
    }

    if (itemChangedMask & DeviceSwitch::kChanged_CurrentPosition)
    {
        uint8_t currentPosition = dev->GetCurrentPosition();
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_SWITCH_CLUSTER_ID, ZCL_CURRENT_POSITION_ATTRIBUTE_ID,
                                               ZCL_INT8U_ATTRIBUTE_TYPE, &currentPosition);
    }

    if (itemChangedMask & DeviceSwitch::kChanged_MultiPressMax)
    {
        uint8_t multiPressMax = dev->GetMultiPressMax();
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), ZCL_SWITCH_CLUSTER_ID, ZCL_MULTI_PRESS_MAX_ATTRIBUTE_ID,
                                               ZCL_INT8U_ATTRIBUTE_TYPE, &multiPressMax);
    }
}

void HandleDevicePowerSourceStatusChanged(DevicePowerSource * dev, DevicePowerSource::Changed_t itemChangedMask)
{
    using namespace app::Clusters;
    if (itemChangedMask &
        (DevicePowerSource::kChanged_Reachable | DevicePowerSource::kChanged_Name | DevicePowerSource::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }

    if (itemChangedMask & DevicePowerSource::kChanged_BatLevel)
    {
        uint8_t batChargeLevel = dev->GetBatChargeLevel();
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), PowerSource::Id, PowerSource::Attributes::BatChargeLevel::Id,
                                               ZCL_INT8U_ATTRIBUTE_TYPE, &batChargeLevel);
    }

    if (itemChangedMask & DevicePowerSource::kChanged_Description)
    {
        MatterReportingAttributeChangeCallback(dev->GetEndpointId(), PowerSource::Id, PowerSource::Attributes::Description::Id);
    }
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                    uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == ZCL_REACHABLE_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->IsReachable() ? 1 : 0;
    }
    else if ((attributeId == ZCL_NODE_LABEL_ATTRIBUTE_ID) && (maxReadLength == 32))
    {
        MutableByteSpan zclNameSpan(buffer, maxReadLength);
        MakeZclCharString(zclNameSpan, dev->GetName());
    }
    else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
    {
        *buffer = (uint16_t) ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadOnOffAttribute(DeviceOnOff * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->IsOn() ? 1 : 0;
    }
    else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
    {
        *buffer = (uint16_t) ZCL_ON_OFF_CLUSTER_REVISION;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteOnOffAttribute(DeviceOnOff * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

    if ((attributeId == ZCL_ON_OFF_ATTRIBUTE_ID) && (dev->IsReachable()))
    {
        if (*buffer)
        {
            dev->SetOnOff(true);
        }
        else
        {
            dev->SetOnOff(false);
        }
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadSwitchAttribute(DeviceSwitch * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    if ((attributeId == ZCL_NUMBER_OF_POSITIONS_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->GetNumberOfPositions();
    }
    else if ((attributeId == ZCL_CURRENT_POSITION_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->GetCurrentPosition();
    }
    else if ((attributeId == ZCL_MULTI_PRESS_MAX_ATTRIBUTE_ID) && (maxReadLength == 1))
    {
        *buffer = dev->GetMultiPressMax();
    }
    else if ((attributeId == ZCL_FEATURE_MAP_SERVER_ATTRIBUTE_ID) && (maxReadLength == 4))
    {
        *(uint32_t *) buffer = dev->GetFeatureMap();
    }
    else if ((attributeId == ZCL_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID) && (maxReadLength == 2))
    {
        *buffer = (uint16_t) ZCL_SWITCH_CLUSTER_REVISION;
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadPowerSourceAttribute(DevicePowerSource * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                             uint16_t maxReadLength)
{
    using namespace app::Clusters;
    if ((attributeId == PowerSource::Attributes::BatChargeLevel::Id) && (maxReadLength == 1))
    {
        *buffer = dev->GetBatChargeLevel();
    }
    else if ((attributeId == PowerSource::Attributes::Order::Id) && (maxReadLength == 1))
    {
        *buffer = dev->GetOrder();
    }
    else if ((attributeId == PowerSource::Attributes::Status::Id) && (maxReadLength == 1))
    {
        *buffer = dev->GetStatus();
    }
    else if ((attributeId == PowerSource::Attributes::Description::Id) && (maxReadLength == 32))
    {
        MutableByteSpan zclDescpitionSpan(buffer, maxReadLength);
        MakeZclCharString(zclDescpitionSpan, dev->GetDescription().c_str());
    }
    else if ((attributeId == PowerSource::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_POWER_SOURCE_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else if ((attributeId == PowerSource::Attributes::FeatureMap::Id) && (maxReadLength == 4))
    {
        uint32_t featureMap = dev->GetFeatureMap();
        memcpy(buffer, &featureMap, sizeof(featureMap));
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    if ((endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != nullptr))
    {
        Device * dev = gDevices[endpointIndex];

        if (clusterId == ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_ID)
        {
            ret = HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_ON_OFF_CLUSTER_ID)
        {
            ret = HandleReadOnOffAttribute(static_cast<DeviceOnOff *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ZCL_SWITCH_CLUSTER_ID)
        {
            ret =
                HandleReadSwitchAttribute(static_cast<DeviceSwitch *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == chip::app::Clusters::PowerSource::Id)
        {
            ret = HandleReadPowerSourceAttribute(static_cast<DevicePowerSource *>(dev), attributeMetadata->attributeId, buffer,
                                                 maxReadLength);
        }
    }

    return ret;
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    // ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback: ep=%d", endpoint);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == ZCL_ON_OFF_CLUSTER_ID))
        {
            ret = HandleWriteOnOffAttribute(static_cast<DeviceOnOff *>(dev), attributeMetadata->attributeId, buffer);
        }
    }

    return ret;
}

void runOnOffRoomAction(Room * room, bool actionOn, EndpointId endpointId, uint16_t actionID, uint32_t invokeID, bool hasInvokeID)
{
    if (hasInvokeID)
    {
        BridgedActions::Events::StateChanged::Type event{ actionID, invokeID, BridgedActions::ActionStateEnum::kActive };
        EventNumber eventNumber;
        chip::app::LogEvent(event, endpointId, eventNumber);
    }

    // Check and run the action for ActionLight1 - ActionLight4
    if (room->getName().compare(ActionLight1.GetLocation()) == 0)
    {
        ActionLight1.SetOnOff(actionOn);
    }
    if (room->getName().compare(ActionLight2.GetLocation()) == 0)
    {
        ActionLight2.SetOnOff(actionOn);
    }
    if (room->getName().compare(ActionLight3.GetLocation()) == 0)
    {
        ActionLight3.SetOnOff(actionOn);
    }
    if (room->getName().compare(ActionLight4.GetLocation()) == 0)
    {
        ActionLight4.SetOnOff(actionOn);
    }

    if (hasInvokeID)
    {
        BridgedActions::Events::StateChanged::Type event{ actionID, invokeID, BridgedActions::ActionStateEnum::kInactive };
        EventNumber eventNumber;
        chip::app::LogEvent(event, endpointId, eventNumber);
    }
}

bool emberAfBridgedActionsClusterInstantActionCallback(app::CommandHandler * commandObj,
                                                       const app::ConcreteCommandPath & commandPath,
                                                       const BridgedActions::Commands::InstantAction::DecodableType & commandData)
{
    bool hasInvokeID      = false;
    uint32_t invokeID     = 0;
    EndpointId endpointID = commandPath.mEndpointId;
    auto & actionID       = commandData.actionID;

    if (commandData.invokeID.HasValue())
    {
        hasInvokeID = true;
        invokeID    = commandData.invokeID.Value();
    }

    if (actionID == action1.getActionId() && action1.getIsVisible())
    {
        // Turn On Lights in Room 1
        runOnOffRoomAction(&room1, true, endpointID, actionID, invokeID, hasInvokeID);
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        return true;
    }
    if (actionID == action2.getActionId() && action2.getIsVisible())
    {
        // Turn On Lights in Room 2
        runOnOffRoomAction(&room2, true, endpointID, actionID, invokeID, hasInvokeID);
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        return true;
    }
    if (actionID == action3.getActionId() && action3.getIsVisible())
    {
        // Turn Off Lights in Room 1
        runOnOffRoomAction(&room1, false, endpointID, actionID, invokeID, hasInvokeID);
        commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        return true;
    }

    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

void ApplicationInit() {}

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gBridgedSwitchDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT_SWITCH, DEVICE_VERSION_DEFAULT },
                                                        { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gBridgedComposedDeviceTypes[] = { { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gComposedSwitchDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT_SWITCH, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gComposedPowerSourceDeviceTypes[] = { { DEVICE_TYPE_POWER_SOURCE, DEVICE_VERSION_DEFAULT } };

#define POLL_INTERVAL_MS (100)
uint8_t poll_prescale = 0;

bool kbhit()
{
    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

void * bridge_polling_thread(void * context)
{
    bool light1_added = true;
    bool light2_added = false;
    while (1)
    {
        if (kbhit())
        {
            int ch = getchar();

            // Commands used for the actions bridge test plan.
            if (ch == '2' && light2_added == false)
            {
                // TC-BR-2 step 2, Add Light2
                AddDeviceEndpoint(&Light2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                                  Span<DataVersion>(gLight2DataVersions), 1);
                light2_added = true;
            }
            else if (ch == '4' && light1_added == true)
            {
                // TC-BR-2 step 4, Remove Light 1
                RemoveDeviceEndpoint(&Light1);
                light1_added = false;
            }
            if (ch == '5' && light1_added == false)
            {
                // TC-BR-2 step 5, Add Light 1 back
                AddDeviceEndpoint(&Light1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                                  Span<DataVersion>(gLight1DataVersions), 1);
                light1_added = true;
            }
            if (ch == 'b')
            {
                // TC-BR-3 step 1b, rename lights
                if (light1_added)
                {
                    Light1.SetName("Light 1b");
                }
                if (light2_added)
                {
                    Light2.SetName("Light 2b");
                }
            }
            if (ch == 'c')
            {
                // TC-BR-3 step 2c, change the state of the lights
                if (light1_added)
                {
                    Light1.Toggle();
                }
                if (light2_added)
                {
                    Light2.Toggle();
                }
            }

            // Commands used for the actions cluster test plan.
            if (ch == 'r')
            {
                // TC-ACT-2.2 step 2c, rename "Room 1"
                room1.setName("Room 1 renamed");
                ActionLight1.SetLocation(room1.getName());
                ActionLight2.SetLocation(room1.getName());
            }
            if (ch == 'f')
            {
                // TC-ACT-2.2 step 2f, move "Action Light 3" from "Room 2" to "Room 1"
                ActionLight3.SetLocation(room1.getName());
            }
            if (ch == 'i')
            {
                // TC-ACT-2.2 step 2i, remove "Room 2" (make it not visible in the endpoint list), do not remove the lights
                room2.setIsVisible(false);
            }
            if (ch == 'l')
            {
                // TC-ACT-2.2 step 2l, add a new "Zone 3" and add "Action Light 2" to the new zone
                room3.setIsVisible(true);
                ActionLight2.SetZone("Zone 3");
            }
            if (ch == 'm')
            {
                // TC-ACT-2.2 step 3c, rename "Turn on Room 1 lights"
                action1.setName("Turn On Room 1");
            }
            if (ch == 'n')
            {
                // TC-ACT-2.2 step 3f, remove "Turn on Room 2 lights"
                action2.setIsVisible(false);
            }
            if (ch == 'o')
            {
                // TC-ACT-2.2 step 3i, add "Turn off Room 1 renamed lights"
                action3.setIsVisible(true);
            }
            continue;
        }

        // Sleep to avoid tight loop reading commands
        usleep(POLL_INTERVAL_MS * 1000);
    }

    return nullptr;
}

int main(int argc, char * argv[])
{
    // Clear out the device database
    memset(gDevices, 0, sizeof(gDevices));

    // Setup Mock Devices
    Light1.SetChangeCallback(&HandleDeviceOnOffStatusChanged);
    Light2.SetChangeCallback(&HandleDeviceOnOffStatusChanged);

    Light1.SetReachable(true);
    Light2.SetReachable(true);

    Switch1.SetChangeCallback(&HandleDeviceSwitchStatusChanged);
    Switch2.SetChangeCallback(&HandleDeviceSwitchStatusChanged);

    Switch1.SetReachable(true);
    Switch2.SetReachable(true);

    // Setup devices for action cluster tests
    ActionLight1.SetChangeCallback(&HandleDeviceOnOffStatusChanged);
    ActionLight2.SetChangeCallback(&HandleDeviceOnOffStatusChanged);
    ActionLight3.SetChangeCallback(&HandleDeviceOnOffStatusChanged);
    ActionLight4.SetChangeCallback(&HandleDeviceOnOffStatusChanged);

    ActionLight1.SetReachable(true);
    ActionLight2.SetReachable(true);
    ActionLight3.SetReachable(true);
    ActionLight4.SetReachable(true);

    // Define composed device with two switches
    ComposedDevice ComposedDevice("Composed Switcher", "Bedroom");
    DeviceSwitch ComposedSwitch1("Composed Switch 1", "Bedroom", EMBER_AF_SWITCH_FEATURE_LATCHING_SWITCH);
    DeviceSwitch ComposedSwitch2("Composed Switch 2", "Bedroom",
                                 EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH | EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_RELEASE |
                                     EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_LONG_PRESS |
                                     EMBER_AF_SWITCH_FEATURE_MOMENTARY_SWITCH_MULTI_PRESS);
    DevicePowerSource ComposedPowerSource("Composed Power Source", "Bedroom", EMBER_AF_POWER_SOURCE_FEATURE_BATTERY);

    ComposedSwitch1.SetChangeCallback(&HandleDeviceSwitchStatusChanged);
    ComposedSwitch2.SetChangeCallback(&HandleDeviceSwitchStatusChanged);
    ComposedPowerSource.SetChangeCallback(&HandleDevicePowerSourceStatusChanged);

    ComposedDevice.SetReachable(true);
    ComposedSwitch1.SetReachable(true);
    ComposedSwitch2.SetReachable(true);
    ComposedPowerSource.SetReachable(true);
    ComposedPowerSource.SetBatChargeLevel(58);

    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    // Init Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    initParams.operationalServicePort = LinuxDeviceOptions::GetInstance().securedDevicePort;
#endif

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

    // Add light 1 -> will be mapped to ZCL endpoints 3
    AddDeviceEndpoint(&Light1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight1DataVersions), 1);

    // Add switch 1..2 --> will be mapped to ZCL endpoints 4,5
    AddDeviceEndpoint(&Switch1, &bridgedSwitchEndpoint, Span<const EmberAfDeviceType>(gBridgedSwitchDeviceTypes),
                      Span<DataVersion>(gSwitch1DataVersions), 1);
    AddDeviceEndpoint(&Switch2, &bridgedSwitchEndpoint, Span<const EmberAfDeviceType>(gBridgedSwitchDeviceTypes),
                      Span<DataVersion>(gSwitch2DataVersions), 1);

    // Add composed Device with two buttons and a power source
    AddDeviceEndpoint(&ComposedDevice, &bridgedComposedDeviceEndpoint, Span<const EmberAfDeviceType>(gBridgedComposedDeviceTypes),
                      Span<DataVersion>(gComposedDeviceDataVersions), 1);
    AddDeviceEndpoint(&ComposedSwitch1, &bridgedSwitchEndpoint, Span<const EmberAfDeviceType>(gComposedSwitchDeviceTypes),
                      Span<DataVersion>(gComposedSwitch1DataVersions), ComposedDevice.GetEndpointId());
    AddDeviceEndpoint(&ComposedSwitch2, &bridgedSwitchEndpoint, Span<const EmberAfDeviceType>(gComposedSwitchDeviceTypes),
                      Span<DataVersion>(gComposedSwitch2DataVersions), ComposedDevice.GetEndpointId());
    AddDeviceEndpoint(&ComposedPowerSource, &bridgedPowerSourceEndpoint,
                      Span<const EmberAfDeviceType>(gComposedPowerSourceDeviceTypes),
                      Span<DataVersion>(gComposedPowerSourceDataVersions), ComposedDevice.GetEndpointId());

    // Add 4 lights for the Action Clusters tests
    AddDeviceEndpoint(&ActionLight1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gActionLight1DataVersions), 1);
    AddDeviceEndpoint(&ActionLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gActionLight2DataVersions), 1);
    AddDeviceEndpoint(&ActionLight3, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gActionLight3DataVersions), 1);
    AddDeviceEndpoint(&ActionLight4, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gActionLight4DataVersions), 1);
    gRooms.push_back(&room1);
    gRooms.push_back(&room2);
    gRooms.push_back(&room3);

    gActions.push_back(&action1);
    gActions.push_back(&action2);
    gActions.push_back(&action3);

    {
        pthread_t poll_thread;
        int res = pthread_create(&poll_thread, nullptr, bridge_polling_thread, nullptr);
        if (res)
        {
            printf("Error creating polling thread: %d\n", res);
            exit(1);
        }
    }

    // Run CHIP

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
