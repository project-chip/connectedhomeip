/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 * @brief Implementation for the Operational Credentials Cluster
 ***************************************************************************/

#include <access/AccessControl.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/server/Dnssd.h>
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
#include <platform/CHIPDeviceLayer.h>
#include <string.h>
#include <tracing/macros.h>

#include "joint-fabric-pki-server.h"

using namespace chip;
using namespace chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::JointFabricPki;
using namespace chip::Controller;
using namespace chip::Credentials;
using namespace chip::Crypto;
using namespace chip::Protocols::InteractionModel;

FabricIndex ecosystemFabricIndex                    = 1;
PersistentStorage * gPersistentStorage              = nullptr;
OperationalCredentialsDelegate * gCredentialsIssuer = nullptr;
const char * gChipToolKvs                           = nullptr;

void SetEcosystemFabricIndex(FabricIndex index)
{
    ecosystemFabricIndex = index;
}

void SetPersistentStorageDelegate(PersistentStorage * storageDelegate)
{
    gPersistentStorage = storageDelegate;
}

void SetOperationalCredentialsIssuer(OperationalCredentialsDelegate * provider)
{
    gCredentialsIssuer = provider;
}

void SetChipToolKvs(const char * chipToolKvs)
{
    gChipToolKvs = chipToolKvs;
}

namespace {

void SendJointFabricResponse(app::CommandHandler * commandObj, const ConcreteCommandPath & path, JointFabricStatusEnum status,
                             uint8_t index, const CharSpan & debug_text)
{
    Commands::JointFabricResponse::Type payload;
    payload.statusCode = status;
    if (status == JointFabricStatusEnum::kOk)
    {
        payload.fabricIndex.Emplace(index);
    }
    if (!debug_text.empty())
    {
        // Max length of DebugText is 128 in the spec.
        const CharSpan & to_send = debug_text.size() > 128 ? debug_text.SubSpan(0, 128) : debug_text;
        payload.debugText.Emplace(to_send);
    }

    commandObj->AddResponse(path, payload);
}

JointFabricStatusEnum ConvertToJointFabricResponseStatus(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        return JointFabricStatusEnum::kOk;
    }
    if (err == CHIP_ERROR_INVALID_PUBLIC_KEY)
    {
        return JointFabricStatusEnum::kInvalidPublicKey;
    }
    if (err == CHIP_ERROR_WRONG_NODE_ID)
    {
        return JointFabricStatusEnum::kInvalidNodeOpId;
    }
    if (err == CHIP_ERROR_UNSUPPORTED_CERT_FORMAT)
    {
        return JointFabricStatusEnum::kInvalidNOC;
    }
    if (err == CHIP_ERROR_WRONG_CERT_DN)
    {
        return JointFabricStatusEnum::kInvalidNOC;
    }
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return JointFabricStatusEnum::kMissingCsr;
    }
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return JointFabricStatusEnum::kTableFull;
    }
    if (err == CHIP_ERROR_FABRIC_EXISTS)
    {
        return JointFabricStatusEnum::kFabricConflict;
    }
    if (err == CHIP_ERROR_INVALID_FABRIC_INDEX)
    {
        return JointFabricStatusEnum::kInvalidFabricIndex;
    }
    if (err == CHIP_ERROR_INVALID_ADMIN_SUBJECT)
    {
        return JointFabricStatusEnum::kInvalidAdminSubject;
    }

    return JointFabricStatusEnum::kInvalidNOC;
}

const FabricInfo * RetrieveEcosystemFabric()
{
    ChipLogDetail(Zcl, "JointFabricPki: Finding ecosystem fabric with fabricIndex 0x%x",
                  static_cast<unsigned>(ecosystemFabricIndex));
    return Server::GetInstance().GetFabricTable().FindFabricWithIndex(ecosystemFabricIndex);
}

CHIP_ERROR CreateAccessControlEntryForNewFabricAdministrator(const Access::SubjectDescriptor & subjectDescriptor,
                                                             FabricIndex fabricIndex, uint64_t subject)
{
    NodeId subjectAsNodeID = static_cast<NodeId>(subject);

    if (!IsOperationalNodeId(subjectAsNodeID) && !IsCASEAuthTag(subjectAsNodeID))
    {
        return CHIP_ERROR_INVALID_ADMIN_SUBJECT;
    }

    Access::AccessControl::Entry entry;
    ReturnErrorOnFailure(Access::GetAccessControl().PrepareEntry(entry));
    ReturnErrorOnFailure(entry.SetFabricIndex(fabricIndex));
    ReturnErrorOnFailure(entry.SetPrivilege(Access::Privilege::kAdminister));
    ReturnErrorOnFailure(entry.SetAuthMode(Access::AuthMode::kCase));
    ReturnErrorOnFailure(entry.AddSubject(nullptr, subject));
    CHIP_ERROR err = Access::GetAccessControl().CreateEntry(&subjectDescriptor, fabricIndex, nullptr, entry);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OpCreds: Failed to add administrative node ACL entry: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    ChipLogProgress(Zcl, "OpCreds: ACL entry created for Fabric index 0x%x CASE Admin Subject 0x" ChipLogFormatX64,
                    static_cast<unsigned>(fabricIndex), ChipLogValueX64(subject));

    return CHIP_NO_ERROR;
}

} // namespace

void MatterJointFabricPkiPluginServerInitCallback() {}

bool emberAfJointFabricPkiClusterJointFabricRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricPki::Commands::JointFabricRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("JointFabricRequest", "JointFabricPki");

    // uint64_t fabricIndex = commandData.fabricIndex;

    auto finalStatus = Status::Failure;
    CHIP_ERROR err   = CHIP_ERROR_INVALID_ARGUMENT;

    // auto & fabricTable = Server::GetInstance().GetFabricTable();
    // const FabricInfo * fabricInfo = RetrieveEcosystemFabric();

    ChipLogProgress(Zcl, "JointFabricPki: Received a JointFabricRequest command");

    // If current fabric is not available, command was invoked over PASE which is not legal
    // VerifyOrExit(fabricInfo != nullptr, finalStatus = Status::Failure);
    VerifyOrExit(ecosystemFabricIndex != kUndefinedFabricIndex, finalStatus = Status::UnsupportedCommand);

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    {
        Commands::SignNOCIssuerRequest::Type response;

        uint8_t icaCsr[Crypto::kMAX_CSR_Buffer_Size] = { 0 };
        // uint8_t icac[Credentials::kMaxCHIPCertLength] = { 0 };
        // uint8_t rcac[Credentials::kMaxCHIPCertLength] = { 0 };
        MutableByteSpan icaCsrSpan{ icaCsr };
        // MutableByteSpan icacSpan{ icac };
        // MutableByteSpan rcacSpan{ rcac };

        if (gPersistentStorage == nullptr)
        {
            err = CHIP_ERROR_INTERNAL;
            VerifyOrExit(gPersistentStorage != nullptr, finalStatus = Status::Failure);
        }

        if (gCredentialsIssuer == nullptr)
        {
            err = CHIP_ERROR_INTERNAL;
            VerifyOrExit(gCredentialsIssuer != nullptr, finalStatus = Status::Failure);
        }

        // hack to obtain private keys from chip-tool
        // gCredentialsIssuer->SetTrackingFabricIndex(fabricIndex);

        err = gPersistentStorage->Init(nullptr, gChipToolKvs);
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);
        err = gCredentialsIssuer->Initialize(*gPersistentStorage);
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

        // ChipLogProgress(Zcl, "JointFabricPki: reading fabric index 0x%x", static_cast<unsigned>(ecosystemFabricIndex));

        // err = fabricTable.FetchRootCert(ecosystemFabricIndex, rcacSpan);
        // VerifyOrExit(err == CHIP_NO_ERROR && rcacSpan.size() > 0, finalStatus = Status::Failure);
        // err = fabricTable.FetchICACert(ecosystemFabricIndex, icacSpan);
        // VerifyOrExit(err == CHIP_NO_ERROR && icacSpan.size() > 0, finalStatus = Status::Failure);

        err = gCredentialsIssuer->ObtainIcaCsr(icaCsrSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && icaCsrSpan.size() > 0, finalStatus = Status::Failure);

        response.NOCIssuerCSR = icaCsrSpan;
        // response.currentNOCIssuerCert = icacSpan;
        // response.currentNOCIssuerRoot = rcacSpan;

        ChipLogProgress(Zcl, "JointFabricPki: JointFabricRequest successful.");
        finalStatus = Status::Success;
        commandObj->AddResponse(commandPath, response);
    }

exit:
    if (finalStatus != Status::Success)
    {
        commandObj->AddStatus(commandPath, finalStatus);
        ChipLogError(Zcl, "JointFabricPki: Failed JointFabricRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                     to_underlying(finalStatus), err.Format());
    }

    return true;
}

bool emberAfJointFabricPkiClusterSignNOCIssuerResponseCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::JointFabricPki::Commands::SignNOCIssuerResponse::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("SignNOCIssuerResponse", "JointFabricPki");
    auto & ICACValue         = commandData.NOCIssuerCert;
    NodeId nodeId            = commandData.nodeId;
    FabricId fabricId        = commandData.fabricId;
    VendorId adminVendorId   = commandData.adminVendorId;
    auto jointFabricResponse = JointFabricStatusEnum::kOk;
    auto nonDefaultStatus    = Status::Success;
    auto * groupDataProvider = Credentials::GetGroupDataProvider();
    CHIP_ERROR err           = CHIP_NO_ERROR;
    FabricIndex fabricIndex  = 0;
    Credentials::GroupDataProvider::KeySet keyset;

    const FabricInfo * newFabricInfo = nullptr;
    const FabricInfo * fabricInfo    = RetrieveEcosystemFabric();
    auto & fabricTable               = Server::GetInstance().GetFabricTable();
    auto & failSafeContext           = Server::GetInstance().GetFailSafeContext();
    auto * secureSession             = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession();

    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);

    ChipLogProgress(Zcl, "JointFabricPki: Received a SignNOCIssuerResponse command");

    VerifyOrExit(ecosystemFabricIndex != kUndefinedFabricIndex, nonDefaultStatus = Status::UnsupportedCommand);
    VerifyOrExit(ICACValue.size() <= Credentials::kMaxCHIPCertLength, nonDefaultStatus = Status::InvalidCommand);

    // If current fabric is not available, command was invoked over PASE which is not legal
    VerifyOrExit(fabricInfo != nullptr,
                 jointFabricResponse = ConvertToJointFabricResponseStatus(CHIP_ERROR_INSUFFICIENT_PRIVILEGE));

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    {
        // TODO: Remove this, maybe accept IPK via this command
        ByteSpan defaultIpkSpan = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();

        uint8_t nocCsr[Crypto::kMAX_CSR_Buffer_Size] = { 0 };
        MutableByteSpan nocCsrSpan{ nocCsr };

        uint8_t nocBuf[Credentials::kMaxDERCertLength] = { 0 };
        MutableByteSpan nocSpan{ nocBuf };

        uint8_t nocChipBuf[Credentials::kMaxCHIPCertLength] = { 0 };
        MutableByteSpan nocChipSpan{ nocChipBuf };

        uint8_t icaDerCert[Credentials::kMaxDERCertLength] = { 0 };
        MutableByteSpan icaDerCertSpan{ icaDerCert };

        uint8_t rootChipBuf[Credentials::kMaxCHIPCertLength] = { 0 };
        MutableByteSpan rootChipSpan{ rootChipBuf };

        err = fabricTable.FetchRootCert(commandObj->GetAccessingFabricIndex(), rootChipSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && rootChipSpan.size() > 0, nonDefaultStatus = Status::Failure);

        fabricTable.RevertPendingFabricData();

        failSafeContext.SetCsrRequestForUpdateNoc(false);

        err = fabricTable.AllocatePendingOperationalKey(MakeOptional(commandObj->GetAccessingFabricIndex()), nocCsrSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && nocCsrSpan.size() > 0, nonDefaultStatus = Status::Failure);

        err = fabricTable.AddNewPendingTrustedRootCert(rootChipSpan);
        VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, nonDefaultStatus = Status::ResourceExhausted);

        failSafeContext.SetAddTrustedRootCertInvoked();

        if (gCredentialsIssuer == nullptr)
        {
            err = CHIP_ERROR_INTERNAL;
            VerifyOrExit(gCredentialsIssuer != nullptr, nonDefaultStatus = Status::Failure);
        }

        ChipLogProgress(Zcl, "JointFabricPki: Node Id for Next NOC Request: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
        ChipLogProgress(Zcl, "JointFabricPki: Fabric Id for Next NOC Request: 0x" ChipLogFormatX64, ChipLogValueX64(fabricId));

        gCredentialsIssuer->SetNodeIdForNextNOCRequest(nodeId);
        gCredentialsIssuer->SetFabricIdForNextNOCRequest(fabricId);
        // TODO: Set CATs for next request

        err = ConvertChipCertToX509Cert(ICACValue, icaDerCertSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && icaDerCertSpan.size() > 0, nonDefaultStatus = Status::Failure);

        err = gCredentialsIssuer->SignNOC(icaDerCertSpan, nocCsrSpan, nocSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && nocSpan.size() > 0, nonDefaultStatus = Status::Failure);

        err = ConvertX509CertToChipCert(nocSpan, nocChipSpan);
        VerifyOrExit(err == CHIP_NO_ERROR && nocChipSpan.size() > 0, nonDefaultStatus = Status::Failure);

        err = fabricTable.AddNewPendingFabricWithOperationalKeystore(nocChipSpan, ICACValue, adminVendorId, &fabricIndex);
        VerifyOrExit(err == CHIP_NO_ERROR, jointFabricResponse = ConvertToJointFabricResponseStatus(err));

        newFabricInfo = fabricTable.FindFabricWithIndex(fabricIndex);
        VerifyOrExit(newFabricInfo != nullptr, nonDefaultStatus = Status::Failure);

        // Set the Identity Protection Key (IPK)
        // The IPK SHALL be the operational group key under GroupKeySetID of 0
        keyset.keyset_id                = Credentials::GroupDataProvider::kIdentityProtectionKeySetId;
        keyset.policy                   = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst;
        keyset.num_keys_used            = 1;
        keyset.epoch_keys[0].start_time = 0;
        memcpy(keyset.epoch_keys[0].key, defaultIpkSpan.data(), defaultIpkSpan.size());

        err = newFabricInfo->GetCompressedFabricIdBytes(compressed_fabric_id);
        VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

        err = groupDataProvider->SetKeySet(fabricIndex, compressed_fabric_id, keyset);
        VerifyOrExit(err == CHIP_NO_ERROR, jointFabricResponse = ConvertToJointFabricResponseStatus(err));

        /**
         * . If the current secure session was established with PASE,
         *   the receiver SHALL:
         *     .. Augment the secure session context with the `FabricIndex` generated above
         *        such that subsequent interactions have the proper accessing fabric.
         *
         * . If the current secure session was established with CASE, subsequent configuration
         *   of the newly installed Fabric requires the opening of a new CASE session from the
         *   Administrator from the Fabric just installed. This Administrator is the one listed
         *   in the `caseAdminSubject` argument.
         *
         */
        if (secureSession->GetSecureSessionType() == SecureSession::Type::kPASE)
        {
            err = secureSession->AdoptFabricIndex(fabricIndex);
            VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);
        }

        // Creating the initial ACL must occur after the PASE session has adopted the fabric index
        // (see above) so that the concomitant event, which is fabric scoped, is properly handled.
        err = CreateAccessControlEntryForNewFabricAdministrator(commandObj->GetSubjectDescriptor(), fabricIndex,
                                                                commandData.caseAdminSubject);
        VerifyOrExit(err != CHIP_ERROR_INTERNAL, nonDefaultStatus = Status::Failure);
        VerifyOrExit(err == CHIP_NO_ERROR, jointFabricResponse = ConvertToJointFabricResponseStatus(err));

        failSafeContext.SetAddNocCommandInvoked(fabricIndex);

        fabricTable.CommitPendingFabricData();

        ChipLogProgress(Zcl, "JointFabricPki: Joint Fabric Index: %" PRIu8, fabricIndex);
    }

    // We might have a new operational identity, so we should start advertising
    // it right away.  Also, we need to withdraw our old operational identity.
    // So we need to StartServer() here.
    app::DnssdServer::Instance().StartServer();

exit:
    // We have an ICA response
    if (nonDefaultStatus == Status::Success)
    {
        SendJointFabricResponse(commandObj, commandPath, jointFabricResponse, ecosystemFabricIndex, CharSpan());
        // Failed to add ICA
        if (jointFabricResponse != JointFabricStatusEnum::kOk)
        {
            ChipLogError(Zcl, "JointFabricPki: Failed add ICA request (err=%" CHIP_ERROR_FORMAT ") with JointFabric error %d",
                         err.Format(), to_underlying(jointFabricResponse));
        }
        // Success
        else
        {
            ChipLogProgress(Zcl, "JointFabricPki: UpdateICA successful.");
        }
    }
    // No NOC response - Failed constraints
    else
    {
        commandObj->AddStatus(commandPath, nonDefaultStatus);
        ChipLogError(Zcl, "JointFabricPki: Failed SignNOCIssuerResponse request with IM error 0x%02x",
                     to_underlying(nonDefaultStatus));
    }

    return true;
}
