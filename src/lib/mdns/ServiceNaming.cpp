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

#include <support/CodeUtils.h>

#include <cstdio>
#include <inttypes.h>

namespace chip {
namespace Mdns {
namespace {

uint8_t HexToInt(char c)
{
    if ('0' <= c && c <= '9')
    {
        return static_cast<uint8_t>(c - '0');
    }
    else if ('a' <= c && c <= 'f')
    {
        return static_cast<uint8_t>(0x0a + c - 'a');
    }
    else if ('A' <= c && c <= 'F')
    {
        return static_cast<uint8_t>(0x0a + c - 'A');
    }

    return UINT8_MAX;
}

} // namespace

CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, const PeerId & peerId)
{
    constexpr size_t kServiceNameLen = 16 + 1 + 16; // 2 * 64-bit value in HEX + hyphen

    ReturnErrorCodeIf(bufferLen <= kServiceNameLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    NodeId nodeId     = peerId.GetNodeId();
    FabricId fabricId = peerId.GetFabricId();

    snprintf(buffer, bufferLen, "%08" PRIX32 "%08" PRIX32 "-%08" PRIX32 "%08" PRIX32, static_cast<uint32_t>(fabricId >> 32),
             static_cast<uint32_t>(fabricId), static_cast<uint32_t>(nodeId >> 32), static_cast<uint32_t>(nodeId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractIdFromInstanceName(const char * name, PeerId * peerId)
{
    ReturnErrorCodeIf(name == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(peerId == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    peerId->SetNodeId(0);
    peerId->SetFabricId(0);

    bool deliminatorFound = false;
    bool hasFabricPart    = false;
    bool hasNodePart      = false;

    for (; *name != '\0'; name++)
    {
        if (*name == '.')
        {
            break;
        }
        else if (*name == '-')
        {
            deliminatorFound = true;
            continue;
        }

        uint8_t val = HexToInt(*name);
        ReturnErrorCodeIf(val == UINT8_MAX, CHIP_ERROR_WRONG_NODE_ID);

        if (deliminatorFound)
        {
            hasNodePart = true;
            peerId->SetNodeId(peerId->GetNodeId() * 16 + val);
        }
        else
        {
            hasFabricPart = true;
            peerId->SetFabricId(peerId->GetFabricId() * 16 + val);
        }
    }

    ReturnErrorCodeIf(!deliminatorFound, CHIP_ERROR_WRONG_NODE_ID);
    ReturnErrorCodeIf(!hasNodePart, CHIP_ERROR_WRONG_NODE_ID);
    ReturnErrorCodeIf(!hasFabricPart, CHIP_ERROR_WRONG_NODE_ID);

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

} // namespace Mdns
} // namespace chip
