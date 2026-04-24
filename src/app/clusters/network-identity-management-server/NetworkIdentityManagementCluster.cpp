/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/network-identity-management-server/NetworkIdentityManagementCluster.h>

#include <app/clusters/network-identity-management-server/Logging.h>
#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/NetworkIdentityManagement/Attributes.h>
#include <clusters/NetworkIdentityManagement/CommandIds.h>
#include <clusters/NetworkIdentityManagement/Commands.h>
#include <clusters/NetworkIdentityManagement/Metadata.h>
#include <clusters/NetworkIdentityManagement/Structs.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkIdentityManagement;
using namespace chip::app::Clusters::NetworkIdentityManagement::Attributes;
using namespace chip::Crypto;

using Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

NetworkIdentityManagementCluster::NetworkIdentityManagementCluster(EndpointId endpoint, NetworkIdentityStorage & storage,
                                                                   Crypto::NetworkIdentityKeystore & keystore,
                                                                   NetworkIdentityManagement::AuthenticatorDriver & authenticator) :
    DefaultServerCluster({ endpoint, Id }),
    mStorage(storage), mKeystore(keystore), mAuthenticator(authenticator)
{}

CHIP_ERROR NetworkIdentityManagementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    mAuthenticator.OnStartup(*this, mStorage);
    return CHIP_NO_ERROR;
}

void NetworkIdentityManagementCluster::Shutdown(ClusterShutdownType type)
{
    mAuthenticator.OnShutdown();
    DefaultServerCluster::Shutdown(type);
}

DataModel::ActionReturnStatus NetworkIdentityManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                              AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        return encoder.Encode(NetworkIdentityManagement::kRevision);
    case ActiveNetworkIdentities::Id:
        return ReadActiveNetworkIdentities(encoder);
    case Clients::Id:
        return ReadClients(encoder);
    case ClientTableSize::Id:
        return encoder.Encode(mStorage.GetClientTableCapacity());
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus NetworkIdentityManagementCluster::ReadActiveNetworkIdentities(AttributeValueEncoder & encoder)
{
    using NetworkIdentityFlags = NetworkIdentityStorage::NetworkIdentityFlags;
    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        constexpr BitFlags<NetworkIdentityFlags> flags(NetworkIdentityFlags::kPopulateIdentifier,
                                                       NetworkIdentityFlags::kPopulateCreatedTimestamp,
                                                       NetworkIdentityFlags::kPopulateClientCount);
        NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iterator(
            mStorage.IterateNetworkIdentities(flags, MutableByteSpan()));
        VerifyOrReturnError(iterator.IsValid(), CHIP_ERROR_INTERNAL);

        NetworkIdentityStorage::NetworkIdentityEntry entry;
        while (iterator.Next(entry))
        {
            Structs::ActiveNetworkIdentityStruct::Type item;
            item.index            = entry.index;
            item.type             = entry.type;
            item.identifier       = entry.identifier;
            item.createdTimestamp = entry.createdTimestamp.count();
            item.current          = entry.current;
            if (!entry.current)
            {
                item.remainingClients = entry.clientCount;
            }
            ReturnErrorOnFailure(listEncoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;

    switch (request.path.mCommandId)
    {
    case AddClient::Id:
        return HandleAddClient(request, input_arguments, handler);
    case RemoveClient::Id:
        return HandleRemoveClient(request, input_arguments, handler);
    case QueryIdentity::Id:
        return HandleQueryIdentity(request, input_arguments, handler);
    case ImportAdminSecret::Id:
        return HandleImportAdminSecret(request, input_arguments, handler);
    case ExportAdminSecret::Id:
        return HandleExportAdminSecret(request, handler);
    default:
        return Status::UnsupportedCommand;
    }
}

CHIP_ERROR NetworkIdentityManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                        ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span<const DataModel::AttributeEntry>(NetworkIdentityManagement::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>());
}

CHIP_ERROR NetworkIdentityManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                              ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace NetworkIdentityManagement::Commands;
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        AddClient::kMetadataEntry,         RemoveClient::kMetadataEntry,      QueryIdentity::kMetadataEntry,
        ImportAdminSecret::kMetadataEntry, ExportAdminSecret::kMetadataEntry,
    };
    return builder.AppendElements(kAcceptedCommands);
}

CHIP_ERROR NetworkIdentityManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                               ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace NetworkIdentityManagement::Commands;
    static constexpr CommandId kGeneratedCommands[] = {
        AddClientResponse::Id,
        QueryIdentityResponse::Id,
        ExportAdminSecretResponse::Id,
    };
    return builder.AppendElements(kGeneratedCommands);
}

CHIP_ERROR NetworkIdentityManagementCluster::FindRetirableNetworkIdentities(uint16_t & outTotalCount, Span<uint16_t> & outIndices)
{
    NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iterator(
        mStorage.IterateNetworkIdentities(NetworkIdentityStorage::NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    VerifyOrReturnError(iterator.IsValid(), CHIP_ERROR_INTERNAL);

    size_t count = iterator.Count();
    VerifyOrReturnError(CanCastTo<uint16_t>(count), CHIP_ERROR_INTERNAL);
    outTotalCount = static_cast<uint16_t>(count);

    size_t found = 0;
    NetworkIdentityStorage::NetworkIdentityEntry entry;
    while (found < outIndices.size() && iterator.Next(entry))
    {
        if (!entry.current && entry.clientCount == 0)
        {
            outIndices[found++] = entry.index;
        }
    }
    outIndices.reduce_size(found);
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkIdentityManagementCluster::RetireNetworkIdentity(uint16_t index)
{
    // Fetch the identity details to pass to the authenticator.
    // We also need the keypair handle so we can destroy it.
    SensitiveDataBuffer<NetworkIdentityStorage::NetworkIdentityBufferSize()> buffer;
    NetworkIdentityStorage::NetworkIdentityEntry entry;
    ReturnErrorOnFailure(mStorage.FindNetworkIdentity(index, entry, NetworkIdentityStorage::NetworkIdentityFlags::kPopulateAll,
                                                      MutableByteSpan(buffer.Bytes(), buffer.Capacity())));

    // Get the ECDSA P256 keypair handle
    P256KeypairHandle ecdsaHandle;
    ReturnErrorOnFailure(entry.GetKeypairHandle(ecdsaHandle));

    // Remove from storage and destroy the keypair
    ReturnErrorOnFailure(mStorage.RemoveNetworkIdentity(index));
    mKeystore.DestroyNetworkIdentityKeypair(ecdsaHandle);

    // Finally notify the authenticator
    mAuthenticator.OnNetworkIdentityRemoved(entry);

    // Note: Calling NotifyAttributeChanged() is handled by the caller
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::HandleImportAdminSecret(const DataModel::InvokeRequest & request,
                                                          TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;

    VerifyOrReturnValue(request.subjectDescriptor.authMode == Access::AuthMode::kCase, Status::UnsupportedAccess);

    ImportAdminSecret::DecodableType commandData;
    VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

    // Decode the NASS TLV payload (also validates the mandatory field is present)
    NetworkAdministratorSecretData newSecretData;
    VerifyOrReturnError(DecodeNetworkAdministratorSecret(commandData.networkAdministratorSharedSecret, newSecretData) ==
                            CHIP_NO_ERROR,
                        Status::InvalidCommand);

    // Check if there's an existing NASS and validate timestamp
    NetworkIdentityStorage::NetworkAdministratorSecretInfo oldSecretInfo;
    CHIP_ERROR err = mStorage.RetrieveNetworkAdministratorSecret(oldSecretInfo);
    ReturnErrorAndLogOnFailure(err.NoErrorIf(CHIP_ERROR_NOT_FOUND), Zcl, "ImportAdminSecret: Failed to retrieve existing NASS");
    bool hasExistingSecret = (err == CHIP_NO_ERROR);

    if (hasExistingSecret)
    {
        // Handle an exact match (same timestamp and raw secret) as a no-op success. This makes
        // the command idempotent under retries, e.g. due to a client not receiving our response.
        if (newSecretData.created == oldSecretInfo.createdTimestamp)
        {
            NetworkAdministratorRawSecret existingRawSecret;
            if (mKeystore.ExportNetworkAdministratorSecret(oldSecretInfo.secretHandle, existingRawSecret) == CHIP_NO_ERROR &&
                IsBufferContentEqualConstantTime(newSecretData.rawSecret.ConstBytes(), existingRawSecret.ConstBytes(),
                                                 NetworkAdministratorRawSecret::Length()))
            {
                return Status::Success;
            }
        }

        // Apart from the exact-match special case handled above, the created timestamp must strictly increase.
        VerifyOrReturnValue(newSecretData.created > oldSecretInfo.createdTimestamp, Status::DynamicConstraintError);
    }

    // Find retirable (non-current, zero-client) Network Identities so we can work out
    // if there will be enough capacity (including retirements) before making any changes.
    uint16_t networkIdentityCount;
    uint16_t retirableIndicesBuf[NetworkIdentityStorage::kMaxNetworkIdentities];
    Span<uint16_t> retirableIndices(retirableIndicesBuf);
    ReturnErrorAndLogOnFailure(FindRetirableNetworkIdentities(networkIdentityCount, retirableIndices), Zcl,
                               "ImportAdminSecret: Failed to check for retirable Network Identities");
    VerifyOrReturnValue(networkIdentityCount - retirableIndices.size() + 1 <= NetworkIdentityStorage::kMaxNetworkIdentities,
                        Status::ResourceExhausted);

    // Give the authenticator driver a chance propagate back an error if it is in a bad state.
    ReturnErrorAndLogOnFailure(mAuthenticator.PrepareNetworkIdentityAddition(), Zcl,
                               "ImportAdminSecret: Authenticator driver not ready");

    // Delete the Network Identities we have identified for retirement. There are no clear
    // requirements for what to do if this fails; technically we could carry on anyway
    // as long as we will have enough table capacity, but if we're encountering storage
    // errors here something is probably wrong anyway and it's safer to bail out.
    // Note: Retirements are not atomic with respect to the overall import operation;
    // this is acceptable since the key invariant is only that the new NASS and the new
    // derived NIs get committed as one unit. (To be retired, NIs have to be non-current,
    // i.e. they came from a NASS that we don't have anymore.) If any NIs are retired we
    // need to notify the ActiveNetworkIdentities even if the actual import fails.
    bool activeNetworkIdentitiesChanged      = false;
    auto notifyActiveNetworkIdentitiesOnExit = ScopeExit([&]() {
        if (activeNetworkIdentitiesChanged)
        {
            NotifyAttributeChanged(ActiveNetworkIdentities::Id);
        }
    });
    for (uint16_t index : retirableIndices)
    {
        ReturnErrorOnFailure(RetireNetworkIdentity(index));
        activeNetworkIdentitiesChanged = true;
    }

    // Import the NASS into the keystore. Place the handle directly in the NetworkAdministratorSecretInfo
    // struct that we will be persisting later, since HkdfKeyHandle is not movable.
    NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
    nassInfo.createdTimestamp = newSecretData.created;
    ReturnErrorAndLogOnFailure(mKeystore.ImportNetworkAdministratorSecret(newSecretData.rawSecret, nassInfo.secretHandle), Zcl,
                               "ImportAdminSecret: Failed to import NASS into keystore");
    auto destroyNassHandleOnExit = ScopeExit([&]() { mKeystore.DestroyNetworkAdministratorSecret(nassInfo.secretHandle); });

    // Derive the ECDSA Network Identity
    P256KeypairHandle ecdsaKeypairHandle;
    uint8_t identityBuf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan identity(identityBuf);
    ReturnErrorAndLogOnFailure(mKeystore.DeriveECDSANetworkIdentity(nassInfo.secretHandle, ecdsaKeypairHandle, identity), Zcl,
                               "ImportAdminSecret: Failed to derive Network Identity");
    auto destroyEcdsaKeypairOnExit = ScopeExit([&]() { mKeystore.DestroyNetworkIdentityKeypair(ecdsaKeypairHandle); });

    // Derive the 20-byte Network Identifier from the identity
    Credentials::CertificateKeyIdStorage identifier;
    ReturnErrorAndLogOnFailure(Credentials::ExtractIdentifierFromChipNetworkIdentity(identity, identifier), Zcl,
                               "ImportAdminSecret: Failed to extract Network Identifier");

    // Reject if an NI with the same identifier already exists. (The only practical
    // way for this to occur is re-importing an old secret with an updated timestamp.)
    {
        NetworkIdentityStorage::NetworkIdentityEntry existingEntry;
        err = mStorage.FindNetworkIdentity(identifier, existingEntry, {}, MutableByteSpan());
        VerifyOrReturnValue(err != CHIP_NO_ERROR, Status::DynamicConstraintError); // conflicting record found
        ReturnErrorAndLogOnFailure(err.NoErrorIf(CHIP_ERROR_NOT_FOUND), Zcl,
                                   "ImportAdminSecret: Failed to check for duplicate Network Identity");
    }

    // Build the NetworkIdentityInfo for storage (keypair handle stored as opaque bytes).
    // Using NetworkIdentityEntry so the convenience overload populates index/current etc.
    NetworkIdentityStorage::NetworkIdentityEntry ecdsaIdentityEntry;
    ecdsaIdentityEntry.type            = IdentityTypeEnum::kEcdsa;
    ecdsaIdentityEntry.identifier      = identifier;
    ecdsaIdentityEntry.compactIdentity = identity;
    ecdsaIdentityEntry.keypairHandle   = ecdsaKeypairHandle.Span();

    // Atomically store NASS + derived identity
    NetworkIdentityStorage::NetworkIdentityEntry * identities[] = { &ecdsaIdentityEntry };
    err = mStorage.StoreNetworkAdministratorSecretAndDerivedIdentities(nassInfo, Span(identities));
    VerifyOrReturnValue(err != CHIP_ERROR_NO_MEMORY, Status::ResourceExhausted);
    ReturnErrorAndLogOnFailure(err, Zcl, "ImportAdminSecret: Failed to store NASS and derived identity");

    // Import successful, disarm ScopeExits
    destroyNassHandleOnExit.release();
    destroyEcdsaKeypairOnExit.release();
    activeNetworkIdentitiesChanged = true; // NotifyAttributeChanged via ScopeExit

    // Destroy old NASS handle if there was one
    if (hasExistingSecret)
    {
        mKeystore.DestroyNetworkAdministratorSecret(oldSecretInfo.secretHandle);
    }

    mAuthenticator.OnNetworkIdentityAdded(ecdsaIdentityEntry);

    ChipLogProgress(Zcl, "NASS successfully imported by fabric %u node 0x" ChipLogFormatX64, //
                    request.subjectDescriptor.fabricIndex, ChipLogValueX64(request.subjectDescriptor.subject));
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::HandleExportAdminSecret(const DataModel::InvokeRequest & request, CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;

    VerifyOrReturnValue(request.subjectDescriptor.authMode == Access::AuthMode::kCase, Status::UnsupportedAccess);

    // Retrieve NASS metadata from storage
    NetworkIdentityStorage::NetworkAdministratorSecretInfo secretInfo;
    CHIP_ERROR err = mStorage.RetrieveNetworkAdministratorSecret(secretInfo);
    VerifyOrReturnValue(err != CHIP_ERROR_NOT_FOUND, Status::NotFound);
    ReturnErrorAndLogOnFailure(err, Zcl, "ExportAdminSecret: Failed to retrieve NASS");

    // Export raw secret via keystore
    NetworkAdministratorRawSecret rawSecret;
    ReturnErrorAndLogOnFailure(mKeystore.ExportNetworkAdministratorSecret(secretInfo.secretHandle, rawSecret), Zcl,
                               "ExportAdminSecret: Failed to export raw secret");

    // Encode as NASS TLV
    Crypto::SensitiveDataBuffer<kNetworkAdministratorSecretDataMaxEncodedLength> encodedBuf;
    MutableByteSpan encodedSpan(encodedBuf.Bytes(), encodedBuf.Capacity());
    ReturnErrorAndLogOnFailure(EncodeNetworkAdministratorSecret({ secretInfo.createdTimestamp, rawSecret }, encodedSpan), Zcl,
                               "ExportAdminSecret: Failed to encode NASS");

    ChipLogProgress(Zcl, "NASS successfully exported by fabric %u node 0x" ChipLogFormatX64, //
                    request.subjectDescriptor.fabricIndex, ChipLogValueX64(request.subjectDescriptor.subject));

    // Send response
    ExportAdminSecretResponse::Type response;
    response.networkAdministratorSharedSecret = encodedSpan;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::HandleQueryIdentity(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                      CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;
    constexpr auto kPopulateNetworkIdentity = NetworkIdentityStorage::NetworkIdentityFlags::kPopulateCompactIdentity;
    constexpr auto kPopulateClientIdentity  = NetworkIdentityStorage::ClientFlags::kPopulateCompactIdentity;

    QueryIdentity::DecodableType commandData;
    VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

    // Exactly one of the three optional fields must be present
    int fieldCount =                                  //
        commandData.networkIdentityIndex.HasValue() + //
        commandData.networkIdentityType.HasValue() +  //
        commandData.identifier.HasValue();
    VerifyOrReturnValue(fieldCount == 1, Status::InvalidCommand);

    uint8_t identityBuf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    MutableByteSpan buffer(identityBuf);
    ByteSpan identity;

    // Perform the relevant type of query. Errors for all branches are handled below.
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;
    if (commandData.networkIdentityIndex.HasValue())
    {
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        err      = mStorage.FindNetworkIdentity(commandData.networkIdentityIndex.Value(), entry, kPopulateNetworkIdentity, buffer);
        identity = entry.compactIdentity; // points into identityBuf
    }
    else if (commandData.networkIdentityType.HasValue())
    {
        VerifyOrReturnValue(commandData.networkIdentityType.Value() == IdentityTypeEnum::kEcdsa, Status::ConstraintError);
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        err = mStorage.FindCurrentNetworkIdentity(commandData.networkIdentityType.Value(), entry, kPopulateNetworkIdentity, buffer);
        identity = entry.compactIdentity; // points into identityBuf
    }
    else if (commandData.identifier.HasValue())
    {
        ByteSpan identifierSpan = commandData.identifier.Value();
        VerifyOrReturnValue(identifierSpan.size() == Credentials::kKeyIdentifierLength, Status::ConstraintError);
        Credentials::CertificateKeyId identifier(identifierSpan.data());

        { // Per spec, identifiers are looked up in the Network Identity table first.
            NetworkIdentityStorage::NetworkIdentityEntry entry;
            err      = mStorage.FindNetworkIdentity(identifier, entry, kPopulateNetworkIdentity, buffer);
            identity = entry.compactIdentity; // points into identityBuf
        }
        if (err == CHIP_ERROR_NOT_FOUND) // Otherwise try the client table
        {
            NetworkIdentityStorage::ClientEntry entry;
            err      = mStorage.FindClient(identifier, entry, kPopulateClientIdentity, buffer);
            identity = entry.compactIdentity; // points into identityBuf
        }
    }

    VerifyOrReturnValue(err != CHIP_ERROR_NOT_FOUND, Status::NotFound);
    ReturnErrorAndLogOnFailure(err, Zcl, "QueryIdentity: Storage lookup failed");

    QueryIdentityResponse::Type response;
    response.identity = identity;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

DataModel::ActionReturnStatus NetworkIdentityManagementCluster::ReadClients(AttributeValueEncoder & encoder)
{
    using ClientFlags = NetworkIdentityStorage::ClientFlags;
    return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR {
        constexpr BitFlags<ClientFlags> flags(ClientFlags::kPopulateIdentifier, ClientFlags::kPopulateNetworkIdentityIndex);
        NetworkIdentityStorage::ClientIterator::AutoReleasing iterator(mStorage.IterateClients(flags, MutableByteSpan()));
        VerifyOrReturnError(iterator.IsValid(), CHIP_ERROR_INTERNAL);

        NetworkIdentityStorage::ClientEntry entry;
        while (iterator.Next(entry))
        {
            Structs::ClientStruct::Type item;
            item.clientIndex      = entry.index;
            item.clientIdentifier = entry.identifier;
            if (entry.networkIdentityIndex != NetworkIdentityStorage::kNullNetworkIdentityIndex)
            {
                item.networkIdentityIndex.SetNonNull(entry.networkIdentityIndex);
            }
            ReturnErrorOnFailure(listEncoder.Encode(item));
        }
        return CHIP_NO_ERROR;
    });
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::HandleAddClient(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                  CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;

    AddClient::DecodableType commandData;
    VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

    // Validate the client identity and derive the 20-byte identifier
    Credentials::CertificateKeyIdStorage identifier;
    VerifyOrReturnValue(Credentials::ValidateChipNetworkIdentity(commandData.clientIdentity, identifier) == CHIP_NO_ERROR,
                        Status::DynamicConstraintError);

    // Check for an existing entry with the same identifier
    NetworkIdentityStorage::ClientEntry existingEntry;
    uint8_t existingIdentityBuf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    CHIP_ERROR err = mStorage.FindClient(identifier, existingEntry, NetworkIdentityStorage::ClientFlags::kPopulateCompactIdentity,
                                         MutableByteSpan(existingIdentityBuf));
    ReturnErrorAndLogOnFailure(err.NoErrorIf(CHIP_ERROR_NOT_FOUND), Zcl, "AddClient: load failure for existing client check");
    if (err == CHIP_NO_ERROR)
    {
        // Same identifier found — check if it's the exact same identity (idempotent) or a conflict
        if (existingEntry.compactIdentity.data_equal(commandData.clientIdentity))
        {
            // Idempotent success: return the existing client index
            AddClientResponse::Type response;
            response.clientIndex = existingEntry.index;
            handler->AddResponse(request.path, response);
            return std::nullopt;
        }
        // Different identity with same identifier — conflict
        return Status::AlreadyExists;
    }

    // Build the ClientInfo for storage. Use ClientEntry so the convenience overload
    // populates index and lets us directly pass it on to the authenticator.
    NetworkIdentityStorage::ClientEntry entry;
    entry.identifier      = identifier;
    entry.compactIdentity = commandData.clientIdentity;

    // Give the authenticator driver a chance propagate back an error if it is in a bad state.
    ReturnErrorAndLogOnFailure(mAuthenticator.PrepareClientAddition(), Zcl, "AddClient: Authenticator driver not ready");

    err = mStorage.AddClient(entry);
    VerifyOrReturnValue(err != CHIP_ERROR_NO_MEMORY, Status::ResourceExhausted);
    ReturnErrorAndLogOnFailure(err, Zcl, "AddClient: Failed to store client");

    mAuthenticator.OnClientAdded(entry);
    NotifyAttributeChanged(Clients::Id);

    ChipLogProgress(Zcl, "Client %" PRIu16 " (" ChipLogFormatKeyId ") added by fabric %u node 0x" ChipLogFormatX64, //
                    entry.index, ChipLogValueKeyId(entry.identifier), request.subjectDescriptor.fabricIndex,
                    ChipLogValueX64(request.subjectDescriptor.subject));

    AddClientResponse::Type response;
    response.clientIndex = entry.index;
    handler->AddResponse(request.path, response);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
NetworkIdentityManagementCluster::HandleRemoveClient(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
                                                     CommandHandler * handler)
{
    using namespace NetworkIdentityManagement::Commands;

    RemoveClient::DecodableType commandData;
    VerifyOrReturnError(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

    // Exactly one of clientIndex / clientIdentifier must be present
    int fieldCount = commandData.clientIndex.HasValue() + commandData.clientIdentifier.HasValue();
    VerifyOrReturnValue(fieldCount == 1, Status::InvalidCommand);

    // Find the client entry — need the full entry for the authenticator driver notification,
    // and must load before RemoveClient deletes the detail record.
    uint8_t clientIdentityBuf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    NetworkIdentityStorage::ClientEntry entry;
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    if (commandData.clientIndex.HasValue())
    {
        err = mStorage.FindClient(commandData.clientIndex.Value(), entry, NetworkIdentityStorage::ClientFlags::kPopulateAll,
                                  MutableByteSpan(clientIdentityBuf));
    }
    else if (commandData.clientIdentifier.HasValue())
    {
        ByteSpan idSpan = commandData.clientIdentifier.Value();
        VerifyOrReturnValue(idSpan.size() == Credentials::kKeyIdentifierLength, Status::ConstraintError);
        Credentials::CertificateKeyId keyId(idSpan.data());
        err = mStorage.FindClient(keyId, entry, NetworkIdentityStorage::ClientFlags::kPopulateAll,
                                  MutableByteSpan(clientIdentityBuf));
    }

    VerifyOrReturnValue(err != CHIP_ERROR_NOT_FOUND, Status::NotFound);
    ReturnErrorAndLogOnFailure(err, Zcl, "RemoveClient: Failed to retrieve client");

    uint16_t removedNIIndex;
    err = mStorage.RemoveClient(entry.index, removedNIIndex);
    ReturnErrorAndLogOnFailure(err, Zcl, "RemoveClient: Failed to remove client");

    // Notify the AuthenticatorDriver. This disconnects the client.
    mAuthenticator.OnClientRemoved(entry);

    NotifyAttributeChanged(Clients::Id);

    // Only notify ActiveNetworkIdentities if the removed client referenced a non-current NI,
    // since RemainingClients is suppressed (null) for the current NI.
    if (removedNIIndex != NetworkIdentityStorage::kNullNetworkIdentityIndex)
    {
        NetworkIdentityStorage::NetworkIdentityEntry niEntry;
        if (mStorage.FindNetworkIdentity(removedNIIndex, niEntry, {}, MutableByteSpan()) == CHIP_NO_ERROR && !niEntry.current)
        {
            NotifyAttributeChanged(ActiveNetworkIdentities::Id);
        }
    }

    ChipLogProgress(Zcl, "Client %" PRIu16 " (" ChipLogFormatKeyId ") removed by fabric %u node 0x" ChipLogFormatX64, //
                    entry.index, ChipLogValueKeyId(entry.identifier), request.subjectDescriptor.fabricIndex,
                    ChipLogValueX64(request.subjectDescriptor.subject));
    return Status::Success;
}

void NetworkIdentityManagementCluster::OnClientAuthenticated(uint16_t clientIndex, uint16_t networkIdentityIndex)
{
    VerifyOrReturn(networkIdentityIndex != 0 && networkIdentityIndex != NetworkIdentityStorage::kNullNetworkIdentityIndex);

    NetworkIdentityStorage::NetworkIdentityEntry entry;
    ReturnAndLogOnFailure(mStorage.FindNetworkIdentity(networkIdentityIndex, entry, {}, MutableByteSpan()), Zcl,
                          "OnClientAuthenticated: Failed to load Network Identity %" PRIu16, networkIdentityIndex);

    uint16_t oldNetworkIdentityIndex;
    ReturnAndLogOnFailure(mStorage.UpdateClientNetworkIdentityIndex(clientIndex, networkIdentityIndex, oldNetworkIdentityIndex),
                          Zcl, "OnClientAuthenticated: Failed to update Network Identity for client %" PRIu16, clientIndex);
    VerifyOrReturn(networkIdentityIndex != oldNetworkIdentityIndex); // no change

    // RemainingClients is NULL for current identities, so we only need to signal a change
    // of the ActiveNetworkIdentities attribute if at least one of the NIs is non-current.
    if (entry.current)
    {
        VerifyOrReturn(oldNetworkIdentityIndex != NetworkIdentityStorage::kNullNetworkIdentityIndex); // no need to notify

        // This shouldn't fail since oldNetworkIdentityIndex was just returned from the storage.
        ReturnAndLogOnFailure(mStorage.FindNetworkIdentity(oldNetworkIdentityIndex, entry, {}, MutableByteSpan()), Zcl,
                              "OnClientAuthenticated: Failed to load previous Network Identity %" PRIu16, oldNetworkIdentityIndex);
        VerifyOrReturn(!entry.current); // both current, no need to notify
    }

    NotifyAttributeChanged(ActiveNetworkIdentities::Id);
}

} // namespace chip::app::Clusters
