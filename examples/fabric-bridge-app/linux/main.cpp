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
#include <app/CommandHandlerInterfaceRegistry.h>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "RpcClient.h"
#include "RpcServer.h"
#endif

#include <string>
#include <sys/ioctl.h>
#include <thread>

using namespace chip;

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

namespace {

constexpr uint16_t kPollIntervalMs = 100;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
constexpr uint16_t kRetryIntervalS = 3;
#endif

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
                CHIP_ERROR err = OpenCommissioningWindow(chip::Controller::CommissioningWindowPasscodeParams()
                                                             .SetNodeId(0x1234)
                                                             .SetTimeout(300)
                                                             .SetDiscriminator(3840)
                                                             .SetIteration(1000));
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

class AdministratorCommissioningCommandHandler : public CommandHandlerInterface
{
public:
    // Register for the AdministratorCommissioning cluster on all endpoints.
    AdministratorCommissioningCommandHandler() :
        CommandHandlerInterface(Optional<EndpointId>::Missing(), AdministratorCommissioning::Id)
    {}

    void InvokeCommand(HandlerContext & handlerContext) override;
};

void AdministratorCommissioningCommandHandler::InvokeCommand(HandlerContext & handlerContext)
{
    using Protocols::InteractionModel::Status;

    EndpointId endpointId = handlerContext.mRequestPath.mEndpointId;
    ChipLogProgress(NotSpecified, "Received command to open commissioning window on Endpoint: %d", endpointId);

    if (handlerContext.mRequestPath.mCommandId != Commands::OpenCommissioningWindow::Id || endpointId == kRootEndpointId)
    {
        // Proceed with default handling in Administrator Commissioning Server
        return;
    }

    handlerContext.SetCommandHandled();

    Commands::OpenCommissioningWindow::DecodableType commandData;
    if (DataModel::Decode(handlerContext.mPayload, commandData) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status status = Status::Failure;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    Device * device = DeviceMgr().GetDevice(endpointId);

    // TODO: issues:#33784, need to make OpenCommissioningWindow synchronous
    if (device != nullptr &&
        OpenCommissioningWindow(chip::Controller::CommissioningWindowVerifierParams()
                                    .SetNodeId(device->GetNodeId())
                                    .SetTimeout(commandData.commissioningTimeout)
                                    .SetDiscriminator(commandData.discriminator)
                                    .SetIteration(commandData.iterations)
                                    .SetSalt(commandData.salt)
                                    .SetVerifier(commandData.PAKEPasscodeVerifier)) == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Commissioning window is now open");
        status = Status::Success;
    }
    else
    {
        ChipLogProgress(NotSpecified, "Commissioning window is failed to open");
    }
#else
    ChipLogProgress(NotSpecified, "Commissioning window failed to open: PW_RPC_FABRIC_BRIDGE_SERVICE not defined");
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE

    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
}

AdministratorCommissioningCommandHandler gAdministratorCommissioningCommandHandler;

} // namespace

void ApplicationInit()
{
    ChipLogDetail(NotSpecified, "Fabric-Bridge: ApplicationInit()");

    CommandHandlerInterfaceRegistry::RegisterCommandHandler(&gAdministratorCommissioningCommandHandler);

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    InitRpcServer(kFabricBridgeServerPort);
    AttemptRpcClientConnect(&DeviceLayer::SystemLayer(), nullptr);
#endif

    // Start a thread for bridge polling
    std::thread pollingThread(BridgePollingThread);
    pollingThread.detach();

    DeviceMgr().Init();
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Fabric-Bridge: ApplicationShutdown()");
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
