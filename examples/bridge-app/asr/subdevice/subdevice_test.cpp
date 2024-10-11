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

#include "SubDevice.h"
#include "SubDeviceManager.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/reporting/reporting.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;
using namespace ::chip::Platform;
using namespace ::chip::app::Clusters;

#ifdef ENABLE_ASR_BRIDGE_SUBDEVICE_TEST

static const int kNodeLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;

// 4 Bridged devices
static SubDevice gLight1("Light 1", "Office");
static SubDevice gLight2("Light 2", "Office");
static SubDevice gLight3("Light 3", "Kitchen");
static SubDevice gLight4("Light 4", "Den");

/* BRIDGED DEVICE ENDPOINT: contains the following clusters:
   - On/Off
   - Descriptor
   - Bridged Device Basic
*/

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OnOff::Id, BOOLEAN, 1, 0), /* on/off */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* device list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
    DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, CHAR_STRING, kNodeLabelSize, 0), /* NodeLabel */
    DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::Reachable::Id, BOOLEAN, 1, 0),              /* Reachable */
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
DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, ZAP_CLUSTER_MASK(SERVER), onOffIncomingCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    // DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasic::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr)
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr,
                            nullptr) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);

DataVersion gLight1DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight2DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight3DataVersions[ArraySize(bridgedLightClusters)];
DataVersion gLight4DataVersions[ArraySize(bridgedLightClusters)];

const EmberAfDeviceType gRootDeviceTypes[]          = { { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT } };
const EmberAfDeviceType gAggregateNodeDeviceTypes[] = { { DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

extern "C" void Sync_SubDevice_test(void);
extern "C" void Add_SubDevice_test(void);
extern "C" void Remove_SubDevice_test(void);

void Sync_SubDevice_test()
{
    gLight1.SetReachable(true);
    gLight2.SetReachable(true);
    gLight3.SetReachable(true);
    gLight4.SetReachable(true);

    // Whenever bridged device changes its state
    gLight1.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight2.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight3.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight4.SetChangeCallback(&HandleDeviceStatusChanged);

    // Add lights 1..4 --> will be mapped to ZCL endpoints 3..6
    AddDeviceEndpoint(&gLight1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight1DataVersions), 1);
    AddDeviceEndpoint(&gLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight2DataVersions), 1);
    AddDeviceEndpoint(&gLight3, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight3DataVersions), 1);
    AddDeviceEndpoint(&gLight4, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight4DataVersions), 1);
}

void Add_SubDevice_test()
{
    gLight2.SetReachable(true);
    gLight2.SetChangeCallback(&HandleDeviceStatusChanged);
    // Re-add Light 2 -- > will be mapped to ZCL endpoint 7
    AddDeviceEndpoint(&gLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLight2DataVersions), 1);
}

void Remove_SubDevice_test()
{
    gLight2.SetChangeCallback(NULL);
    gLight2.SetReachable(false);
    // Remove Light 2 -- Others Lights remain
    RemoveDeviceEndpoint(&gLight2);
}
#endif // ENABLE_ASR_BRIDGE_SUBDEVICE_TEST
