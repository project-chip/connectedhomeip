/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "Device.h"
#include "DeviceCallbacks.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "shell_extension/launch.h"
#include "shell_extension/openthread_cli_register.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <bridged-actions-stub.h>
#include <common/Esp32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ZclString.h>
#include <platform/ESP32/ESP32Utils.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#endif // CONFIG_ENABLE_CHIP_SHELL

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

namespace {
#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
chip::DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
chip::DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

std::unique_ptr<chip::app::Clusters::Actions::ActionsDelegateImpl> sActionsDelegateImpl;
std::unique_ptr<chip::app::Clusters::Actions::ActionsServer> sActionsServer;
} // namespace

extern const char TAG[] = "bridge-app";

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::Platform;
using namespace ::chip::Credentials;
using namespace ::chip::app::Clusters;

static AppDeviceCallbacks AppCallback;

static const int kNodeLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
static const int kDescriptorAttributeArraySize = 254;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT]; // number of dynamic endpoints count

// 4 Bridged devices
static Device gLight1("Light 1", "Office");
static Device gLight2("Light 2", "Office");
static Device gLight3("Light 3", "Kitchen");
static Device gLight4("Light 4", "Den");

// Track which lights are currently added
static bool gLight1Added = false;
static bool gLight2Added = false;
static bool gLight3Added = false;
static bool gLight4Added = false;

// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100

// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

/* BRIDGED DEVICE ENDPOINT: contains the following clusters:
   - On/Off
   - Descriptor
   - Bridged Device Basic Information
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

// Declare Bridged Device Basic Information cluster attributes
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
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr,
                            nullptr) DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);

DataVersion gLight1DataVersions[MATTER_ARRAY_SIZE(bridgedLightClusters)];
DataVersion gLight2DataVersions[MATTER_ARRAY_SIZE(bridgedLightClusters)];
DataVersion gLight3DataVersions[MATTER_ARRAY_SIZE(bridgedLightClusters)];
DataVersion gLight4DataVersions[MATTER_ARRAY_SIZE(bridgedLightClusters)];

/* REVISION definitions:
 */

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (2u)
#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (NULL == gDevices[index])
        {
            gDevices[index] = dev;
            CHIP_ERROR err;
            while (true)
            {
                dev->SetEndpointId(gCurrentEndpointId);
                err =
                    emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
                if (err == CHIP_NO_ERROR)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                else if (err != CHIP_ERROR_ENDPOINT_EXISTS)
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

CHIP_ERROR RemoveDeviceEndpoint(Device * dev)
{
    for (uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
    {
        if (gDevices[index] == dev)
        {
            // Silence complaints about unused ep when progress logging
            // disabled.
            [[maybe_unused]] EndpointId ep = emberAfClearDynamicEndpoint(index);
            gDevices[index]                = NULL;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INTERNAL;
}

Protocols::InteractionModel::Status HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId,
                                                                          uint8_t * buffer, uint16_t maxReadLength)
{
    using namespace BridgedDeviceBasicInformation::Attributes;
    ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%" PRIu32 ", maxReadLength=%u", attributeId,
                    maxReadLength);

    if ((attributeId == Reachable::Id) && (maxReadLength == 1))
    {
        *buffer = dev->IsReachable() ? 1 : 0;
    }
    else if ((attributeId == NodeLabel::Id) && (maxReadLength == 32))
    {
        MutableByteSpan zclNameSpan(buffer, maxReadLength);
        MakeZclCharString(zclNameSpan, dev->GetName());
    }
    else if ((attributeId == ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleReadOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                             uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%" PRIu32 ", maxReadLength=%u", attributeId, maxReadLength);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        *buffer = dev->IsOn() ? 1 : 0;
    }
    else if ((attributeId == OnOff::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_ON_OFF_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status HandleWriteOnOffAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%" PRIu32, attributeId);

    VerifyOrReturnError((attributeId == OnOff::Attributes::OnOff::Id) && dev->IsReachable(),
                        Protocols::InteractionModel::Status::Failure);
    dev->SetOnOff(*buffer == 1);
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if ((endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != NULL))
    {
        Device * dev = gDevices[endpointIndex];

        if (clusterId == BridgedDeviceBasicInformation::Id)
        {
            return HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == OnOff::Id)
        {
            return HandleReadOnOffAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
    }

    return Protocols::InteractionModel::Status::Failure;
}

Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == OnOff::Id))
        {
            return HandleWriteOnOffAttribute(dev, attributeMetadata->attributeId, buffer);
        }
    }

    return Protocols::InteractionModel::Status::Failure;
}

namespace {
void CallReportingCallback(intptr_t closure)
{
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
}

void ScheduleReportingCallback(Device * dev, ClusterId cluster, AttributeId attribute)
{
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    DeviceLayer::PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
}
} // anonymous namespace

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    if (itemChangedMask & Device::kChanged_Reachable)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
    }

    if (itemChangedMask & Device::kChanged_State)
    {
        ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);
    }

    if (itemChangedMask & Device::kChanged_Name)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::NodeLabel::Id);
    }
}

const EmberAfDeviceType gRootDeviceTypes[]          = { { DEVICE_TYPE_ROOT_NODE, DEVICE_VERSION_DEFAULT } };
const EmberAfDeviceType gAggregateNodeDeviceTypes[] = { { DEVICE_TYPE_BRIDGE, DEVICE_VERSION_DEFAULT } };

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

#if CONFIG_ENABLE_CHIP_SHELL
using chip::Shell::Engine;
using chip::Shell::shell_command_t;

Engine sShellBridgeSubCommands;

static bool GetLightByNumber(int num, Device ** dev, bool ** added, DataVersion ** dataVersions)
{
    switch (num)
    {
    case 1:
        *dev          = &gLight1;
        *added        = &gLight1Added;
        *dataVersions = gLight1DataVersions;
        return true;
    case 2:
        *dev          = &gLight2;
        *added        = &gLight2Added;
        *dataVersions = gLight2DataVersions;
        return true;
    case 3:
        *dev          = &gLight3;
        *added        = &gLight3Added;
        *dataVersions = gLight3DataVersions;
        return true;
    case 4:
        *dev          = &gLight4;
        *added        = &gLight4Added;
        *dataVersions = gLight4DataVersions;
        return true;
    default:
        return false;
    }
}

static CHIP_ERROR BridgeHelpHandler(int argc, char ** argv)
{
    sShellBridgeSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeAddHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        ESP_LOGE(TAG, "Usage: bridge add <1-4>");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char * end;
    long lightNumLong = strtol(argv[0], &end, 10);
    if (end == argv[0] || *end != '\0')
    {
        ESP_LOGE(TAG, "Invalid light number: %s", argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    int lightNum = static_cast<int>(lightNumLong);

    Device * dev;
    bool * added;
    DataVersion * dataVersions;

    if (!GetLightByNumber(lightNum, &dev, &added, &dataVersions))
    {
        ESP_LOGE(TAG, "Invalid light number: %d (must be 1-4)", lightNum);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (*added)
    {
        ESP_LOGI(TAG, "Light %d is already added", lightNum);
        return CHIP_NO_ERROR;
    }

    int result = AddDeviceEndpoint(dev, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                                   Span<DataVersion>(dataVersions, MATTER_ARRAY_SIZE(bridgedLightClusters)), 1);
    if (result >= 0)
    {
        *added = true;
        ESP_LOGI(TAG, "Added Light %d to endpoint %d", lightNum, dev->GetEndpointId());
    }
    else
    {
        ESP_LOGE(TAG, "Failed to add Light %d", lightNum);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeRemoveHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        ESP_LOGE(TAG, "Usage: bridge remove <1-4>");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char * end;
    long lightNumLong = strtol(argv[0], &end, 10);
    if (end == argv[0] || *end != '\0')
    {
        ESP_LOGE(TAG, "Invalid light number: %s", argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    int lightNum = static_cast<int>(lightNumLong);

    Device * dev;
    bool * added;
    DataVersion * dataVersions;

    if (!GetLightByNumber(lightNum, &dev, &added, &dataVersions))
    {
        ESP_LOGE(TAG, "Invalid light number: %d (must be 1-4)", lightNum);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!*added)
    {
        ESP_LOGI(TAG, "Light %d is not currently added", lightNum);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = RemoveDeviceEndpoint(dev);
    if (err == CHIP_NO_ERROR)
    {
        *added = false;
        ESP_LOGI(TAG, "Removed Light %d", lightNum);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to remove Light %d", lightNum);
        return err;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeListHandler(int argc, char ** argv)
{
    ESP_LOGI(TAG, "Bridged devices:");

    struct
    {
        int num;
        Device * dev;
        bool added;
    } lights[] = {
        { 1, &gLight1, gLight1Added },
        { 2, &gLight2, gLight2Added },
        { 3, &gLight3, gLight3Added },
        { 4, &gLight4, gLight4Added },
    };

    for (const auto & light : lights)
    {
        if (light.added)
        {
            ESP_LOGI(TAG, "  Light %d: \"%s\" (endpoint %d, %s)", light.num, light.dev->GetName(), light.dev->GetEndpointId(),
                     light.dev->IsOn() ? "ON" : "OFF");
        }
        else
        {
            ESP_LOGI(TAG, "  Light %d: not added", light.num);
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeRenameHandler(int argc, char ** argv)
{
    // Rename all added lights to "Light Xb" (TC-BR-3)
    struct
    {
        int num;
        Device * dev;
        bool added;
    } lights[] = {
        { 1, &gLight1, gLight1Added },
        { 2, &gLight2, gLight2Added },
        { 3, &gLight3, gLight3Added },
        { 4, &gLight4, gLight4Added },
    };

    for (const auto & light : lights)
    {
        if (light.added)
        {
            char newName[16];
            snprintf(newName, sizeof(newName), "Light %db", light.num);
            light.dev->SetName(newName);
            ESP_LOGI(TAG, "Renamed Light %d to '%s'", light.num, newName);
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeToggleHandler(int argc, char ** argv)
{
    // Toggle all added lights
    struct
    {
        int num;
        Device * dev;
        bool added;
    } lights[] = {
        { 1, &gLight1, gLight1Added },
        { 2, &gLight2, gLight2Added },
        { 3, &gLight3, gLight3Added },
        { 4, &gLight4, gLight4Added },
    };

    for (const auto & light : lights)
    {
        if (light.added)
        {
            light.dev->SetOnOff(!light.dev->IsOn());
            ESP_LOGI(TAG, "Toggled Light %d: now %s", light.num, light.dev->IsOn() ? "ON" : "OFF");
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return BridgeHelpHandler(argc, argv);
    }
    return sShellBridgeSubCommands.ExecCommand(argc, argv);
}

static void RegisterBridgeCommands()
{
    static const shell_command_t sBridgeSubCommands[] = {
        { &BridgeHelpHandler, "help", "Usage: bridge <subcommand>" },
        { &BridgeAddHandler, "add", "Add a light: bridge add <1-4>" },
        { &BridgeRemoveHandler, "remove", "Remove a light: bridge remove <1-4>" },
        { &BridgeListHandler, "list", "List all bridged devices" },
        { &BridgeRenameHandler, "rename", "Rename all lights to 'Light Xb'" },
        { &BridgeToggleHandler, "toggle", "Toggle all lights on/off" },
    };

    static const shell_command_t sBridgeCommand = { &BridgeCommandHandler, "bridge",
                                                    "Bridge commands. Usage: bridge <subcommand>" };

    sShellBridgeSubCommands.RegisterCommands(sBridgeSubCommands, MATTER_ARRAY_SIZE(sBridgeSubCommands));
    Engine::Root().RegisterCommands(&sBridgeCommand, 1);
}
#endif // CONFIG_ENABLE_CHIP_SHELL

static void InitServer(intptr_t context)
{
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    Esp32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

    // A bridge has root node device type on EP0 and aggregate node device type (bridge) at EP1
    emberAfSetDeviceTypeList(0, Span<const EmberAfDeviceType>(gRootDeviceTypes));
    emberAfSetDeviceTypeList(1, Span<const EmberAfDeviceType>(gAggregateNodeDeviceTypes));

    // Add lights 1..3 --> will be mapped to ZCL endpoints 3, 4, 5
    if (AddDeviceEndpoint(&gLight1, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                          Span<DataVersion>(gLight1DataVersions), 1) >= 0)
    {
        gLight1Added = true;
    }

    if (AddDeviceEndpoint(&gLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                          Span<DataVersion>(gLight2DataVersions), 1) >= 0)
    {
        gLight2Added = true;
    }

    if (AddDeviceEndpoint(&gLight3, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                          Span<DataVersion>(gLight3DataVersions), 1) >= 0)
    {
        gLight3Added = true;
    }

    // Remove Light 2 -- Lights 1 & 3 will remain mapped to endpoints 3 & 5
    if (RemoveDeviceEndpoint(&gLight2) == CHIP_NO_ERROR)
    {
        gLight2Added = false;
    }

    // Add Light 4 -- > will be mapped to ZCL endpoint 6
    if (AddDeviceEndpoint(&gLight4, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                          Span<DataVersion>(gLight4DataVersions), 1) >= 0)
    {
        gLight4Added = true;
    }

    // Re-add Light 2 -- > will be mapped to ZCL endpoint 7
    if (AddDeviceEndpoint(&gLight2, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                          Span<DataVersion>(gLight2DataVersions), 1) >= 0)
    {
        gLight2Added = true;
    }
}

void emberAfActionsClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(endpoint == 1,
                   ChipLogError(Zcl, "Actions cluster delegate is not implemented for endpoint with id %d.", endpoint));
    VerifyOrReturn(emberAfContainsServer(endpoint, app::Clusters::Actions::Id) == true,
                   ChipLogError(Zcl, "Endpoint %d does not support Actions cluster.", endpoint));
    VerifyOrReturn(!sActionsDelegateImpl && !sActionsServer);

    sActionsDelegateImpl = std::make_unique<app::Clusters::Actions::ActionsDelegateImpl>();
    sActionsServer       = std::make_unique<app::Clusters::Actions::ActionsServer>(endpoint, *sActionsDelegateImpl.get());

    sActionsServer->Init();
}

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default()  failed: %s", esp_err_to_name(err));
        return;
    }

#if CONFIG_ENABLE_CHIP_SHELL
#if CONFIG_OPENTHREAD_CLI
    chip::RegisterOpenThreadCliCommands();
#endif
    chip::LaunchShell();
    RegisterBridgeCommands();
#endif

    CHIP_ERROR chip_err = CHIP_NO_ERROR;

    // bridge will have own database named gDevices.
    // Clear database
    memset(gDevices, 0, sizeof(gDevices));

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize the Wi-Fi stack");
        return;
    }
#endif

    gLight1.SetReachable(true);
    gLight2.SetReachable(true);
    gLight3.SetReachable(true);
    gLight4.SetReachable(true);

    // Whenever bridged device changes its state
    gLight1.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight2.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight3.SetChangeCallback(&HandleDeviceStatusChanged);
    gLight4.SetChangeCallback(&HandleDeviceStatusChanged);

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    chip_err = deviceMgr.Init(&AppCallback);
    if (chip_err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %" CHIP_ERROR_FORMAT, chip_err.Format());
        return;
    }

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}
