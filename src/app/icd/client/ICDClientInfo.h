/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/support/CodeUtils.h>
#include <stddef.h>

namespace chip {
namespace app {

struct ICDClientInfo
{
    ScopedNodeId peer_node;
    uint32_t start_icd_counter         = 0;
    uint32_t offset                    = 0;
    uint64_t monitored_subject         = static_cast<uint64_t>(0);
    Crypto::Aes128KeyHandle shared_key = Crypto::Aes128KeyHandle();

    ICDClientInfo() {}
    ICDClientInfo(const ICDClientInfo & other) { *this = other; }

    ICDClientInfo & operator=(const ICDClientInfo & other)
    {
        peer_node         = other.peer_node;
        start_icd_counter = other.start_icd_counter;
        offset            = other.offset;
        monitored_subject = other.monitored_subject;
        ByteSpan buf(other.shared_key.As<Crypto::Aes128KeyByteArray>());
        memcpy(shared_key.AsMutable<Crypto::Aes128KeyByteArray>(), buf.data(), sizeof(Crypto::Aes128KeyByteArray));
        return *this;
    }
};

} // namespace app
} // namespace chip
