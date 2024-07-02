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
namespace Dnssd {

CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId)
{
    ReturnErrorCodeIf(bufferLen <= Operational::kInstanceNameMaxLength, CHIP_ERROR_BUFFER_TOO_SMALL);

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

    int idx = 0;
    for (size_t i = 0; i < macOrEui64.size(); ++i)
    {
        idx += snprintf(buffer + idx, 3, "%02X", macOrEui64.data()[i]);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MakeServiceSubtype(char * buffer, size_t bufferLen, DiscoveryFilter subtype)
{
    int requiredSize;
    switch (subtype.type)
    {
    case DiscoveryFilterType::kShortDiscriminator:
        // 4-bit number
        if (subtype.code >= 1 << 4)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_S%u", static_cast<uint16_t>(subtype.code));
        break;
    case DiscoveryFilterType::kLongDiscriminator:
        // 12-bit number
        if (subtype.code >= 1 << 12)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_L%u", static_cast<uint16_t>(subtype.code));
        break;
    case DiscoveryFilterType::kVendorId:
        if (subtype.code >= 1 << 16)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_V%u", static_cast<uint16_t>(subtype.code));
        break;
    case DiscoveryFilterType::kDeviceType:
        requiredSize = snprintf(buffer, bufferLen, "_T%" PRIu32, static_cast<uint32_t>(subtype.code));
        break;
    case DiscoveryFilterType::kCommissioningMode:
        requiredSize = snprintf(buffer, bufferLen, "_CM");
        break;
    case DiscoveryFilterType::kCommissioner:
        if (subtype.code > 1)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        requiredSize = snprintf(buffer, bufferLen, "_D%u", static_cast<uint16_t>(subtype.code));
        break;
    case DiscoveryFilterType::kCompressedFabricId:
        requiredSize = snprintf(buffer, bufferLen, "_I");
        return Encoding::Uint64ToHex(subtype.code, &buffer[requiredSize], bufferLen - static_cast<size_t>(requiredSize),
                                     Encoding::HexFlags::kUppercaseAndNullTerminate);
        break;
    case DiscoveryFilterType::kInstanceName:
        requiredSize = snprintf(buffer, bufferLen, "%s", subtype.instanceName);
        break;
    case DiscoveryFilterType::kNone:
        requiredSize = 0;
        buffer[0]    = '\0';
        break;
    }
    return (static_cast<size_t>(requiredSize) <= (bufferLen - 1)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR MakeServiceTypeName(char * buffer, size_t bufferLen, DiscoveryFilter nameDesc, DiscoveryType type)
{
    int requiredSize;
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
        else if (type == DiscoveryType::kOperational)
        {
            requiredSize = snprintf(buffer, bufferLen, kOperationalServiceName);
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
        else if (type == DiscoveryType::kOperational)
        {
            requiredSize =
                snprintf(buffer + subtypeLen, bufferLen - subtypeLen, ".%s.%s", kSubtypeServiceNamePart, kOperationalServiceName);
        }
        else
        {
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }

    return (static_cast<size_t>(requiredSize) <= (bufferLen - 1)) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

} // namespace Dnssd
} // namespace chip
