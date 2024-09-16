/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <lib/core/ScopedNodeId.h>
#include <lib/core/TLV.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <transport/raw/TCPConfig.h>
#include <transport/raw/TCPParamsStorageInterface.h>

namespace chip {
namespace Transport {
/**
 * @brief Class to store and retrieve peer TCP support parameters.
 * These parameters are received as part of operational session negotiations
 * with peer.
 *
 */
class PeerTCPParamsStorage : public TCPParamsStorageInterface
{
public:
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mTCPParamsStorage = storage;
        return CHIP_NO_ERROR;
    }

    static constexpr size_t kMaxTrackedNodes = CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE;

    virtual ~PeerTCPParamsStorage(){};

    /**
     * Recover supported transports and max TCP message size values for a given
     * fabric-scoped node identity.
     *
     * @param node the node for which to recover TCP parameter information
     * @param supportedTransports the transport types supported by the node
     * @param maxTCPMessageSize the maximum TCP message size that the node is capable of receiving
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_KEY_NOT_FOUND if no TCP parameter information can be found, else an
     * appropriate CHIP error on failure
     */
    CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, uint16_t & supportedTransports, uint32_t & maxTCPMessageSize) override;
    /**
     * Save TCP parameter information to storage.
     *
     * @param node the node for which to recover TCP parameter information
     * @param supportedTransports the transport types supported by the node
     * @param maxTCPMessageSize the maximum TCP message size that the node is capable of receiving
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    CHIP_ERROR SaveTCPParams(const ScopedNodeId & node, const uint16_t & supportedTransports,
                             const uint32_t & maxTCPMessageSize) override;

    CHIP_ERROR DeleteTCPParams(const ScopedNodeId & node) override;
    /**
     * Remove all TCP parameter information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     *
     * @param fabricIndex the index of the fabric for which to remove TCP parameter information
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    CHIP_ERROR DeleteAllTCPParams(FabricIndex fabricIndex) override;

    static StorageKeyName GetStorageKey(const ScopedNodeId & node);

private:
    static constexpr size_t MaxScopedNodeIdSize() { return TLV::EstimateStructOverhead(sizeof(NodeId), sizeof(FabricIndex)); }

    static constexpr size_t MaxNodesListSize()
    {
        // The max size of the list is (1 byte control + bytes for actual value) times max number of list items
        return TLV::EstimateStructOverhead((1 + MaxScopedNodeIdSize()) * kMaxTrackedNodes);
    }

    struct TrackedNodesList
    {
        size_t mSize;
        ScopedNodeId mNodes[kMaxTrackedNodes];
    };

    static constexpr size_t MaxTCPParamsInfoSize() { return TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint32_t)); }

    CHIP_ERROR SaveTCPParamsToStorage(const ScopedNodeId & node, const uint16_t & supportedTransports,
                                      const uint32_t & maxTCPMessageSize);

    CHIP_ERROR LoadTCPParamsFromStorage(const ScopedNodeId & node, uint16_t & supportedTransports, uint32_t & maxTCPMessageSize);

    CHIP_ERROR DeleteTCPParamsFromStorage(const ScopedNodeId & node);

    CHIP_ERROR SaveTrackedNodesList(const TrackedNodesList & list);
    CHIP_ERROR LoadTrackedNodesList(TrackedNodesList & list);

    static constexpr TLV::Tag kFabricIndexTag         = TLV::ContextTag(1);
    static constexpr TLV::Tag kPeerNodeIdTag          = TLV::ContextTag(2);
    static constexpr TLV::Tag kSupportedTransportsTag = TLV::ContextTag(3);
    static constexpr TLV::Tag kMaxTCPMessageSizeTag   = TLV::ContextTag(4);

    PersistentStorageDelegate * mTCPParamsStorage;
};

} // namespace Transport
} // namespace chip
