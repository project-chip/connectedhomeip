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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
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
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/PeerId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <string.h>
#include <tracing/macros.h>

using namespace chip;
using namespace ::chip::Transport;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;
using namespace chip::Credentials;
using namespace chip::Protocols::InteractionModel;

namespace {

void SendNOCResponse(app::CommandHandler * commandObj, const ConcreteCommandPath & path, NodeOperationalCertStatusEnum status,
                     uint8_t index, const CharSpan & debug_text);
NodeOperationalCertStatusEnum ConvertToNOCResponseStatus(CHIP_ERROR err);

constexpr auto kDACCertificate = CertificateChainTypeEnum::kDACCertificate;
constexpr auto kPAICertificate = CertificateChainTypeEnum::kPAICertificate;

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
        const auto & fabricTable = Server::GetInstance().GetFabricTable();
        for (const auto & fabricInfo : fabricTable)
        {
            Clusters::OperationalCredentials::Structs::NOCStruct::Type noc;
            uint8_t nocBuf[kMaxCHIPCertLength];
            uint8_t icacBuf[kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            MutableByteSpan icacSpan{ icacBuf };
            FabricIndex fabricIndex = fabricInfo.GetFabricIndex();

            noc.fabricIndex = fabricIndex;

            if (accessingFabricIndex == fabricIndex)
            {

                ReturnErrorOnFailure(fabricTable.FetchNOCCert(fabricIndex, nocSpan));
                ReturnErrorOnFailure(fabricTable.FetchICACert(fabricIndex, icacSpan));

                noc.noc = nocSpan;
                if (!icacSpan.empty())
                {
                    noc.icac.SetNonNull(icacSpan);
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
        const auto & fabricTable = Server::GetInstance().GetFabricTable();

        for (const auto & fabricInfo : fabricTable)
        {
            Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricDescriptor;
            FabricIndex fabricIndex = fabricInfo.GetFabricIndex();

            fabricDescriptor.fabricIndex = fabricIndex;
            fabricDescriptor.nodeID      = fabricInfo.GetPeerId().GetNodeId();
            fabricDescriptor.vendorID    = fabricInfo.GetVendorId();
            fabricDescriptor.fabricID    = fabricInfo.GetFabricId();

            fabricDescriptor.label = fabricInfo.GetFabricLabel();

            Crypto::P256PublicKey pubKey;
            ReturnErrorOnFailure(fabricTable.FetchRootPubkey(fabricIndex, pubKey));
            fabricDescriptor.rootPublicKey = ByteSpan{ pubKey.ConstBytes(), pubKey.Length() };

            ReturnErrorOnFailure(encoder.Encode(fabricDescriptor));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR OperationalCredentialsAttrAccess::ReadRootCertificates(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    // It is OK to have duplicates.
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        const auto & fabricTable = Server::GetInstance().GetFabricTable();

        for (const auto & fabricInfo : fabricTable)
        {
            uint8_t certBuf[kMaxCHIPCertLength];
            MutableByteSpan cert{ certBuf };
            ReturnErrorOnFailure(fabricTable.FetchRootCert(fabricInfo.GetFabricIndex(), cert));
            ReturnErrorOnFailure(encoder.Encode(ByteSpan{ cert }));
        }

        {
            uint8_t certBuf[kMaxCHIPCertLength];
            MutableByteSpan cert{ certBuf };
            CHIP_ERROR err = fabricTable.FetchPendingNonFabricAssociatedRootCert(cert);
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                // No pending root cert, do nothing
            }
            else if (err != CHIP_NO_ERROR)
            {
                return err;
            }
            else
            {
                ReturnErrorOnFailure(encoder.Encode(ByteSpan{ cert }));
            }
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

const FabricInfo * RetrieveCurrentFabric(CommandHandler * aCommandHandler)
{
    FabricIndex index = aCommandHandler->GetAccessingFabricIndex();
    ChipLogDetail(Zcl, "OpCreds: Finding fabric with fabricIndex 0x%x", static_cast<unsigned>(index));
    return Server::GetInstance().GetFabricTable().FindFabricWithIndex(index);
}

CHIP_ERROR DeleteFabricFromTable(FabricIndex fabricIndex)
{
    ReturnErrorOnFailure(Server::GetInstance().GetFabricTable().Delete(fabricIndex));
    return CHIP_NO_ERROR;
}

void CleanupSessionsForFabric(SessionManager & sessionMgr, FabricIndex fabricIndex)
{
    sessionMgr.ExpireAllSessionsForFabric(fabricIndex);
}

void FailSafeCleanup(const chip::DeviceLayer::ChipDeviceEvent * event)
{
    ChipLogError(Zcl, "OpCreds: Proceeding to FailSafeCleanup on fail-safe expiry!");

    FabricIndex fabricIndex = event->FailSafeTimerExpired.fabricIndex;

    // If an AddNOC or UpdateNOC command has been successfully invoked, terminate all CASE sessions associated with the Fabric
    // whose Fabric Index is recorded in the Fail-Safe context (see ArmFailSafe Command) by clearing any associated Secure
    // Session Context at the Server.
    if (event->FailSafeTimerExpired.addNocCommandHasBeenInvoked || event->FailSafeTimerExpired.updateNocCommandHasBeenInvoked)
    {
        SessionManager & sessionMgr = Server::GetInstance().GetSecureSessionManager();
        CleanupSessionsForFabric(sessionMgr, fabricIndex);
    }

    auto & fabricTable = Server::GetInstance().GetFabricTable();
    fabricTable.RevertPendingFabricData();

    // If an AddNOC command had been successfully invoked, achieve the equivalent effect of invoking the RemoveFabric command
    // against the Fabric Index stored in the Fail-Safe Context for the Fabric Index that was the subject of the AddNOC
    // command.
    if (event->FailSafeTimerExpired.addNocCommandHasBeenInvoked)
    {
        CHIP_ERROR err;
        err = DeleteFabricFromTable(fabricIndex);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "OpCreds: failed to delete fabric at index %u: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
        }
    }
}

void OnPlatformEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        ChipLogError(Zcl, "OpCreds: Got FailSafeTimerExpired");
        FailSafeCleanup(event);
    }
}

} // anonymous namespace

class OpCredsFabricTableDelegate : public chip::FabricTable::Delegate
{
public:
    // Gets called when a fabric is about to be deleted
    void FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        // The Leave event SHOULD be emitted by a Node prior to permanently leaving the Fabric.
        for (auto endpoint : EnabledEndpointsWithServerCluster(BasicInformation::Id))
        {
            // If Basic cluster is implemented on this endpoint
            BasicInformation::Events::Leave::Type event;
            event.fabricIndex = fabricIndex;
            EventNumber eventNumber;

            if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
            {
                ChipLogError(Zcl, "OpCredsFabricTableDelegate: Failed to record Leave event");
            }
        }

        // Try to send the queued events as soon as possible for this fabric. If the just emitted leave event won't
        // be sent this time, it will likely not be delivered at all for the following reasons:
        // - removing the fabric expires all associated ReadHandlers, so all subscriptions to
        //   the leave event will be cancelled.
        // - removing the fabric removes all associated access control entries, so generating
        //   subsequent reports containing the leave event will fail the access control check.
        InteractionModelEngine::GetInstance()->GetReportingEngine().ScheduleUrgentEventDeliverySync(MakeOptional(fabricIndex));
    }

    // Gets called when a fabric is deleted
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        ChipLogProgress(Zcl, "OpCreds: Fabric index 0x%x was removed", static_cast<unsigned>(fabricIndex));

        // We need to withdraw the advertisement for the now-removed fabric, so need
        // to restart advertising altogether.
        app::DnssdServer::Instance().StartServer();

        EventManagement::GetInstance().FabricRemoved(fabricIndex);

        NotifyFabricTableChanged();
    }

    // Gets called when a fabric is added/updated, but not necessarily committed to storage
    void OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex) override { NotifyFabricTableChanged(); }

    // Gets called when a fabric in FabricTable is persisted to storage
    void OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        const FabricInfo * fabric = fabricTable.FindFabricWithIndex(fabricIndex);
        // Safety check, but should not happen by the code paths involved
        VerifyOrReturn(fabric != nullptr);

        ChipLogProgress(Zcl,
                        "OpCreds: Fabric index 0x%x was committed to storage. Compressed Fabric Id 0x" ChipLogFormatX64
                        ", FabricId " ChipLogFormatX64 ", NodeId " ChipLogFormatX64 ", VendorId 0x%04X",
                        static_cast<unsigned>(fabric->GetFabricIndex()), ChipLogValueX64(fabric->GetCompressedFabricId()),
                        ChipLogValueX64(fabric->GetFabricId()), ChipLogValueX64(fabric->GetNodeId()), fabric->GetVendorId());
    }

private:
    void NotifyFabricTableChanged()
    {
        // Opcreds cluster is always on Endpoint 0
        MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::CommissionedFabrics::Id);
        MatterReportingAttributeChangeCallback(0, OperationalCredentials::Id, OperationalCredentials::Attributes::Fabrics::Id);
    }
};

OpCredsFabricTableDelegate gFabricDelegate;

void MatterOperationalCredentialsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);

    Server::GetInstance().GetFabricTable().AddFabricDelegate(&gFabricDelegate);

    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler);
}

bool emberAfOperationalCredentialsClusterRemoveFabricCallback(app::CommandHandler * commandObj,
                                                              const app::ConcreteCommandPath & commandPath,
                                                              const Commands::RemoveFabric::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("RemoveFabric", "OperationalCredentials");
    auto & fabricBeingRemoved = commandData.fabricIndex;

    ChipLogProgress(Zcl, "OpCreds: Received a RemoveFabric Command for FabricIndex 0x%x",
                    static_cast<unsigned>(fabricBeingRemoved));

    if (!IsValidFabricIndex(fabricBeingRemoved))
    {
        ChipLogError(Zcl, "OpCreds: Failed RemoveFabric due to invalid FabricIndex");
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    commandObj->FlushAcksRightAwayOnSlowCommand();

    CHIP_ERROR err = DeleteFabricFromTable(fabricBeingRemoved);
    SuccessOrExit(err);

    // Notification was already done by FabricTable delegate

exit:
    // Not using ConvertToNOCResponseStatus here because it's pretty
    // AddNOC/UpdateNOC specific.
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        ChipLogError(Zcl, "OpCreds: Failed RemoveFabric due to FabricIndex not found locally");
        SendNOCResponse(commandObj, commandPath, NodeOperationalCertStatusEnum::kInvalidFabricIndex, fabricBeingRemoved,
                        CharSpan());
    }
    else if (err != CHIP_NO_ERROR)
    {
        // We have no idea what happened; just report failure.
        ChipLogError(Zcl, "OpCreds: Failed RemoveFabric due to internal error (err = %" CHIP_ERROR_FORMAT ")", err.Format());
        StatusIB status(err);
        commandObj->AddStatus(commandPath, status.mStatus);
    }
    else
    {
        ChipLogProgress(Zcl, "OpCreds: RemoveFabric successful");
        SendNOCResponse(commandObj, commandPath, NodeOperationalCertStatusEnum::kOk, fabricBeingRemoved, CharSpan());

        chip::Messaging::ExchangeContext * ec = commandObj->GetExchangeContext();
        FabricIndex currentFabricIndex        = commandObj->GetAccessingFabricIndex();
        if (currentFabricIndex == fabricBeingRemoved)
        {
            ec->AbortAllOtherCommunicationOnFabric();
        }
        else
        {
            SessionManager * sessionManager = ec->GetExchangeMgr()->GetSessionManager();
            CleanupSessionsForFabric(*sessionManager, fabricBeingRemoved);
        }
    }
    return true;
}

bool emberAfOperationalCredentialsClusterUpdateFabricLabelCallback(app::CommandHandler * commandObj,
                                                                   const app::ConcreteCommandPath & commandPath,
                                                                   const Commands::UpdateFabricLabel::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("UpdateFabricLabel", "OperationalCredentials");
    auto & label        = commandData.label;
    auto ourFabricIndex = commandObj->GetAccessingFabricIndex();
    auto finalStatus    = Status::Failure;
    auto & fabricTable  = Server::GetInstance().GetFabricTable();

    ChipLogProgress(Zcl, "OpCreds: Received an UpdateFabricLabel command");

    if (label.size() > 32)
    {
        ChipLogError(Zcl, "OpCreds: Failed UpdateFabricLabel due to invalid label size %u", static_cast<unsigned>(label.size()));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    for (const auto & fabricInfo : fabricTable)
    {
        if (fabricInfo.GetFabricLabel().data_equal(label) && fabricInfo.GetFabricIndex() != ourFabricIndex)
        {
            ChipLogError(Zcl, "Fabric label already in use");
            SendNOCResponse(commandObj, commandPath, NodeOperationalCertStatusEnum::kLabelConflict, ourFabricIndex, CharSpan());
            return true;
        }
    }

    // Set Label on fabric. Any error on this is basically an internal error...
    // NOTE: if an UpdateNOC had caused a pending fabric, that pending fabric is
    //       the one updated thereafter. Otherwise, the data is committed to storage
    //       as soon as the update is done.
    CHIP_ERROR err = fabricTable.SetFabricLabel(ourFabricIndex, label);
    VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

    finalStatus = Status::Success;

    // Succeeded at updating the label, mark Fabrics table changed.
    MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                           OperationalCredentials::Attributes::Fabrics::Id);
exit:
    if (finalStatus == Status::Success)
    {
        SendNOCResponse(commandObj, commandPath, NodeOperationalCertStatusEnum::kOk, ourFabricIndex, CharSpan());
    }
    else
    {
        commandObj->AddStatus(commandPath, finalStatus);
    }
    return true;
}

namespace {

void SendNOCResponse(app::CommandHandler * commandObj, const ConcreteCommandPath & path, NodeOperationalCertStatusEnum status,
                     uint8_t index, const CharSpan & debug_text)
{
    Commands::NOCResponse::Type payload;
    payload.statusCode = status;
    if (status == NodeOperationalCertStatusEnum::kOk)
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

NodeOperationalCertStatusEnum ConvertToNOCResponseStatus(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        return NodeOperationalCertStatusEnum::kOk;
    }
    if (err == CHIP_ERROR_INVALID_PUBLIC_KEY)
    {
        return NodeOperationalCertStatusEnum::kInvalidPublicKey;
    }
    if (err == CHIP_ERROR_WRONG_NODE_ID)
    {
        return NodeOperationalCertStatusEnum::kInvalidNodeOpId;
    }
    if (err == CHIP_ERROR_UNSUPPORTED_CERT_FORMAT)
    {
        return NodeOperationalCertStatusEnum::kInvalidNOC;
    }
    if (err == CHIP_ERROR_WRONG_CERT_DN)
    {
        return NodeOperationalCertStatusEnum::kInvalidNOC;
    }
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return NodeOperationalCertStatusEnum::kMissingCsr;
    }
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return NodeOperationalCertStatusEnum::kTableFull;
    }
    if (err == CHIP_ERROR_FABRIC_EXISTS)
    {
        return NodeOperationalCertStatusEnum::kFabricConflict;
    }
    if (err == CHIP_ERROR_INVALID_FABRIC_INDEX)
    {
        return NodeOperationalCertStatusEnum::kInvalidFabricIndex;
    }
    if (err == CHIP_ERROR_INVALID_ADMIN_SUBJECT)
    {
        return NodeOperationalCertStatusEnum::kInvalidAdminSubject;
    }

    return NodeOperationalCertStatusEnum::kInvalidNOC;
}

} // namespace

bool emberAfOperationalCredentialsClusterAddNOCCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::AddNOC::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddNOC", "OperationalCredentials");
    auto & NOCValue          = commandData.NOCValue;
    auto & ICACValue         = commandData.ICACValue;
    auto & adminVendorId     = commandData.adminVendorId;
    auto & ipkValue          = commandData.IPKValue;
    auto * groupDataProvider = Credentials::GetGroupDataProvider();
    auto nocResponse         = NodeOperationalCertStatusEnum::kOk;
    auto nonDefaultStatus    = Status::Success;
    bool needRevert          = false;

    CHIP_ERROR err             = CHIP_NO_ERROR;
    FabricIndex newFabricIndex = kUndefinedFabricIndex;
    Credentials::GroupDataProvider::KeySet keyset;
    const FabricInfo * newFabricInfo = nullptr;
    auto & fabricTable               = Server::GetInstance().GetFabricTable();

    auto * secureSession   = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession();
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();

    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);

    bool csrWasForUpdateNoc = false; //< Output param of HasPendingOperationalKey
    bool hasPendingKey      = fabricTable.HasPendingOperationalKey(csrWasForUpdateNoc);

    ChipLogProgress(Zcl, "OpCreds: Received an AddNOC command");

    VerifyOrExit(NOCValue.size() <= Credentials::kMaxCHIPCertLength, nonDefaultStatus = Status::InvalidCommand);
    VerifyOrExit(!ICACValue.HasValue() || ICACValue.Value().size() <= Credentials::kMaxCHIPCertLength,
                 nonDefaultStatus = Status::InvalidCommand);
    VerifyOrExit(ipkValue.size() == Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, nonDefaultStatus = Status::InvalidCommand);
    VerifyOrExit(IsVendorIdValidOperationally(adminVendorId), nonDefaultStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);

    // Must have had a previous CSR request, not tagged for UpdateNOC
    VerifyOrExit(hasPendingKey, nocResponse = NodeOperationalCertStatusEnum::kMissingCsr);
    VerifyOrExit(!csrWasForUpdateNoc, nonDefaultStatus = Status::ConstraintError);

    // Internal error that would prevent IPK from being added
    VerifyOrExit(groupDataProvider != nullptr, nonDefaultStatus = Status::Failure);

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    // We can't possibly have a matching root based on the fact that we don't have
    // a shared root store. Therefore we would later fail path validation due to
    // missing root. Let's early-bail with InvalidNOC.
    VerifyOrExit(failSafeContext.AddTrustedRootCertHasBeenInvoked(), nocResponse = NodeOperationalCertStatusEnum::kInvalidNOC);

    // Check this explicitly before adding the fabric so we don't need to back out changes if this is an error.
    VerifyOrExit(IsOperationalNodeId(commandData.caseAdminSubject) || IsCASEAuthTag(commandData.caseAdminSubject),
                 nocResponse = NodeOperationalCertStatusEnum::kInvalidAdminSubject);

    err = fabricTable.AddNewPendingFabricWithOperationalKeystore(NOCValue, ICACValue.ValueOr(ByteSpan{}), adminVendorId,
                                                                 &newFabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // From here if we error-out, we should revert the fabric table pending updates
    needRevert = true;

    newFabricInfo = fabricTable.FindFabricWithIndex(newFabricIndex);
    VerifyOrExit(newFabricInfo != nullptr, nonDefaultStatus = Status::Failure);

    // Set the Identity Protection Key (IPK)
    // The IPK SHALL be the operational group key under GroupKeySetID of 0
    keyset.keyset_id                = Credentials::GroupDataProvider::kIdentityProtectionKeySetId;
    keyset.policy                   = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst;
    keyset.num_keys_used            = 1;
    keyset.epoch_keys[0].start_time = 0;
    memcpy(keyset.epoch_keys[0].key, ipkValue.data(), Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);

    err = newFabricInfo->GetCompressedFabricIdBytes(compressed_fabric_id);
    VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);

    err = groupDataProvider->SetKeySet(newFabricIndex, compressed_fabric_id, keyset);
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
     *   in the `caseAdminSubject` argument.
     *
     */
    if (secureSession->GetSecureSessionType() == SecureSession::Type::kPASE)
    {
        err = secureSession->AdoptFabricIndex(newFabricIndex);
        VerifyOrExit(err == CHIP_NO_ERROR, nonDefaultStatus = Status::Failure);
    }

    // Creating the initial ACL must occur after the PASE session has adopted the fabric index
    // (see above) so that the concomitant event, which is fabric scoped, is properly handled.
    err = CreateAccessControlEntryForNewFabricAdministrator(commandObj->GetSubjectDescriptor(), newFabricIndex,
                                                            commandData.caseAdminSubject);
    VerifyOrExit(err != CHIP_ERROR_INTERNAL, nonDefaultStatus = Status::Failure);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // The Fabric Index associated with the armed fail-safe context SHALL be updated to match the Fabric
    // Index just allocated.
    failSafeContext.SetAddNocCommandInvoked(newFabricIndex);

    // Done all intermediate steps, we are now successful
    needRevert = false;

    // We might have a new operational identity, so we should start advertising it right away.
    app::DnssdServer::Instance().AdvertiseOperational();

    // Notify the attributes containing fabric metadata can be read with new data
    MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                           OperationalCredentials::Attributes::Fabrics::Id);

    // Notify we have one more fabric
    MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                           OperationalCredentials::Attributes::CommissionedFabrics::Id);

exit:
    if (needRevert)
    {
        // Here, on revert, we DO NOT call FabricTable::Delete as this would also remove the existing
        // trusted root previously added. It possibly got reverted in case of the worst kinds of errors,
        // but a better impl of the innards of FabricTable::CommitPendingFabricData would make it work.
        fabricTable.RevertPendingOpCertsExceptRoot();

        // Revert IPK and ACL entries added, ignoring errors, since some steps may have been skipped
        // and error handling does not assist.
        if (groupDataProvider != nullptr)
        {
            (void) groupDataProvider->RemoveFabric(newFabricIndex);
        }

        (void) Access::GetAccessControl().DeleteAllEntriesForFabric(newFabricIndex);

        MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::CommissionedFabrics::Id);
        MatterReportingAttributeChangeCallback(commandPath.mEndpointId, OperationalCredentials::Id,
                                               OperationalCredentials::Attributes::Fabrics::Id);
    }

    // We have an NOC response
    if (nonDefaultStatus == Status::Success)
    {
        SendNOCResponse(commandObj, commandPath, nocResponse, newFabricIndex, CharSpan());
        // Failed to add NOC
        if (nocResponse != NodeOperationalCertStatusEnum::kOk)
        {
            ChipLogError(Zcl, "OpCreds: Failed AddNOC request (err=%" CHIP_ERROR_FORMAT ") with OperationalCert error %d",
                         err.Format(), to_underlying(nocResponse));
        }
        // Success
        else
        {
            ChipLogProgress(Zcl, "OpCreds: successfully created fabric index 0x%x via AddNOC",
                            static_cast<unsigned>(newFabricIndex));
        }
    }
    // No NOC response - Failed constraints
    else
    {
        commandObj->AddStatus(commandPath, nonDefaultStatus);
        ChipLogError(Zcl, "OpCreds: Failed AddNOC request with IM error 0x%02x", to_underlying(nonDefaultStatus));
    }

    return true;
}

bool emberAfOperationalCredentialsClusterUpdateNOCCallback(app::CommandHandler * commandObj,
                                                           const app::ConcreteCommandPath & commandPath,
                                                           const Commands::UpdateNOC::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("UpdateNOC", "OperationalCredentials");
    auto & NOCValue  = commandData.NOCValue;
    auto & ICACValue = commandData.ICACValue;

    auto nocResponse      = NodeOperationalCertStatusEnum::kOk;
    auto nonDefaultStatus = Status::Success;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;

    ChipLogProgress(Zcl, "OpCreds: Received an UpdateNOC command");

    auto & fabricTable            = Server::GetInstance().GetFabricTable();
    auto & failSafeContext        = Server::GetInstance().GetFailSafeContext();
    const FabricInfo * fabricInfo = RetrieveCurrentFabric(commandObj);

    bool csrWasForUpdateNoc = false; //< Output param of HasPendingOperationalKey
    bool hasPendingKey      = fabricTable.HasPendingOperationalKey(csrWasForUpdateNoc);

    VerifyOrExit(NOCValue.size() <= Credentials::kMaxCHIPCertLength, nonDefaultStatus = Status::InvalidCommand);
    VerifyOrExit(!ICACValue.HasValue() || ICACValue.Value().size() <= Credentials::kMaxCHIPCertLength,
                 nonDefaultStatus = Status::InvalidCommand);
    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()),
                 nonDefaultStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), nonDefaultStatus = Status::ConstraintError);

    // Must have had a previous CSR request, tagged for UpdateNOC
    VerifyOrExit(hasPendingKey, nocResponse = NodeOperationalCertStatusEnum::kMissingCsr);
    VerifyOrExit(csrWasForUpdateNoc, nonDefaultStatus = Status::ConstraintError);

    // If current fabric is not available, command was invoked over PASE which is not legal
    VerifyOrExit(fabricInfo != nullptr, nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INSUFFICIENT_PRIVILEGE));
    fabricIndex = fabricInfo->GetFabricIndex();

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    err = fabricTable.UpdatePendingFabricWithOperationalKeystore(fabricIndex, NOCValue, ICACValue.ValueOr(ByteSpan{}));
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // Flag on the fail-safe context that the UpdateNOC command was invoked.
    failSafeContext.SetUpdateNocCommandInvoked();

    // We might have a new operational identity, so we should start advertising
    // it right away.  Also, we need to withdraw our old operational identity.
    // So we need to StartServer() here.
    app::DnssdServer::Instance().StartServer();

    // Attribute notification was already done by fabric table
exit:
    // We have an NOC response
    if (nonDefaultStatus == Status::Success)
    {
        SendNOCResponse(commandObj, commandPath, nocResponse, fabricIndex, CharSpan());
        // Failed to update NOC
        if (nocResponse != NodeOperationalCertStatusEnum::kOk)
        {
            ChipLogError(Zcl, "OpCreds: Failed UpdateNOC request (err=%" CHIP_ERROR_FORMAT ") with OperationalCert error %d",
                         err.Format(), to_underlying(nocResponse));
        }
        // Success
        else
        {
            ChipLogProgress(Zcl, "OpCreds: UpdateNOC successful.");

            // On success, revoke all CASE sessions on the fabric hosting the exchange.
            // From spec:
            //
            //    All internal data reflecting the prior operational identifier of the Node within the Fabric
            //    SHALL be revoked and removed, to an outcome equivalent to the disappearance of the prior Node,
            //    except for the ongoing CASE session context, which SHALL temporarily remain valid until the
            //    `NOCResponse` has been successfully delivered or until the next transport-layer error, so
            //    that the response can be received by the Administrator invoking the command.

            commandObj->GetExchangeContext()->AbortAllOtherCommunicationOnFabric();
        }
    }
    // No NOC response - Failed constraints
    else
    {
        commandObj->AddStatus(commandPath, nonDefaultStatus);
        ChipLogError(Zcl, "OpCreds: Failed UpdateNOC request with IM error 0x%02x", to_underlying(nonDefaultStatus));
    }

    return true;
}

bool emberAfOperationalCredentialsClusterCertificateChainRequestCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::CertificateChainRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CertificateChainRequest", "OperationalCredentials");
    auto & certificateType = commandData.certificateType;

    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t derBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan derBufSpan(derBuf);

    Commands::CertificateChainResponse::Type response;

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    VerifyOrExit(commandObj != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    if (certificateType == kDACCertificate)
    {
        ChipLogProgress(Zcl, "OpCreds: Certificate Chain request received for DAC");
        SuccessOrExit(err = dacProvider->GetDeviceAttestationCert(derBufSpan));
    }
    else if (certificateType == kPAICertificate)
    {
        ChipLogProgress(Zcl, "OpCreds: Certificate Chain request received for PAI");
        SuccessOrExit(err = dacProvider->GetProductAttestationIntermediateCert(derBufSpan));
    }
    else
    {
        ChipLogError(Zcl, "OpCreds: Certificate Chain request received for unknown type: %d", static_cast<int>(certificateType));
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

    response.certificate = derBufSpan;
    commandObj->AddResponse(commandPath, response);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OpCreds: Failed CertificateChainRequest: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAttestationRequestCallback(app::CommandHandler * commandObj,
                                                                    const app::ConcreteCommandPath & commandPath,
                                                                    const Commands::AttestationRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AttestationRequest", "OperationalCredentials");
    auto & attestationNonce = commandData.attestationNonce;

    auto finalStatus = Status::Failure;
    CHIP_ERROR err   = CHIP_ERROR_INVALID_ARGUMENT;
    ByteSpan tbsSpan;

    Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen = 0;
    MutableByteSpan attestationElementsSpan;
    uint8_t certDeclBuf[Credentials::kMaxCMSSignedCDMessage]; // Sized to hold the example certificate declaration with 100 PIDs.
                                                              // See DeviceAttestationCredsExample
    MutableByteSpan certDeclSpan(certDeclBuf);

    // TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    // TODO: in future versions, retrieve vendor information to populate the fields below.
    uint32_t timestamp = 0;
    Credentials::DeviceAttestationVendorReservedConstructor emptyVendorReserved(nullptr, 0);

    // TODO: in future versions, also retrieve and use firmware Information
    const ByteSpan kEmptyFirmwareInfo;

    ChipLogProgress(Zcl, "OpCreds: Received an AttestationRequest command");

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();

    VerifyOrExit(attestationNonce.size() == Credentials::kExpectedAttestationNonceSize, finalStatus = Status::InvalidCommand);

    if (dacProvider == nullptr)
    {
        err = CHIP_ERROR_INTERNAL;
        VerifyOrExit(dacProvider != nullptr, finalStatus = Status::Failure);
    }

    err = dacProvider->GetCertificationDeclaration(certDeclSpan);
    VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

    attestationElementsLen = TLV::EstimateStructOverhead(certDeclSpan.size(), attestationNonce.size(), sizeof(uint64_t) * 8);

    if (!attestationElements.Alloc(attestationElementsLen + attestationChallenge.size()))
    {
        err = CHIP_ERROR_NO_MEMORY;
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::ResourceExhausted);
    }

    attestationElementsSpan = MutableByteSpan{ attestationElements.Get(), attestationElementsLen };
    err = Credentials::ConstructAttestationElements(certDeclSpan, attestationNonce, timestamp, kEmptyFirmwareInfo,
                                                    emptyVendorReserved, attestationElementsSpan);
    VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

    // Append attestation challenge in the back of the reserved space for the signature
    memcpy(attestationElements.Get() + attestationElementsSpan.size(), attestationChallenge.data(), attestationChallenge.size());
    tbsSpan = ByteSpan{ attestationElements.Get(), attestationElementsSpan.size() + attestationChallenge.size() };

    {
        Crypto::P256ECDSASignature signature;
        MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };

        // Generate attestation signature
        err = dacProvider->SignWithDeviceAttestationKey(tbsSpan, signatureSpan);
        ClearSecretData(attestationElements.Get() + attestationElementsSpan.size(), attestationChallenge.size());
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);
        VerifyOrExit(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), finalStatus = Status::Failure);

        Commands::AttestationResponse::Type response;

        response.attestationElements  = attestationElementsSpan;
        response.attestationSignature = signatureSpan;

        ChipLogProgress(Zcl, "OpCreds: AttestationRequest successful.");
        finalStatus = Status::Success;
        commandObj->AddResponse(commandPath, response);
    }

exit:
    if (finalStatus != Status::Success)
    {
        commandObj->AddStatus(commandPath, finalStatus);
        ChipLogError(Zcl, "OpCreds: Failed AttestationRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                     to_underlying(finalStatus), err.Format());
    }

    return true;
}

bool emberAfOperationalCredentialsClusterCSRRequestCallback(app::CommandHandler * commandObj,
                                                            const app::ConcreteCommandPath & commandPath,
                                                            const Commands::CSRRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CSRRequest", "OperationalCredentials");
    ChipLogProgress(Zcl, "OpCreds: Received a CSRRequest command");

    chip::Platform::ScopedMemoryBuffer<uint8_t> nocsrElements;
    MutableByteSpan nocsrElementsSpan;
    auto finalStatus = Status::Failure;
    ByteSpan tbsSpan;

    // Start with CHIP_ERROR_INVALID_ARGUMENT so that cascading errors yield correct
    // logs by the end. We use finalStatus as our overall success marker, not error
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    auto & fabricTable     = Server::GetInstance().GetFabricTable();
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();

    auto & CSRNonce     = commandData.CSRNonce;
    bool isForUpdateNoc = commandData.isForUpdateNOC.ValueOr(false);

    // TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
    // Retrieve attestation challenge
    ByteSpan attestationChallenge =
        commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();

    failSafeContext.SetCsrRequestForUpdateNoc(isForUpdateNoc);
    const FabricInfo * fabricInfo = RetrieveCurrentFabric(commandObj);

    VerifyOrExit(CSRNonce.size() == Credentials::kExpectedAttestationNonceSize, finalStatus = Status::InvalidCommand);

    // If current fabric is not available, command was invoked over PASE which is not legal if IsForUpdateNOC is true.
    VerifyOrExit(!isForUpdateNoc || (fabricInfo != nullptr), finalStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), finalStatus = Status::FailsafeRequired);
    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), finalStatus = Status::ConstraintError);

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    // Prepare NOCSRElements structure
    {
        constexpr size_t csrLength = Crypto::kMIN_CSR_Buffer_Size;
        size_t nocsrLengthEstimate = 0;
        ByteSpan kNoVendorReserved;
        Platform::ScopedMemoryBuffer<uint8_t> csr;
        MutableByteSpan csrSpan;

        // Generate the actual CSR from the ephemeral key
        if (!csr.Alloc(csrLength))
        {
            err = CHIP_ERROR_NO_MEMORY;
            VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::ResourceExhausted);
        }
        csrSpan = MutableByteSpan{ csr.Get(), csrLength };

        Optional<FabricIndex> fabricIndexForCsr;
        if (isForUpdateNoc)
        {
            fabricIndexForCsr.SetValue(commandObj->GetAccessingFabricIndex());
        }

        err = fabricTable.AllocatePendingOperationalKey(fabricIndexForCsr, csrSpan);

        if (csrSpan.size() > csrLength)
        {
            err = CHIP_ERROR_INTERNAL;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "OpCreds: AllocatePendingOperationalKey returned %" CHIP_ERROR_FORMAT, err.Format());
            VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);
        }

        ChipLogProgress(Zcl, "OpCreds: AllocatePendingOperationalKey succeeded");

        // Encode the NOCSR elements with the CSR and Nonce
        nocsrLengthEstimate = TLV::EstimateStructOverhead(csrSpan.size(),  // CSR buffer
                                                          CSRNonce.size(), // CSR Nonce
                                                          0u               // no vendor reserved data
        );

        if (!nocsrElements.Alloc(nocsrLengthEstimate + attestationChallenge.size()))
        {
            err = CHIP_ERROR_NO_MEMORY;
            VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::ResourceExhausted);
        }

        nocsrElementsSpan = MutableByteSpan{ nocsrElements.Get(), nocsrLengthEstimate };

        err = Credentials::ConstructNOCSRElements(ByteSpan{ csrSpan.data(), csrSpan.size() }, CSRNonce, kNoVendorReserved,
                                                  kNoVendorReserved, kNoVendorReserved, nocsrElementsSpan);
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

        // Append attestation challenge in the back of the reserved space for the signature
        memcpy(nocsrElements.Get() + nocsrElementsSpan.size(), attestationChallenge.data(), attestationChallenge.size());
        tbsSpan = ByteSpan{ nocsrElements.Get(), nocsrElementsSpan.size() + attestationChallenge.size() };

        {
            Credentials::DeviceAttestationCredentialsProvider * dacProvider =
                Credentials::GetDeviceAttestationCredentialsProvider();
            Crypto::P256ECDSASignature signature;
            MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };

            // Generate attestation signature
            err = dacProvider->SignWithDeviceAttestationKey(tbsSpan, signatureSpan);
            ClearSecretData(nocsrElements.Get() + nocsrElementsSpan.size(), attestationChallenge.size());
            VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);
            VerifyOrExit(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), finalStatus = Status::Failure);

            Commands::CSRResponse::Type response;

            response.NOCSRElements        = nocsrElementsSpan;
            response.attestationSignature = signatureSpan;

            ChipLogProgress(Zcl, "OpCreds: CSRRequest successful.");
            finalStatus = Status::Success;
            commandObj->AddResponse(commandPath, response);
        }
    }

exit:
    // If failed constraints or internal errors, send a status report instead of the response sent above
    if (finalStatus != Status::Success)
    {
        commandObj->AddStatus(commandPath, finalStatus);
        ChipLogError(Zcl, "OpCreds: Failed CSRRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                     to_underlying(finalStatus), err.Format());
    }

    return true;
}

bool emberAfOperationalCredentialsClusterAddTrustedRootCertificateCallback(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const Commands::AddTrustedRootCertificate::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("AddTrustedRootCertificate", "OperationalCredentials");

    auto & fabricTable = Server::GetInstance().GetFabricTable();
    auto finalStatus   = Status::Failure;

    // Start with CHIP_ERROR_INVALID_ARGUMENT so that cascading errors yield correct
    // logs by the end. We use finalStatus as our overall success marker, not error
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    auto & rootCertificate = commandData.rootCACertificate;
    auto & failSafeContext = Server::GetInstance().GetFailSafeContext();

    ChipLogProgress(Zcl, "OpCreds: Received an AddTrustedRootCertificate command");

    VerifyOrExit(rootCertificate.size() <= Credentials::kMaxCHIPCertLength, finalStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), finalStatus = Status::FailsafeRequired);

    // Can only add a single trusted root cert per fail-safe
    VerifyOrExit(!failSafeContext.AddTrustedRootCertHasBeenInvoked(), finalStatus = Status::ConstraintError);

    // If we successfully invoked AddNOC/UpdateNOC, this command cannot possibly
    // be useful in the context.
    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), finalStatus = Status::ConstraintError);

    // Flush acks before really slow work
    commandObj->FlushAcksRightAwayOnSlowCommand();

    err = ValidateChipRCAC(rootCertificate);
    VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::InvalidCommand);

    err = fabricTable.AddNewPendingTrustedRootCert(rootCertificate);
    VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, finalStatus = Status::ResourceExhausted);

    // CHIP_ERROR_INVALID_ARGUMENT by the time we reach here means bad format
    VerifyOrExit(err != CHIP_ERROR_INVALID_ARGUMENT, finalStatus = Status::InvalidCommand);
    VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);

    // Got here, so we succeeded, mark AddTrustedRootCert has having been invoked.
    ChipLogProgress(Zcl, "OpCreds: AddTrustedRootCertificate successful.");
    finalStatus = Status::Success;
    failSafeContext.SetAddTrustedRootCertInvoked();

exit:
    if (finalStatus != Status::Success)
    {
        ChipLogError(Zcl, "OpCreds: Failed AddTrustedRootCertificate request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                     to_underlying(finalStatus), err.Format());
    }

    commandObj->AddStatus(commandPath, finalStatus);
    return true;
}
