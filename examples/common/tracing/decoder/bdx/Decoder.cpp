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

#include <protocols/bdx/BdxMessages.h>

namespace {
constexpr const char * kProtocolName = "Bulk Data Exchange";

constexpr const char * kUnknown            = "Unknown";
constexpr const char * kSendInit           = "Send Init";
constexpr const char * kSendAccept         = "Send Accept";
constexpr const char * kReceiveInit        = "Receive Init";
constexpr const char * kReceiveAccept      = "Receive Accept";
constexpr const char * kBlockQuery         = "Block Query";
constexpr const char * kBlock              = "Block";
constexpr const char * kBlockEOF           = "Block End Of File";
constexpr const char * kBlockAck           = "Block Ack";
constexpr const char * kBlockAckEOF        = "Block Ack End Of File";
constexpr const char * kBlockQueryWithSkip = "Block Query With Skip";
} // namespace

using MessageType = chip::bdx::MessageType;

namespace chip {
namespace trace {
namespace bdx {

const char * ToProtocolName()
{
    return kProtocolName;
}

const char * ToProtocolMessageTypeName(uint8_t protocolCode)
{
    switch (protocolCode)
    {
    case to_underlying(MessageType::SendInit):
        return kSendInit;
    case to_underlying(MessageType::SendAccept):
        return kSendAccept;
    case to_underlying(MessageType::ReceiveInit):
        return kReceiveInit;
    case to_underlying(MessageType::ReceiveAccept):
        return kReceiveAccept;
    case to_underlying(MessageType::BlockQuery):
        return kBlockQuery;
    case to_underlying(MessageType::Block):
        return kBlock;
    case to_underlying(MessageType::BlockEOF):
        return kBlockEOF;
    case to_underlying(MessageType::BlockAck):
        return kBlockAck;
    case to_underlying(MessageType::BlockAckEOF):
        return kBlockAckEOF;
    case to_underlying(MessageType::BlockQueryWithSkip):
        return kBlockQueryWithSkip;
    default:
        return kUnknown;
    }
}

CHIP_ERROR LogAsProtocolMessage(uint8_t protocolCode, const uint8_t * data, size_t len)
{
    // TODO Implement messages decoding

    switch (protocolCode)
    {
    case to_underlying(MessageType::SendInit):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::SendAccept):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::ReceiveInit):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::ReceiveAccept):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::BlockQuery):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::Block):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::BlockEOF):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::BlockAck):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::BlockAckEOF):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    case to_underlying(MessageType::BlockQueryWithSkip):
        return CHIP_ERROR_NOT_IMPLEMENTED;
    default:
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

} // namespace bdx
} // namespace trace
} // namespace chip
