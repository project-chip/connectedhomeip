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

#include <protocols/echo/Echo.h>

namespace {
constexpr char kProtocolName[] = "Echo";

constexpr char kUnknown[]      = "Unknown";
constexpr char kEchoRequest[]  = "Echo Request";
constexpr char kEchoResponse[] = "Echo Response";
} // namespace

using MessageType = chip::Protocols::Echo::MsgType;

namespace chip {
namespace trace {
namespace echo {

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::EchoRequest):
        return kEchoRequest;
    case to_underlying(MessageType::EchoResponse):
        return kEchoResponse;
    default:
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len)
{
    // TODO Implement messages decoding

    switch (protocolCode)
    {
    case to_underlying(MessageType::EchoRequest):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::EchoResponse):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

} // namespace echo
} // namespace trace
} // namespace chip
