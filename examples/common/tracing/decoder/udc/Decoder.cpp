/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "Decoder.h"

#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

namespace {
constexpr const char * kProtocolName = "User Directed Commissioning";

constexpr const char * kUnknown                   = "Unknown";
constexpr const char * kIdentificationDeclaration = "Identification Declaration";
} // namespace

using MessageType = chip::Protocols::UserDirectedCommissioning::MsgType;

namespace chip {
namespace trace {
namespace udc {

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::IdentificationDeclaration):
        return kIdentificationDeclaration;
    default:
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len)
{
    // TODO Implement messages decoding

    switch (protocolCode)
    {
    case to_underlying(MessageType::IdentificationDeclaration):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

} // namespace udc
} // namespace trace
} // namespace chip
