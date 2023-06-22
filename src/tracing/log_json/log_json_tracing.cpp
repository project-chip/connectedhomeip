/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include <tracing/log_json/log_json_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/StringBuilder.h>
#include <transport/TracingStructs.h>

#include <json/json.h>

#include <sstream>
#include <string>

namespace chip {
namespace Tracing {
namespace LogJson {

namespace {

using chip::StringBuilder;

/// Writes the given value to chip log
void LogJsonValue(Json::Value const & value)
{
    Json::StreamWriterBuilder builder;

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::stringstream output;

    writer->write(value, &output);

    ChipLogProgress(Automation, "%s", output.str().c_str());
}

void DecodePayloadHeader(Json::Value & value, const PayloadHeader * payloadHeader)
{

    value["exchangeFlags"] = payloadHeader->GetExchangeFlags();
    value["exchangeId"]    = payloadHeader->GetExchangeID();
    value["protocolId"]    = payloadHeader->GetProtocolID().ToFullyQualifiedSpecForm();
    value["messageType"]   = payloadHeader->GetMessageType();
    value["initiator"]     = payloadHeader->IsInitiator();
    value["needsAck"]      = payloadHeader->NeedsAck();

    const Optional<uint32_t> & acknowledgedMessageCounter = payloadHeader->GetAckMessageCounter();
    if (acknowledgedMessageCounter.HasValue())
    {
        value["ackMessageCounter"] = acknowledgedMessageCounter.Value();
    }
}

void DecodePacketHeader(Json::Value & value, const PacketHeader * packetHeader)
{
    value["msgCounter"]    = packetHeader->GetMessageCounter();
    value["sessionId"]     = packetHeader->GetSessionId();
    value["flags"]         = packetHeader->GetMessageFlags();
    value["securityFlags"] = packetHeader->GetSecurityFlags();

    {
        const Optional<NodeId> & nodeId = packetHeader->GetSourceNodeId();
        if (nodeId.HasValue())
        {
            value["sourceNodeId"] = nodeId.Value();
        }
    }

    {
        const Optional<NodeId> & nodeId = packetHeader->GetDestinationNodeId();
        if (nodeId.HasValue())
        {
            value["destinationNodeId"] = nodeId.Value();
        }
    }

    {
        const Optional<GroupId> & groupId = packetHeader->GetDestinationGroupId();
        if (groupId.HasValue())
        {
            value["groupId"] = groupId.Value();
        }
    }
}

void DecodePayloadData(Json::Value & value, chip::ByteSpan payload)
{
    value["payloadSize"] = static_cast<Json::Value::UInt>(payload.size());

    // TODO: a decode would be useful however it likely requires more decode
    //       metadata
}

} // namespace

void LogJsonBackend::TraceBegin(const char * label, const char * group)
{
    Json::Value value;
    value["event"] = "TraceBegin";
    value["label"] = label;
    value["group"] = group;
    LogJsonValue(value);
}

void LogJsonBackend::TraceEnd(const char * label, const char * group)
{
    Json::Value value;
    value["event"] = "TraceEnd";
    value["label"] = label;
    value["group"] = group;
    LogJsonValue(value);
}

void LogJsonBackend::TraceInstant(const char * label, const char * group)
{
    Json::Value value;
    value["event"] = "TraceInstant";
    value["label"] = label;
    value["group"] = group;
    LogJsonValue(value);
}

void LogJsonBackend::LogMessageSend(MessageSendInfo & info)
{
    Json::Value value;
    value["event"] = "MessageSend";

    switch (info.messageType)
    {
    case OutgoingMessageType::kGroupMessage:
        value["messageType"] = "Group";
        break;
    case OutgoingMessageType::kSecureSession:
        value["messageType"] = "Secure";
        break;
    case OutgoingMessageType::kUnauthenticated:
        value["messageType"] = "Unauthenticated";
        break;
    }

    DecodePayloadHeader(value["payloadHeader"], info.payloadHeader);
    DecodePacketHeader(value["packetHeader"], info.packetHeader);
    DecodePayloadData(value["payload"], info.payload);

    LogJsonValue(value);
}

void LogJsonBackend::LogMessageReceived(MessageReceivedInfo & info)
{
    Json::Value value;

    value["event"] = "MessageReceived";

    switch (info.messageType)
    {
    case IncomingMessageType::kGroupMessage:
        value["messageType"] = "Group";
        break;
    case IncomingMessageType::kSecureUnicast:
        value["messageType"] = "Secure";
        break;
    case IncomingMessageType::kUnauthenticated:
        value["messageType"] = "Unauthenticated";
        break;
    }

    DecodePayloadHeader(value["payloadHeader"], info.payloadHeader);
    DecodePacketHeader(value["packetHeader"], info.packetHeader);
    DecodePayloadData(value["payload"], info.payload);

    LogJsonValue(value);
}

void LogJsonBackend::LogNodeLookup(NodeLookupInfo & info)
{
    Json::Value value;

    value["event"]                = "LogNodeLookup";
    value["node_id"]              = info.request->GetPeerId().GetNodeId();
    value["compressed_fabric_id"] = info.request->GetPeerId().GetCompressedFabricId();
    value["min_lookup_time_ms"]   = info.request->GetMinLookupTime().count();
    value["max_lookup_time_ms"]   = info.request->GetMaxLookupTime().count();

    LogJsonValue(value);
}

void LogJsonBackend::LogNodeDiscovered(NodeDiscoveredInfo & info)
{
    Json::Value value;
    value["event"] = "LogNodeDiscovered";

    value["node_id"]              = info.peerId->GetNodeId();
    value["compressed_fabric_id"] = info.peerId->GetCompressedFabricId();

    switch (info.type)
    {
    case chip::Tracing::DiscoveryInfoType::kIntermediateResult:
        value["type"] = "intermediate";
        break;
    case chip::Tracing::DiscoveryInfoType::kResolutionDone:
        value["type"] = "done";
        break;
    case chip::Tracing::DiscoveryInfoType::kRetryDifferent:
        value["type"] = "retry-different";
        break;
    }

    {
        Json::Value result;

        char address_buff[chip::Transport::PeerAddress::kMaxToStringSize];

        info.result->address.ToString(address_buff);

        result["supports_tcp"] = info.result->supportsTcp;
        result["address"]      = address_buff;

        result["mrp"]["idle_retransmit_timeout_ms"]   = info.result->mrpRemoteConfig.mIdleRetransTimeout.count();
        result["mrp"]["active_retransmit_timeout_ms"] = info.result->mrpRemoteConfig.mActiveRetransTimeout.count();

        value["result"] = result;
    }

    LogJsonValue(value);
}

void LogJsonBackend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info)
{
    Json::Value value;

    value["event"]                = "LogNodeDiscoveryFailed";
    value["node_id"]              = info.peerId->GetNodeId();
    value["compressed_fabric_id"] = info.peerId->GetCompressedFabricId();
    value["error"]                = chip::ErrorStr(info.error);

    LogJsonValue(value);
}

} // namespace LogJson
} // namespace Tracing
} // namespace chip
