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

#include "BridgedAdministratorCommissioning.h"
#include "BridgedDevice.h"
#include "BridgedDeviceBasicInformationImpl.h"
#include "BridgedDeviceManager.h"
#include "CommissionableInit.h"
#include "CommissionerControl.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>
#include <lib/support/CHIPArgParser.hpp>

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
#include "RpcClient.h"
#include "RpcServer.h"
#endif

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

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
constexpr uint16_t kRetryIntervalS = 3;
#endif

uint16_t gFabricAdminServerPort = 33001;
uint16_t gLocalServerPort       = 33002;

BridgedDeviceBasicInformationImpl gBridgedDeviceBasicInformationAttributes;

constexpr uint16_t kOptionFabricAdminServerPortNumber = 0xFF01;
constexpr uint16_t kOptionLocalServerPortNumber       = 0xFF02;

ArgParser::OptionDef sProgramCustomOptionDefs[] = {
    { "fabric-admin-server-port", ArgParser::kArgumentRequired, kOptionFabricAdminServerPortNumber },
    { "local-server-port", ArgParser::kArgumentRequired, kOptionLocalServerPortNumber },
    {},
};

const char sProgramCustomOptionHelp[] = "  --fabric-admin-server-port <port>\n"
                                        "       The fabric-admin RPC port number to connect to (default: 33001).\n"
                                        "  --local-server-port <port>\n"
                                        "       The port number for local RPC server (default: 33002).\n"
                                        "\n";

bool HandleCustomOption(const char * aProgram, ArgParser::OptionSet * aOptions, int aIdentifier, const char * aName,
                        const char * aValue)
{
    switch (aIdentifier)
    {
    case kOptionFabricAdminServerPortNumber:
        gFabricAdminServerPort = atoi(aValue);
        break;
    case kOptionLocalServerPortNumber:
        gLocalServerPort = atoi(aValue);
        break;
    default:
        ArgParser::PrintArgError("%s: INTERNAL ERROR: Unhandled option: %s\n", aProgram, aName);
        return false;
    }

    return true;
}

ArgParser::OptionSet sProgramCustomOptions = { HandleCustomOption, sProgramCustomOptionDefs, "GENERAL OPTIONS",
                                               sProgramCustomOptionHelp };

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
void AttemptRpcClientConnect(System::Layer * systemLayer, void * appState)
{
    if (StartRpcClient() == CHIP_NO_ERROR)
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
        OpenCommissioningWindow(Controller::CommissioningWindowVerifierParams()
                                    .SetNodeId(device->GetScopedNodeId().GetNodeId())
                                    .SetTimeout(commandData.commissioningTimeout)
                                    .SetDiscriminator(commandData.discriminator)
                                    .SetIteration(commandData.iterations)
                                    .SetSalt(commandData.salt)
                                    .SetVerifier(commandData.PAKEPasscodeVerifier),
                                device->GetScopedNodeId().GetFabricIndex()) == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Commissioning window is now open");
        status = Status::Success;
    }
    else
    {
        ChipLogProgress(NotSpecified, "Commissioning window failed to open");
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

    handlerContext.SetCommandHandled();

    BridgedDeviceBasicInformation::Commands::KeepActive::DecodableType commandData;
    if (DataModel::Decode(handlerContext.mPayload, commandData) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::InvalidCommand);
        return;
    }

    const uint32_t kMinTimeoutMs = 30 * 1000;
    const uint32_t kMaxTimeoutMs = 60 * 60 * 1000;
    if (commandData.timeoutMs < kMinTimeoutMs || commandData.timeoutMs > kMaxTimeoutMs)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::ConstraintError);
        return;
    }

    BridgedDevice * device = BridgeDeviceMgr().GetDevice(endpointId);
    if (device == nullptr || !device->IsIcd())
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::Failure);
        return;
    }

    Status status = Status::Failure;

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    if (KeepActive(device->GetScopedNodeId(), commandData.stayActiveDuration, commandData.timeoutMs) == CHIP_NO_ERROR)
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

BridgedAdministratorCommissioning gBridgedAdministratorCommissioning;
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
    SetRpcRemoteServerPort(gFabricAdminServerPort);
    InitRpcServer(gLocalServerPort);
    AttemptRpcClientConnect(&DeviceLayer::SystemLayer(), nullptr);
#endif

    BridgeDeviceMgr().Init();
    VerifyOrDie(gBridgedAdministratorCommissioning.Init() == CHIP_NO_ERROR);

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
    if (ChipLinuxAppInit(argc, argv, &sProgramCustomOptions) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
