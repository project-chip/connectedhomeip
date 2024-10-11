/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <string.h> // For mem* functions.

#include <optional>

#include <app/util/basic-types.h>
#include <lib/core/NodeId.h>

static_assert(sizeof(chip::NodeId) == sizeof(uint64_t), "Unexpected node if size");

/**
 * @brief Defines binding types.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberBindingType
#else
typedef uint8_t EmberBindingType;
enum
#endif
{
    /** A binding that is currently not in use. */
    MATTER_UNUSED_BINDING = 0,
    /** A unicast binding whose 64-bit identifier is the destination EUI64. */
    MATTER_UNICAST_BINDING = 1,
    /** A multicast binding whose 64-bit identifier is the group address. This
     * binding can be used to send messages to the group and to receive
     * messages sent to the group. */
    MATTER_MULTICAST_BINDING = 3,
};

/**
 * @brief Size of EUI64 (an IEEE address) in bytes (8).
 */
#define EUI64_SIZE 8

/**
 * @brief EUI 64-bit ID (an IEEE address).
 */
typedef uint8_t EmberEUI64[EUI64_SIZE];

/**
 * @brief 16-bit ZigBee network address.
 */
typedef uint16_t EmberNodeId;

/**
 * @brief 802.15.4 PAN ID.
 */
typedef uint16_t EmberPanId;

/** @brief Defines an entry in the binding table.
 *
 * A binding entry specifies a local endpoint, a remote endpoint, a
 * cluster ID and either the destination EUI64 (for unicast bindings) or the
 * 64-bit group address (for multicast bindings).
 */
struct EmberBindingTableEntry
{
    static EmberBindingTableEntry ForNode(chip::FabricIndex fabric, chip::NodeId node, chip::EndpointId localEndpoint,
                                          chip::EndpointId remoteEndpoint, std::optional<chip::ClusterId> cluster)
    {
        EmberBindingTableEntry entry = {
            .type        = MATTER_UNICAST_BINDING,
            .fabricIndex = fabric,
            .local       = localEndpoint,
            .clusterId   = cluster,
            .remote      = remoteEndpoint,
            .nodeId      = node,
        };
        return entry;
    }

    static EmberBindingTableEntry ForGroup(chip::FabricIndex fabric, chip::GroupId group, chip::EndpointId localEndpoint,
                                           std::optional<chip::ClusterId> cluster)
    {
        EmberBindingTableEntry entry = {
            .type        = MATTER_MULTICAST_BINDING,
            .fabricIndex = fabric,
            .local       = localEndpoint,
            .clusterId   = cluster,
            .remote      = 0,
            .groupId     = group,
        };
        return entry;
    }

    /** The type of binding. */
    EmberBindingType type = MATTER_UNUSED_BINDING;

    chip::FabricIndex fabricIndex;
    /** The endpoint on the local node. */
    chip::EndpointId local;
    /** A cluster ID that matches one from the local endpoint's simple descriptor.
     * This cluster ID is set by the provisioning application to indicate which
     * part an endpoint's functionality is bound to this particular remote node
     * and is used to distinguish between unicast and multicast bindings. Note
     * that a binding can be used to to send messages with any cluster ID, not
     * just that listed in the binding.
     */
    std::optional<chip::ClusterId> clusterId;
    /** The endpoint on the remote node (specified by \c identifier). */
    chip::EndpointId remote;
    /** A 64-bit destination identifier.  This is either:
     * - The destination chip::NodeId, for unicasts.
     * - A multicast ChipGroupId, for multicasts.
     * Which one is being used depends on the type of this binding.
     */
    union
    {
        chip::NodeId nodeId;
        chip::GroupId groupId;
    };

    bool operator==(EmberBindingTableEntry const & other) const
    {
        if (type != other.type)
        {
            return false;
        }

        if (type == MATTER_MULTICAST_BINDING && groupId != other.groupId)
        {
            return false;
        }

        if (type == MATTER_UNICAST_BINDING && (nodeId != other.nodeId || remote != other.remote))
        {
            return false;
        }

        return fabricIndex == other.fabricIndex && local == other.local && clusterId == other.clusterId;
    }
};

/**
 * @brief Function pointer for timer callback
 */
typedef void (*TimerCallback)(chip::EndpointId);

/** @brief The control structure for events.
 *
 * It holds the callback and its parameters.
 */
typedef struct
{
    /* Callback information */
    TimerCallback callback;
    chip::EndpointId endpoint;

} EmberEventControl;

/**
 * @brief Returns the value of the bitmask \c bits within
 * the register or byte \c reg.
 */
#define READBITS(reg, bits) ((reg) & (bits))

#define MILLISECOND_TICKS_PER_SECOND 1000
#define MILLISECOND_TICKS_PER_DECISECOND (MILLISECOND_TICKS_PER_SECOND / 10)
