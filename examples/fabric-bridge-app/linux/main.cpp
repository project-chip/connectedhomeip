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

#include "CommissionableInit.h"
#include "Device.h"
#include "DeviceManager.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <lib/support/ZclString.h>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "RpcClient.h"
#include "RpcServer.h"
#endif

#include <string>
#include <sys/ioctl.h>
#include <thread>

using namespace chip;

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (2u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP (0u)

namespace {

constexpr uint16_t kPollIntervalMs = 100;
constexpr uint16_t kRetryIntervalS = 3;

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
#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
            else if (ch == 'o')
            {
                CHIP_ERROR err = OpenCommissioningWindow(0x1234);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(NotSpecified, "Failed to call OpenCommissioningWindow RPC: %" CHIP_ERROR_FORMAT, err.Format());
                }
            }
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
            continue;
        }

        // Sleep to avoid tight loop reading commands
        usleep(kPollIntervalMs * 1000);
    }
}

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
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
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE

} // namespace

void ApplicationInit()
{
#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
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

    Device * dev = GetDeviceManager()->GetDevice(endpointIndex);
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

    Device * dev = GetDeviceManager()->GetDevice(endpointIndex);
    if (dev != nullptr && dev->IsReachable())
    {
        ChipLogProgress(NotSpecified, "emberAfExternalAttributeWriteCallback: ep=%d, clusterId=%d", endpoint, clusterId);
        ret = Protocols::InteractionModel::Status::Success;
    }

    return ret;
}
