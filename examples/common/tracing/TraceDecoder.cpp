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

#include "TraceDecoder.h"
#include "decoder/TraceDecoderProtocols.h"
#include "decoder/logging/Log.h"

#include <fstream>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/StringBuilder.h>
#include <transport/raw/MessageHeader.h>

constexpr uint16_t kMaxLineLen    = 4096;
constexpr const char jsonPrefix[] = "    json\t";

namespace chip {
namespace trace {
namespace {

// Json keys
constexpr char kProtocolIdKey[]         = "protocol_id";
constexpr char kProtocolCodeKey[]       = "protocol_opcode";
constexpr char kSessionIdKey[]          = "session_id";
constexpr char kExchangeIdKey[]         = "exchange_id";
constexpr char kMessageCounterKey[]     = "msg_counter";
constexpr char kSecurityFlagsKey[]      = "security_flags";
constexpr char kMessageFlagsKey[]       = "msg_flags";
constexpr char kSourceNodeIdKey[]       = "source_node_id";
constexpr char kDestinationNodeIdKey[]  = "dest_node_id";
constexpr char kDestinationGroupIdKey[] = "group_id";
constexpr char kExchangeFlagsKey[]      = "exchange_flags";
constexpr char kIsInitiatorKey[]        = "is_initiator";
constexpr char kNeedsAckKey[]           = "is_ack_requested";
constexpr char kAckMsgKey[]             = "acknowledged_msg_counter";
constexpr char kPayloadDataKey[]        = "payload_hex";
constexpr char kPayloadSizeKey[]        = "payload_size";
constexpr char kDirectionKey[]          = "direction";
constexpr char kPeerAddress[]           = "peer_address";

bool IsOutbound(const Json::Value & json)
{
    VerifyOrReturnValue(json.isMember(kDirectionKey), false);
    return strcmp(json[kDirectionKey].asCString(), "outbound") == 0;
}

bool IsInbound(const Json::Value & json)
{
    VerifyOrReturnValue(json.isMember(kDirectionKey), false);
    return strcmp(json[kDirectionKey].asCString(), "inbound") == 0;
}

} // namespace

using namespace logging;

CHIP_ERROR TraceDecoder::ReadFile(const char * fp)
{
    std::ifstream file(fp);
    VerifyOrReturnError(file.is_open(), CHIP_ERROR_INVALID_ARGUMENT);

    char line[kMaxLineLen];
    while (file.getline(line, sizeof(line)))
    {
        ReturnErrorOnFailure(ReadString(line));
    }

    file.close();
    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::ReadString(const char * str)
{
    if (strncmp(str, jsonPrefix, strlen(jsonPrefix)) != 0)
    {
        // Not a json string. Ignore it.
        return CHIP_NO_ERROR;
    }
    str += strlen(jsonPrefix);

    Json::Reader reader;
    Json::Value json;
    VerifyOrReturnError(reader.parse(str, json), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(json.isMember(kProtocolIdKey), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(json.isMember(kProtocolCodeKey), CHIP_ERROR_INCORRECT_STATE);

    return LogJSON(json);
}

CHIP_ERROR TraceDecoder::LogJSON(Json::Value & json)
{
    auto protocol       = json[kProtocolIdKey].asLargestUInt();
    uint16_t vendorId   = static_cast<uint16_t>(protocol >> 16);
    uint16_t protocolId = protocol & 0xFFFF;
    if (!mOptions.IsProtocolEnabled(chip::Protocols::Id(chip::VendorId(vendorId), protocolId)))
    {
        return CHIP_NO_ERROR;
    }

    if (!mOptions.mEnableMessageInitiator && IsOutbound(json))
    {
        return CHIP_NO_ERROR;
    }

    if (!mOptions.mEnableMessageResponder && IsInbound(json))
    {
        return CHIP_NO_ERROR;
    }

    bool isResponse = IsInbound(json);
    ReturnErrorOnFailure(LogAndConsumeProtocol(json));
    ReturnErrorOnFailure(MaybeLogAndConsumeHeaderFlags(json));
    ReturnErrorOnFailure(MaybeLogAndConsumePayload(json, isResponse));
    ReturnErrorOnFailure(MaybeLogAndConsumeOthers(json));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumeHeaderFlags(Json::Value & json)
{
    auto scopedIndent = ScopedLogIndent("Header Flags");
    ReturnErrorOnFailure(MaybeLogAndConsumeSecurityFlags(json));
    ReturnErrorOnFailure(MaybeLogAndConsumeMessageFlags(json));
    ReturnErrorOnFailure(MaybeLogAndConsumeExchangeFlags(json));

    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumeOthers(Json::Value & json)
{
    std::vector<std::string> keys = json.getMemberNames();
    if (keys.size())
    {
        auto scopedIndent = ScopedLogIndent("Additional Fields");
        for (std::vector<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it)
        {
            auto key   = (*it).c_str();
            auto value = json[key].asString();
            Log(key, value.c_str());
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::LogAndConsumeProtocol(Json::Value & json)
{
    char protocolInfo[256]  = {};
    char protocolDetail[32] = {};

    auto id     = json[kProtocolIdKey].asLargestUInt();
    auto opcode = static_cast<uint8_t>(json[kProtocolCodeKey].asLargestUInt());

    uint16_t vendorId   = static_cast<uint16_t>(id >> 16);
    uint16_t protocolId = (id & 0xFFFF);

    chip::StringBuilderBase builder(protocolInfo, sizeof(protocolInfo));

    builder.Add(IsInbound(json) ? "<< from " : ">> to ");

    if (json.isMember(kPeerAddress))
    {
        builder.Add(json[kPeerAddress].asCString());
    }
    else
    {
        builder.Add("UNKNOWN");
    }

    builder.Add(" ");
    auto msgCounter = static_cast<uint32_t>(json[kMessageCounterKey].asLargestUInt());
    memset(protocolDetail, '\0', sizeof(protocolDetail));
    snprintf(protocolDetail, sizeof(protocolDetail), "| %u |", msgCounter);
    builder.Add(protocolDetail);

    builder.Add(" [");
    builder.Add(ToProtocolName(vendorId, protocolId));

    builder.Add("  ");
    memset(protocolDetail, '\0', sizeof(protocolDetail));
    snprintf(protocolDetail, sizeof(protocolDetail), "(%u)", protocolId);
    builder.Add(protocolDetail);

    builder.Add(" / ");
    builder.Add(ToProtocolMessageTypeName(vendorId, protocolId, opcode));

    builder.Add(" ");
    memset(protocolDetail, '\0', sizeof(protocolDetail));
    snprintf(protocolDetail, sizeof(protocolDetail), "(0x%02x)", opcode);
    builder.Add(protocolDetail);

    if (vendorId != chip::VendorId::Common)
    {
        builder.Add(" / VendorId = ");
        builder.Add(vendorId);
    }

    builder.Add(" / Session = ");
    builder.Add(static_cast<uint16_t>(json[kSessionIdKey].asLargestUInt()));

    builder.Add(" / Exchange = ");
    builder.Add(static_cast<uint16_t>(json[kExchangeIdKey].asLargestUInt()));
    builder.Add("]");

    ChipLogProgress(DataManagement, "%s", builder.c_str());

    json.removeMember(kSessionIdKey);
    json.removeMember(kExchangeIdKey);
    json.removeMember(kMessageCounterKey);
    json.removeMember(kDirectionKey);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumePayload(Json::Value & json, bool isResponse)
{
    auto size = static_cast<uint16_t>(json[kPayloadSizeKey].asLargestUInt());
    if (size)
    {
        {
            auto payload      = json[kPayloadDataKey].asString();
            auto scopedIndent = ScopedLogIndentWithSize("Decrypted Payload", size);
            Log("data", payload.c_str());
        }

        bool shouldDecode   = !isResponse || mOptions.mEnableProtocolInteractionModelResponse;
        auto payload        = json[kPayloadDataKey].asString();
        auto id             = json[kProtocolIdKey].asLargestUInt();
        uint16_t vendorId   = static_cast<uint16_t>(id >> 16);
        uint16_t protocolId = (id & 0xFFFF);
        auto protocolCode   = static_cast<uint8_t>(json[kProtocolCodeKey].asLargestUInt());
        ReturnErrorOnFailure(
            LogAsProtocolMessage(vendorId, protocolId, protocolCode, payload.c_str(), payload.size(), shouldDecode));
        Log(" ");
    }

    json.removeMember(kPayloadDataKey);
    json.removeMember(kPayloadSizeKey);
    json.removeMember(kProtocolIdKey);
    json.removeMember(kProtocolCodeKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumeSecurityFlags(Json::Value & json)
{
    VerifyOrReturnError(json.isMember(kSecurityFlagsKey), CHIP_NO_ERROR);

    auto flags = static_cast<uint8_t>(json[kSecurityFlagsKey].asLargestUInt());
    if (flags)
    {
        auto scopedIndent = ScopedLogIndentWithFlags("Security", flags);

        if (flags & to_underlying(chip::Header::SecFlagValues::kPrivacyFlag))
        {
            Log("Privacy", "true");
        }

        if (flags & to_underlying(chip::Header::SecFlagValues::kControlMsgFlag))
        {
            Log("ControlMsg", "true");
        }

        if (flags & to_underlying(chip::Header::SecFlagValues::kMsgExtensionFlag))
        {
            Log("MsgExtension", "true");
        }
    }

    json.removeMember(kSecurityFlagsKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumeMessageFlags(Json::Value & json)
{
    VerifyOrReturnError(json.isMember(kMessageFlagsKey), CHIP_NO_ERROR);

    auto flags = static_cast<uint8_t>(json[kMessageFlagsKey].asLargestUInt());
    if (flags)
    {
        auto scopedIndent = ScopedLogIndentWithFlags("Message", flags);

        if (flags & to_underlying(chip::Header::MsgFlagValues::kSourceNodeIdPresent))
        {
            auto id = json[kSourceNodeIdKey].asLargestUInt();
            LogAsHex("SourceNodeId", id);
        }

        if (flags & to_underlying(chip::Header::MsgFlagValues::kDestinationNodeIdPresent))
        {
            auto id = json[kDestinationNodeIdKey].asLargestUInt();
            LogAsHex("DestinationNodeId", id);
        }

        if (flags & to_underlying(chip::Header::MsgFlagValues::kDestinationGroupIdPresent))
        {
            auto id = static_cast<uint16_t>(json[kDestinationGroupIdKey].asLargestUInt());
            LogAsHex("DestinationGroupIdPresent", id);
        }
    }

    json.removeMember(kMessageFlagsKey);
    json.removeMember(kSourceNodeIdKey);
    json.removeMember(kDestinationNodeIdKey);
    json.removeMember(kDestinationGroupIdKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR TraceDecoder::MaybeLogAndConsumeExchangeFlags(Json::Value & json)
{
    VerifyOrReturnError(json.isMember(kExchangeFlagsKey), CHIP_NO_ERROR);

    auto flags = static_cast<uint8_t>(json[kExchangeFlagsKey].asLargestUInt());
    if (flags)
    {

        auto scopedIndent = ScopedLogIndentWithFlags("Exchange", flags);

        if (flags & to_underlying(chip::Header::ExFlagValues::kExchangeFlag_Initiator))
        {
            ChipLogDetail(DataManagement, "        Initiator = true");
        }

        if (flags & to_underlying(chip::Header::ExFlagValues::kExchangeFlag_AckMsg))
        {
            auto ackMsgCounter = static_cast<uint32_t>(json[kAckMsgKey].asLargestUInt());
            Log("AckMsg", ackMsgCounter);
        }

        if (flags & to_underlying(chip::Header::ExFlagValues::kExchangeFlag_NeedsAck))
        {
            Log("NeedsAck", "true");
        }

        if (flags & to_underlying(chip::Header::ExFlagValues::kExchangeFlag_SecuredExtension))
        {
            Log("SecuredExtension", "true");
        }

        if (flags & to_underlying(chip::Header::ExFlagValues::kExchangeFlag_VendorIdPresent))
        {
            Log("VendorIdPresent", "true");
        }
    }

    json.removeMember(kExchangeFlagsKey);
    json.removeMember(kIsInitiatorKey);
    json.removeMember(kAckMsgKey);
    json.removeMember(kNeedsAckKey);
    return CHIP_NO_ERROR;
}

} // namespace trace
} // namespace chip
