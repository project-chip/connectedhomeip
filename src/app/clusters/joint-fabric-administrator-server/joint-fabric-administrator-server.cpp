/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 * @brief Implementation for the Joint Fabric Administrator Cluster
 ***************************************************************************/

#include <access/AccessControl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/joint-fabric-administrator-server/JCMCommissionee.h>
#include <app/reporting/reporting.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>

#include <lib/support/logging/CHIPLogging.h>
#include <optional>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <string.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::JointFabricAdministrator;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::Protocols::InteractionModel;

namespace JointFabricAdministratorCluster = chip::app::Clusters::JointFabricAdministrator;

class JointFabricAdministratorGlobalInstance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    JointFabricAdministratorGlobalInstance() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), JointFabricAdministratorCluster::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), JointFabricAdministratorCluster::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadAdministratorFabricIndex(AttributeValueEncoder & aEncoder);

    void InvokeCommand(HandlerContext & ctx) override;

    void HandleOJCW(HandlerContext & ctx, const Commands::OpenJointCommissioningWindow::DecodableType & commandData);
    void HandleAnnounceJointFabricAdministrator(HandlerContext & ctx,
                                                const Commands::AnnounceJointFabricAdministrator::DecodableType & commandData);
    void HandleICACCSRRequest(HandlerContext & ctx, const Commands::ICACCSRRequest::DecodableType & commandData);
    void HandleAddICAC(HandlerContext & ctx, const Commands::AddICAC::DecodableType & commandData);
    void HandleTransferAnchorRequest(HandlerContext & ctx, const Commands::TransferAnchorRequest::DecodableType & commandData);
    void HandleTransferAnchorComplete(HandlerContext & ctx, const Commands::TransferAnchorComplete::DecodableType & commandData);

    static void OnTrustVerificationCompletion(CHIP_ERROR err);

    // Cleans up asynchronous resources used in HandleAnnounceJointFabricAdministrator
    void CleanupAnnounceJFA();

    std::optional<CommandHandler::Handle> mActiveCommandHandle;
    std::optional<JCMCommissionee> mActiveCommissionee;
};

JointFabricAdministratorGlobalInstance gJointFabricAdministratorGlobalInstance;

CHIP_ERROR JointFabricAdministratorGlobalInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == JointFabricAdministratorCluster::Id);

    switch (aPath.mAttributeId)
    {
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR JointFabricAdministratorGlobalInstance::ReadAdministratorFabricIndex(AttributeValueEncoder & aEncoder)
{
    return CHIP_NO_ERROR;
}

void JointFabricAdministratorGlobalInstance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::OpenJointCommissioningWindow::Id:
        CommandHandlerInterface::HandleCommand<Commands::OpenJointCommissioningWindow::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleOJCW(ctx, commandData); });
        break;
    case Commands::AnnounceJointFabricAdministrator::Id:
        CommandHandlerInterface::HandleCommand<Commands::AnnounceJointFabricAdministrator::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleAnnounceJointFabricAdministrator(ctx, commandData); });
        break;
    case Commands::ICACCSRRequest::Id:
        CommandHandlerInterface::HandleCommand<Commands::ICACCSRRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleICACCSRRequest(ctx, commandData); });
        break;
    case Commands::AddICAC::Id:
        CommandHandlerInterface::HandleCommand<Commands::AddICAC::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleAddICAC(ctx, commandData); });
        break;
    case Commands::TransferAnchorRequest::Id:
        CommandHandlerInterface::HandleCommand<Commands::TransferAnchorRequest::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleTransferAnchorRequest(ctx, commandData); });
        break;
    case Commands::TransferAnchorComplete::Id:
        CommandHandlerInterface::HandleCommand<Commands::TransferAnchorComplete::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleTransferAnchorComplete(ctx, commandData); });
        break;
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
void JointFabricAdministratorGlobalInstance::HandleOJCW(HandlerContext & ctx,
                                                        const Commands::OpenJointCommissioningWindow::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("OpenJointCommissioningWindow", "JointFabricAdministrator");
    auto commissioningTimeout = System::Clock::Seconds16(commandData.commissioningTimeout);
    auto & pakeVerifier       = commandData.PAKEPasscodeVerifier;
    auto & discriminator      = commandData.discriminator;
    auto & iterations         = commandData.iterations;
    auto & salt               = commandData.salt;

    using namespace chip::app::Clusters::JointFabricAdministrator;

    Optional<StatusCodeEnum> status = Optional<StatusCodeEnum>::Missing();
    Status globalStatus             = Status::Success;
    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open joint commissioning window");

    FabricIndex fabricIndex       = ctx.mCommandHandler.GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(StatusCodeEnum::kBusy));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(StatusCodeEnum::kBusy));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations, status.Emplace(StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations, status.Emplace(StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length, status.Emplace(StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length, status.Emplace(StatusCodeEnum::kPAKEParameterError));

    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = Status::InvalidCommand);

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR, status.Emplace(StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(commissionMgr.OpenJointCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                            fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(StatusCodeEnum::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open joint commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        TEMPORARY_RETURN_IGNORED ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, to_underlying(status.Value()));
    }
    else
    {
        if (globalStatus != Status::Success)
        {
            ChipLogError(Zcl, "Failed to open joint commissioning window. Global status " ChipLogFormatIMStatus,
                         ChipLogValueIMStatus(globalStatus));
        }
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, globalStatus);
    }
}

void JointFabricAdministratorGlobalInstance::HandleAnnounceJointFabricAdministrator(
    HandlerContext & ctx, const Commands::AnnounceJointFabricAdministrator::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AnnounceJointFabricAdministrator", "JointFabricAdministrator");
    ChipLogProgress(JointFabric, "Received an AnnounceJointFabricAdministrator command with endpointID=%u", commandData.endpointID);

    std::optional<Status> globalStatus = std::nullopt;
    ConcreteCommandPath cachedPath(ctx.mRequestPath.mEndpointId, ctx.mRequestPath.mClusterId, ctx.mRequestPath.mCommandId);

    auto onComplete = [this, cachedPath](const CHIP_ERROR & err) {
        if (mActiveCommandHandle.has_value())
        {
            auto * commandHandler = mActiveCommandHandle.value().Get();
            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(JointFabric, "Successfully verified trust against commissioning fabric administrator");
                commandHandler->AddStatus(cachedPath, Status::Success);
            }
            else
            {
                ChipLogError(JointFabric, "Failed to verify trust against commissioning fabric administrator");
                commandHandler->AddStatus(cachedPath, Status::Failure);
            }
        }

        // TODO: Potential edge case: if TrustVerification is interrupted such that this callback isn't invoked,
        // CleanupAnnounceJFA wouldn't run and HandleAnnounceJointFabricAdministrator will permanently return Status::Busy.
        CleanupAnnounceJFA();
    };

    VerifyOrExit(commandData.endpointID != kInvalidEndpointId, globalStatus = Status::ConstraintError);

    // If we have an active commissionee, that means another AnnounceJointFabricAdministrator command is already being serviced.
    // Respond that the server is busy.
    VerifyOrExit(!mActiveCommissionee.has_value() && !mActiveCommandHandle.has_value(), globalStatus = Status::Busy);

    mActiveCommandHandle.emplace(&ctx.mCommandHandler);
    mActiveCommissionee.emplace(mActiveCommandHandle.value(), commandData.endpointID, std::move(onComplete));

    mActiveCommissionee->VerifyTrustAgainstCommissionerAdmin();

    return;
exit:
    if (globalStatus.has_value())
    {
        ChipLogProgress(JointFabric, "Failed to handle AnnounceJointFabricAdministrator");
        ctx.mCommandHandler.AddStatus(cachedPath, globalStatus.value());
    }
}

void JointFabricAdministratorGlobalInstance::CleanupAnnounceJFA()
{
    mActiveCommissionee.reset();
    mActiveCommandHandle.reset();
}

void JointFabricAdministratorGlobalInstance::HandleICACCSRRequest(HandlerContext & ctx,
                                                                  const Commands::ICACCSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ICACCSRRequest", "JointFabricAdministrator");
    ChipLogProgress(Zcl, "JointFabricAdministrator: Received an ICACCSRRequest command");

    auto nonDefaultStatus           = Status::Success;
    auto & failSafeContext          = Server::GetInstance().GetFailSafeContext();
    auto & jointFabricAdministrator = Server::GetInstance().GetJointFabricAdministrator();

    uint8_t buf[Credentials::kMaxDERCertLength];
    MutableByteSpan icacCsr(buf, Credentials::kMaxDERCertLength);
    Commands::ICACCSRResponse::Type response;

    // command must be invoked over CASE
    VerifyOrExit(ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase,
                 nonDefaultStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    /* TODO spec.: If the <<ref_FabricTableVendorIdVerificationProcedure, FabricFabric Table Vendor ID Verification Procedure>>
     * has not been executed against the initiator of this command, the command SHALL fail
     * with a <<ref_JFVidNotVerified, JfVidNotVerified>> status code SHALL be sent back to the initiator.*/

    VerifyOrExit(!failSafeContext.AddICACCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);

    VerifyOrExit(jointFabricAdministrator.GetDelegate() != nullptr, nonDefaultStatus = Status::Failure);
    VerifyOrExit(jointFabricAdministrator.GetDelegate()->GetIcacCsr(icacCsr) == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    response.icaccsr = icacCsr;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);

exit:
    if (nonDefaultStatus != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
    }
}

void JointFabricAdministratorGlobalInstance::HandleAddICAC(HandlerContext & ctx,
                                                           const Commands::AddICAC::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddICAC", "JointFabricAdministrator");
    ChipLogProgress(Zcl, "JointFabricAdministrator: Received an AddICAC command");

    auto nonDefaultStatus  = Status::Success;
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();

    // command must be invoked over CASE
    VerifyOrExit(ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase,
                 nonDefaultStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.AddICACCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);
    failSafeContext.SetAddICACHasBeenInvoked();

    /* TODO: implement rest of the AddICAC checks */

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
}

void JointFabricAdministratorGlobalInstance::HandleTransferAnchorRequest(
    HandlerContext & ctx, const Commands::TransferAnchorRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("TransferAnchorRequest", "JointFabricAdministrator");
}

void JointFabricAdministratorGlobalInstance::HandleTransferAnchorComplete(
    HandlerContext & ctx, const Commands::TransferAnchorComplete::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("TransferAnchorComplete", "JointFabricAdministrator");
}
#else
void JointFabricAdministratorGlobalInstance::HandleOJCW(HandlerContext & ctx,
                                                        const Commands::OpenJointCommissioningWindow::DecodableType & commandData)
{}

void JointFabricAdministratorGlobalInstance::HandleAnnounceJointFabricAdministrator(
    HandlerContext & ctx, const Commands::AnnounceJointFabricAdministrator::DecodableType & commandData)
{}

void JointFabricAdministratorGlobalInstance::HandleICACCSRRequest(HandlerContext & ctx,
                                                                  const Commands::ICACCSRRequest::DecodableType & commandData)
{}

void JointFabricAdministratorGlobalInstance::HandleAddICAC(HandlerContext & ctx,
                                                           const Commands::AddICAC::DecodableType & commandData)
{}

void JointFabricAdministratorGlobalInstance::HandleTransferAnchorRequest(
    HandlerContext & ctx, const Commands::TransferAnchorRequest::DecodableType & commandData)
{}

void JointFabricAdministratorGlobalInstance::HandleTransferAnchorComplete(
    HandlerContext & ctx, const Commands::TransferAnchorComplete::DecodableType & commandData)
{}
#endif

void MatterJointFabricAdministratorPluginServerInitCallback()
{
    ChipLogProgress(DataManagement, "JointFabricAdministrator: initializing");
    AttributeAccessInterfaceRegistry::Instance().Register(&gJointFabricAdministratorGlobalInstance);
    ReturnOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gJointFabricAdministratorGlobalInstance));
}

void MatterJointFabricAdministratorPluginServerShutdownCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(&gJointFabricAdministratorGlobalInstance);
    ReturnOnFailure(CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(&gJointFabricAdministratorGlobalInstance));
}
