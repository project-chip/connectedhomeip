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

#include "Bridge.h"

#include "BridgedAdministratorCommissioning.h"
#include "BridgedDevice.h"
#include "BridgedDeviceBasicInformationImpl.h"
#include "BridgedDeviceManager.h"
#include "FabricBridge.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;
using namespace chip::app::Clusters::BridgedDeviceBasicInformation;

// This is declared here and not in a header because zap/embr assumes all clusters
// are defined in a static endpoint in the .zap file. From there, the codegen will
// automatically use PluginApplicationCallbacksHeader.jinja to declare and call
// the respective Init callbacks. However, because EcosystemInformation cluster is only
// ever on a dynamic endpoint, this doesn't get declared and called for us, so we
// need to declare and call it ourselves where the application is initialized.
void MatterEcosystemInformationPluginServerInitCallback();

namespace bridge {

namespace {

class AdministratorCommissioningCommandHandler : public CommandHandlerInterface
{
public:
    // Register for the AdministratorCommissioning cluster on all endpoints.
    AdministratorCommissioningCommandHandler() :
        CommandHandlerInterface(Optional<EndpointId>::Missing(), AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Init();

    void InvokeCommand(HandlerContext & handlerContext) override;

private:
    CommandHandlerInterface * mOriginalCommandHandlerInterface = nullptr;
};

CHIP_ERROR AdministratorCommissioningCommandHandler::Init()
{
    mOriginalCommandHandlerInterface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(kRootEndpointId, AdministratorCommissioning::Id);
    VerifyOrReturnError(mOriginalCommandHandlerInterface, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(mOriginalCommandHandlerInterface));
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

void AdministratorCommissioningCommandHandler::InvokeCommand(HandlerContext & handlerContext)
{
    using Protocols::InteractionModel::Status;

    EndpointId endpointId = handlerContext.mRequestPath.mEndpointId;

    if (handlerContext.mRequestPath.mCommandId != AdministratorCommissioning::Commands::OpenCommissioningWindow::Id ||
        endpointId == kRootEndpointId)
    {
        // Proceed with default handling in Administrator Commissioning Server
        mOriginalCommandHandlerInterface->InvokeCommand(handlerContext);
        return;
    }

    ChipLogProgress(NotSpecified, "Received command to open commissioning window on Endpoint: %d", endpointId);

    handlerContext.SetCommandHandled();

    AdministratorCommissioning::Commands::OpenCommissioningWindow::DecodableType commandData;
    if (DataModel::Decode(handlerContext.mPayload, commandData) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status status                       = Status::Failure;
    BridgedDevice * device              = BridgedDeviceManager::Instance().GetDevice(endpointId);
    FabricAdminDelegate * adminDelegate = FabricBridge::Instance().GetDelegate();

    if (!device)
    {
        ChipLogError(NotSpecified, "Commissioning window failed to open: device is null");
        return;
    }

    if (!adminDelegate)
    {
        ChipLogError(NotSpecified, "Commissioning window failed to open: adminDelegate is null");
        return;
    }

    auto nodeId      = device->GetScopedNodeId().GetNodeId();
    auto fabricIndex = device->GetScopedNodeId().GetFabricIndex();

    Controller::CommissioningWindowVerifierParams params;
    params.SetNodeId(nodeId)
        .SetTimeout(commandData.commissioningTimeout)
        .SetDiscriminator(commandData.discriminator)
        .SetIteration(commandData.iterations)
        .SetSalt(commandData.salt)
        .SetVerifier(commandData.PAKEPasscodeVerifier);

    CHIP_ERROR err = adminDelegate->OpenCommissioningWindow(params, fabricIndex);

    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Commissioning window is now open");
        status = Status::Success;
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to open commissioning window. Error: %" CHIP_ERROR_FORMAT, err.Format());
    }

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

    BridgedDevice * device = BridgedDeviceManager::Instance().GetDevice(endpointId);
    if (device == nullptr || !device->IsIcd())
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::Failure);
        return;
    }

    Status status                       = Status::Failure;
    FabricAdminDelegate * adminDelegate = FabricBridge::Instance().GetDelegate();

    if (adminDelegate)
    {
        CHIP_ERROR err =
            adminDelegate->KeepActive(device->GetScopedNodeId(), commandData.stayActiveDuration, commandData.timeoutMs);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(NotSpecified, "KeepActive successfully processed");
            status = Status::Success;
        }
        else
        {
            ChipLogProgress(NotSpecified, "KeepActive failed to process: %s", ErrorStr(err));
        }
    }
    else
    {
        ChipLogProgress(NotSpecified, "Operation failed: adminDelegate is null");
    }

    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, status);
}

BridgedAdministratorCommissioning gBridgedAdministratorCommissioning;
BridgedDeviceBasicInformationImpl gBridgedDeviceBasicInformationAttributes;
AdministratorCommissioningCommandHandler gAdministratorCommissioningCommandHandler;
BridgedDeviceInformationCommandHandler gBridgedDeviceInformationCommandHandler;

} // namespace

CHIP_ERROR BridgeInit(FabricAdminDelegate * delegate)
{
    MatterEcosystemInformationPluginServerInitCallback();
    ReturnErrorOnFailure(gAdministratorCommissioningCommandHandler.Init());
    ReturnErrorOnFailure(
        CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gBridgedDeviceInformationCommandHandler));
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(&gBridgedDeviceBasicInformationAttributes),
                        CHIP_ERROR_INTERNAL);

    BridgedDeviceManager::Instance().Init();
    FabricBridge::Instance().SetDelegate(delegate);
    ReturnErrorOnFailure(gBridgedAdministratorCommissioning.Init());
    ReturnErrorOnFailure(CommissionerControlInit(delegate));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BridgeShutdown()
{
    CHIP_ERROR err = CommissionerControlShutdown();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to shutdown Commissioner Control Server");
    }

    return err;
}

} // namespace bridge
