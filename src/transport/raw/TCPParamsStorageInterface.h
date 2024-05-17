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
#include <transport/raw/TCPConfig.h>

namespace chip {
namespace Transport {
/**
 * @brief Interface to store and retrieve TCP session parameters.
 * These parameters are received as part of operational session negotiations
 * with peer before being stored. Before, attempting to establish a TCP session,
 * the caller uses these APIs to retrieve the corresponding TCP parameters for
 * the specific peer.
 *
 */
class TCPParamsStorageInterface
{
public:
    static constexpr size_t kMaxTrackedNodes = CHIP_CONFIG_PEER_TCP_PARAMS_CACHE_SIZE;

    virtual ~TCPParamsStorageInterface(){};

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
    virtual CHIP_ERROR FindByScopedNodeId(const ScopedNodeId & node, uint16_t & supportedTransports,
                                          uint32_t & maxTCPMessageSize) = 0;
    /**
     * Save TCP parameter information to storage.
     *
     * @param node the node for which to recover TCP parameter information
     * @param supportedTransports the transport types supported by the node
     * @param maxTCPMessageSize the maximum TCP message size that the node is capable of receiving
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR SaveTCPParams(const ScopedNodeId & node, const uint16_t & supportedTransports,
                                     const uint32_t & maxTCPMessageSize) = 0;

    virtual CHIP_ERROR DeleteTCPParams(const ScopedNodeId & node) = 0;
    /**
     * Remove all TCP parameter information associated with the specified
     * fabric index.  If no entries for the fabric index exist, this is a no-op
     * and is considered successful.
     *
     * @param fabricIndex the index of the fabric for which to remove TCP parameter information
     * @return CHIP_NO_ERROR on success, else an appropriate CHIP error on failure
     */
    virtual CHIP_ERROR DeleteAllTCPParams(FabricIndex fabricIndex) = 0;
};

} // namespace Transport
} // namespace chip
