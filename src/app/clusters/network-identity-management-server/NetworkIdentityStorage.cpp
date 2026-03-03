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

#include <app/clusters/network-identity-management-server/NetworkIdentityStorage.h>

#include <lib/support/CodeUtils.h>

namespace chip::app::Clusters {

CHIP_ERROR NetworkIdentityStorage::NetworkIdentityInfo::GetKeypairHandle(Crypto::P256KeypairHandle & outHandle)
{
    VerifyOrReturnError(type == NetworkIdentityManagement::IdentityTypeEnum::kEcdsa, CHIP_ERROR_INCORRECT_STATE);
    size_t length = keypairHandle.size();
    VerifyOrReturnError(length > 0, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(outHandle.SetLength(length));
    memcpy(outHandle.Bytes(), keypairHandle.data(), length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR
NetworkIdentityStorage::StoreNetworkAdministratorSecretAndDerivedIdentities(const NetworkAdministratorSecretInfo & nassInfo,
                                                                            Span<NetworkIdentityEntry * const> identities)
{
    VerifyOrReturnError(identities.size() <= kMaxNetworkIdentities, CHIP_ERROR_INVALID_ARGUMENT);

    uint16_t outIndices[kMaxNetworkIdentities];
    const NetworkIdentityInfo * infoPtrs[kMaxNetworkIdentities];
    for (size_t i = 0; i < identities.size(); i++)
    {
        infoPtrs[i] = identities.data()[i];
    }

    ReturnErrorOnFailure(StoreNetworkAdministratorSecretAndDerivedIdentities(
        nassInfo, Span<const NetworkIdentityInfo * const>(infoPtrs, identities.size()), Span(outIndices, identities.size())));

    for (size_t i = 0; i < identities.size(); i++)
    {
        auto * entry            = identities.data()[i];
        entry->index            = outIndices[i];
        entry->current          = true;
        entry->createdTimestamp = nassInfo.createdTimestamp;
        entry->clientCount      = 0;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkIdentityStorage::AddClient(ClientEntry & entry)
{
    ReturnErrorOnFailure(AddClient(static_cast<const ClientInfo &>(entry), entry.index));
    entry.networkIdentityIndex = kNullNetworkIdentityIndex;
    return CHIP_NO_ERROR;
}

CHIP_ERROR NetworkIdentityStorage::FindNetworkIdentityByIterating(NetworkIdentityEntry & outEntry,
                                                                  BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer,
                                                                  bool (*predicate)(const NetworkIdentityEntry &, const void *),
                                                                  const void * context)
{
    // Because the iterator itself can't fail, IterateNetworkIdentities() has to
    // front-load the buffer check and return nullptr for both BUFFER_TOO_SMALL and NO_MEMORY.
    // Check the buffer size here manually so we distinguish the two error scenarios.
    VerifyOrReturnError(buffer.size() >= NetworkIdentityBufferSize(flags), CHIP_ERROR_BUFFER_TOO_SMALL);

    NetworkIdentityIterator::AutoReleasing iterator(IterateNetworkIdentities(flags, buffer));
    VerifyOrReturnValue(iterator.IsValid(), CHIP_ERROR_NO_MEMORY);

    while (iterator.Next(outEntry))
    {
        if (predicate(outEntry, context))
        {
            return CHIP_NO_ERROR;
        }
    }

    outEntry = {};
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR NetworkIdentityStorage::FindNetworkIdentityByIterating(const uint16_t index, NetworkIdentityEntry & outEntry,
                                                                  BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer)
{
    return FindNetworkIdentityByIterating(
        outEntry, flags, buffer,
        [](const NetworkIdentityEntry & entry, const void * ctx) { return entry.index == *static_cast<decltype(&index)>(ctx); },
        &index);
}

CHIP_ERROR NetworkIdentityStorage::FindNetworkIdentityByIterating(const Credentials::CertificateKeyId & identifier,
                                                                  NetworkIdentityEntry & outEntry,
                                                                  BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer)
{
    flags.Set(NetworkIdentityFlags::kPopulateIdentifier); // needed for the predicate
    return FindNetworkIdentityByIterating(
        outEntry, flags, buffer,
        [](const NetworkIdentityEntry & entry, const void * ctx) {
            return static_cast<decltype(&identifier)>(ctx)->data_equal(entry.identifier);
        },
        &identifier);
}

CHIP_ERROR NetworkIdentityStorage::FindCurrentNetworkIdentityByIterating(NetworkIdentityManagement::IdentityTypeEnum type,
                                                                         NetworkIdentityEntry & outEntry,
                                                                         BitFlags<NetworkIdentityFlags> flags,
                                                                         MutableByteSpan buffer)
{
    return FindNetworkIdentityByIterating(
        outEntry, flags, buffer,
        [](const NetworkIdentityEntry & entry, const void * ctx) {
            return entry.current && entry.type == *static_cast<const NetworkIdentityManagement::IdentityTypeEnum *>(ctx);
        },
        &type);
}

} // namespace chip::app::Clusters
