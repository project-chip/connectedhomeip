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
#include <app/ConcreteAttributePath.h>
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

#include "ActionCluster.h"
#include "Backend.h"
#include "CommissionableInit.h"
#include "Device.h"
#include "main.h"
#include <app/server/Server.h>

#include "AppMain.h"

#include "bridge/BridgeClustersImpl.h"

#ifdef PW_RPC_ENABLED
#include "Rpc.h"
#include "bridge_service.h"
#include "pw_rpc_system_server/rpc_server.h"
static chip::rpc::Bridge bridge_service;
#endif

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

static EndpointId gCurrentEndpointId;
static EndpointId gFirstDynamicEndpointId;
static Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT];
Room gRooms[kMaxRooms];

struct CommonAttributeAccessInterface : public chip::app::AttributeAccessInterface
{
    using chip::app::AttributeAccessInterface::AttributeAccessInterface;

    // Find a cluster given a specific endpoint/cluster. Returns nullptr if no such
    // cluster exists at that path.
    static CommonCluster * FindCluster(const chip::app::ConcreteClusterPath & path);

    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override;

    void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override;
    void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override;
};

CommonCluster * CommonAttributeAccessInterface::FindCluster(const chip::app::ConcreteClusterPath & path)
{
    Device * dev = FindDeviceEndpoint(path.mEndpointId);
    if (dev)
    {
        for (auto c : dev->clusters())
        {
            if (c->GetClusterId() == path.mClusterId)
                return static_cast<CommonCluster *>(c);
        }
    }
    return nullptr;
}

CHIP_ERROR CommonAttributeAccessInterface::Read(const chip::app::ConcreteReadAttributePath & aPath,
                                                chip::app::AttributeValueEncoder & aEncoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
    if (!a)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    return a->Read(aPath, aEncoder);
}

CHIP_ERROR CommonAttributeAccessInterface::Write(const chip::app::ConcreteDataAttributePath & aPath,
                                                 chip::app::AttributeValueDecoder & aDecoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    return c->ForwardWriteToBridge(aPath, aDecoder);
}

void CommonAttributeAccessInterface::OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
            a->ListWriteBegin(aPath);
    }
}

void CommonAttributeAccessInterface::OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
            a->ListWriteEnd(aPath, aWriteWasSuccessful);
    }
}

chip::Optional<chip::ClusterId> LookupClusterByName(const char * name)
{
    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (!strcmp(name, cluster.name))
        {
            return chip::Optional<chip::ClusterId>(cluster.id);
        }
    }
    return chip::Optional<chip::ClusterId>();
}

std::unique_ptr<GeneratedCluster> CreateCluster(const char * name)
{
    auto id = LookupClusterByName(name);
    return id.HasValue() ? CreateCluster(id.Value()) : nullptr;
}

std::unique_ptr<GeneratedCluster> CreateCluster(chip::ClusterId id)
{
    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (id == cluster.id)
        {
            return std::unique_ptr<GeneratedCluster>(cluster.ctor(::operator new(cluster.size)));
        }
    }
    return nullptr;
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Actions::Commands::InstantAction::DecodableType & commandData)
{
    // No actions are implemented, just return status NotFound.
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

Device * FindDeviceEndpoint(chip::EndpointId id)
{
    for (auto dev : gDevices)
    {
        if (dev && dev->GetEndpointId() == id)
            return dev;
    }
    return nullptr;
}

int AddDeviceEndpoint(Device * dev)
{
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            while (true)
            {
                // Todo: Update this to schedule the work rather than use this lock
                dev->SetEndpointId(gCurrentEndpointId);
                ret =
                    emberAfSetDynamicEndpoint(index, gCurrentEndpointId, dev->endpointType(), dev->versions(), dev->deviceTypes());
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: %d!", ret);
                    gDevices[index] = nullptr;
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
            // Todo: Update this to schedule the work rather than use this lock
            DeviceLayer::StackLock lock;
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

Room * FindRoom(const std::string & name)
{
    for (auto & room : gRooms)
    {
        if (room.GetName() == name)
            return &room;
    }
    return nullptr;
}

chip::Span<Action *> GetActionListInfo(chip::EndpointId parentId)
{
    return chip::Span<Action *>();
}

void ApplicationInit()
{
#ifdef PW_RPC_ENABLED
    chip::rpc::Init();

    pw::rpc::system_server::Server().RegisterService(bridge_service);
#endif

    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;
    StartUserInput();
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    std::vector<CommonAttributeAccessInterface> clusterAccess;
    clusterAccess.reserve(std::extent<decltype(clusters::kKnownClusters)>::value);
    for (auto & entry : clusters::kKnownClusters)
    {
        // Desciptor clusters should not be overridden.
        if (entry.id != chip::app::Clusters::Descriptor::Id)
        {
            clusterAccess.emplace_back(chip::Optional<EndpointId>(), entry.id);
            registerAttributeAccessOverride(&clusterAccess.back());
        }
    }

    ChipLinuxAppMainLoop();
    return 0;
}
