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

#include <app/server/Server.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OperationalCredentials/Enums.h>
#include <clusters/OperationalCredentials/AttributeIds.h>
#include <clusters/OperationalCredentials/Commands.h>
#include <clusters/OperationalCredentials/Metadata.h>
#include <credentials/CHIPCert.h>
#include <credentials/CertificationDeclaration.h>
#include <credentials/DeviceAttestationConstructor.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <lib/support/CodeUtils.h>
#include <tracing/macros.h>

/****************************************************************************
 * @file
 * @brief Implementation for the Operational Credentials Cluster
 ***************************************************************************/

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalCredentials;
using namespace chip::Credentials;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr auto kDACCertificate = CertificateChainTypeEnum::kDACCertificate;
constexpr auto kPAICertificate = CertificateChainTypeEnum::kPAICertificate;

// Get the attestation challenge for the current session in progress. Only valid when called
// synchronously from inside a CommandHandler. If not called in CASE/PASE session context,
// return an empty span. This will for sure make the procedures that rely on the challenge
// fail, which is intended as it never should have reached here.
// TODO: Create an alternative way to retrieve the Attestation Challenge without this huge amount of calls.
ByteSpan GetAttestationChallengeFromCurrentSession(app::CommandHandler * commandObj)
{
    VerifyOrDie((commandObj != nullptr) && (commandObj->GetExchangeContext() != nullptr));
    Transport::Session::SessionType sessionType = commandObj->GetExchangeContext()->GetSessionHandle()->GetSessionType();
    VerifyOrReturnValue(sessionType == Transport::Session::SessionType::kSecure, ByteSpan{});

    ByteSpan attestationChallenge =
        commandObj->GetExchangeContext()->GetSessionHandle()->AsSecureSession()->GetCryptoContext().GetAttestationChallenge();
    return attestationChallenge;
}
} // anonymous namespace


CHIP_ERROR ReadNOCs(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        const auto & fabricTable = Server::GetInstance().GetFabricTable();
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
                icacOrVvscSpan = MutableByteSpan { icacOrVvscBuf };
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

CHIP_ERROR ReadSupportedFabrics(AttributeValueEncoder & aEncoder)
{
    uint8_t fabricCount = CHIP_CONFIG_MAX_FABRICS;

    return aEncoder.Encode(fabricCount);
}

CHIP_ERROR ReadFabricsList(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        const auto & fabricTable = Server::GetInstance().GetFabricTable();

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

CHIP_ERROR ReadCommissionedFabrics(AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(Server::GetInstance().GetFabricTable().FabricCount());
}

CHIP_ERROR ReadRootCertificates(AttributeValueEncoder & aEncoder)
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

std::optional<DataModel::ActionReturnStatus> HandleCertificateChainRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, Commands::CertificateChainRequest::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("CertificateChainRequest", "OperationalCredentials");
    auto & certificateType = commandData.certificateType;

    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t derBuf[Credentials::kMaxDERCertLength];
    MutableByteSpan derBufSpan(derBuf);

    Commands::CertificateChainResponse::Type response;

    Credentials::DeviceAttestationCredentialsProvider * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();


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
        return std::nullopt;
    }

    response.certificate = derBufSpan;
    commandObj->AddResponse(commandPath, response);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "OpCreds: Failed CertificateChainRequest: %" CHIP_ERROR_FORMAT, err.Format());
        commandObj->AddStatus(commandPath, Status::Failure);
    }

    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus> HandleAttestationRequest(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, Commands::AttestationRequest::DecodableType & commandData)
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

    ByteSpan attestationChallenge = GetAttestationChallengeFromCurrentSession(commandObj);

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
        Crypto::ClearSecretData(attestationElements.Get() + attestationElementsSpan.size(), attestationChallenge.size());
        VerifyOrExit(err == CHIP_NO_ERROR, finalStatus = Status::Failure);
        VerifyOrExit(signatureSpan.size() == Crypto::P256ECDSASignature::Capacity(), finalStatus = Status::Failure);

        Commands::AttestationResponse::Type response;

        response.attestationElements  = attestationElementsSpan;
        response.attestationSignature = signatureSpan;

        ChipLogProgress(Zcl, "OpCreds: AttestationRequest successful.");
        finalStatus = Protocols::InteractionModel::Status::Success;
        commandObj->AddResponse(commandPath, response);
    }

exit:
    if (finalStatus != Status::Success)
    {
        commandObj->AddStatus(commandPath, finalStatus);
        ChipLogError(Zcl, "OpCreds: Failed AttestationRequest request with IM error 0x%02x (err = %" CHIP_ERROR_FORMAT ")",
                     to_underlying(finalStatus), err.Format());
    }

    return std::nullopt;
}

/************************************************************************************************************/

CHIP_ERROR OperationalCredentialsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    return listBuilder.Append(Span(OperationalCredentials::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>());
}

DataModel::ActionReturnStatus OperationalCredentialsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    switch(request.path.mAttributeId)
    {
    case OperationalCredentials::Attributes::ClusterRevision::Id:
        return encoder.Encode(OperationalCredentials::kRevision);
    case OperationalCredentials::Attributes::FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case OperationalCredentials::Attributes::NOCs::Id:
        return ReadNOCs(encoder);
    case OperationalCredentials::Attributes::Fabrics::Id:
        return ReadFabricsList(encoder);
    case OperationalCredentials::Attributes::SupportedFabrics::Id:
        return ReadSupportedFabrics(encoder);
    case OperationalCredentials::Attributes::CommissionedFabrics::Id:
        return ReadCommissionedFabrics(encoder);
    case OperationalCredentials::Attributes::TrustedRootCertificates::Id:
        return ReadRootCertificates(encoder);
    case OperationalCredentials::Attributes::CurrentFabricIndex::Id:
        return encoder.Encode(encoder.AccessingFabricIndex());
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OperationalCredentialsCluster::AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) 
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Commands::AttestationRequest::kMetadataEntry,
        Commands::CertificateChainRequest::kMetadataEntry,
        Commands::CSRRequest::kMetadataEntry,
        Commands::AddNOC::kMetadataEntry,
        Commands::UpdateNOC::kMetadataEntry,
        Commands::UpdateFabricLabel::kMetadataEntry,
        Commands::RemoveFabric::kMetadataEntry,
        Commands::AddTrustedRootCertificate::kMetadataEntry,
        Commands::SetVIDVerificationStatement::kMetadataEntry,
        Commands::SignVIDVerificationRequest::kMetadataEntry
    };

    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR OperationalCredentialsCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = {
        Commands::AttestationResponse::Id,
        Commands::CertificateChainResponse::Id,
        Commands::CSRResponse::Id,
        Commands::NOCResponse::Id,
        Commands::SignVIDVerificationResponse::Id
    };

    return builder.ReferenceExisting(kGeneratedCommands);
}

std::optional<DataModel::ActionReturnStatus> OperationalCredentialsCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler) 
{
    switch(request.path.mCommandId)
    {
    case OperationalCredentials::Commands::AttestationRequest::Id:
    {
        OperationalCredentials::Commands::AttestationRequest::DecodableType requestData;
        ReturnErrorOnFailure(requestData.Decode(input_arguments));
        return HandleAttestationRequest(handler, request.path, requestData);
    }
    case OperationalCredentials::Commands::CertificateChainRequest::Id:
    {
        OperationalCredentials::Commands::CertificateChainRequest::DecodableType requestData;
        ReturnErrorOnFailure(requestData.Decode(input_arguments));
        return HandleCertificateChainRequest(handler, request.path, requestData);
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}