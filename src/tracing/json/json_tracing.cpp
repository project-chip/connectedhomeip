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

#include <tracing/json/json_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/StringBuilder.h>
#include <transport/TracingStructs.h>

#include <json/json.h>

#include <errno.h>

#include <sstream>
#include <string>

namespace chip {
namespace Tracing {
namespace Json {

namespace {

using chip::StringBuilder;

void DecodePayloadHeader(::Json::Value & value, const PayloadHeader * payloadHeader)
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

void DecodePacketHeader(::Json::Value & value, const PacketHeader * packetHeader)
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

void DecodePayloadData(::Json::Value & value, chip::ByteSpan payload)
{
    value["payloadSize"] = static_cast<::Json::Value::UInt>(payload.size());

    // TODO: a decode would be useful however it likely requires more decode
    //       metadata
}

} // namespace

JsonBackend::~JsonBackend()
{
    CloseFile();
}

void JsonBackend::TraceBegin(const char * label, const char * group)
{
    ::Json::Value value;

    value["event"] = "TraceBegin";
    value["label"] = label;
    value["group"] = group;
    OutputValue(value);
}

void JsonBackend::TraceEnd(const char * label, const char * group)
{
    ::Json::Value value;
    value["event"] = "TraceEnd";
    value["label"] = label;
    value["group"] = group;
    OutputValue(value);
}

void JsonBackend::TraceInstant(const char * label, const char * group)
{
    ::Json::Value value;
    value["event"] = "TraceInstant";
    value["label"] = label;
    value["group"] = group;
    OutputValue(value);
}

void JsonBackend::LogMessageSend(MessageSendInfo & info)
{
    ::Json::Value value;
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

    OutputValue(value);
}

void JsonBackend::LogMessageReceived(MessageReceivedInfo & info)
{
    ::Json::Value value;

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

    OutputValue(value);
}

void JsonBackend::LogNodeLookup(NodeLookupInfo & info)
{
    ::Json::Value value;

    value["event"]                = "LogNodeLookup";
    value["node_id"]              = info.request->GetPeerId().GetNodeId();
    value["compressed_fabric_id"] = info.request->GetPeerId().GetCompressedFabricId();
    value["min_lookup_time_ms"]   = info.request->GetMinLookupTime().count();
    value["max_lookup_time_ms"]   = info.request->GetMaxLookupTime().count();

    OutputValue(value);
}

void JsonBackend::LogNodeDiscovered(NodeDiscoveredInfo & info)
{
    ::Json::Value value;
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
        ::Json::Value result;

        char address_buff[chip::Transport::PeerAddress::kMaxToStringSize];

        info.result->address.ToString(address_buff);

        result["supports_tcp"] = info.result->supportsTcp;
        result["address"]      = address_buff;

        result["mrp"]["idle_retransmit_timeout_ms"]   = info.result->mrpRemoteConfig.mIdleRetransTimeout.count();
        result["mrp"]["active_retransmit_timeout_ms"] = info.result->mrpRemoteConfig.mActiveRetransTimeout.count();

        value["result"] = result;
    }

    OutputValue(value);
}

void JsonBackend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info)
{
    ::Json::Value value;

    value["event"]                = "LogNodeDiscoveryFailed";
    value["node_id"]              = info.peerId->GetNodeId();
    value["compressed_fabric_id"] = info.peerId->GetCompressedFabricId();
    value["error"]                = chip::ErrorStr(info.error);

    OutputValue(value);
}

void JsonBackend::CloseFile()
{
    if (!mOutputFile.is_open())
    {
        return;
    }

    mOutputFile << "]\n";

    mOutputFile.close();
}

CHIP_ERROR JsonBackend::OpenFile(const char * path)
{
    CloseFile();
    mOutputFile.open(path, std::ios_base::out);

    if (!mOutputFile)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    mOutputFile << "[\n";
    mFirstRecord = true;

    return CHIP_NO_ERROR;
}

void JsonBackend::OutputValue(::Json::Value & value)
{
    ::Json::StreamWriterBuilder builder;
    std::unique_ptr<::Json::StreamWriter> writer(builder.newStreamWriter());

    if (mOutputFile.is_open())
    {
        if (!mFirstRecord)
        {
            mOutputFile << ",\n";
        }
        else
        {
            mFirstRecord = false;
        }
        value["time_ms"] = chip::System::SystemClock().GetMonotonicTimestamp().count();
        writer->write(value, &mOutputFile);
        mOutputFile.flush();
    }
    else
    {
        std::stringstream output;
        writer->write(value, &output);
        ChipLogProgress(Automation, "%s", output.str().c_str());
    }
}

} // namespace Json
} // namespace Tracing
} // namespace chip
