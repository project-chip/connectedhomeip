/*
 *
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

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

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
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "CommissionableInit.h"
#include "Device.h"
<<<<<<< HEAD
#include "DeviceManager.h"
=======
>>>>>>> 9631848953 (Add bridge_enable_pw_rpc build flag)
#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE)
#include "RpcClient.h"
#include "RpcServer.h"
#endif

#include <cassert>
#include <chrono>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <thread>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace {

constexpr uint16_t kPollIntervalMs = 100;
constexpr uint16_t kRetryIntervalS = 3;

EndpointId gCurrentEndpointId;
EndpointId gFirstDynamicEndpointId;
Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];

bool KeyboardHit()
{
    int bytesWaiting;
    ioctl(0, FIONREAD, &bytesWaiting);
    return bytesWaiting > 0;
}

void BridgePollingThread()
{
    while (true)
    {
        if (KeyboardHit())
        {
            int ch = getchar();
            if (ch == 'e')
            {
                ChipLogProgress(NotSpecified, "Exiting.....");
                exit(0);
            }
#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE)
            else if (ch == 'o')
            {
                if (OpenCommissioningWindow(0x1234) != CHIP_NO_ERROR)
                {
                    ChipLogError(NotSpecified, "Failed to call OpenCommissioningWindow RPC");
                }
            }
#endif
            continue;
        }

        // Sleep to avoid tight loop reading commands
        usleep(kPollIntervalMs * 1000);
    }
}

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE)
void AttemptRpcClientConnect(System::Layer * systemLayer, void * appState)
{
    if (InitRpcClient(kFabricAdminServerPort) == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Connected to Fabric-Admin");
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to connect to Fabric-Admin, retry in %d seconds....", kRetryIntervalS);
        systemLayer->StartTimer(System::Clock::Seconds16(kRetryIntervalS), AttemptRpcClientConnect, nullptr);
    }
}
#endif

DeviceManager gDeviceManager;

} // namespace

// REVISION DEFINITIONS:
// =================================================================================

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (2u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP (0u)

void ApplicationInit()
{
    // Clear out the device database
    memset(gDevices, 0, sizeof(gDevices));

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE)
    InitRpcServer(kFabricBridgeServerPort);

    AttemptRpcClientConnect(&DeviceLayer::SystemLayer(), nullptr);
#endif

    // Start a thread for bridge polling
    std::thread pollingThread(BridgePollingThread);
    pollingThread.detach();
}

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}

// External attribute read callback function
Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                                         const EmberAfAttributeMetadata * attributeMetadata,
                                                                         uint8_t * buffer, uint16_t maxReadLength)
{
    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    AttributeId attributeId = attributeMetadata->attributeId;

    Device * dev = gDeviceManager.GetDevice(endpointIndex);
    if (dev != nullptr && clusterId == app::Clusters::BridgedDeviceBasicInformation::Id)
    {
        using namespace app::Clusters::BridgedDeviceBasicInformation::Attributes;
        ChipLogProgress(NotSpecified, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId,
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
        else if ((attributeId == FeatureMap::Id) && (maxReadLength == 4))
        {
            uint32_t featureMap = ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP;
            memcpy(buffer, &featureMap, sizeof(featureMap));
        }
        else
        {
            return Protocols::InteractionModel::Status::Failure;
        }
        return Protocols::InteractionModel::Status::Success;
    }

    return Protocols::InteractionModel::Status::Failure;
}

// External attribute write callback function
Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                                          const EmberAfAttributeMetadata * attributeMetadata,
                                                                          uint8_t * buffer)
{
    uint16_t endpointIndex                  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    Protocols::InteractionModel::Status ret = Protocols::InteractionModel::Status::Failure;

    Device * dev = gDeviceManager.GetDevice(endpointIndex);
    if (dev != nullptr && dev->IsReachable())
    {
        ChipLogProgress(NotSpecified, "emberAfExternalAttributeWriteCallback: ep=%d, clusterId=%d", endpoint, clusterId);
        ret = Protocols::InteractionModel::Status::Success;
    }

    return ret;
}
