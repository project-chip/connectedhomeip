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

// Dynamic device storage - data versions for bridged devices (devices tracked in gDevices)
static constexpr size_t kMaxBridgedDevices = CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static DataVersion * gBridgedDataVersions[kMaxBridgedDevices];
static size_t gBridgedDeviceCount = 0;

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

// Find device index by endpoint ID
static CHIP_ERROR FindDeviceByEndpoint(EndpointId endpointId, uint16_t &index)
{
    for (size_t i = 0; i < kMaxBridgedDevices; i++)
    {
        if (gDevices[i] != nullptr && gDevices[i]->GetEndpointId() == endpointId)
        {
            index = static_cast<uint16_t>(i);
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

static CHIP_ERROR BridgeHelpHandler(int argc, char ** argv)
{
    sShellBridgeSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeAddHandler(int argc, char ** argv)
{
    if (argc > 2)
    {
        ESP_LOGE(TAG, "Usage: bridge add [name] [location]");
        ESP_LOGE(TAG, "  name: optional device name (default: 'Light <N>')");
        ESP_LOGE(TAG, "  location: optional location (default: 'Room')");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if we have room for more devices
    if (gBridgedDeviceCount >= kMaxBridgedDevices)
    {
        ESP_LOGE(TAG, "Max bridged devices reached (%u). Cannot add more.", kMaxBridgedDevices);
        return CHIP_ERROR_NO_MEMORY;
    }

    // Create device name - use device count + 1 for default naming
    char defaultName[32];
    const char * name;
    if (argc >= 1)
    {
        name = argv[0];
    }
    else
    {
        snprintf(defaultName, sizeof(defaultName), "Light %u", gBridgedDeviceCount + 1);
        name = defaultName;
    }

    const char * location = (argc >= 2) ? argv[1] : "Room";

    // Allocate new device and data versions
    Device * newDevice = new (std::nothrow) Device(name, location);
    if (newDevice == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for new device");
        return CHIP_ERROR_NO_MEMORY;
    }

    DataVersion * newDataVersions = new (std::nothrow) DataVersion[MATTER_ARRAY_SIZE(bridgedLightClusters)];
    if (newDataVersions == nullptr)
    {
        delete newDevice;
        ESP_LOGE(TAG, "Failed to allocate memory for data versions");
        return CHIP_ERROR_NO_MEMORY;
    }
    memset(newDataVersions, 0, sizeof(DataVersion) * MATTER_ARRAY_SIZE(bridgedLightClusters));

    // Set device as reachable and configure callback
    newDevice->SetReachable(true);
    newDevice->SetChangeCallback(&HandleDeviceStatusChanged);

    // Try to add the device endpoint
    int result = AddDeviceEndpoint(newDevice, &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                                   Span<DataVersion>(newDataVersions, MATTER_ARRAY_SIZE(bridgedLightClusters)), 1);
    if (result < 0)
    {
        delete newDevice;
        delete[] newDataVersions;
        ESP_LOGE(TAG, "Failed to add device endpoint - no available endpoints");
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    // Find first empty slot in our tracking array
    bool slotFound = false;
    for (size_t i = 0; i < kMaxBridgedDevices; i++)
    {
        if (gDevices[i] == nullptr)
        {
            gDevices[i]      = newDevice;
            gBridgedDataVersions[i] = newDataVersions;
            gBridgedDeviceCount++;
            slotFound = true;
            break;
        }
    }

    if (!slotFound)
    {
        // This shouldn't happen if gBridgedDeviceCount is accurate, but handle it gracefully
        ESP_LOGE(TAG, "Internal error: no slot found despite count check. Cleaning up.");
        RemoveDeviceEndpoint(newDevice);
        delete newDevice;
        delete[] newDataVersions;
        return CHIP_ERROR_INTERNAL;
    }

    ESP_LOGI(TAG, "Added '%s' @ %s (endpoint %d) [%u/%u]", name, location, newDevice->GetEndpointId(), gBridgedDeviceCount,
             kMaxBridgedDevices);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeRemoveHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        ESP_LOGE(TAG, "Usage: bridge remove <endpoint>");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    char * end;
    chip::EndpointId endpointId = strtoul(argv[0], &end, 10);
    if (end == argv[0] || *end != '\0' || endpointId < 0 || endpointId > 0xFFFF)
    {
        ESP_LOGE(TAG, "Invalid endpoint ID: %s (must be 0-65535)", argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint16_t index;
    CHIP_ERROR err = FindDeviceByEndpoint(endpointId, index);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "No device at endpoint %u", endpointId);
        return err;
    }

    const char * name = gDevices[index]->GetName();
    err = RemoveDeviceEndpoint(gDevices[index]);
    if (err == CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "Removed '%s' from endpoint %ld", name, endpointId);
        delete gDevices[index];
        delete[] gBridgedDataVersions[index];
        gDevices[index]      = nullptr;
        gBridgedDataVersions[index] = nullptr;
        gBridgedDeviceCount--;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to remove '%s'", name);
        return err;
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeListHandler(int argc, char ** argv)
{
    ESP_LOGI(TAG, "Bridged devices (%u/%u):", gBridgedDeviceCount, kMaxBridgedDevices);

    if (gBridgedDeviceCount == 0)
    {
        ESP_LOGI(TAG, "  (none)");
    }
    else
    {
        for (size_t i = 0; i < kMaxBridgedDevices; i++)
        {
            if (gDevices[i] != nullptr)
            {
                ESP_LOGI(TAG, "  \"%s\" @ %s (endpoint %d, %s)", gDevices[i]->GetName(), gDevices[i]->GetLocation(),
                         gDevices[i]->GetEndpointId(), gDevices[i]->IsOn() ? "ON" : "OFF");
            }
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeToggleHandler(int argc, char ** argv)
{
    if (argc == 1)
    {
        // Perform OnOff::Toggle on the bridged endpoint: bridge toggle <endpoint>
        char * end;
        chip::EndpointId endpointId = strtoul(argv[0], &end, 10);
        if (end == argv[0] || *end != '\0' || endpointId < 0 || endpointId > 0xFFFF)
        {
            ESP_LOGE(TAG, "Invalid endpoint ID: %s (must be 0-65535)", argv[0]);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        uint16_t index;
        CHIP_ERROR err = FindDeviceByEndpoint(endpointId, index);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "No device at endpoint %u", endpointId);
            return err;
        }

        gDevices[index]->SetOnOff(!gDevices[index]->IsOn());
        ESP_LOGI(TAG, "Toggled '%s' (endpoint %u): now %s", gDevices[index]->GetName(), endpointId,
                 gDevices[index]->IsOn() ? "ON" : "OFF");
    }
    else
    {
        // Toggle all devices
        ESP_LOGI(TAG, "Toggling all devices:");
        for (size_t i = 0; i < kMaxBridgedDevices; i++)
        {
            if (gDevices[i] != nullptr)
            {
                gDevices[i]->SetOnOff(!gDevices[i]->IsOn());
                ESP_LOGI(TAG, "  '%s': now %s", gDevices[i]->GetName(), gDevices[i]->IsOn() ? "ON" : "OFF");
            }
        }
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeMaxHandler(int argc, char ** argv)
{
    ESP_LOGI(TAG, "Bridge endpoint limits:");
    ESP_LOGI(TAG, "  Max bridged devices: %u", kMaxBridgedDevices);
    ESP_LOGI(TAG, "  Current devices: %u", gBridgedDeviceCount);
    ESP_LOGI(TAG, "  Available slots: %u", kMaxBridgedDevices - gBridgedDeviceCount);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR BridgeRemoveAllHandler(int argc, char ** argv)
{
    ESP_LOGI(TAG, "Removing all bridged devices...");
    size_t removedCount = 0;

    for (size_t i = 0; i < kMaxBridgedDevices; i++)
    {
        if (gDevices[i] != nullptr)
        {
            const char * name = gDevices[i]->GetName();
            CHIP_ERROR err    = RemoveDeviceEndpoint(gDevices[i]);
            if (err == CHIP_NO_ERROR)
            {
                ESP_LOGI(TAG, "  Removed '%s'", name);
                delete gDevices[i];
                delete[] gBridgedDataVersions[i];
                gDevices[i]      = nullptr;
                gBridgedDataVersions[i] = nullptr;
                removedCount++;
            }
            else
            {
                ESP_LOGE(TAG, "  Failed to remove '%s'", name);
            }
        }
    }
    gBridgedDeviceCount = gBridgedDeviceCount - removedCount;

    ESP_LOGI(TAG, "Removed %d devices total", removedCount);
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
        { &BridgeAddHandler, "add", "Add device: bridge add [name] [location]" },
        { &BridgeRemoveHandler, "remove", "Remove device: bridge remove <endpoint>" },
        { &BridgeRemoveAllHandler, "remove_all", "Remove all bridged devices" },
        { &BridgeMaxHandler, "max", "Show max endpoint limits" },
        { &BridgeListHandler, "list", "List all bridged devices" },
        { &BridgeToggleHandler, "toggle", "Toggle: bridge toggle [endpoint] or all" },
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

    // Bridge starts with no bridged devices - use shell commands to add them:
    //   bridge add [name] [location]
    //   bridge remove <endpoint>
    //   bridge list
    //   bridge max
    ESP_LOGI(TAG, "Bridge ready. Use 'bridge add [name] [location]' to add devices. Max devices: %u", kMaxBridgedDevices);
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
    memset(gDevices, 0, sizeof(gDevices));
    memset(gBridgedDataVersions, 0, sizeof(gBridgedDataVersions));
    gBridgedDeviceCount = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize the Wi-Fi stack");
        return;
    }
#endif

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
