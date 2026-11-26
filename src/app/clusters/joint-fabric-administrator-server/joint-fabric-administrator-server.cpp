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
#include <app-common/zap-generated/cluster-objects.h>
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
#include <lib/support/ScopedMemoryBuffer.h>
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
    void OnFailSafeTimerExpired();

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

    CHIP_ERROR VerifyAddICACStep1(const FabricIndex accessFabric, const Commands::AddICAC::DecodableType & commandData);
    CHIP_ERROR VerifyAddICACPublicKey(const Commands::AddICAC::DecodableType & commandData);
    CHIP_ERROR VerifyAddICACDNEncodingRules(const Commands::AddICAC::DecodableType & commandData);

    // Cleans up asynchronous resources used in HandleAnnounceJointFabricAdministrator
    void CleanupAnnounceJFA();

    std::optional<CommandHandler::Handle> mActiveCommandHandle;
    std::optional<JCMCommissionee> mActiveCommissionee;
};

JointFabricAdministratorGlobalInstance gJointFabricAdministratorGlobalInstance;

namespace {
void OnPlatformEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        auto * instance = reinterpret_cast<JointFabricAdministratorGlobalInstance *>(arg);
        VerifyOrReturn(instance != nullptr);
        instance->OnFailSafeTimerExpired();
    }
}
} // anonymous namespace

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

    Optional<ICACCSRResponseStatusCodeEnum> status = Optional<ICACCSRResponseStatusCodeEnum>::Missing();
    Status globalStatus                            = Status::Success;
    Spake2pVerifier verifier;
    DataModel::Nullable<FabricIndex> administratorFabricIndex;

    ChipLogProgress(Zcl, "Received command to open joint commissioning window");

    FabricIndex fabricIndex       = ctx.mCommandHandler.GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(ICACCSRResponseStatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(Attributes::AdministratorFabricIndex::Get(ctx.mRequestPath.mEndpointId, administratorFabricIndex) ==
                     Status::Success,
                 globalStatus = Status::Failure);
    VerifyOrExit(!administratorFabricIndex.IsNull() && administratorFabricIndex.Value() != 0,
                 status.Emplace(ICACCSRResponseStatusCodeEnum::kInvalidAdministratorFabricIndex));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(ICACCSRResponseStatusCodeEnum::kBusy));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(ICACCSRResponseStatusCodeEnum::kBusy));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations, globalStatus = Status::InvalidCommand);
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations, globalStatus = Status::InvalidCommand);
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length, globalStatus = Status::InvalidCommand);
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length, globalStatus = Status::InvalidCommand);

    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = Status::InvalidCommand);

    VerifyOrExit(pakeVerifier.size() == kSpake2p_VerifierSerialized_Length, globalStatus = Status::InvalidCommand);
    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR, globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissionMgr.OpenJointCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                            fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 globalStatus = Status::InvalidCommand);
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
    const FabricIndex accessingFabricIndex = ctx.mCommandHandler.GetAccessingFabricIndex();

    // The NodeId of the peer that invoked this command (JFA-A). For a CASE
    // session the subject descriptor carries the authenticated caller NodeId.
    // Note: GetFabricTable().FindFabricWithIndex(accessingFabricIndex)->GetNodeId()
    // would return our own NodeId on that fabric, not the caller's, so use the
    // subject descriptor instead.
    const Access::SubjectDescriptor subjectDescriptor = ctx.mCommandHandler.GetSubjectDescriptor();
    const NodeId peerJFANodeId =
        (subjectDescriptor.authMode == Access::AuthMode::kCase) ? subjectDescriptor.subject : kUndefinedNodeId;

    // Ensure that the accessing fabric is not marked as verified before we proceed
    Server::GetInstance().GetJointFabricAdministrator().ClearVidVerificationForFabric();

    auto onComplete = [this, cachedPath, accessingFabricIndex, peerJFANodeId](const CHIP_ERROR & err) {

        if (mActiveCommandHandle.has_value())
        {
            auto * commandHandler = mActiveCommandHandle.value().Get();
            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(JointFabric, "Successfully verified trust against commissioning fabric administrator");
                Server::GetInstance().GetJointFabricAdministrator().SetVidVerificationForFabric(accessingFabricIndex);

                // Record the NodeId of the peer JFA (JFA-A) that invoked this command so the
                // delegate (JFAManager) can expose it as an indicator that JCM is in progress.
                // peerJFANodeId is the authenticated caller NodeId from the CASE subject
                // descriptor, not our own NodeId on the accessing fabric.
                auto * delegate = Server::GetInstance().GetJointFabricAdministrator().GetDelegate();
                if (delegate != nullptr && peerJFANodeId != kUndefinedNodeId)
                {
                    // JFA-A invoked this command over the joint fabric; the device must reach
                    // JFA-A on that same fabric index (accessingFabricIndex), not on its own
                    // locally-commissioned fabric index.
                    delegate->SetPeerJFANodeId(peerJFANodeId, accessingFabricIndex);
                    ChipLogProgress(JointFabric,
                                    "HandleAnnounceJointFabricAdministrator: peer JFA-A NodeId=0x" ChipLogFormatX64
                                    " on fabric index %u",
                                    ChipLogValueX64(peerJFANodeId), static_cast<unsigned>(accessingFabricIndex));
                }



                commandHandler->AddStatus(cachedPath, Status::Success);
            }
            else
            {
                ChipLogError(JointFabric, "Failed to verify trust against commissioning fabric administrator");
                commandHandler->AddStatus(cachedPath, Status::Failure);
            }
        }

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

void JointFabricAdministratorGlobalInstance::OnFailSafeTimerExpired()
{
    CleanupAnnounceJFA();
    Server::GetInstance().GetJointFabricAdministrator().ClearVidVerificationForFabric();
    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(kInvalidEndpointId);

    // If JF materials were already pushed to the local commissioner (JFC-B),
    // tell it to discard them so stale credentials cannot be used.
    // Also clear the recorded peer JFA-A NodeId since this JCM attempt is aborted.
    auto * delegate = Server::GetInstance().GetJointFabricAdministrator().GetDelegate();
    if (delegate != nullptr)
    {
        CHIP_ERROR clearErr = delegate->ClearJFMaterials();
        if (clearErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "OnFailSafeTimerExpired: ClearJFMaterials failed: %" CHIP_ERROR_FORMAT, clearErr.Format());
        }
        delegate->ClearPeerJFANodeId();
    }
}

void JointFabricAdministratorGlobalInstance::HandleICACCSRRequest(HandlerContext & ctx,
                                                                  const Commands::ICACCSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ICACCSRRequest", "JointFabricAdministrator");
    ChipLogProgress(Zcl, "JointFabricAdministrator: Received an ICACCSRRequest command");

    auto nonDefaultStatus                          = Status::Success;
    Optional<ICACCSRResponseStatusCodeEnum> status = Optional<ICACCSRResponseStatusCodeEnum>::Missing();
    auto & failSafeContext                         = Server::GetInstance().GetFailSafeContext();
    auto & jointFabricAdministrator                = Server::GetInstance().GetJointFabricAdministrator();

    uint8_t buf[Credentials::kMaxDERCertLength];
    MutableByteSpan icacCsr(buf, Credentials::kMaxDERCertLength);
    Commands::ICACCSRResponse::Type response;
    DataModel::Nullable<FabricIndex> administratorFabricIndex;

    // command must be invoked over CASE
    VerifyOrExit(ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase,
                 nonDefaultStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.AddICACCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);

    VerifyOrExit(jointFabricAdministrator.WasVidVerificationExecutedForFabric(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 status.Emplace(ICACCSRResponseStatusCodeEnum::kVIDNotVerified));

    VerifyOrExit(Attributes::AdministratorFabricIndex::Get(ctx.mRequestPath.mEndpointId, administratorFabricIndex) ==
                     Status::Success,
                 nonDefaultStatus = Status::Failure);
    VerifyOrExit(!administratorFabricIndex.IsNull(),
                 status.Emplace(ICACCSRResponseStatusCodeEnum::kInvalidAdministratorFabricIndex));

    VerifyOrExit(jointFabricAdministrator.GetDelegate() != nullptr, nonDefaultStatus = Status::Failure);
    VerifyOrExit(jointFabricAdministrator.GetDelegate()->GetIcacCsr(icacCsr) == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    response.icaccsr = MakeOptional(icacCsr);
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);

exit:
    if (status.HasValue())
    {
        TEMPORARY_RETURN_IGNORED ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, to_underlying(status.Value()));
    }
    else if (nonDefaultStatus != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
    }
}

void JointFabricAdministratorGlobalInstance::HandleAddICAC(HandlerContext & ctx,
                                                           const Commands::AddICAC::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddICAC", "JointFabricAdministrator");
    ChipLogProgress(Zcl, "JointFabricAdministrator: Received an AddICAC command");

    auto nonDefaultStatus                   = Status::Success;
    auto & failSafeContext                  = Server::GetInstance().GetFailSafeContext();
    Optional<ICACResponseStatusEnum> status = Optional<ICACResponseStatusEnum>::Missing();
    // command must be invoked over CASE
    VerifyOrExit(ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase,
                 nonDefaultStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.AddICACCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);
    failSafeContext.SetAddICACHasBeenInvoked();

    {
        CHIP_ERROR step1Err = VerifyAddICACStep1(ctx.mCommandHandler.GetAccessingFabricIndex(), commandData);
        if (step1Err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AddICAC: VerifyAddICACStep1 failed: %" CHIP_ERROR_FORMAT, step1Err.Format());
            status.Emplace(ICACResponseStatusEnum::kInvalidICAC);
            goto exit;
        }
    }

    {
        CHIP_ERROR step2Err = VerifyAddICACPublicKey(commandData);
        if (step2Err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AddICAC: VerifyAddICACPublicKey failed: %" CHIP_ERROR_FORMAT, step2Err.Format());
            status.Emplace(ICACResponseStatusEnum::kInvalidPublicKey);
            goto exit;
        }
    }

    {
        CHIP_ERROR step3Err = VerifyAddICACDNEncodingRules(commandData);
        if (step3Err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AddICAC: VerifyAddICACDNEncodingRules failed: %" CHIP_ERROR_FORMAT, step3Err.Format());
            status.Emplace(ICACResponseStatusEnum::kInvalidICAC);
            goto exit;
        }
    }

    {
        auto & jointFabricAdministrator = Server::GetInstance().GetJointFabricAdministrator();
        if (jointFabricAdministrator.GetDelegate() != nullptr)
        {
            CHIP_ERROR storeErr = jointFabricAdministrator.GetDelegate()->StoreCrossSignedICAC(commandData.ICACValue);
            if (storeErr != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "JointFabricAdministrator: StoreCrossSignedICAC failed: %" CHIP_ERROR_FORMAT, storeErr.Format());
                status.Emplace(ICACResponseStatusEnum::kInvalidICAC);
                goto exit;
            }

            // After storing the cross-signed ICAC, push the JF materials to the
            // local commissioner (JFC-B) so it can issue Fabric-A NOCs for device
            // migration. The RCAC and IPK are taken from the accessing fabric which
            // is the joint fabric (Fabric A) this device was just admitted to.
            {
                FabricIndex accessingFabric = ctx.mCommandHandler.GetAccessingFabricIndex();

                uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
                MutableByteSpan rcacSpan(rcacBuf, sizeof(rcacBuf));
                CHIP_ERROR rcacErr = Server::GetInstance().GetFabricTable().FetchRootCert(accessingFabric, rcacSpan);
                if (rcacErr != CHIP_NO_ERROR)
                {
                    ChipLogError(Zcl, "HandleAddICAC: FetchRootCert failed: %" CHIP_ERROR_FORMAT, rcacErr.Format());
                    // Non-fatal for the AddICAC response; log and skip PushJFMaterials.
                }
                else
                {
                    uint8_t ipkBuf[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
                    ByteSpan ipkSpan;

                    // IMPORTANT: The IPK pushed into the migration pipeline is ultimately delivered to
                    // the migrated device as the AddNOC IPKValue. Per the Matter spec, AddNOC IPKValue
                    // must be the RAW EPOCH IPK; the receiving device internally derives its operational
                    // group key as DeriveGroupOperationalKey(IPKValue, compressedFabricId). It must NOT be
                    // the already-derived operational key.
                    //
                    // GroupDataProvider::GetIpkKeySet() returns the OPERATIONAL (derived) key, not the raw
                    // epoch key (the epoch key is not retained in storage). Using it here would make the
                    // device double-derive: Derive(operationalA, cfidA) != operationalA, so the device's
                    // CASE destinationId HMAC key would differ from JFA-B's, producing the "No shared
                    // trusted root" (0xC9) failure on the Fabric-A reconnect.
                    //
                    // In this Joint Fabric example setup, Fabric A's epoch IPK is the default test IPK
                    // (the same value ExampleOperationalCredentialsIssuer falls back to). Push that raw
                    // epoch IPK so the migrated device re-derives the correct operational key for Fabric A
                    // and the CASE destinationId matches.
                    {
                        ByteSpan defaultIpkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
                        if (defaultIpkSpan.size() == Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES)
                        {
                            memcpy(ipkBuf, defaultIpkSpan.data(), Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
                            ipkSpan = ByteSpan(ipkBuf, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
                        }
                        else
                        {
                            ChipLogError(Zcl, "HandleAddICAC: unexpected default IPK size %u",
                                         static_cast<unsigned>(defaultIpkSpan.size()));
                        }
                    }

                    if (!ipkSpan.empty())
                    {
                        CHIP_ERROR pushErr =
                            jointFabricAdministrator.GetDelegate()->PushJFMaterials(commandData.ICACValue, rcacSpan, ipkSpan);
                        if (pushErr == CHIP_NO_ERROR)
                        {
                            ChipLogProgress(Zcl, "HandleAddICAC: JF materials pushed to local commissioner");
                        }
                        else
                        {
                            ChipLogError(Zcl, "HandleAddICAC: PushJFMaterials failed: %" CHIP_ERROR_FORMAT, pushErr.Format());
                        }
                    }
                }
            }
        }
    }

exit:
    {
        Commands::ICACResponse::Type response;
        response.statusCode = status.ValueOr(ICACResponseStatusEnum::kOk);
        if (nonDefaultStatus != Status::Success && !status.HasValue())
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
            return;
        }
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
}

CHIP_ERROR JointFabricAdministratorGlobalInstance::VerifyAddICACStep1(const FabricIndex accessFabric,
                                                                      const Commands::AddICAC::DecodableType & commandData)
{
    constexpr uint8_t kMaxCertsInChain = 2;
    uint8_t rcacBuf[Credentials::kMaxCHIPCertLength];
    MutableByteSpan rcacSpan{ rcacBuf };
    ChipCertificateSet certificates;
    ValidationContext validContext;

    CHIP_ERROR err;

    err = Server::GetInstance().GetFabricTable().FetchRootCert(accessFabric, rcacSpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACStep1: FetchRootCert failed (fabric %u): %" CHIP_ERROR_FORMAT,
                     static_cast<unsigned>(accessFabric), err.Format());
        return err;
    }
    ChipLogProgress(Zcl, "VerifyAddICACStep1: RCAC fetched, size=%u, ICAC size=%u",
                    static_cast<unsigned>(rcacSpan.size()),
                    static_cast<unsigned>(commandData.ICACValue.size()));

    err = certificates.Init(kMaxCertsInChain);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACStep1: certificates.Init failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = certificates.LoadCert(rcacSpan, BitFlags<CertDecodeFlags>(CertDecodeFlags::kIsTrustAnchor));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACStep1: LoadCert(RCAC) failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = certificates.LoadCert(commandData.ICACValue, BitFlags<CertDecodeFlags>(CertDecodeFlags::kGenerateTBSHash));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACStep1: LoadCert(ICAC) failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    validContext.Reset();
    validContext.mRequiredKeyUsages.Set(KeyUsageFlags::kKeyCertSign);
    validContext.mRequiredCertType = CertType::kICA;
    err = certificates.ValidateCert(certificates.GetLastCert(), validContext);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACStep1: ValidateCert failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(Zcl, "VerifyAddICACStep1: ICAC validated against RCAC successfully");
    }
    return err;
}

CHIP_ERROR JointFabricAdministratorGlobalInstance::VerifyAddICACPublicKey(const Commands::AddICAC::DecodableType & commandData)
{
    uint8_t csrBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan icacCsr{ csrBuf };
    Crypto::P256PublicKey csrPubKey;
    Credentials::P256PublicKeySpan icacPubKeySpan;

    CHIP_ERROR err;
    auto & jointFabricAdministrator = Server::GetInstance().GetJointFabricAdministrator();
    VerifyOrReturnError(jointFabricAdministrator.GetDelegate() != nullptr, CHIP_ERROR_INCORRECT_STATE);

    err = jointFabricAdministrator.GetDelegate()->GetIcacCsr(icacCsr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACPublicKey: GetIcacCsr failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    ChipLogProgress(Zcl, "VerifyAddICACPublicKey: CSR size=%u", static_cast<unsigned>(icacCsr.size()));

    err = Crypto::VerifyCertificateSigningRequest(icacCsr.data(), icacCsr.size(), csrPubKey);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACPublicKey: VerifyCertificateSigningRequest failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    err = Credentials::ExtractPublicKeyFromChipCert(commandData.ICACValue, icacPubKeySpan);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "VerifyAddICACPublicKey: ExtractPublicKeyFromChipCert failed: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    Credentials::P256PublicKeySpan csrPubKeySpan(csrPubKey.ConstBytes());
    if (memcmp(icacPubKeySpan.data(), csrPubKeySpan.data(), icacPubKeySpan.size()) != 0)
    {
        ChipLogError(Zcl, "VerifyAddICACPublicKey: public key mismatch between CSR and ICAC");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ChipLogProgress(Zcl, "VerifyAddICACPublicKey: public key match OK");
    return CHIP_NO_ERROR;
}

CHIP_ERROR
JointFabricAdministratorGlobalInstance::VerifyAddICACDNEncodingRules(const Commands::AddICAC::DecodableType & commandData)
{
    Credentials::ChipCertificateData certData;
    return Credentials::DecodeChipCert(commandData.ICACValue, certData);
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
    ReturnOnFailure(DeviceLayer::PlatformMgr().AddEventHandler(
        OnPlatformEventHandler, reinterpret_cast<intptr_t>(&gJointFabricAdministratorGlobalInstance)));
}

void MatterJointFabricAdministratorPluginServerShutdownCallback()
{
    DeviceLayer::PlatformMgr().RemoveEventHandler(OnPlatformEventHandler,
                                                  reinterpret_cast<intptr_t>(&gJointFabricAdministratorGlobalInstance));
    AttributeAccessInterfaceRegistry::Instance().Unregister(&gJointFabricAdministratorGlobalInstance);
    ReturnOnFailure(CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(&gJointFabricAdministratorGlobalInstance));
}
