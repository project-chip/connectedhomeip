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

#pragma once

#include <lib/core/CHIPError.h>
#include <stdint.h>

// Spec 4.4.1.3
// ===== Session ID (16 bits)
// An unsigned integer value identifying the session associated with this message.
// The session identifies the particular key used to encrypt a message out of the set of
// available keys (either session or group), and the particular encryption/message
// integrity algorithm to use for the message.The Session ID field is always present.
// A Session ID of 0 SHALL indicate an unsecured session with no encryption or message integrity checking.
//
// The Session ID is allocated from a global numerical space shared across all fabrics and nodes on the resident process instance.
//

namespace chip {

class SessionIDAllocator
{
public:
    SessionIDAllocator() {}
    ~SessionIDAllocator() {}

    CHIP_ERROR Allocate(uint16_t & id);
    void Free(uint16_t id);
    CHIP_ERROR Reserve(uint16_t id);
    CHIP_ERROR ReserveUpTo(uint16_t id);
    uint16_t Peek();

private:
    static constexpr uint16_t kMaxSessionID       = UINT16_MAX;
    static constexpr uint16_t kUnsecuredSessionId = 0;

    static uint16_t sNextAvailable;
};

} // namespace chip
