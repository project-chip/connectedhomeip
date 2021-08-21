/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "ServiceNaming.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>

#include <cstdio>
#include <inttypes.h>
#include <string.h>

namespace chip {
namespace Mdns {

CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId)
{
    ReturnErrorCodeIf(bufferLen <= kOperationalServiceNamePrefix, CHIP_ERROR_BUFFER_TOO_SMALL);

    NodeId nodeId               = peerId.GetNodeId();
    CompressedFabricId fabricId = peerId.GetCompressedFabricId();

    snprintf(buffer, bufferLen, "%08" PRIX32 "%08" PRIX32 "-%08" PRIX32 "%08" PRIX32, static_cast<uint32_t>(fabricId >> 32),
             static_cast<uint32_t>(fabricId), static_cast<uint32_t>(nodeId >> 32), static_cast<uint32_t>(nodeId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractIdFromInstanceName(const char * name, PeerId * peerId)
{
    ReturnErrorCodeIf(name == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(peerId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Make sure the string is long enough.
    static constexpr size_t fabricIdByteLength   = 8;
    static constexpr size_t fabricIdStringLength = fabricIdByteLength * 2;
    static constexpr size_t nodeIdByteLength     = 8;
    static constexpr size_t nodeIdStringLength   = nodeIdByteLength * 2;
    static constexpr size_t totalLength          = fabricIdStringLength + nodeIdStringLength + 1; // +1 for '-'

    // Ensure we have at least totalLength chars.
    size_t len = strnlen(name, totalLength);
    ReturnErrorCodeIf(len < totalLength, CHIP_ERROR_INVALID_ARGUMENT);

    // Check that we have a proper terminator.
    ReturnErrorCodeIf(name[totalLength] != '\0' && name[totalLength] != '.', CHIP_ERROR_WRONG_NODE_ID);

    // Check what we have a separator where we expect.
    ReturnErrorCodeIf(name[fabricIdStringLength] != '-', CHIP_ERROR_WRONG_NODE_ID);

    static constexpr size_t bufferSize = max(fabricIdByteLength, nodeIdByteLength);
    uint8_t buf[bufferSize];

    ReturnErrorCodeIf(Encoding::HexToBytes(name, fabricIdStringLength, buf, bufferSize) == 0, CHIP_ERROR_WRONG_NODE_ID);
    // Buf now stores the fabric id, as big-endian bytes.
    static_assert(fabricIdByteLength == sizeof(uint64_t), "Wrong number of bytes");
    peerId->SetCompressedFabricId(Encoding::BigEndian::Get64(buf));

    ReturnErrorCodeIf(Encoding::HexToBytes(name + fabricIdStringLength + 1, nodeIdStringLength, buf, bufferSize) == 0,
                      CHIP_ERROR_WRONG_NODE_ID);
    // Buf now stores the node id id, as big-endian bytes.
    static_assert(nodeIdByteLength == sizeof(uint64_t), "Wrong number of bytes");
    peerId->SetNodeId(Encoding::BigEndian::Get64(buf));

    return CHIP_NO_ERROR;
}

CHIP_ERROR MakeHostName(char * buffer, size_t bufferLen, const chip::ByteSpan & macOrEui64)
{
    ReturnErrorCodeIf(bufferLen < macOrEui64.size() * 2 + 1, CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t idx = 0;
    for (size_t i = 0; i < macOrEui64.size(); ++i)
    {
        idx += snprintf(buffer + idx, 3, "%02X", macOrEui64.data()[i]);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MakeServiceSubtype(char * buffer, size_t bufferLen, DiscoveryFilter subtype)
{
    size_t requiredSize;
    switch (subtype.type)
    {
    case DiscoveryFilterType::kShort:
        // 4-bit number
        if (subtype.code >= 1 << 4)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_S%u", subtype.code);
        break;
    case DiscoveryFilterType::kLong:
        // 12-bit number
        if (subtype.code >= 1 << 12)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_L%u", subtype.code);
        break;
    case DiscoveryFilterType::kVendor:
        // Vendor ID is 16-bit, so if it fits in the code, it's good.
        // NOTE: size here is wrong, will be changed in upcming PR to remove leading zeros.
        requiredSize = snprintf(buffer, bufferLen, "_V%u", subtype.code);
        break;
    case DiscoveryFilterType::kDeviceType:
        // TODO: Not totally clear the size required here: see spec issue #3226
        requiredSize = snprintf(buffer, bufferLen, "_T%u", subtype.code);
        break;
    case DiscoveryFilterType::kCommissioningMode:
        if (subtype.code > 1)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_C%u", subtype.code);
        break;
    case DiscoveryFilterType::kCommissioner:
        if (subtype.code > 1)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_D%u", subtype.code);
        break;
    case DiscoveryFilterType::kCommissioningModeFromCommand:
        // 1 is the only valid value
        if (subtype.code != 1)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_A1");
        break;
    case DiscoveryFilterType::kInstanceName:
        requiredSize = snprintf(buffer, bufferLen, "%s", subtype.instanceName);
        break;
    case DiscoveryFilterType::kNone:
        requiredSize = 0;
        buffer[0]    = '\0';
        break;
    }
    return (requiredSize <= (bufferLen - 1)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR MakeServiceTypeName(char * buffer, size_t bufferLen, DiscoveryFilter nameDesc, DiscoveryType type)
{
    size_t requiredSize;
    if (nameDesc.type == DiscoveryFilterType::kNone)
    {
        if (type == DiscoveryType::kCommissionableNode)
        {
            requiredSize = snprintf(buffer, bufferLen, kCommissionableServiceName);
        }
        else if (type == DiscoveryType::kCommissionerNode)
        {
            requiredSize = snprintf(buffer, bufferLen, kCommissionerServiceName);
        }
        else
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }
    else
    {
        ReturnErrorOnFailure(MakeServiceSubtype(buffer, bufferLen, nameDesc));
        size_t subtypeLen = strlen(buffer);
        if (type == DiscoveryType::kCommissionableNode)
        {
            requiredSize = snprintf(buffer + subtypeLen, bufferLen - subtypeLen, ".%s.%s", kSubtypeServiceNamePart,
                                    kCommissionableServiceName);
        }
        else if (type == DiscoveryType::kCommissionerNode)
        {
            requiredSize =
                snprintf(buffer + subtypeLen, bufferLen - subtypeLen, ".%s.%s", kSubtypeServiceNamePart, kCommissionerServiceName);
        }
        else
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }

    return (requiredSize <= (bufferLen - 1)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

} // namespace Mdns
} // namespace chip
