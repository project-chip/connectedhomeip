/*
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

#include <cstddef>
#include <cstdint>

namespace mdns {
namespace Minimal {

// Assigned by IANA: https://www.iana.org/assignments/dns-parameters/dns-parameters.xhtml#dns-parameters-4
enum class QType : uint16_t
{
    A         = 1,
    NS        = 2,
    CNAME     = 5,
    SOA       = 6,
    NULLVALUE = 10,
    WKS       = 11,
    PTR       = 12,
    HINFO     = 13,
    MINFO     = 14,
    MX        = 15,
    TXT       = 16,
    ISDN      = 20,
    AAAA      = 28,
    SRV       = 33,
    DNAM      = 39,
    ANY       = 255,
};

/// Flag encoded in QCLASS requesting unicast answers
constexpr uint16_t kQClassUnicastAnswerFlag = 0x8000;
// Flag used to indicate receiver should flush cache rather than appending. Used for Response RR's.
// See https://datatracker.ietf.org/doc/html/rfc6762#section-10.2.
constexpr uint16_t kQClassResponseFlushBit = 0x8000;

enum class QClass : uint16_t
{
    IN  = 1,
    ANY = 255,

    // Unicast version for the class
    IN_UNICAST = IN | kQClassUnicastAnswerFlag,
    IN_FLUSH   = IN | kQClassResponseFlushBit,
};

enum class ResourceType
{
    kAnswer,
    kAuthority,
    kAdditional,
};

} // namespace Minimal
} // namespace mdns
