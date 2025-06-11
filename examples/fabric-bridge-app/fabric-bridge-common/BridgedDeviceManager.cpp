/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "BridgedDeviceManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/util.h>
#include <crypto/RandUtils.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>

#include <cstdio>
#include <optional>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace bridge {

namespace {

constexpr uint8_t kMaxRetries      = 10;
constexpr int kNodeLabelSize       = 32;
constexpr int kUniqueIdSize        = 32;
constexpr int kVendorNameSize      = 32;
constexpr int kProductNameSize     = 32;
constexpr int kHardwareVersionSize = 64;
constexpr int kSoftwareVersionSize = 64;

// Current ZCL implementation of Struct uses a max-size array of 254 bytes
constexpr int kDescriptorAttributeArraySize = 254;

#define ZCL_ADMINISTRATOR_COMMISSIONING_CLUSTER_REVISION (1u)

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

// (taken from matter-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

// ---------------------------------------------------------------------------
//
// SYNCED DEVICE ENDPOINT: contains the following clusters:
//   - Descriptor
//   - Bridged Device Basic Information
//   - Administrator Commissioning

// clang-format off
// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
  DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* device list */
  DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* server list */
  DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* client list */
  DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize, 0),  /* parts list */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic Information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
  // The attributes below are MANDATORY in the Bridged Device Information Cluster
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::Reachable::Id, BOOLEAN, 1, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::UniqueID::Id, CHAR_STRING, kUniqueIdSize, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::FeatureMap::Id, BITMAP32, 4, 0),

  // The attributes below are OPTIONAL in the bridged device, however they are MANDATORY in the BasicInformation cluster
  // so they can always be provided if desired
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::VendorName::Id, CHAR_STRING, kVendorNameSize, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::VendorID::Id, INT16U, 2, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductName::Id, CHAR_STRING, kProductNameSize, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::ProductID::Id, INT16U, 2, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, CHAR_STRING, kNodeLabelSize, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::HardwareVersion::Id, INT16U, 2, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::HardwareVersionString::Id, CHAR_STRING,
                            kHardwareVersionSize, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::SoftwareVersion::Id, INT32U, 4, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::SoftwareVersionString::Id, CHAR_STRING,
                            kSoftwareVersionSize, 0),
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Ecosystem Information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(ecosystemInformationBasicAttrs)
    DECLARE_DYNAMIC_ATTRIBUTE(EcosystemInformation::Attributes::DeviceDirectory::Id, ARRAY, kDescriptorAttributeArraySize, 0),
    DECLARE_DYNAMIC_ATTRIBUTE(EcosystemInformation::Attributes::LocationDirectory::Id, ARRAY, kDescriptorAttributeArraySize, 0),
    DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Administrator Commissioning cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(AdministratorCommissioningAttrs)
  DECLARE_DYNAMIC_ATTRIBUTE(AdministratorCommissioning::Attributes::WindowStatus::Id, ENUM8, 1, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(AdministratorCommissioning::Attributes::AdminFabricIndex::Id, FABRIC_IDX, 1, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(AdministratorCommissioning::Attributes::AdminVendorId::Id, VENDOR_ID, 2, 0),
  DECLARE_DYNAMIC_ATTRIBUTE(AdministratorCommissioning::Attributes::ClusterRevision::Id, INT16U, ZCL_ADMINISTRATOR_COMMISSIONING_CLUSTER_REVISION, 0),
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();
// clang-format on

constexpr CommandId bridgedDeviceBasicInformationCommands[] = {
    app::Clusters::BridgedDeviceBasicInformation::Commands::KeepActive::Id,
    kInvalidCommandId,
};

constexpr CommandId administratorCommissioningCommands[] = {
    app::Clusters::AdministratorCommissioning::Commands::OpenCommissioningWindow::Id,
    app::Clusters::AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::Id,
    app::Clusters::AdministratorCommissioning::Commands::RevokeCommissioning::Id,
    kInvalidCommandId,
};

// clang-format off
// Declare Cluster List for Bridged Node endpoint
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedNodeClusters)
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(EcosystemInformation::Id, ecosystemInformationBasicAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(AdministratorCommissioning::Id, AdministratorCommissioningAttrs, ZAP_CLUSTER_MASK(SERVER),
                            administratorCommissioningCommands, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(icdBridgedNodeClusters)
    DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, ZAP_CLUSTER_MASK(SERVER), nullptr, nullptr),
    DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ZAP_CLUSTER_MASK(SERVER),
                            bridgedDeviceBasicInformationCommands, nullptr),
    DECLARE_DYNAMIC_CLUSTER(AdministratorCommissioning::Id, AdministratorCommissioningAttrs, ZAP_CLUSTER_MASK(SERVER),
                            administratorCommissioningCommands, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;
// clang-format on

// Declare Bridged Node endpoint
DECLARE_DYNAMIC_ENDPOINT(sBridgedNodeEndpoint, bridgedNodeClusters);
DECLARE_DYNAMIC_ENDPOINT(sIcdBridgedNodeEndpoint, icdBridgedNodeClusters);

// TODO: this is a single version array, however we may have many
//       different clusters that are independent.
DataVersion sBridgedNodeDataVersions[MATTER_ARRAY_SIZE(bridgedNodeClusters)];

const EmberAfDeviceType sBridgedDeviceTypes[] = { { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

} // namespace

void BridgedDeviceManager::Init()
{
    mFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    mCurrentEndpointId = mFirstDynamicEndpointId;
}

std::optional<uint16_t> BridgedDeviceManager::AddDeviceEndpoint(std::unique_ptr<BridgedDevice> dev,
                                                                chip::EndpointId parentEndpointId)
{
    EmberAfEndpointType * ep = dev->IsIcd() ? &sIcdBridgedNodeEndpoint : &sBridgedNodeEndpoint;

    const chip::Span<const EmberAfDeviceType> & deviceTypeList = Span<const EmberAfDeviceType>(sBridgedDeviceTypes);

    // TODO: this shares data version among different clusters, which seems incorrect
    const chip::Span<chip::DataVersion> & dataVersionStorage = Span<DataVersion>(sBridgedNodeDataVersions);

    assertChipStackLockedByCurrentThread();

    if (dev->GetBridgedAttributes().uniqueId.empty())
    {
        dev->SetUniqueId(GenerateUniqueId());
    }
    else if (GetDeviceByUniqueId(dev->GetBridgedAttributes().uniqueId) != nullptr)
    {
        ChipLogProgress(NotSpecified, "A device with unique id '%s' already exists", dev->GetBridgedAttributes().uniqueId.c_str());
        return std::nullopt;
    }

    for (uint16_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
    {
        if (mDevices[index])
        {
            continue;
        }

        for (int retryCount = 0; retryCount < kMaxRetries; retryCount++)
        {
            dev->SetEndpointId(mCurrentEndpointId);
            dev->SetParentEndpointId(parentEndpointId);
            CHIP_ERROR err =
                emberAfSetDynamicEndpoint(index, mCurrentEndpointId, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(NotSpecified, "Added device with Id=[%d:0x" ChipLogFormatX64 "] to dynamic endpoint %d (index=%d)",
                                dev->GetScopedNodeId().GetFabricIndex(), ChipLogValueX64(dev->GetScopedNodeId().GetNodeId()),
                                mCurrentEndpointId, index);
                mDevices[index] = std::move(dev);
                return index;
            }
            if (err != CHIP_ERROR_ENDPOINT_EXISTS)
            {
                return std::nullopt; // Return error as endpoint addition failed due to an error other than endpoint already exists
            }
            // Increment the endpoint ID and handle wrap condition
            if (++mCurrentEndpointId < mFirstDynamicEndpointId)
            {
                mCurrentEndpointId = mFirstDynamicEndpointId;
            }
        }
        ChipLogError(NotSpecified, "Failed to add dynamic endpoint after %d retries", kMaxRetries);
        return std::nullopt; // Return error as all retries are exhausted
    }

    ChipLogProgress(NotSpecified, "Failed to add dynamic endpoint: No endpoints available!");
    return std::nullopt;
}

int BridgedDeviceManager::RemoveDeviceEndpoint(BridgedDevice * dev)
{
    assertChipStackLockedByCurrentThread();

    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (mDevices[index].get() == dev)
        {
            // Silence complaints about unused ep when progress logging
            // disabled.
            [[maybe_unused]] EndpointId ep = emberAfClearDynamicEndpoint(index);
            mDevices[index]                = nullptr;
            ChipLogProgress(NotSpecified, "Removed device %s from dynamic endpoint %d (index=%d)",
                            dev->GetBridgedAttributes().uniqueId.c_str(), ep, index);
            return index;
        }
        index++;
    }
    return -1;
}

BridgedDevice * BridgedDeviceManager::GetDevice(chip::EndpointId endpointId) const
{
    assertChipStackLockedByCurrentThread();

    for (uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; ++index)
    {
        if (mDevices[index] && mDevices[index]->GetEndpointId() == endpointId)
        {
            return mDevices[index].get();
        }
    }
    return nullptr;
}

std::string BridgedDeviceManager::GenerateUniqueId()
{
    char rand_buffer[kUniqueIdSize + 1];
    memset(rand_buffer, 0, sizeof(rand_buffer));

    static const char kRandCharChoices[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (true)
    {
        /// for nice viewing, prefix the generated value
        memcpy(rand_buffer, "GEN-", 4);
        for (unsigned idx = 4; idx < kUniqueIdSize; idx++)
        {
            rand_buffer[idx] = kRandCharChoices[Crypto::GetRandU8() % (sizeof(kRandCharChoices) - 1)];
        }

        // we know zero-terminated due to the memset
        std::string uniqueIdChoice = rand_buffer;

        if (!GetDeviceByUniqueId(uniqueIdChoice))
        {
            return uniqueIdChoice;
        }
    }
}

BridgedDevice * BridgedDeviceManager::GetDeviceByUniqueId(const std::string & id)
{
    assertChipStackLockedByCurrentThread();

    for (uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; ++index)
    {
        if (mDevices[index] && mDevices[index]->GetBridgedAttributes().uniqueId == id)
        {
            return mDevices[index].get();
        }
    }
    return nullptr;
}

BridgedDevice * BridgedDeviceManager::GetDeviceByScopedNodeId(chip::ScopedNodeId scopedNodeId) const
{
    assertChipStackLockedByCurrentThread();

    for (uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; ++index)
    {
        if (mDevices[index] && mDevices[index]->GetScopedNodeId() == scopedNodeId)
        {
            return mDevices[index].get();
        }
    }
    return nullptr;
}

std::optional<uint16_t> BridgedDeviceManager::RemoveDeviceByScopedNodeId(chip::ScopedNodeId scopedNodeId)
{
    assertChipStackLockedByCurrentThread();

    for (uint16_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; ++index)
    {
        if (mDevices[index] && mDevices[index]->GetScopedNodeId() == scopedNodeId)
        {
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            mDevices[index] = nullptr;
            ChipLogProgress(NotSpecified, "Removed device with Id=[%d:0x" ChipLogFormatX64 "] from dynamic endpoint %d (index=%d)",
                            scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()), ep, index);
            return index;
        }
    }
    return std::nullopt;
}

} // namespace bridge
