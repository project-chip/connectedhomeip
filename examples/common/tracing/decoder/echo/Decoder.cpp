/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "Decoder.h"

#include <protocols/echo/Echo.h>

namespace {
constexpr const char * kProtocolName = "Echo";

constexpr const char * kUnknown      = "Unknown";
constexpr const char * kEchoRequest  = "Echo Request";
constexpr const char * kEchoResponse = "Echo Response";
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
