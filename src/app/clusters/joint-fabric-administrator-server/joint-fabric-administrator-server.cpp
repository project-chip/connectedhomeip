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

    Optional<JointFabricAdministrator::StatusCodeEnum> status = Optional<JointFabricAdministrator::StatusCodeEnum>::Missing();
    Status globalStatus                                       = Status::Success;
    Spake2pVerifier verifier;

    ChipLogProgress(Zcl, "Received command to open joint commissioning window");

    FabricIndex fabricIndex       = ctx.mCommandHandler.GetAccessingFabricIndex();
    const FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    auto & commissionMgr          = Server::GetInstance().GetCommissioningWindowManager();

    VerifyOrExit(fabricInfo != nullptr, status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(failSafeContext.IsFailSafeFullyDisarmed(), status.Emplace(JointFabricAdministrator::StatusCodeEnum::kBusy));

    VerifyOrExit(!commissionMgr.IsCommissioningWindowOpen(), status.Emplace(JointFabricAdministrator::StatusCodeEnum::kBusy));
    VerifyOrExit(iterations >= kSpake2p_Min_PBKDF_Iterations,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(iterations <= kSpake2p_Max_PBKDF_Iterations,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(salt.size() >= kSpake2p_Min_PBKDF_Salt_Length,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(salt.size() <= kSpake2p_Max_PBKDF_Salt_Length,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));

    VerifyOrExit(commissioningTimeout <= commissionMgr.MaxCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(commissioningTimeout >= commissionMgr.MinCommissioningTimeout(), globalStatus = Status::InvalidCommand);
    VerifyOrExit(discriminator <= kMaxDiscriminatorValue, globalStatus = Status::InvalidCommand);

    VerifyOrExit(verifier.Deserialize(pakeVerifier) == CHIP_NO_ERROR,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    VerifyOrExit(commissionMgr.OpenJointCommissioningWindow(commissioningTimeout, discriminator, verifier, iterations, salt,
                                                            fabricIndex, fabricInfo->GetVendorId()) == CHIP_NO_ERROR,
                 status.Emplace(JointFabricAdministrator::StatusCodeEnum::kPAKEParameterError));
    ChipLogProgress(Zcl, "Commissioning window is now open");

exit:
    if (status.HasValue())
    {
        ChipLogError(Zcl, "Failed to open joint commissioning window. Cluster status 0x%02x", to_underlying(status.Value()));
        ctx.mCommandHandler.AddClusterSpecificFailure(ctx.mRequestPath, to_underlying(status.Value()));
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
    ChipLogProgress(JointFabric, "emberAfJointFabricAdministratorClusterAnnounceJointFabricAdministratorCallback: %u",
                    commandData.endpointID);

    auto nonDefaultStatus = Status::Success;
    VerifyOrExit(commandData.endpointID != kInvalidEndpointId, nonDefaultStatus = Status::ConstraintError);

    Server::GetInstance().GetJointFabricAdministrator().SetPeerJFAdminClusterEndpointId(commandData.endpointID);

    /* TODO: execute Fabric Table Vendor ID Verification */

exit:
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
}

void JointFabricAdministratorGlobalInstance::HandleICACCSRRequest(HandlerContext & ctx,
                                                                  const Commands::ICACCSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("ICACCSRRequest", "JointFabricAdministrator");
    ChipLogProgress(Zcl, "JointFabricAdministrator: Received an ICACCSRRequest command");

    auto nonDefaultStatus           = Status::Success;
    auto & failSafeContext          = Server::GetInstance().GetFailSafeContext();
    auto & jointFabricAdministrator = Server::GetInstance().GetJointFabricAdministrator();
    P256PublicKey pubKey;

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

    VerifyOrExit(VerifyCertificateSigningRequest(icacCsr.data(), icacCsr.size(), pubKey) == CHIP_NO_ERROR,
                 nonDefaultStatus = Status::Failure);
    Server::GetInstance().GetJointFabricAdministrator().SetIcacCsrPubKey(pubKey);

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

    CHIP_ERROR err                                         = CHIP_NO_ERROR;
    auto nonDefaultStatus                                  = Status::Success;
    auto & failSafeContext                                 = Server::GetInstance().GetFailSafeContext();
    auto & fabricTable                                     = Server::GetInstance().GetFabricTable();
    auto & jointFabricAdministrator                        = Server::GetInstance().GetJointFabricAdministrator();
    CertificateChainValidationResult chainValidationResult = CertificateChainValidationResult::kSuccess;
    Commands::ICACResponse::Type response;

    // Heap-allocated buffers to minimize stack usage
    chip::Platform::ScopedMemoryBuffer<uint8_t> anchorCAChipCertBuf;
    VerifyOrReturn(anchorCAChipCertBuf.Alloc(kMaxCHIPCertLength));
    MutableByteSpan anchorCAChipCert(anchorCAChipCertBuf.Get(), kMaxCHIPCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> crossSignedICACX509Buf;
    VerifyOrReturn(crossSignedICACX509Buf.Alloc(kMaxDERCertLength));
    MutableByteSpan crossSignedICACX509(crossSignedICACX509Buf.Get(), kMaxDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> crossSignedICACChipCertBuf;
    VerifyOrReturn(crossSignedICACChipCertBuf.Alloc(kMaxCHIPCertLength));
    MutableByteSpan crossSignedICACChipCert(crossSignedICACChipCertBuf.Get(), kMaxCHIPCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> anchorCAX509Buf;
    VerifyOrReturn(anchorCAX509Buf.Alloc(kMaxDERCertLength));
    MutableByteSpan anchorCAX509(anchorCAX509Buf.Get(), kMaxDERCertLength);

    Crypto::P256PublicKey crossSignedICACX509PubKey;
    Crypto::P256PublicKey icacCsrPubKey;
    FabricId anchorFabricId = kUndefinedFabricId;

    // command must be invoked over CASE
    VerifyOrExit(ctx.mCommandHandler.GetSubjectDescriptor().authMode == Access::AuthMode::kCase,
                 nonDefaultStatus = Status::InvalidCommand);

    // fail-safe timer must be armed
    VerifyOrExit(failSafeContext.IsFailSafeArmed(ctx.mCommandHandler.GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    // only one AddICAC command per fail-safe timer
    VerifyOrExit(!failSafeContext.AddICACCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);
    failSafeContext.SetAddICACHasBeenInvoked();

    // Fetch and convert the root certificate
    err = fabricTable.FetchRootCert(ctx.mCommandHandler.GetAccessingFabricIndex(), anchorCAChipCert);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    err = ConvertChipCertToX509Cert(anchorCAChipCert, anchorCAX509);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    // Convert the incoming ICAC to X.509
    err = ConvertChipCertToX509Cert(commandData.ICACValue, crossSignedICACX509);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    // Validate the certificate chain
    err = ValidateCertificateChain(anchorCAX509.data(), anchorCAX509.size(), nullptr, 0, crossSignedICACX509.data(),
                                   crossSignedICACX509.size(), chainValidationResult);
    VerifyOrExit((err == CHIP_NO_ERROR) && (chainValidationResult == CertificateChainValidationResult::kSuccess),
                 response.statusCode = ICACResponseStatusEnum::kInvalidICAC);

    // Extract and compare public keys
    err = ExtractPubkeyFromX509Cert(crossSignedICACX509, crossSignedICACX509PubKey);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    icacCsrPubKey = jointFabricAdministrator.GetIcacCsrPubKey();
    VerifyOrExit(icacCsrPubKey.Matches(crossSignedICACX509PubKey), response.statusCode = ICACResponseStatusEnum::kInvalidPublicKey);

    // Convert back to CHIP cert and extract Fabric ID
    err = ConvertX509CertToChipCert(crossSignedICACX509, crossSignedICACChipCert);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    err = ExtractFabricIdFromCert(crossSignedICACChipCert, &anchorFabricId);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::ConstraintError);

    VerifyOrExit(fabricTable.FindFabricWithIndex(ctx.mCommandHandler.GetAccessingFabricIndex())->GetFabricId() == anchorFabricId,
                 response.statusCode = ICACResponseStatusEnum::kInvalidICAC);

    // Notify delegate
    VerifyOrExit(jointFabricAdministrator.GetDelegate() != nullptr, nonDefaultStatus = Status::Failure);
    jointFabricAdministrator.GetDelegate()->OnAddICAC(crossSignedICACChipCert);
    response.statusCode = ICACResponseStatusEnum::kOk;

exit:
    if (nonDefaultStatus != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, nonDefaultStatus);
    }
    else
    {
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
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
