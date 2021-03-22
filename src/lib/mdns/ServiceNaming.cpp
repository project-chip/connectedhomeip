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

CHIP_ERROR MakeInstanceName(char * buffer, size_t bufferLen, uint64_t fabricId, uint64_t nodeId)
{
    constexpr size_t kServiceNameLen = 16 + 1 + 16; // 2 * 64-bit value in HEX + hyphen

    ReturnErrorCodeIf(bufferLen <= kServiceNameLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    snprintf(buffer, bufferLen, "%08" PRIX32 "%08" PRIX32 "-%08" PRIX32 "%08" PRIX32, static_cast<uint32_t>(fabricId >> 32),
             static_cast<uint32_t>(fabricId), static_cast<uint32_t>(nodeId >> 32), static_cast<uint32_t>(nodeId));

    return CHIP_NO_ERROR;
}

} // namespace Mdns
} // namespace chip
