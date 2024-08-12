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

#include "BridgedDevice.h"
#include "BridgedDeviceBasicInformationImpl.h"
#include "BridgedDeviceManager.h"
#include "CommissionableInit.h"
#include "CommissionerControl.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "RpcClient.h"
#include "RpcServer.h"
#endif

#include <sys/ioctl.h>
#include <thread>

// This is declared here and not in a header because zap/embr assumes all clusters
// are defined in a static endpoint in the .zap file. From there, the codegen will
// automatically use PluginApplicationCallbacksHeader.jinja to declare and call
// the respective Init callbacks. However, because EcosystemInformation cluster is only
// ever on a dynamic endpoint, this doesn't get declared and called for us, so we
// need to declare and call it ourselves where the application is initialized.
void MatterEcosystemInformationPluginServerInitCallback();

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;
using namespace chip::app::Clusters::BridgedDeviceBasicInformation;

namespace {

constexpr uint16_t kPollIntervalMs = 100;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
constexpr uint16_t kRetryIntervalS = 3;
#endif

BridgedDeviceBasicInformationImpl gBridgedDeviceBasicInformationAttributes;

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

    if (handlerContext.mRequestPath.mCommandId != AdministratorCommissioning::Commands::OpenCommissioningWindow::Id ||
        endpointId == kRootEndpointId)
    {
        // Proceed with default handling in Administrator Commissioning Server
        return;
    }

    handlerContext.SetCommandHandled();

    AdministratorCommissioning::Commands::OpenCommissioningWindow::DecodableType commandData;
    if (DataModel::Decode(handlerContext.mPayload, commandData) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status status = Status::Failure;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    BridgedDevice * device = BridgeDeviceMgr().GetDevice(endpointId);

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

class BridgedDeviceInformationCommandHandler : public CommandHandlerInterface
{
public:
    // Register for the BridgedDeviceBasicInformation cluster on all endpoints.
    BridgedDeviceInformationCommandHandler() :
        CommandHandlerInterface(Optional<EndpointId>::Missing(), BridgedDeviceBasicInformation::Id)
    {}

    void InvokeCommand(HandlerContext & handlerContext) override;
};

void BridgedDeviceInformationCommandHandler::InvokeCommand(HandlerContext & handlerContext)
{
    using Protocols::InteractionModel::Status;
    VerifyOrReturn(handlerContext.mRequestPath.mCommandId == BridgedDeviceBasicInformation::Commands::KeepActive::Id);

    EndpointId endpointId = handlerContext.mRequestPath.mEndpointId;
    ChipLogProgress(NotSpecified, "Received command to KeepActive on Endpoint: %d", endpointId);

    BridgedDevice * device = BridgeDeviceMgr().GetDevice(endpointId);

    handlerContext.SetCommandHandled();

    if (device == nullptr || !device->IsIcd())
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::Failure);
        return;
    }

    BridgedDeviceBasicInformation::Commands::KeepActive::DecodableType commandData;
    if (DataModel::Decode(handlerContext.mPayload, commandData) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status status = Status::Failure;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    if (KeepActive(device->GetNodeId(), commandData.stayActiveDuration) == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "KeepActive successfully processed");
        status = Status::Success;
    }
    else
    {
        ChipLogProgress(NotSpecified, "KeepActive failed to process");
    }
#else
    ChipLogProgress(NotSpecified, "Unable to properly call KeepActive: PW_RPC_FABRIC_BRIDGE_SERVICE not defined");
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE

    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
}

AdministratorCommissioningCommandHandler gAdministratorCommissioningCommandHandler;
BridgedDeviceInformationCommandHandler gBridgedDeviceInformationCommandHandler;

} // namespace

void ApplicationInit()
{
    ChipLogDetail(NotSpecified, "Fabric-Bridge: ApplicationInit()");

    MatterEcosystemInformationPluginServerInitCallback();
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gAdministratorCommissioningCommandHandler);
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gBridgedDeviceInformationCommandHandler);
    AttributeAccessInterfaceRegistry::Instance().Register(&gBridgedDeviceBasicInformationAttributes);

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    InitRpcServer(kFabricBridgeServerPort);
    AttemptRpcClientConnect(&DeviceLayer::SystemLayer(), nullptr);
#endif

    // Start a thread for bridge polling
    std::thread pollingThread(BridgePollingThread);
    pollingThread.detach();

    BridgeDeviceMgr().Init();

    VerifyOrDieWithMsg(CommissionerControlInit() == CHIP_NO_ERROR, NotSpecified,
                       "Failed to initialize Commissioner Control Server");
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Fabric-Bridge: ApplicationShutdown()");

    if (CommissionerControlShutdown() != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to shutdown Commissioner Control Server");
    }
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
