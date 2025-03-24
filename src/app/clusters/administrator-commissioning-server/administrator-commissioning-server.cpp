/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Administrator Commissioning Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceControlServer.h>
#include <protocols/interaction_model/Constants.h>
#include <setup_payload/SetupPayload.h>
#include <system/SystemClock.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;
using namespace chip::Protocols;
using namespace chip::Crypto;
using chip::Protocols::InteractionModel::Status;

class AdministratorCommissioningServer : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the AdministratorCommissioning cluster on all endpoints.
    AdministratorCommissioningServer() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::AdministratorCommissioning::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), Clusters::AdministratorCommissioning::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    void InvokeCommand(HandlerContext & context) override;

    // Methods to handle the various commands this cluster may receive.
    void OpenCommissioningWindow(HandlerContext & context, const Commands::OpenCommissioningWindow::DecodableType & commandData);
#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
    void OpenBasicCommissioningWindow(HandlerContext & context,
                                      const Commands::OpenBasicCommissioningWindow::DecodableType & commandData);
#endif
    void RevokeCommissioning(HandlerContext & context, const Commands::RevokeCommissioning::DecodableType & commandData);
};

AdministratorCommissioningServer gAdminCommissioningServer;

CHIP_ERROR AdministratorCommissioningServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::AdministratorCommissioning::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::WindowStatus::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().CommissioningWindowStatusForCluster());
    }
    case Attributes::AdminFabricIndex::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().GetOpenerFabricIndex());
    }
    case Attributes::AdminVendorId::Id: {
        return aEncoder.Encode(Server::GetInstance().GetCommissioningWindowManager().GetOpenerVendorId());
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

void AdministratorCommissioningServer::InvokeCommand(HandlerContext & context)
{
    switch (context.mRequestPath.mCommandId)
    {
    case Commands::OpenCommissioningWindow::Id:
        HandleCommand<Commands::OpenCommissioningWindow::DecodableType>(
            context, [this](HandlerContext & ctx, const auto & commandData) { OpenCommissioningWindow(ctx, commandData); });
        break;
#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
    case Commands::OpenBasicCommissioningWindow::Id:
        HandleCommand<Commands::OpenBasicCommissioningWindow::DecodableType>(
            context, [this](HandlerContext & ctx, const auto & commandData) { OpenBasicCommissioningWindow(ctx, commandData); });
        break;
#endif
    case Commands::RevokeCommissioning::Id:
        HandleCommand<Commands::RevokeCommissioning::DecodableType>(
            context, [this](HandlerContext & ctx, const auto & commandData) { RevokeCommissioning(ctx, commandData); });
        break;
    }
}

void AdministratorCommissioningServer::OpenCommissioningWindow(HandlerContext & context,
                                                               const Commands::OpenCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);
    auto & pakeVerifier       = commandData.PAKEPasscodeVerifier;
    auto & discriminator      = commandData.discriminator;
    auto & iterations         = commandData.iterations;
    auto & salt               = commandData.salt;

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    Status globalStatus         = Status::Success;
    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open commissioning window");

    FabricIndex fabricIndex       = context.mCommandHandler.GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::kBusy));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = Status::InvalidCommand);

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR, status.Emplace(StatusCode::kPAKEParameterError));
    VerifyOrExit(commissionMgr.OpenEnhancedCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                               fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        context.mCommandHandler.AddClusterSpecificFailure(context.mRequestPath, to_underlying(status.Value()));
    }
    else
    {
        if (globalStatus != Status::Success)
        {
            ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(globalStatus));
        }
        context.mCommandHandler.AddStatus(context.mRequestPath, globalStatus);
    }
}

#ifdef ADMINISTRATOR_COMMISSIONING_ENABLE_OPEN_BASIC_COMMISSIONING_WINDOW_CMD
void AdministratorCommissioningServer::OpenBasicCommissioningWindow(
    CommandHandlerInterface::HandlerContext & context, const Commands::OpenBasicCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenBasicCommissioningWindow", "AdministratorCommissioning");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);

    Optional<StatusCode> status = Optional<StatusCode>::Missing();
    Status globalStatus         = Status::Success;
    ChipLogProgress(Zcl, "Received command to open basic commissioning window");

    FabricIndex fabricIndex       = context.mCommandHandler.GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCode::kPAKEParameterError));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCode::kBusy));
    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissionMgr.OpenBasicCommissioningWindowForAdministratorCommissioningCluster(
                     commissioningTimeout, fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCode::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        context.mCommandHandler.AddClusterSpecificFailure(context.mRequestPath, to_underlying(status.Value()));
    }
    else
    {
        if (globalStatus != Status::Success)
        {
            ChipLogError(Zcl, "Failed to open commissioning window. Global status " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(globalStatus));
        }
        context.mCommandHandler.AddStatus(context.mRequestPath, globalStatus);
    }
}
#endif

void AdministratorCommissioningServer::RevokeCommissioning(CommandHandlerInterface::HandlerContext & context,
                                                           const Commands::RevokeCommissioning::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RevokeCommissioning", "AdministratorCommissioning");
    ChipLogProgress(Zcl, "Received command to close commissioning window");

    Server::GetInstance().GetFailSafeContext().ForceFailSafeTimerExpiry();

    if (!Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogError(Zcl, "Commissioning window is currently not open");
        context.mCommandHandler.AddClusterSpecificFailure(context.mRequestPath, to_underlying(StatusCode::kWindowNotOpen));
    }
    else
    {
        Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
        ChipLogProgress(Zcl, "Commissioning window is now closed");
        context.mCommandHandler.AddStatus(context.mRequestPath, Status::Success);
    }
}

void MatterAdministratorCommissioningPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initiating Admin Commissioning cluster.");
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gAdminCommissioningServer);
    AttributeAccessInterfaceRegistry::Instance().Register(&gAdminCommissioningServer);
}
