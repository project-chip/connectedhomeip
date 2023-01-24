/*
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *      This file defines the CHIP CASE Session object that provides
 *      APIs for constructing a secure session using a certificate from the device's
 *      operational credentials.
 */

#pragma once

#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/secure_channel/DefaultSessionResumptionStorage.h>

namespace chip {

/**
 * An example SessionResumptionStorage using PersistentStorageDelegate as it backend.
 */
class SimpleSessionResumptionStorage : public DefaultSessionResumptionStorage
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SaveIndex(const SessionIndex & index) override;
    CHIP_ERROR LoadIndex(SessionIndex & index) override;

    CHIP_ERROR SaveLink(ConstResumptionIdView resumptionId, const ScopedNodeId & node) override;
    CHIP_ERROR LoadLink(ConstResumptionIdView resumptionId, ScopedNodeId & node) override;
    CHIP_ERROR DeleteLink(ConstResumptionIdView resumptionId) override;

    CHIP_ERROR SaveState(const ScopedNodeId & node, ConstResumptionIdView resumptionId,
                         const Crypto::P256ECDHDerivedSecret & sharedSecret, const CATValues & peerCATs) override;
    CHIP_ERROR LoadState(const ScopedNodeId & node, ResumptionIdStorage & resumptionId,
                         Crypto::P256ECDHDerivedSecret & sharedSecret, CATValues & peerCATs) override;
    CHIP_ERROR DeleteState(const ScopedNodeId & node) override;

    static StorageKeyName GetStorageKey(const ScopedNodeId & node);
    static StorageKeyName GetStorageKey(ConstResumptionIdView resumptionId);

private:
    static constexpr size_t MaxScopedNodeIdSize() { return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex)); }

    static constexpr size_t MaxIndexSize()
    {
        // The max size of the list is (1 byte control + bytes for actual value) times max number of list items
        return TLV::EstimateStructOverhead((1 + MaxScopedNodeIdSize()) * CHIP_CONFIG_CASE_SESSION_RESUME_CACHE_SIZE);
    }

    static constexpr size_t MaxStateSize()
    {
        return TLV::EstimateStructOverhead(kResumptionIdSize, Crypto::P256ECDHDerivedSecret::Capacity(),
                                           CATValues::kSerializedLength);
    }

    static constexpr TLV::Tag kFabricIndexTag  = TLV::ContextTag(1);
    static constexpr TLV::Tag kPeerNodeIdTag   = TLV::ContextTag(2);
    static constexpr TLV::Tag kResumptionIdTag = TLV::ContextTag(3);
    static constexpr TLV::Tag kSharedSecretTag = TLV::ContextTag(4);
    static constexpr TLV::Tag kCATTag          = TLV::ContextTag(5);

    PersistentStorageDelegate * mStorage;
};

} // namespace chip
