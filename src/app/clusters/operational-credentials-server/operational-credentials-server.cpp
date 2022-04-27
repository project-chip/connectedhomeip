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
 * @brief Implementation for the Operational Credentials Cluster
 ***************************************************************************/

#include <access/AccessControl.h>
#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <string.h>
#include <trace/trace.h>
#if CHIP_CRYPTO_HSM
#include <crypto/hsm/CHIPCryptoPALHsm.h>
#endif

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;
using namespace chip::Protocols::InteractionModel;

namespace {

void SendNOCResponse(app::CommandHandler * commandObj, const ConcreteCommandPath & path, OperationalCertStatus status,
                     uint8_t index, const CharSpan & debug_text);
OperationalCertStatus ConvertToNOCResponseStatus(CHIP_ERROR err);

constexpr uint8_t kDACCertificate = 1;
constexpr uint8_t kPAICertificate = 2;

CHIP_ERROR CreateAccessControlEntryForNewFabricAdministrator(FabricIndex fabricIndex, NodeId subject)
{
    Access::AccessControl::Entry entry;
    ReturnErrorOnFailure(Access::GetAccessControl().PrepareEntry(entry));
    ReturnErrorOnFailure(entry.SetFabricIndex(fabricIndex));
    ReturnErrorOnFailure(entry.SetPrivilege(Access::Privilege::kAdminister));
    ReturnErrorOnFailure(entry.SetAuthMode(Access::AuthMode::kCase));
    ReturnErrorOnFailure(entry.AddSubject(nullptr, subject));
    ReturnErrorOnFailure(Access::GetAccessControl().CreateEntry(nullptr, entry));

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: ACL entry created for Fabric %X CASE Admin NodeId 0x" ChipLogFormatX64,
                   fabricIndex, ChipLogValueX64(subject));

    // TODO: event notification for newly created ACL entry

    return CHIP_NO_ERROR;
}

class OperationalCredentialsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OperationalCredentials cluster on all endpoints.
    OperationalCredentialsAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), Clusters::OperationalCredentials::Id)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadNOCs(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedFabrics(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadCommissionedFabrics(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFabricsList(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadRootCertificates(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR OperationalCredentialsAttrAccess::ReadNOCs(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto accessingFabricIndex = aEncoder.AccessingFabricIndex();

    return aEncoder.EncodeList([accessingFabricIndex](const auto & encoder) -> CHIP_ERROR {
        for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            Clusters::OperationalCredentials::Structs::NOCStruct::Type noc;

            if (!fabricInfo.IsInitialized())
                continue;

            noc.fabricIndex = fabricInfo.GetFabricIndex();

            if (accessingFabricIndex == fabricInfo.GetFabricIndex())
            {
                ByteSpan icac;

                ReturnErrorOnFailure(fabricInfo.GetNOCCert(noc.noc));
                ReturnErrorOnFailure(fabricInfo.GetICACert(icac));

                if (!icac.empty())
                {
                    noc.icac.SetNonNull(icac);
                }
            }

            ReturnErrorOnFailure(encoder.Encode(noc));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR OperationalCredentialsAttrAccess::ReadSupportedFabrics(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    uint8_t fabricCount = CHIP_CONFIG_MAX_FABRICS;

    return aEncoder.Encode(fabricCount);
}

CHIP_ERROR OperationalCredentialsAttrAccess::ReadCommissionedFabrics(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(Server::GetInstance().GetFabricTable().FabricCount());
}

CHIP_ERROR OperationalCredentialsAttrAccess::ReadFabricsList(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            if (!fabricInfo.IsInitialized())
                continue;

            Clusters::OperationalCredentials::Structs::FabricDescriptor::Type fabricDescriptor;

            fabricDescriptor.fabricIndex = fabricInfo.GetFabricIndex();
            fabricDescriptor.nodeId      = fabricInfo.GetPeerId().GetNodeId();
            fabricDescriptor.vendorId    = fabricInfo.GetVendorId();
            fabricDescriptor.fabricId    = fabricInfo.GetFabricId();

            fabricDescriptor.label = fabricInfo.GetFabricLabel();

            Credentials::P256PublicKeySpan pubKey;
            ReturnErrorOnFailure(fabricInfo.GetRootPubkey(pubKey));
            fabricDescriptor.rootPublicKey = pubKey;

            ReturnErrorOnFailure(encoder.Encode(fabricDescriptor));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR OperationalCredentialsAttrAccess::ReadRootCertificates(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
        {
            ByteSpan cert;

            if (!fabricInfo.IsInitialized())
                continue;

            ReturnErrorOnFailure(fabricInfo.GetRootCert(cert));
            ReturnErrorOnFailure(encoder.Encode(cert));
        }

        return CHIP_NO_ERROR;
    });
}

OperationalCredentialsAttrAccess gAttrAccess;

CHIP_ERROR OperationalCredentialsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Clusters::OperationalCredentials::Id);

    switch (aPath.mAttributeId)
    {
    case Attributes::NOCs::Id: {
        return ReadNOCs(aPath.mEndpointId, aEncoder);
    }
    case Attributes::SupportedFabrics::Id: {
        return ReadSupportedFabrics(aPath.mEndpointId, aEncoder);
    }
    case Attributes::CommissionedFabrics::Id: {
        return ReadCommissionedFabrics(aPath.mEndpointId, aEncoder);
    }
    case Attributes::Fabrics::Id: {
        return ReadFabricsList(aPath.mEndpointId, aEncoder);
    }
    case Attributes::TrustedRootCertificates::Id: {
        return ReadRootCertificates(aPath.mEndpointId, aEncoder);
    }
    case Attributes::CurrentFabricIndex::Id: {
        return aEncoder.Encode(aEncoder.AccessingFabricIndex());
    }
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

// Utility to compute Attestation signature for NOCSRResponse and AttestationResponse
CHIP_ERROR ComputeAttestationSignature(app::CommandHandler * commandObj,
                                       Credentials::DeviceAttestationCredentialsProvider * dacProvider, const ByteSpan & payload,
                                       MutableByteSpan & signatureSpan)
{
    uint8_t md[Crypto::kSHA256_Hash_Length];
    MutableByteSpan messageDigestSpan(md);

    VerifyOrReturnError(signatureSpan.size() >= Crypto::P256ECDSASignature::Capacity(), CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    Hash_SHA256_stream hashStream;
    ReturnErrorOnFailure(hashStream.Begin());
    ReturnErrorOnFailure(hashStream.AddData(payload));
    ReturnErrorOnFailure(hashStream.AddData(attestationChallenge));
    ReturnErrorOnFailure(hashStream.Finish(messageDigestSpan));

    ReturnErrorOnFailure(dacProvider->SignWithDeviceAttestationKey(messageDigestSpan, signatureSpan));
    VerifyOrReturnError(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

FabricInfo * RetrieveCurrentFabric(CommandHandler * aCommandHandler)
{
    FabricIndex index = aCommandHandler->GetAccessingFabricIndex();
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Finding fabric with fabricIndex 0x%x", static_cast<unsigned>(index));
    return Server::GetInstance().GetFabricTable().FindFabricWithIndex(index);
}

CHIP_ERROR DeleteFabricFromTable(FabricIndex fabricIndex)
{
    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().Delete(fabricIndex));

    // We need to withdraw the advertisement for the now-removed fabric, so need
    // to restart advertising altogether.
    app::DnssdServer::Instance().StartServer();

    return CHIP_NO_ERROR;
}

void CleanupFabricContext(SessionManager & sessionMgr, FabricIndex fabricIndex)
{
    InteractionModelEngine::GetInstance()->CloseTransactionsFromFabricIndex(fabricIndex);
    sessionMgr.ExpireAllPairingsForFabric(fabricIndex);
}

void FailSafeCleanup(const chip::DeviceLayer::ChipDeviceEvent * event)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to FailSafeCleanup");

    FabricIndex fabricIndex = event->FailSafeTimerExpired.PeerFabricIndex;

    // If an AddNOC or UpdateNOC command has been successfully invoked, terminate all CASE sessions associated with the Fabric
    // whose Fabric Index is recorded in the Fail-Safe context (see ArmFailSafe Command) by clearing any associated Secure
    // Session Context at the Server.
    if (event->FailSafeTimerExpired.AddNocCommandHasBeenInvoked || event->FailSafeTimerExpired.UpdateNocCommandHasBeenInvoked)
    {
        CASESessionManager * caseSessionManager = Server::GetInstance().GetCASESessionManager();
        if (caseSessionManager)
        {
            FabricInfo * fabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
            VerifyOrReturn(fabricInfo != nullptr);

            caseSessionManager->ReleaseSessionsForFabric(fabricInfo->GetCompressedId());
        }

        SessionManager & sessionMgr = Server::GetInstance().GetSecureSessionManager();
        CleanupFabricContext(sessionMgr, fabricIndex);
    }

    // If an AddNOC command had been successfully invoked, achieve the equivalent effect of invoking the RemoveFabric command
    // against the Fabric Index stored in the Fail-Safe Context for the Fabric Index that was the subject of the AddNOC
    // command.
    if (event->FailSafeTimerExpired.AddNocCommandHasBeenInvoked)
    {
        DeleteFabricFromTable(fabricIndex);
    }

    // If an UpdateNOC command had been successfully invoked, revert the state of operational key pair, NOC and ICAC for that
    // Fabric to the state prior to the Fail-Safe timer being armed, for the Fabric Index that was the subject of the UpdateNOC
    // command.
    if (event->FailSafeTimerExpired.UpdateNocCommandHasBeenInvoked)
    {
        // TODO: Revert the state of operational key pair, NOC and ICAC
    }
}

void OnPlatformEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        FailSafeCleanup(event);
    }
}

} // anonymous namespace

// As per specifications section 11.22.5.1. Constant RESP_MAX
constexpr size_t kMaxRspLen = 900;

void fabricListChanged()
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Call to fabricListChanged");

    // Currently, we only manage FabricsList attribute in endpoint 0, OperationalCredentials cluster is always required to be on
    // EP0.
    MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
    MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id,
                                           OperationalCredentials::Attributes::CommissionedFabrics::Id);
}

// TODO: The code currently has two sources of truths for fabrics, the fabricInfo table + the attributes. There should only be one,
// the attributes list. Currently the attributes are not persisted so we are keeping the fabric table to have the
// fabrics/admrins be persisted. Once attributes are persisted, there should only be one sorce of truth, the attributes list and
// only that should be modifed to perosst/read/write fabrics.
// TODO: Once attributes are persisted, implement reading/writing/manipulation fabrics around that and remove fabricTable
// logic.
class OpCredsFabricTableDelegate : public FabricTableDelegate
{

    // Gets called when a fabric is deleted from KVS store
    void OnFabricDeletedFromStorage(CompressedFabricId compressedFabricId, FabricIndex fabricIndex) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Fabric index 0x%x was deleted from fabric storage.",
                       static_cast<unsigned>(fabricIndex));
        fabricListChanged();

        // The Leave event SHOULD be emitted by a Node prior to permanently
        // leaving the Fabric.
        for (auto endpoint : EnabledEndpointsWithServerCluster(Basic::Id))
        {
            // If Basic cluster is implemented on this endpoint
            Basic::Events::Leave::Type event;
            EventNumber eventNumber;

            if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
            {
                ChipLogError(Zcl, "OpCredsFabricTableDelegate: Failed to record Leave event");
            }
        }
    }

    // Gets called when a fabric is loaded into the FabricTable from storage
    void OnFabricRetrievedFromStorage(FabricInfo * fabric) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Fabric index 0x%x was retrieved from storage. FabricId 0x" ChipLogFormatX64
                       ", NodeId 0x" ChipLogFormatX64 ", VendorId 0x%04X",
                       static_cast<unsigned>(fabric->GetFabricIndex()), ChipLogValueX64(fabric->GetFabricId()),
                       ChipLogValueX64(fabric->GetPeerId().GetNodeId()), fabric->GetVendorId());
        fabricListChanged();
    }

    // Gets called when a fabric in FabricTable is persisted to storage
    void OnFabricPersistedToStorage(FabricInfo * fabric) override
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG,
                       "OpCreds: Fabric  index 0x%x was persisted to storage. FabricId " ChipLogFormatX64
                       ", NodeId " ChipLogFormatX64 ", VendorId 0x%04X",
                       static_cast<unsigned>(fabric->GetFabricIndex()), ChipLogValueX64(fabric->GetFabricId()),
                       ChipLogValueX64(fabric->GetPeerId().GetNodeId()), fabric->GetVendorId());
        fabricListChanged();
    }
};

OpCredsFabricTableDelegate gFabricDelegate;

void MatterOperationalCredentialsPluginServerInitCallback(void)
{
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Initiating OpCreds cluster by writing fabrics list from fabric table.");

    registerAttributeAccessOverride(&gAttrAccess);

    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);

    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);
}

namespace {
class FabricCleanupExchangeDelegate : public chip::Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        return CHIP_NO_ERROR;
    }
    void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override {}
    void OnExchangeClosing(chip::Messaging::ExchangeContext * ec) override
    {
        SessionManager * sessionManager = ec->GetExchangeMgr()->GetSessionManager();
        FabricIndex currentFabricIndex  = ec->GetSessionHandle()->GetFabricIndex();
        CleanupFabricContext(*sessionManager, currentFabricIndex);
    }
};

FabricCleanupExchangeDelegate gFabricCleanupExchangeDelegate;

} // namespace

bool emberAfOperationalCredentialsClusterRemoveFabricCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::RemoveFabric::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("RemoveFabric", "OperationalCredentials");
    auto & fabricBeingRemoved = commandData.fabricIndex;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: RemoveFabric"); // TODO: Generate emberAfFabricClusterPrintln

    CHIP_ERROR err = DeleteFabricFromTable(fabricBeingRemoved);
    SuccessOrExit(err);

exit:
    fabricListChanged();
    // Not using ConvertToNOCResponseStatus here because it's pretty
    // AddNOC/UpdateNOC specific.
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        SendNOCResponse(commandObj, commandPath, OperationalCertStatus::kInvalidFabricIndex, fabricBeingRemoved, CharSpan());
    }
    else if (err != CHIP_NO_ERROR)
    {
        // We have no idea what happened; just report failure.
        StatusIB status(err);
        commandObj->AddStatus(commandPath, status.mStatus);
    }
    else
    {
        SendNOCResponse(commandObj, commandPath, OperationalCertStatus::kSuccess, fabricBeingRemoved, CharSpan());

        chip::Messaging::ExchangeContext * ec = commandObj->GetExchangeContext();
        FabricIndex currentFabricIndex        = commandObj->GetAccessingFabricIndex();
        if (currentFabricIndex == fabricBeingRemoved)
        {
            // If the current fabric is being removed, don't expire the secure sessions immediately as they are
            // still needed to send a pending message generated by emberAfSendImmediateDefaultResponse().
            // Hijack the exchange delegate here (as no more messages should be received on this exchange),
            // and wait for it to close, before expiring the secure sessions for the fabric. Also, suppress MRP
            // usage since the MRP engine still holds an exchange even after it's closed, and the engine references
            // the associated session object.
            // TODO: https://github.com/project-chip/connectedhomeip/issues/9642
            ec->SetAutoRequestAck(false);
            ec->SetDelegate(&gFabricCleanupExchangeDelegate);
        }
        else
        {
            SessionManager * sessionManager = ec->GetExchangeMgr()->GetSessionManager();
            CleanupFabricContext(*sessionManager, fabricBeingRemoved);
        }
    }
    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
                                                                   const Commands::UpdateFabricLabel::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("UpdateFabricLabel", "OperationalCredentials");
    auto & Label        = commandData.label;
    auto ourFabricIndex = commandObj->GetAccessingFabricIndex();

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateFabricLabel");

    for (auto & fabricInfo : Server::GetInstance().GetFabricTable())
    {
        if (fabricInfo.GetFabricLabel().data_equal(Label) && fabricInfo.GetFabricIndex() != ourFabricIndex)
        {
            ChipLogError(Zcl, "Fabric label already in use");
            SendNOCResponse(commandObj, commandPath, OperationalCertStatus::kLabelConflict, ourFabricIndex, CharSpan());
            return true;
        }
    }

    CHIP_ERROR err;

    // Fetch current fabric
    FabricInfo * fabric = RetrieveCurrentFabric(commandObj);
    VerifyOrExit(fabric != nullptr, err = CHIP_ERROR_INVALID_FABRIC_ID);

    // Set Label on fabric
    err = fabric->SetFabricLabel(Label);
    SuccessOrExit(err);

    // Persist updated fabric
    err = Server::GetInstance().GetFabricTable().Store(fabric->GetFabricIndex());
    SuccessOrExit(err);

exit:
    fabricListChanged();

    SendNOCResponse(commandObj, commandPath, ConvertToNOCResponseStatus(err), ourFabricIndex, CharSpan());
    return true;
}

namespace {

// TODO: Manage ephemeral RCAC/ICAC/NOC storage to avoid a full FabricInfo being needed here.
FabricInfo gFabricBeingCommissioned;

void SendNOCResponse(app::CommandHandler * commandObj, const ConcreteCommandPath & path, OperationalCertStatus status,
                     uint8_t index, const CharSpan & debug_text)
{
    Commands::NOCResponse::Type payload;
    payload.statusCode = status;
    if (status == OperationalCertStatus::kSuccess)
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

OperationalCertStatus ConvertToNOCResponseStatus(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        return OperationalCertStatus::kSuccess;
    }
    if (err == CHIP_ERROR_INVALID_PUBLIC_KEY)
    {
        return OperationalCertStatus::kInvalidPublicKey;
    }
    if (err == CHIP_ERROR_INVALID_FABRIC_ID || err == CHIP_ERROR_WRONG_NODE_ID)
    {
        return OperationalCertStatus::kInvalidNodeOpId;
    }
    if (err == CHIP_ERROR_CA_CERT_NOT_FOUND || err == CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED ||
        err == CHIP_ERROR_CERT_PATH_TOO_LONG || err == CHIP_ERROR_CERT_USAGE_NOT_ALLOWED || err == CHIP_ERROR_CERT_EXPIRED ||
        err == CHIP_ERROR_CERT_NOT_VALID_YET || err == CHIP_ERROR_UNSUPPORTED_CERT_FORMAT ||
        err == CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE || err == CHIP_ERROR_CERT_LOAD_FAILED || err == CHIP_ERROR_CERT_NOT_TRUSTED ||
        err == CHIP_ERROR_WRONG_CERT_DN)
    {
        return OperationalCertStatus::kInvalidNOC;
    }
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return OperationalCertStatus::kTableFull;
    }
    if (err == CHIP_ERROR_FABRIC_EXISTS)
    {
        return OperationalCertStatus::kFabricConflict;
    }

    return OperationalCertStatus::kInvalidNOC;
}

} // namespace

bool emberAfOperationalCredentialsClusterAddNOCCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::AddNOC::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("AddNOC", "OperationalCredentials");
    auto & NOCValue          = commandData.NOCValue;
    auto & ICACValue         = commandData.ICACValue;
    auto & adminVendorId     = commandData.adminVendorId;
    auto & ipkValue          = commandData.IPKValue;
    auto * groupDataProvider = Credentials::GetGroupDataProvider();
    auto nocResponse         = OperationalCertStatus::kSuccess;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;
    Credentials::GroupDataProvider::KeySet keyset;
    FabricInfo * newFabricInfo = nullptr;

    auto * secureSession = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession();

    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Received AddNOC command");

    if (nullptr == groupDataProvider)
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::Failure));
        return true;
    }

    FailSafeContext & failSafeContext = DeviceControlServer::DeviceControlSvr().GetFailSafeContext();

    if (!failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()))
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::UnsupportedAccess));
        return true;
    }

    if (failSafeContext.NocCommandHasBeenInvoked())
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::ConstraintError));
        return true;
    }

    err = gFabricBeingCommissioned.SetNOCCert(NOCValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = gFabricBeingCommissioned.SetICACert(ICACValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    gFabricBeingCommissioned.SetVendorId(adminVendorId);

    err = Server::GetInstance().GetFabricTable().AddNewFabric(gFabricBeingCommissioned, &fabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // The Fabric Index associated with the armed fail-safe context SHALL be updated to match the Fabric
    // Index just allocated.
    err = failSafeContext.SetAddNocCommandInvoked(fabricIndex);
    if (err != CHIP_NO_ERROR)
    {
        Server::GetInstance().GetFabricTable().Delete(fabricIndex);
        nocResponse = ConvertToNOCResponseStatus(err);
        SuccessOrExit(err);
    }

    // Keep this after other possible failures, so it doesn't need to be rolled back in case of
    // subsequent failures. This should only typically fail if there is no space for the new entry.
    err = CreateAccessControlEntryForNewFabricAdministrator(fabricIndex, commandData.caseAdminNode);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // Set the Identity Protection Key (IPK)
    VerifyOrExit(ipkValue.size() == Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES,
                 nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INVALID_ARGUMENT));
    // The IPK SHALL be the operational group key under GroupKeySetID of 0
    keyset.keyset_id     = Credentials::GroupDataProvider::kIdentityProtectionKeySetId;
    keyset.policy        = GroupKeyManagement::GroupKeySecurityPolicy::kTrustFirst;
    keyset.num_keys_used = 1;
    memcpy(keyset.epoch_keys[0].key, ipkValue.data(), Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);

    newFabricInfo = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrExit(newFabricInfo != nullptr, nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INTERNAL));
    err = newFabricInfo->GetCompressedId(compressed_fabric_id);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = groupDataProvider->SetKeySet(fabricIndex, compressed_fabric_id, keyset);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    /**
     * . If the current secure session was established with PASE,
     *   the receiver SHALL:
     *     .. Augment the secure session context with the `FabricIndex` generated above
     *        such that subsequent interactions have the proper accessing fabric.
     *
     * . If the current secure session was established with CASE, subsequent configuration
     *   of the newly installed Fabric requires the opening of a new CASE session from the
     *   Administrator from the Fabric just installed. This Administrator is the one listed
     *   in the `CaseAdminNode` argument.
     *
     */
    if (secureSession->GetSecureSessionType() == SecureSession::Type::kPASE)
    {
        err = secureSession->AdoptFabricIndex(fabricIndex);
        VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));
    }

    // We might have a new operational identity, so we should start advertising it right away.
    app::DnssdServer::Instance().AdvertiseOperational();

exit:

    gFabricBeingCommissioned.Reset();
    SendNOCResponse(commandObj, commandPath, nocResponse, fabricIndex, CharSpan());

    if (nocResponse != OperationalCertStatus::kSuccess)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddNOC request. Status %d", to_underlying(nocResponse));
    }
    else
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: successfully created fabric index %u via AddNOC",
                       static_cast<unsigned>(fabricIndex));
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateNOCCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::UpdateNOC::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("UpdateNOC", "OperationalCredentials");
    auto & NOCValue  = commandData.NOCValue;
    auto & ICACValue = commandData.ICACValue;

    auto nocResponse = OperationalCertStatus::kSuccess;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: an administrator requested an update to the NOC");

    FailSafeContext & failSafeContext = DeviceControlServer::DeviceControlSvr().GetFailSafeContext();

    if (!failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()))
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::UnsupportedAccess));
        return true;
    }

    if (failSafeContext.NocCommandHasBeenInvoked())
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::ConstraintError));
        return true;
    }

    // Fetch current fabric
    FabricInfo * fabric = RetrieveCurrentFabric(commandObj);
    VerifyOrExit(fabric != nullptr, nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INVALID_FABRIC_ID));

    // Flag on the fail-safe context that the UpdateNOC command was invoked.
    err = failSafeContext.SetUpdateNocCommandInvoked();
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = fabric->SetNOCCert(NOCValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    err = fabric->SetICACert(ICACValue);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    fabricIndex = fabric->GetFabricIndex();

    // We might have a new operational identity, so we should start advertising
    // it right away.  Also, we need to withdraw our old operational identity.
    // So we need to StartServer() here.
    app::DnssdServer::Instance().StartServer();

exit:

    SendNOCResponse(commandObj, commandPath, nocResponse, fabricIndex, CharSpan());

    if (nocResponse != OperationalCertStatus::kSuccess)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed UpdateNOC request. Status %d", to_underlying(nocResponse));
    }
    else
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: UpdateNOC successful.");
    }

    return true;
}

bool emberAfOperationalCredentialsClusterCertificateChainRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CertificateChainRequest::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("CertificateChainRequest", "OperationalCredentials");
    auto & certificateType = commandData.certificateType;

    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t derBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan derBufSpan(derBuf);

    Commands::CertificateChainResponse::Type response;

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (certificateType == kDACCertificate)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Certificate Chain request received for DAC");
        SuccessOrExit(err = dacProvider->GetDeviceAttestationCert(derBufSpan));
    }
    else if (certificateType == kPAICertificate)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Certificate Chain request received for PAI");
        SuccessOrExit(err = dacProvider->GetProductAttestationIntermediateCert(derBufSpan));
    }
    else
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Certificate Chain request received for unknown type: %d",
                       static_cast<int>(certificateType));
        SuccessOrExit(err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    response.certificate = derBufSpan;
    commandObj->AddResponse(commandPath, response);

exit:
    if (err != CHIP_NO_ERROR)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed CertificateChainRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAttestationRequestCallback(app::CommandHandler * commandObj,
                                                                    const app::ConcreteCommandPath & commandPath,
                                                                    const Commands::AttestationRequest::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("AttestationRequest", "OperationalCredentials");
    auto & attestationNonce = commandData.attestationNonce;

    CHIP_ERROR err = CHIP_NO_ERROR;
    Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen = 0;
    MutableByteSpan attestationElementsSpan;
    uint8_t certDeclBuf[Credentials::kMaxCMSSignedCDMessage]; // Sized to hold the example certificate declaration with 100 PIDs.
                                                              // See DeviceAttestationCredsExample
    MutableByteSpan certDeclSpan(certDeclBuf);
    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: received an AttestationRequest");

    // TODO: retrieve vendor information to populate the fields below.
    uint32_t timestamp = 0;
    // TODO: Also retrieve and use firmware Information
    const ByteSpan kEmptyFirmwareInfo;
    Credentials::DeviceAttestationVendorReservedConstructor emptyVendorReserved(nullptr, 0);

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(attestationNonce.size() == Credentials::kExpectedAttestationNonceSize, err = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(err = dacProvider->GetCertificationDeclaration(certDeclSpan));

    attestationElementsLen = TLV::EstimateStructOverhead(certDeclSpan.size(), attestationNonce.size(), sizeof(uint64_t) * 8);

    VerifyOrExit(attestationElements.Alloc(attestationElementsLen), err = CHIP_ERROR_NO_MEMORY);
    attestationElementsSpan = MutableByteSpan{ attestationElements.Get(), attestationElementsLen };
    SuccessOrExit(err = Credentials::ConstructAttestationElements(certDeclSpan, attestationNonce, timestamp, kEmptyFirmwareInfo,
                                                                  emptyVendorReserved, attestationElementsSpan));

    VerifyOrExit(attestationElementsSpan.size() <= kMaxRspLen, err = CHIP_ERROR_INTERNAL);

    // Prepare response payload with signature
    {
        Commands::AttestationResponse::Type response;

        Crypto::P256ECDSASignature signature;
        MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };
        SuccessOrExit(err = ComputeAttestationSignature(commandObj, dacProvider, attestationElementsSpan, signatureSpan));

        response.attestationElements = attestationElementsSpan;
        response.signature           = signatureSpan;
        commandObj->AddResponse(commandPath, response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AttestationRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterCSRRequestCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::CSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("CSRRequest", "OperationalCredentials");
    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has requested a CSR");

    CHIP_ERROR err = CHIP_NO_ERROR;
    Platform::ScopedMemoryBuffer<uint8_t> csr;
    chip::Platform::ScopedMemoryBuffer<uint8_t> nocsrElements;
    MutableByteSpan nocsrElementsSpan;

    FailSafeContext & failSafeContext = DeviceControlServer::DeviceControlSvr().GetFailSafeContext();

    if (!failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()))
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::UnsupportedAccess));
        return true;
    }

    auto & CSRNonce = commandData.CSRNonce;

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(CSRNonce.size() == Credentials::kExpectedAttestationNonceSize, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Prepare NOCSRElements structure
    {
#ifdef ENABLE_HSM_CASE_OPS_KEY
        Crypto::P256KeypairHSM keypair;
#else
        Crypto::P256Keypair keypair;
#endif
        size_t csrLength           = Crypto::kMAX_CSR_Length;
        size_t nocsrLengthEstimate = 0;
        ByteSpan kNoVendorReserved;

        // Always generate a new operational keypair for any new CSRRequest
        if (gFabricBeingCommissioned.GetOperationalKey() != nullptr)
        {
            gFabricBeingCommissioned.GetOperationalKey()->Clear();
        }

#ifdef ENABLE_HSM_CASE_OPS_KEY
        keypair.CreateOperationalKey(gFabricBeingCommissioned.GetFabricIndex());
#else
        keypair.Initialize();
#endif
        SuccessOrExit(err = gFabricBeingCommissioned.SetOperationalKeypair(&keypair));

        // Generate the actual CSR from the ephemeral key
        VerifyOrExit(csr.Alloc(csrLength), err = CHIP_ERROR_NO_MEMORY);

        err = gFabricBeingCommissioned.GetOperationalKey()->NewCertificateSigningRequest(csr.Get(), csrLength);
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: NewCertificateSigningRequest returned %" CHIP_ERROR_FORMAT, err.Format());
        SuccessOrExit(err);
        VerifyOrExit(csrLength <= Crypto::kMAX_CSR_Length, err = CHIP_ERROR_INTERNAL);

        // Encode the NOCSR elements with the CSR and Nonce
        nocsrLengthEstimate = TLV::EstimateStructOverhead(csrLength,       // CSR buffer
                                                          CSRNonce.size(), // CSR Nonce
                                                          0u               // no vendor reserved data
        );

        VerifyOrExit(nocsrElements.Alloc(nocsrLengthEstimate), err = CHIP_ERROR_NO_MEMORY);

        nocsrElementsSpan = MutableByteSpan{ nocsrElements.Get(), nocsrLengthEstimate };
        SuccessOrExit(err = Credentials::ConstructNOCSRElements(ByteSpan{ csr.Get(), csrLength }, CSRNonce, kNoVendorReserved,
                                                                kNoVendorReserved, kNoVendorReserved, nocsrElementsSpan));
        VerifyOrExit(nocsrElementsSpan.size() <= kMaxRspLen, err = CHIP_ERROR_INTERNAL);
    }

    // Prepare response payload with signature
    {
        Commands::CSRResponse::Type response;

        Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

        Crypto::P256ECDSASignature signature;
        MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };
        SuccessOrExit(err = ComputeAttestationSignature(commandObj, dacProvider, nocsrElementsSpan, signatureSpan));

        response.NOCSRElements        = nocsrElementsSpan;
        response.attestationSignature = signatureSpan;
        commandObj->AddResponse(commandPath, response);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        // TODO: Replace this error handling with fail-safe since it's not transactional against root certs
        gFabricBeingCommissioned.Reset();
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed CSRRequest: %s", ErrorStr(err));
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_FAILURE);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAddTrustedRootCertificateCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::AddTrustedRootCertificate::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("AddTrustedRootCertificate", "OperationalCredentials");
    auto & RootCertificate = commandData.rootCertificate;

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: commissioner has added a trusted root Cert");

    FailSafeContext & failSafeContext = DeviceControlServer::DeviceControlSvr().GetFailSafeContext();

    if (!failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()))
    {
        LogErrorOnFailure(commandObj->AddStatus(commandPath, Status::UnsupportedAccess));
        return true;
    }

    // TODO: Ensure we do not duplicate roots in storage, and detect "same key, different cert" errors
    // TODO: Validate cert signature prior to setting.
    VerifyOrExit(gFabricBeingCommissioned.SetRootCert(RootCertificate) == CHIP_NO_ERROR, status = EMBER_ZCL_STATUS_INVALID_FIELD);

exit:
    emberAfSendImmediateDefaultResponse(status);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        gFabricBeingCommissioned.Reset();
        emberAfPrintln(EMBER_AF_PRINT_DEBUG, "OpCreds: Failed AddTrustedRootCert request.");
    }
    else
    {
        MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::TrustedRootCertificates::Id);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterRemoveTrustedRootCertificateCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::RemoveTrustedRootCertificate::DecodableType & commandData)
{
    MATTER_TRACE_EVENT_SCOPE("RemoveTrustedRootCertificate", "OperationalCredentials");
    // TODO: Implement the logic for RemoveTrustedRootCertificate
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);

    MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                           OperationalCredentials::Attributes::TrustedRootCertificates::Id);

    return true;
}
