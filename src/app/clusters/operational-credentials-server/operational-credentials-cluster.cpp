/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>

#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server/Server.h>
#include <clusters/OperationalCredentials/AttributeIds.h>
#include <clusters/OperationalCredentials/Commands.h>
#include <clusters/OperationalCredentials/Enums.h>
#include <clusters/OperationalCredentials/Metadata.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;
using namespace chip::Credentials;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Transport;
namespace {

constexpr auto kDACCertificate                = CertificateChainTypeEnum::kDACCertificate;
constexpr auto kPAICertificate                = CertificateChainTypeEnum::kPAICertificate;
constexpr auto kNocResponseMaxDebugTextLength = 128;

// Get the attestation challenge for the current session in progress. Only valid when called
// synchronously from inside a CommandHandler. If not called in CASE/PASE session context,
// return an empty span. This will for sure make the procedures that rely on the challenge
// fail, which is intended as it never should have reached here.
// TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
ByteSpan GetAttestationChallengeFromCurrentSession(app::CommandHandler * commandObj)
{
    VerifyOrDie((commandObj != nullptr) && (commandObj->GetExchangeContext() != nullptr));
    SessionHandle sessionHandle                 = commandObj->GetExchangeContext()->GetSessionHandle();
    Transport::Session::SessionType sessionType = sessionHandle->GetSessionType();
    VerifyOrReturnValue(sessionType == Transport::Session::SessionType::kSecure, ByteSpan{});

    ByteSpan attestationChallenge = sessionHandle->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    return attestationChallenge;
}

const FabricInfo * RetrieveCurrentFabric(CommandHandler * aCommandHandler, FabricTable & fabricTable)
{
    FabricIndex index = aCommandHandler->GetAccessingFabricIndex();
    return fabricTable.FindFabricWithIndex(index);
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
        const CharSpan & to_send =
            debug_text.size() > kNocResponseMaxDebugTextLength ? debug_text.SubSpan(0, kNocResponseMaxDebugTextLength) : debug_text;
        payload.debugText.Emplace(to_send);
    }

    commandObj->AddResponse(path, payload);
}

CHIP_ERROR ReadNOCs(AttributeValueEncoder & aEncoder, FabricTable & fabricTable)
{
    return aEncoder.EncodeList([&fabricTable](const auto & encoder) -> CHIP_ERROR {
        for (const auto & fabricInfo : fabricTable)
        {
            OperationalCredentials::Structs::NOCStruct::Type nocStruct;
            uint8_t nocBuf[kMaxCHIPCertLength];
            uint8_t icacOrVvscBuf[kMaxCHIPCertLength];
            MutableByteSpan nocSpan{ nocBuf };
            MutableByteSpan icacOrVvscSpan{ icacOrVvscBuf };
            FabricIndex fabricIndex = fabricInfo.GetFabricIndex();

            nocStruct.fabricIndex = fabricIndex;

            ReturnErrorOnFailure(fabricTable.FetchNOCCert(fabricIndex, nocSpan));
            nocStruct.noc = nocSpan;

            // ICAC and VVSC are mutually exclusive. ICAC is nullable, VVSC is optional.
            ReturnErrorOnFailure(fabricTable.FetchICACert(fabricIndex, icacOrVvscSpan));
            if (!icacOrVvscSpan.empty())
            {
                nocStruct.icac.SetNonNull(icacOrVvscSpan);
            }
            else
            {
                icacOrVvscSpan = MutableByteSpan{ icacOrVvscBuf };
                ReturnErrorOnFailure(fabricTable.FetchVVSC(fabricIndex, icacOrVvscSpan));
                if (!icacOrVvscSpan.empty())
                {
                    nocStruct.vvsc = MakeOptional(icacOrVvscSpan);
                }
            }

            ReturnErrorOnFailure(encoder.Encode(nocStruct));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ReadFabricsList(AttributeValueEncoder & aEncoder, FabricTable & fabricTable)
{
    return aEncoder.EncodeList([&fabricTable](const auto & encoder) -> CHIP_ERROR {
        for (const auto & fabricInfo : fabricTable)
        {
            OperationalCredentials::Structs::FabricDescriptorStruct::Type fabricDescriptor;
            FabricIndex fabricIndex = fabricInfo.GetFabricIndex();

            fabricDescriptor.fabricIndex = fabricIndex;
            fabricDescriptor.nodeID      = fabricInfo.GetPeerId().GetNodeId();
            fabricDescriptor.vendorID    = fabricInfo.GetVendorId();
            fabricDescriptor.fabricID    = fabricInfo.GetFabricId();

            fabricDescriptor.label = fabricInfo.GetFabricLabel();

            Crypto::P256PublicKey pubKey;
            ReturnErrorOnFailure(fabricTable.FetchRootPubkey(fabricIndex, pubKey));
            fabricDescriptor.rootPublicKey = ByteSpan{ pubKey.ConstBytes(), pubKey.Length() };

            uint8_t vidVerificationStatement[Crypto::kVendorIdVerificationStatementV1Size];
            MutableByteSpan vidVerificationStatementSpan{ vidVerificationStatement };
            ReturnErrorOnFailure(fabricTable.FetchVIDVerificationStatement(fabricIndex, vidVerificationStatementSpan));
            if (!vidVerificationStatementSpan.empty())
            {
                fabricDescriptor.VIDVerificationStatement = MakeOptional(vidVerificationStatementSpan);
            }

            ReturnErrorOnFailure(encoder.Encode(fabricDescriptor));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ReadRootCertificates(AttributeValueEncoder & aEncoder, FabricTable & fabricTable)
{
    // It is OK to have duplicates.
    return aEncoder.EncodeList([&fabricTable](const auto & encoder) -> CHIP_ERROR {
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

std::optional<DataModel::ActionReturnStatus> HandleCSRRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                              TLV::TLVReader & input_arguments, FabricTable & fabricTable,
                                                              FailSafeContext & failSafeContext,
                                                              Credentials::DeviceAttestationCredentialsProvider * dacProvider)
{
    MATTER_TRACE_SCOPE("CSRRequest", "OperationalCredentials");
    Commands::CSRRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    ChipLogProgress(Zcl, "OpCreds: Received a CSRRequest command");

    chip::Platform::ScopedMemoryBuffer<uint8_t> nocsrElements;
    MutableByteSpan nocsrElementsSpan;
    auto errorStatus = Status::Failure;
    ByteSpan tbsSpan;

    // Start with CHIP_ERROR_INVALID_ARGUMENT so that cascading errors yield correct
    // logs by the end. We use finalStatus as our overall success marker, not error
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    auto & CSRNonce     = commandData.CSRNonce;
    bool isForUpdateNoc = commandData.isForUpdateNOC.ValueOr(false);

    ByteSpan attestationChallenge = GetAttestationChallengeFromCurrentSession(commandObj);

    failSafeContext.SetCsrRequestForUpdateNoc(isForUpdateNoc);
    const FabricInfo * fabricInfo = RetrieveCurrentFabric(commandObj, fabricTable);

    VerifyOrExit(CSRNonce.size() == Credentials::kExpectedAttestationNonceSize, errorStatus = Status::InvalidCommand);

    // If current fabric is not available, command was invoked over PASE which is not legal if IsForUpdateNOC is true.
    VerifyOrExit(!isForUpdateNoc || (fabricInfo != nullptr), errorStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), errorStatus = Status::FailsafeRequired);
    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), errorStatus = Status::ConstraintError);

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
            VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::ResourceExhausted);
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
            VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);
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
            VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::ResourceExhausted);
        }

        nocsrElementsSpan = MutableByteSpan{ nocsrElements.Get(), nocsrLengthEstimate };

        VerifyOrExit(nocsrElementsSpan.size() >= nocsrLengthEstimate, errorStatus = Status::ConstraintError);

        err = Credentials::ConstructNOCSRElements(ByteSpan{ csrSpan.data(), csrSpan.size() }, CSRNonce, kNoVendorReserved,
                                                  kNoVendorReserved, kNoVendorReserved, nocsrElementsSpan);
        VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);

        // Append attestation challenge in the back of the reserved space for the signature
        memcpy(nocsrElements.Get() + nocsrElementsSpan.size(), attestationChallenge.data(), attestationChallenge.size());
        tbsSpan = ByteSpan{ nocsrElements.Get(), nocsrElementsSpan.size() + attestationChallenge.size() };

        {
            Crypto::P256ECDSASignature signature;
            MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };

            // Generate attestation signature
            err = dacProvider->SignWithDeviceAttestationKey(tbsSpan, signatureSpan);
            Crypto::ClearSecretData(nocsrElements.Get() + nocsrElementsSpan.size(), attestationChallenge.size());
            VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);
            VerifyOrExit(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), errorStatus = Status::Failure);

            Commands::CSRResponse::Type response;

            response.NOCSRElements        = nocsrElementsSpan;
            response.attestationSignature = signatureSpan;

            ChipLogProgress(Zcl, "OpCreds: CSRRequest successful.");
            commandObj->AddResponse(commandPath, response);
            return std::nullopt;
        }
    }
exit:
    // If failed constraints or internal errors, send a status report instead of the response sent above
    ChipLogError(Zcl, "OpCreds: Failed CSRRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                 to_underlying(errorStatus), err.Format());
    return errorStatus;
}

std::optional<DataModel::ActionReturnStatus> HandleAddNOC(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          TLV::TLVReader & input_arguments, FabricTable & fabricTable,
                                                          FailSafeContext & failSafeContext, DnssdServer & dnssdServer,
                                                          CommissioningWindowManager & commissioningWindowManager,
                                                          bool & reportChange)
{
    MATTER_TRACE_SCOPE("AddNOC", "OperationalCredentials");
    Commands::AddNOC::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    auto & NOCValue          = commandData.NOCValue;
    auto & ICACValue         = commandData.ICACValue;
    auto & adminVendorId     = commandData.adminVendorId;
    auto & ipkValue          = commandData.IPKValue;
    auto * groupDataProvider = Credentials::GetGroupDataProvider();
    auto nocResponse         = NodeOperationalCertStatusEnum::kOk;
    auto errorStatus         = Status::Success;
    bool needRevert          = false;

    CHIP_ERROR err             = CHIP_NO_ERROR;
    FabricIndex newFabricIndex = kUndefinedFabricIndex;
    Credentials::GroupDataProvider::KeySet keyset;
    const FabricInfo * newFabricInfo = nullptr;

    auto * secureSession = commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession();

    uint8_t compressed_fabric_id_buffer[sizeof(uint64_t)];
    MutableByteSpan compressed_fabric_id(compressed_fabric_id_buffer);

    bool csrWasForUpdateNoc = false; //< Output param of HasPendingOperationalKey
    bool hasPendingKey      = fabricTable.HasPendingOperationalKey(csrWasForUpdateNoc);

    ChipLogProgress(Zcl, "OpCreds: Received an AddNOC command");

    VerifyOrExit(NOCValue.size() <= Credentials::kMaxCHIPCertLength, errorStatus = Status::InvalidCommand);
    VerifyOrExit(!ICACValue.HasValue() || ICACValue.Value().size() <= Credentials::kMaxCHIPCertLength,
                 errorStatus = Status::InvalidCommand);
    VerifyOrExit(ipkValue.size() == Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, errorStatus = Status::InvalidCommand);
    VerifyOrExit(IsVendorIdValidOperationally(adminVendorId), errorStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), errorStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), errorStatus = Status::ConstraintError);

    // Must have had a previous CSR request, not tagged for UpdateNOC
    VerifyOrExit(hasPendingKey, nocResponse = NodeOperationalCertStatusEnum::kMissingCsr);
    VerifyOrExit(!csrWasForUpdateNoc, errorStatus = Status::ConstraintError);

    // Internal error that would prevent IPK from being added
    VerifyOrExit(groupDataProvider != nullptr, errorStatus = Status::Failure);

    // We can't possibly have a matching root based on the fact that we don't have
    // a shared root store. Therefore we would later fail path validation due to
    // missing root. Let's early-bail with InvalidNOC.
    VerifyOrExit(failSafeContext.AddTrustedRootCertHasBeenInvoked(), nocResponse = NodeOperationalCertStatusEnum::kInvalidNOC);

    // Check this explicitly before adding the fabric so we don't need to back out changes if this is an error.
    VerifyOrExit(IsOperationalNodeId(commandData.caseAdminSubject) || IsCASEAuthTag(commandData.caseAdminSubject),
                 nocResponse = NodeOperationalCertStatusEnum::kInvalidAdminSubject);

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    // These checks should only run during JCM.
    if (commissioningWindowManager.IsJCM())
    {
        // NOC must contain an Administrator CAT
        CATValues cats;
        err = ExtractCATsFromOpCert(NOCValue, cats);
        VerifyOrExit(err == CHIP_NO_ERROR && cats.ContainsIdentifier(kAdminCATIdentifier),
                     nocResponse = NodeOperationalCertStatusEnum::kInvalidNOC);

        // CaseAdminSubject must contain an Anchor CAT
        CASEAuthTag tag = CASEAuthTagFromNodeId(commandData.caseAdminSubject);
        VerifyOrExit(IsCASEAuthTag(commandData.caseAdminSubject) && IsValidCASEAuthTag(tag) &&
                         (GetCASEAuthTagIdentifier(tag) == kAnchorCATIdentifier),
                     nocResponse = NodeOperationalCertStatusEnum::kInvalidAdminSubject);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC

    err = fabricTable.AddNewPendingFabricWithOperationalKeystore(NOCValue, ICACValue.ValueOr(ByteSpan{}), adminVendorId,
                                                                 &newFabricIndex);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // From here if we error-out, we should revert the fabric table pending updates
    needRevert = true;

    newFabricInfo = fabricTable.FindFabricWithIndex(newFabricIndex);
    VerifyOrExit(newFabricInfo != nullptr, errorStatus = Status::Failure);

    // Set the Identity Protection Key (IPK)
    // The IPK SHALL be the operational group key under GroupKeySetID of 0
    keyset.keyset_id                = Credentials::GroupDataProvider::kIdentityProtectionKeySetId;
    keyset.policy                   = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst;
    keyset.num_keys_used            = 1;
    keyset.epoch_keys[0].start_time = 0;
    memcpy(keyset.epoch_keys[0].key, ipkValue.data(), Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);

    err = newFabricInfo->GetCompressedFabricIdBytes(compressed_fabric_id);
    VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);

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
        VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);
    }

    // Creating the initial ACL must occur after the PASE session has adopted the fabric index
    // (see above) so that the concomitant event, which is fabric scoped, is properly handled.
    err = CreateAccessControlEntryForNewFabricAdministrator(commandObj->GetSubjectDescriptor(), newFabricIndex,
                                                            commandData.caseAdminSubject);
    VerifyOrExit(err != CHIP_ERROR_INTERNAL, errorStatus = Status::Failure);
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // The Fabric Index associated with the armed fail-safe context SHALL be updated to match the Fabric
    // Index just allocated.
    failSafeContext.SetAddNocCommandInvoked(newFabricIndex);

    // Done all intermediate steps, we are now successful
    needRevert = false;

    // We might have a new operational identity, so we should start advertising it right away.
    err = dnssdServer.AdvertiseOperational();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Operational advertising failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    reportChange = true;

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

        reportChange = true;
    }

    // We have an NOC response
    if (errorStatus == Status::Success)
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

        return std::nullopt;
    }

    // No NOC response - Failed constraints
    ChipLogError(Zcl, "OpCreds: Failed AddNOC request with IM error 0x%02x", to_underlying(errorStatus));
    return errorStatus;
}

std::optional<DataModel::ActionReturnStatus> HandleUpdateNOC(CommandHandler * commandObj, TLV::TLVReader & input_arguments,
                                                             const DataModel::InvokeRequest & request, FabricTable & fabricTable,
                                                             FailSafeContext & failSafeContext, DnssdServer & dnssdServer)
{
    MATTER_TRACE_SCOPE("UpdateNOC", "OperationalCredentials");
    Commands::UpdateNOC::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    auto & NOCValue  = commandData.NOCValue;
    auto & ICACValue = commandData.ICACValue;

    auto nocResponse = NodeOperationalCertStatusEnum::kOk;
    auto errorStatus = Status::Success;

    CHIP_ERROR err          = CHIP_NO_ERROR;
    FabricIndex fabricIndex = 0;

    ChipLogProgress(Zcl, "OpCreds: Received an UpdateNOC command");

    const FabricInfo * fabricInfo = RetrieveCurrentFabric(commandObj, fabricTable);

    bool csrWasForUpdateNoc = false; //< Output param of HasPendingOperationalKey
    bool hasPendingKey      = fabricTable.HasPendingOperationalKey(csrWasForUpdateNoc);

    VerifyOrExit(NOCValue.size() <= Credentials::kMaxCHIPCertLength, errorStatus = Status::InvalidCommand);
    VerifyOrExit(!ICACValue.HasValue() || ICACValue.Value().size() <= Credentials::kMaxCHIPCertLength,
                 errorStatus = Status::InvalidCommand);
    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), errorStatus = Status::FailsafeRequired);

    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), errorStatus = Status::ConstraintError);

    // Must have had a previous CSR request, tagged for UpdateNOC
    VerifyOrExit(hasPendingKey, nocResponse = NodeOperationalCertStatusEnum::kMissingCsr);
    VerifyOrExit(csrWasForUpdateNoc, errorStatus = Status::ConstraintError);

    // If current fabric is not available, command was invoked over PASE which is not legal
    VerifyOrExit(fabricInfo != nullptr, nocResponse = ConvertToNOCResponseStatus(CHIP_ERROR_INSUFFICIENT_PRIVILEGE));
    fabricIndex = fabricInfo->GetFabricIndex();

    err = fabricTable.UpdatePendingFabricWithOperationalKeystore(fabricIndex, NOCValue, ICACValue.ValueOr(ByteSpan{}));
    VerifyOrExit(err == CHIP_NO_ERROR, nocResponse = ConvertToNOCResponseStatus(err));

    // Flag on the fail-safe context that the UpdateNOC command was invoked.
    failSafeContext.SetUpdateNocCommandInvoked();

    // We might have a new operational identity, so we should start advertising
    // it right away.  Also, we need to withdraw our old operational identity.
    // So we need to StartServer() here.
    dnssdServer.StartServer();

    // Attribute notification was already done by fabric table
exit:
    // We have an NOC response
    if (errorStatus == Status::Success)
    {
        SendNOCResponse(commandObj, request.path, nocResponse, fabricIndex, CharSpan());
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
        return std::nullopt;
    }
    // No NOC response - Failed constraints
    ChipLogError(Zcl, "OpCreds: Failed UpdateNOC request with IM error 0x%02x", to_underlying(errorStatus));
    return errorStatus;
}

std::optional<DataModel::ActionReturnStatus> HandleUpdateFabricLabel(CommandHandler * commandObj, TLV::TLVReader & input_arguments,
                                                                     const DataModel::InvokeRequest & request,
                                                                     FabricTable & fabricTable)
{
    MATTER_TRACE_SCOPE("UpdateFabricLabel", "OperationalCredentials");
    Commands::UpdateFabricLabel::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    auto & label        = commandData.label;
    auto ourFabricIndex = commandObj->GetAccessingFabricIndex();

    ChipLogProgress(Zcl, "OpCreds: Received an UpdateFabricLabel command");

    if (label.size() > 32)
    {
        // TODO: This error should probably be a ConstraintError instead of InvalidCommand,
        // need to confirm if current tests are expecting the later.
        ChipLogError(Zcl, "OpCreds: Failed UpdateFabricLabel due to invalid label size %u", static_cast<unsigned>(label.size()));
        return Status::InvalidCommand;
    }

    for (const auto & fabricInfo : fabricTable)
    {
        if (fabricInfo.GetFabricLabel().data_equal(label) && fabricInfo.GetFabricIndex() != ourFabricIndex)
        {
            ChipLogError(Zcl, "Fabric label already in use");
            SendNOCResponse(commandObj, request.path, NodeOperationalCertStatusEnum::kLabelConflict, ourFabricIndex, CharSpan());
            return std::nullopt;
        }
    }

    // Set Label on fabric. Any error on this is basically an internal error...
    // NOTE: if an UpdateNOC had caused a pending fabric, that pending fabric is
    //       the one updated thereafter. Otherwise, the data is committed to storage
    //       as soon as the update is done.
    CHIP_ERROR err = fabricTable.SetFabricLabel(ourFabricIndex, label);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);

    SendNOCResponse(commandObj, request.path, NodeOperationalCertStatusEnum::kOk, ourFabricIndex, CharSpan());
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
HandleAddTrustedRootCertificate(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                TLV::TLVReader & input_arguments, FabricTable & fabricTable, FailSafeContext & failSafeContext)
{
    MATTER_TRACE_SCOPE("AddTrustedRootCertificate", "OperationalCredentials");
    Commands::AddTrustedRootCertificate::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    auto finalStatus = Status::Failure;

    // Start with CHIP_ERROR_INVALID_ARGUMENT so that cascading errors yield correct
    // logs by the end. We use finalStatus as our overall success marker, not error
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    auto & rootCertificate = commandData.rootCACertificate;

    ChipLogProgress(Zcl, "OpCreds: Received an AddTrustedRootCertificate command");

    VerifyOrExit(rootCertificate.size() <= Credentials::kMaxCHIPCertLength, finalStatus = Status::InvalidCommand);

    VerifyOrExit(failSafeContext.IsFailSafeArmed(commandObj->GetAccessingFabricIndex()), finalStatus = Status::FailsafeRequired);

    // Can only add a single trusted root cert per fail-safe
    VerifyOrExit(!failSafeContext.AddTrustedRootCertHasBeenInvoked(), finalStatus = Status::ConstraintError);

    // If we successfully invoked AddNOC/UpdateNOC, this command cannot possibly
    // be useful in the context.
    VerifyOrExit(!failSafeContext.NocCommandHasBeenInvoked(), finalStatus = Status::ConstraintError);

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
    return finalStatus;
}

std::optional<DataModel::ActionReturnStatus>
HandleSetVIDVerificationStatement(CommandHandler * commandObj, TLV::TLVReader & input_arguments,
                                  const DataModel::InvokeRequest & request, FabricTable & fabricTable,
                                  FailSafeContext & failSafeContext, bool & reportChange)
{
    Commands::SetVIDVerificationStatement::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments, request.GetAccessingFabricIndex()));

    FabricIndex fabricIndex = commandObj->GetAccessingFabricIndex();

    ChipLogProgress(Zcl, "OpCreds: Received a SetVIDVerificationStatement Command for FabricIndex 0x%x",
                    static_cast<unsigned>(fabricIndex));

    if (!commandData.vendorID.HasValue() && !commandData.VIDVerificationStatement.HasValue() && !commandData.vvsc.HasValue())
    {
        return Status::InvalidCommand;
    }

    if (commandData.vendorID.HasValue() && !IsVendorIdValidOperationally(commandData.vendorID.Value()))
    {
        return Status::ConstraintError;
    }

    CHIP_ERROR err = fabricTable.SetVIDVerificationStatementElements(
        fabricIndex, commandData.vendorID, commandData.VIDVerificationStatement, commandData.vvsc, reportChange);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "SetVIDVerificationStatement failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return Status::ConstraintError;
    }
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return Status::InvalidCommand;
    }
    if (err != CHIP_NO_ERROR)
    {
        // We have no idea what happened; just report failure.
        return err;
    }

    // No error during execution, but no response was added so return Success.
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> HandleRemoveFabric(CommandHandler * commandObj,
                                                                const ConcreteCommandPath & commandPath,
                                                                TLV::TLVReader & input_arguments, FabricTable & fabricTable)
{
    MATTER_TRACE_SCOPE("RemoveFabric", "OperationalCredentials");
    Commands::RemoveFabric::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    auto & fabricBeingRemoved = commandData.fabricIndex;

    ChipLogProgress(Zcl, "OpCreds: Received a RemoveFabric Command for FabricIndex 0x%x",
                    static_cast<unsigned>(fabricBeingRemoved));

    if (!IsValidFabricIndex(fabricBeingRemoved))
    {
        ChipLogError(Zcl, "OpCreds: Failed RemoveFabric due to invalid FabricIndex");
        return Status::InvalidCommand;
    }

    CHIP_ERROR err = fabricTable.Delete(fabricBeingRemoved);
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
        return std::nullopt;
    }
    if (err != CHIP_NO_ERROR)
    {
        // We have no idea what happened; just report failure.
        ChipLogError(Zcl, "OpCreds: Failed RemoveFabric due to internal error (err = %" CHIP_ERROR_FORMAT ")", err.Format());
        return err;
    }

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
        sessionManager->ExpireAllSessionsForFabric(fabricBeingRemoved);
    }
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> HandleSignVIDVerificationRequest(CommandHandler * commandObj,
                                                                              const ConcreteCommandPath & commandPath,
                                                                              TLV::TLVReader & input_arguments,
                                                                              FabricTable & fabricTable)
{
    Commands::SignVIDVerificationRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    ChipLogProgress(Zcl, "OpCreds: Received a SignVIDVerificationRequest Command for FabricIndex 0x%x",
                    static_cast<unsigned>(commandData.fabricIndex));

    if (!IsValidFabricIndex(commandData.fabricIndex) ||
        (commandData.clientChallenge.size() != Crypto::kVendorIdVerificationClientChallengeSize))
    {
        return Status::ConstraintError;
    }

    FabricTable::SignVIDVerificationResponseData responseData;
    ByteSpan attestationChallenge = GetAttestationChallengeFromCurrentSession(commandObj);

    CHIP_ERROR err = fabricTable.SignVIDVerificationRequest(commandData.fabricIndex, commandData.clientChallenge,
                                                            attestationChallenge, responseData);
    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return Status::ConstraintError;
    }

    if (err != CHIP_NO_ERROR)
    {
        // We have no idea what happened; just report failure.
        return err;
    }

    Commands::SignVIDVerificationResponse::Type response;
    response.fabricIndex          = responseData.fabricIndex;
    response.fabricBindingVersion = responseData.fabricBindingVersion;
    response.signature            = responseData.signature.Span();
    commandObj->AddResponse(commandPath, response);

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
HandleCertificateChainRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                              TLV::TLVReader & input_arguments, Credentials::DeviceAttestationCredentialsProvider * dacProvider)
{
    MATTER_TRACE_SCOPE("CertificateChainRequest", "OperationalCredentials");
    Commands::CertificateChainRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    auto & certificateType = commandData.certificateType;

    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t derBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan derBufSpan(derBuf);

    Commands::CertificateChainResponse::Type response;

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
        return Status::InvalidCommand;
    }

    response.certificate = derBufSpan;
    commandObj->AddResponse(commandPath, response);
    return std::nullopt;

exit:
    ChipLogError(Zcl, "OpCreds: Failed CertificateChainRequest: %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}

std::optional<DataModel::ActionReturnStatus>
HandleAttestationRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, TLV::TLVReader & input_arguments,
                         Credentials::DeviceAttestationCredentialsProvider * dacProvider)
{
    MATTER_TRACE_SCOPE("AttestationRequest", "OperationalCredentials");
    OperationalCredentials::Commands::AttestationRequest::DecodableType commandData;
    ReturnErrorOnFailure(commandData.Decode(input_arguments));

    auto & attestationNonce = commandData.attestationNonce;
    auto errorStatus        = Status::Failure;
    CHIP_ERROR err          = CHIP_ERROR_INVALID_ARGUMENT;
    ByteSpan tbsSpan;

    Platform::ScopedMemoryBuffer<uint8_t> attestationElements;
    size_t attestationElementsLen = 0;
    MutableByteSpan attestationElementsSpan;
    uint8_t certDeclBuf[Credentials::kMaxCMSSignedCDMessage]; // Sized to hold the example certificate declaration with 100 PIDs.
                                                              // See DeviceAttestationCredsExample
    MutableByteSpan certDeclSpan(certDeclBuf);

    ByteSpan attestationChallenge = GetAttestationChallengeFromCurrentSession(commandObj);

    // TODO: in future versions, retrieve vendor information to populate the fields below.
    uint32_t timestamp = 0;
    Credentials::DeviceAttestationVendorReservedConstructor emptyVendorReserved(nullptr, 0);

    // TODO: in future versions, also retrieve and use firmware Information
    const ByteSpan kEmptyFirmwareInfo;

    ChipLogProgress(Zcl, "OpCreds: Received an AttestationRequest command");

    VerifyOrExit(attestationNonce.size() == Credentials::kExpectedAttestationNonceSize, errorStatus = Status::InvalidCommand);

    if (dacProvider == nullptr)
    {
        err = CHIP_ERROR_INTERNAL;
        VerifyOrExit(dacProvider != nullptr, errorStatus = Status::Failure);
    }

    err = dacProvider->GetCertificationDeclaration(certDeclSpan);
    VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);

    attestationElementsLen = TLV::EstimateStructOverhead(certDeclSpan.size(), attestationNonce.size(), sizeof(uint64_t) * 8);

    if (!attestationElements.Alloc(attestationElementsLen + attestationChallenge.size()))
    {
        err = CHIP_ERROR_NO_MEMORY;
        VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::ResourceExhausted);
    }

    attestationElementsSpan = MutableByteSpan{ attestationElements.Get(), attestationElementsLen };
    err = Credentials::ConstructAttestationElements(certDeclSpan, attestationNonce, timestamp, kEmptyFirmwareInfo,
                                                    emptyVendorReserved, attestationElementsSpan);
    VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);

    // Append attestation challenge in the back of the reserved space for the signature
    memcpy(attestationElements.Get() + attestationElementsSpan.size(), attestationChallenge.data(), attestationChallenge.size());
    tbsSpan = ByteSpan{ attestationElements.Get(), attestationElementsSpan.size() + attestationChallenge.size() };

    {
        Crypto::P256ECDSASignature signature;
        MutableByteSpan signatureSpan{ signature.Bytes(), signature.Capacity() };

        // Generate attestation signature
        err = dacProvider->SignWithDeviceAttestationKey(tbsSpan, signatureSpan);
        Crypto::ClearSecretData(attestationElements.Get() + attestationElementsSpan.size(), attestationChallenge.size());
        VerifyOrExit(err == CHIP_NO_ERROR, errorStatus = Status::Failure);
        VerifyOrExit(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), errorStatus = Status::Failure);

        Commands::AttestationResponse::Type response;

        response.attestationElements  = attestationElementsSpan;
        response.attestationSignature = signatureSpan;

        ChipLogProgress(Zcl, "OpCreds: AttestationRequest successful.");
        commandObj->AddResponse(commandPath, response);
        return std::nullopt;
    }

exit:
    ChipLogError(Zcl, "OpCreds: Failed AttestationRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                 to_underlying(errorStatus), err.Format());
    return errorStatus;
}

void OnPlatformEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kFailSafeTimerExpired)
    {
        ChipLogError(Zcl, "OpCreds: Got FailSafeTimerExpired");
        OperationalCredentialsCluster::FailSafeCleanup(event, reinterpret_cast<OperationalCredentialsCluster *>(arg));
    }
}
} // anonymous namespace

void OperationalCredentialsCluster::FailSafeCleanup(const DeviceLayer::ChipDeviceEvent * event,
                                                    OperationalCredentialsCluster * cluster)
{
    VerifyOrDie(cluster != nullptr);
    ChipLogError(Zcl, "OpCreds: Proceeding to FailSafeCleanup on fail-safe expiry!");

    bool nocAddedDuringFailsafe          = event->FailSafeTimerExpired.addNocCommandHasBeenInvoked;
    bool nocUpdatedDuringFailsafe        = event->FailSafeTimerExpired.updateNocCommandHasBeenInvoked;
    bool nocAddedOrUpdatedDuringFailsafe = nocAddedDuringFailsafe || nocUpdatedDuringFailsafe;

    FabricIndex fabricIndex = event->FailSafeTimerExpired.fabricIndex;

    // Report Fabrics table change if SetVIDVerificationStatement had been called.
    // There are 4 cases:
    //   1- Fail-safe started, AddNOC/UpdateNOC for fabric A, VVS set for fabric A after that: Need to mark dirty here.
    //   2- Fail-safe started, UpdateNOC/AddNOC for fabric A, VVS set for fabric B after that: No need to mark dirty.
    //   3- Fail-safe started, no UpdateNOC/AddNOC, VVS set for fabric X: No need to mark dirty.
    //   4- ail-safe started, VVS set for fabric A, UpdateNOC for fabric A: No need to mark dirty.
    //
    // Right now we will mark dirty no matter what, as the state-keeping logic for cases 2-4 above
    // was very complex and more likely to be less maintainable than possibly over-reporting Fabrics
    // attribute in this corner case of fail-safe expiry.
    if (event->FailSafeTimerExpired.setVidVerificationStatementHasBeenInvoked)
    {
        // Opcreds cluster is always on Endpoint 0.
        // Only `Fabrics` attribute is reported since `NOCs` is not reportable (`C` quality).```
        cluster->NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
    }

    // If an AddNOC or UpdateNOC command has been successfully invoked, terminate all CASE sessions associated with the Fabric
    // whose Fabric Index is recorded in the Fail-Safe context (see ArmFailSafe Command) by clearing any associated Secure
    // Session Context at the Server.
    if (nocAddedOrUpdatedDuringFailsafe)
    {
        cluster->GetSessionManager().ExpireAllSessionsForFabric(fabricIndex);
    }

    cluster->GetFabricTable().RevertPendingFabricData();

    // If an AddNOC command had been successfully invoked, achieve the equivalent effect of invoking the RemoveFabric command
    // against the Fabric Index stored in the Fail-Safe Context for the Fabric Index that was the subject of the AddNOC
    // command.
    if (nocAddedDuringFailsafe)
    {
        CHIP_ERROR err = cluster->GetFabricTable().Delete(fabricIndex);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "OpCreds: failed to delete fabric at index %u: %" CHIP_ERROR_FORMAT, fabricIndex, err.Format());
        }
    }

    if (nocUpdatedDuringFailsafe)
    {
        // Operational identities/records available may have changed due to NodeID update. Need to refresh all records.
        // The case of fabric removal that reverts AddNOC is handled by the `DeleteFabricFromTable` flow above.
        cluster->GetDNSSDServer().StartServer();
    }
}

CHIP_ERROR OperationalCredentialsCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    mOpCredsContext.fabricTable.AddFabricDelegate(this);
    DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void OperationalCredentialsCluster::Shutdown()
{
    DeviceLayer::PlatformMgrImpl().RemoveEventHandler(OnPlatformEventHandler);
    mOpCredsContext.fabricTable.RemoveFabricDelegate(this);
    DefaultServerCluster::Shutdown();
}

CHIP_ERROR OperationalCredentialsCluster::Attributes(const ConcreteClusterPath & path,
                                                     ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(OperationalCredentials::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>());
}

DataModel::ActionReturnStatus OperationalCredentialsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case OperationalCredentials::Attributes::ClusterRevision::Id:
        return encoder.Encode(OperationalCredentials::kRevision);
    case OperationalCredentials::Attributes::FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case OperationalCredentials::Attributes::NOCs::Id:
        return ReadNOCs(encoder, mOpCredsContext.fabricTable);
    case OperationalCredentials::Attributes::Fabrics::Id:
        return ReadFabricsList(encoder, mOpCredsContext.fabricTable);
    case OperationalCredentials::Attributes::SupportedFabrics::Id:
        return encoder.Encode(static_cast<uint8_t>(CHIP_CONFIG_MAX_FABRICS));
    case OperationalCredentials::Attributes::CommissionedFabrics::Id:
        return encoder.Encode(mOpCredsContext.fabricTable.FabricCount());
    case OperationalCredentials::Attributes::TrustedRootCertificates::Id:
        return ReadRootCertificates(encoder, mOpCredsContext.fabricTable);
    case OperationalCredentials::Attributes::CurrentFabricIndex::Id:
        return encoder.Encode(static_cast<uint8_t>(encoder.AccessingFabricIndex()));
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OperationalCredentialsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = { Commands::AttestationRequest::kMetadataEntry,
                                                                             Commands::CertificateChainRequest::kMetadataEntry,
                                                                             Commands::CSRRequest::kMetadataEntry,
                                                                             Commands::AddNOC::kMetadataEntry,
                                                                             Commands::UpdateNOC::kMetadataEntry,
                                                                             Commands::UpdateFabricLabel::kMetadataEntry,
                                                                             Commands::RemoveFabric::kMetadataEntry,
                                                                             Commands::AddTrustedRootCertificate::kMetadataEntry,
                                                                             Commands::SetVIDVerificationStatement::kMetadataEntry,
                                                                             Commands::SignVIDVerificationRequest::kMetadataEntry };

    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR OperationalCredentialsCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                            ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = { Commands::AttestationResponse::Id, Commands::CertificateChainResponse::Id,
                                                        Commands::CSRResponse::Id, Commands::NOCResponse::Id,
                                                        Commands::SignVIDVerificationResponse::Id };

    return builder.ReferenceExisting(kGeneratedCommands);
}

std::optional<DataModel::ActionReturnStatus> OperationalCredentialsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                          TLV::TLVReader & input_arguments,
                                                                                          CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case OperationalCredentials::Commands::AttestationRequest::Id:
        return HandleAttestationRequest(handler, request.path, input_arguments, GetDACProvider());
    case OperationalCredentials::Commands::CertificateChainRequest::Id:
        return HandleCertificateChainRequest(handler, request.path, input_arguments, GetDACProvider());
    case OperationalCredentials::Commands::CSRRequest::Id:
        return HandleCSRRequest(handler, request.path, input_arguments, GetFabricTable(), GetFailSafeContext(), GetDACProvider());
    case OperationalCredentials::Commands::AddNOC::Id: {
        bool reportChange = false;
        std::optional<DataModel::ActionReturnStatus> returnStatus =
            HandleAddNOC(handler, request.path, input_arguments, GetFabricTable(), GetFailSafeContext(), GetDNSSDServer(),
                         GetCommissioningWindowManager(), reportChange);
        if (reportChange)
        {
            // Notify the attributes containing fabric metadata can be read with new data
            NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
            // Notify we have one more fabric
            NotifyAttributeChanged(OperationalCredentials::Attributes::CommissionedFabrics::Id);
        }
        return returnStatus;
    }
    case OperationalCredentials::Commands::UpdateNOC::Id:
        return HandleUpdateNOC(handler, input_arguments, request, GetFabricTable(), GetFailSafeContext(), GetDNSSDServer());
    case OperationalCredentials::Commands::UpdateFabricLabel::Id: {
        std::optional<DataModel::ActionReturnStatus> returnStatus =
            HandleUpdateFabricLabel(handler, input_arguments, request, GetFabricTable());
        if (!returnStatus.has_value())
        {
            // Succeeded at updating the label, mark Fabrics table changed.
            NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
        }
        return returnStatus;
    }
    case OperationalCredentials::Commands::RemoveFabric::Id:
        return HandleRemoveFabric(handler, request.path, input_arguments, GetFabricTable());
    case OperationalCredentials::Commands::AddTrustedRootCertificate::Id:
        return HandleAddTrustedRootCertificate(handler, request.path, input_arguments, GetFabricTable(), GetFailSafeContext());
    case OperationalCredentials::Commands::SetVIDVerificationStatement::Id: {
        bool reportChange                                         = false;
        std::optional<DataModel::ActionReturnStatus> returnStatus = HandleSetVIDVerificationStatement(
            handler, input_arguments, request, GetFabricTable(), GetFailSafeContext(), reportChange);
        if (reportChange)
        {
            // Handle dirty-marking if anything changed. Only `Fabrics` attribute is reported since `NOCs`
            // is not reportable (`C` quality).
            mOpCredsContext.failSafeContext.RecordSetVidVerificationStatementHasBeenInvoked();
            // Report if Fabric attribute has changed.
            NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
        }
        return returnStatus;
    }
    case OperationalCredentials::Commands::SignVIDVerificationRequest::Id:
        return HandleSignVIDVerificationRequest(handler, request.path, input_arguments, GetFabricTable());
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

void OperationalCredentialsCluster::FabricWillBeRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{

    // TODO: Most of this implementation seems to be related to BasicInformation cluster, not sure if this link with OpCreds is ok.
    // The Leave event SHOULD be emitted by a Node prior to permanently leaving the Fabric.
    // This applies only for the BasicInformation cluster that is always in the Root endpoint and it
    // is mandatory.
    BasicInformation::Events::Leave::Type event;
    event.fabricIndex = fabricIndex;
    (void) mContext->interactionContext.eventsGenerator.GenerateEvent(event, kRootEndpointId);

    // Try to send the queued events as soon as possible for this fabric. If the just emitted leave event won't
    // be sent this time, it will likely not be delivered at all for the following reasons:
    // - removing the fabric expires all associated ReadHandlers, so all subscriptions to
    //   the leave event will be cancelled.
    // - removing the fabric removes all associated access control entries, so generating
    //   subsequent reports containing the leave event will fail the access control check.
    mContext->interactionContext.eventsGenerator.ScheduleUrgentEventDeliverySync(fabricIndex);
}

void OperationalCredentialsCluster::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    ChipLogProgress(Zcl, "OpCreds: Fabric index 0x%x was removed", static_cast<unsigned>(fabricIndex));

    // We need to withdraw the advertisement for the now-removed fabric, so need
    // to restart advertising altogether.
    GetDNSSDServer().StartServer();

    EventManagement::GetInstance().FabricRemoved(fabricIndex);

    NotifyAttributeChanged(OperationalCredentials::Attributes::CommissionedFabrics::Id);
    NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
}

void OperationalCredentialsCluster::OnFabricUpdated(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    NotifyAttributeChanged(OperationalCredentials::Attributes::CommissionedFabrics::Id);
    NotifyAttributeChanged(OperationalCredentials::Attributes::Fabrics::Id);
}

FabricTable & OperationalCredentialsCluster::GetFabricTable()
{
    return mOpCredsContext.fabricTable;
}

FailSafeContext & OperationalCredentialsCluster::GetFailSafeContext()
{
    return mOpCredsContext.failSafeContext;
}

Credentials::DeviceAttestationCredentialsProvider * OperationalCredentialsCluster::GetDACProvider()
{
    // TODO: This dependency should be removed after fixing #41122 so we don't depend on external singletons,
    return Credentials::GetDeviceAttestationCredentialsProvider();
}

SessionManager & OperationalCredentialsCluster::GetSessionManager()
{
    return mOpCredsContext.sessionManager;
}

DnssdServer & OperationalCredentialsCluster::GetDNSSDServer()
{
    return mOpCredsContext.dnssdServer;
}

CommissioningWindowManager & OperationalCredentialsCluster::GetCommissioningWindowManager()
{
    return mOpCredsContext.commissioningWindowManager;
}

void OperationalCredentialsCluster::OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex)
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
