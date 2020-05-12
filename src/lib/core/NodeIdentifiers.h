/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *  @file
 *    Header that defines CHIP constants, bitmasks and handling functions for
 *    node and fabric id values.
 */

#ifndef ID_CONSTANTS_H
#define ID_CONSTANTS_H

namespace chip {

// Special node id values.
enum
{
    kNodeIdNotSpecified = 0ULL,
    kAnyNodeId          = 0xFFFFFFFFFFFFFFFFULL
};

// Special fabric id values.
enum
{
    kFabricIdNotSpecified = 0ULL,

    /** Default fabric ID, which should only be used for testing purposes. */
    kFabricIdDefaultForTest = 1ULL,

    // Ids >= kReservedFabricIdStart and <= kMaxFabricId are reserved for special uses.  The
    // meanings of values in this range are context-specific; e.g. in the IdentifyRequest
    // message, these value are used to match devices that are/are not members of a fabric.
    kReservedFabricIdStart = 0xFFFFFFFFFFFFFF00ULL,

    kMaxFabricId = 0xFFFFFFFFFFFFFFFFULL
};

/**
 * Bit field definitions for IEEE EUI-64 Identifiers.
 */
enum
{
    kEUI64_UL_Mask = 0x0200000000000000ULL, /**< Bitmask for the Universal/Local (U/L) bit within an EUI-64 identifier.
                                                 A value of 0 indicates the id is Universally (globally) administered.
                                                 A value of 1 indicates the id is Locally administered. */

    kEUI64_UL_Unversal = 0, /**< Universal/Local bit value indicating a Universally administered EUI-64 identifier. */

    kEUI64_UL_Local = kEUI64_UL_Mask, /**< Universal/Local bit value indicating a Locally administered EUI-64 identifier. */

    kEUI64_IG_Mask = 0x0100000000000000ULL, /**< Bitmask for the Individual/Group (I/G) bit within an EUI-64 identifier.
                                                 A value of 0 indicates the id is an individual address.
                                                 A value of 1 indicates the id is a group address. */

    kEUI64_IG_Individual = 0, /**< Individual/Group bit value indicating an individual address EUI-64 identifier. */

    kEUI64_IG_Group = kEUI64_IG_Mask, /**< Individual/Group bit value indicating an group address EUI-64 identifier. */
};

/**
 * Special ranges of CHIP Node Ids.
 */
enum
{
    kMaxAlwaysLocalChipNodeId = 0x000000000000FFFFULL, /**< CHIP node identifiers less or equal than this value are considered
                                                           local for testing convenience. */
};

/**
 *  Convert an IPv6 address interface identifier to a CHIP node identifier.
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalChipNodeId
 *  (65535) are considered 'local', and have their universal/local bit is set to zero.
 *  This simplifies the string representation of the corresponding IPv6 addresses.
 *  For example a ULA for node identifier \c 10 would be \c FD00:0:1:1\::A.
 *
 *  @note
 *    When trying to determine if an interface identifier matches a particular node identifier,
 *    always convert the interface identifier to a node identifier and then compare, not
 *    the other way around. This allows for cases where the universal/local bit may not
 *    have been set to 1, either by error or because another addressing convention was
 *    being followed.
 *
 *  @param[in]    interfaceId    The 64 bit interface identifier.
 *
 *  @return the mapped 64 bit CHIP node identifier.
 *
 */
inline uint64_t IPv6InterfaceIdToChipNodeId(uint64_t interfaceId)
{
    return (interfaceId <= kMaxAlwaysLocalChipNodeId) ? interfaceId : (interfaceId ^ kEUI64_UL_Mask);
}

/**
 *  Convert a CHIP node identifier to an IPv6 address interface identifier.
 *
 *  CHIP node identifiers are Universal/Local EUI-64s, which per RFC-3513 are converted to
 *  interface identifiers by inverting the universal/local bit (bit 57 counting the LSB as 0).
 *
 *  As a convenience to testing, node identifiers less or equal than #kMaxAlwaysLocalChipNodeId
 *  (65535) are considered 'local', and have their universal/local bit is set to zero.
 *  This simplifies the string representation of the corresponding IPv6 addresses.
 *  For example a ULA for node identifier \c 10 would be \c FD00:0:1:1\::A.
 *
 *  @note
 *    When trying to determine if an interface identifier matches a particular node identifier,
 *    always convert the interface identifier to a node identifier and then compare, not
 *    the other way around. This allows for cases where the universal/local bit may not
 *    have been set to 1, either by error or because another addressing convention was
 *    being followed.
 *
 *  @param[in]    nodeId    The 64-bit CHIP node identifier.
 *
 *  @return the IPv6 interface identifier.
 *
 */
inline uint64_t ChipNodeIdToIPv6InterfaceId(uint64_t nodeId)
{
    return (nodeId <= kMaxAlwaysLocalChipNodeId) ? nodeId : (nodeId ^ kEUI64_UL_Mask);
}

} // namespace chip

#endif
