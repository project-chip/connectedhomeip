/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
