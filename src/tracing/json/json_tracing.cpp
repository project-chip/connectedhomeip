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
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/StringBuilder.h>
#include <lib/support/StringSplitter.h>
#include <log_json/log_json_build_config.h>
#include <tracing/metric_event.h>
#include <transport/TracingStructs.h>

#include <json/json.h>

#include <errno.h>

#include <filesystem>
#include <sstream>
#include <string>

#if MATTER_LOG_JSON_DECODE_HEX
#include <lib/support/BytesToHex.h> // nogncheck
#endif

#if MATTER_LOG_JSON_DECODE_FULL
#include <lib/format/protocol_decoder.h> // nogncheck
#include <tlv/meta/clusters_meta.h>      // nogncheck
#include <tlv/meta/protocols_meta.h>     // nogncheck
#endif

namespace chip {
namespace Tracing {
namespace Json {

namespace {

using chip::StringBuilder;

#if MATTER_LOG_JSON_DECODE_FULL

using namespace chip::Decoders;

using PayloadDecoderType = chip::Decoders::PayloadDecoder<64, 2048>;

/// Figures out a unique name within a json object.
///
/// Decoded keys may be duplicated, like list elements are denoted as "[]".
/// The existing code does not attempt to encode lists and everything is an object,
/// so this name builder attempts to find unique keys for elements inside a json.
///
/// In particular a repeated "Anonymous<>", "Anonymous<>", ... will become "Anonymous<0>", ...
class UniqueNameBuilder
{
public:
    UniqueNameBuilder(chip::StringBuilderBase & formatter) : mFormatter(formatter) {}
    const char * c_str() const { return mFormatter.c_str(); }

    // Figure out the next unique name in the given value
    //
    // After this returns, c_str() will return a name based on `baseName` that is
    // not a key of `value` (unless on overflows, which are just logged)
    void ComputeNextUniqueName(const char * baseName, ::Json::Value & value)
    {
        FirstName(baseName);
        while (value.isMember(mFormatter.c_str()))
        {
            NextName(baseName);
            if (!mFormatter.Fit())
            {
                ChipLogError(Automation, "Potential data loss: insufficient space for unique keys in json");
                return;
            }
        }
    }

private:
    void FirstName(const char * baseName)
    {
        if (strcmp(baseName, "Anonymous<>") == 0)
        {
            mFormatter.Reset().Add("Anonymous<0>");
        }
        else
        {
            mFormatter.Reset().Add(baseName);
        }
    }

    void NextName(const char * baseName)
    {
        if (strcmp(baseName, "Anonymous<>") == 0)
        {
            mFormatter.Reset().Add("Anonymous<").Add(mUniqueIndex++).Add(">");
        }
        else
        {
            mFormatter.Reset().Add(baseName).Add("@").Add(mUniqueIndex++);
        }
    }

    chip::StringBuilderBase & mFormatter;
    int mUniqueIndex = 0;
};

// Gets the current value of the decoder until a NEST exit is returned
::Json::Value GetPayload(PayloadDecoderType & decoder)
{
    ::Json::Value value;
    PayloadEntry entry;
    StringBuilder<128> formatter;
    UniqueNameBuilder nameBuilder(formatter);

    while (decoder.Next(entry))
    {
        switch (entry.GetType())
        {
        case PayloadEntry::IMPayloadType::kNestingEnter:
            // PayloadEntry validity is only until any decoder calls are made,
            // because the entry Name/Value may point into a shared Decoder buffer.
            //
            // As such entry.GetName() is only valid here and would not be valid once
            // GetPayload() is called as GetPayload calls decoder.Next, which invalidates
            // internal name and value buffers (makes them point to the next element).
            //
            // TLDR: name MUST be used and saved before GetPayload is executed.
            nameBuilder.ComputeNextUniqueName(entry.GetName(), value);
            value[nameBuilder.c_str()] = GetPayload(decoder);
            break;
        case PayloadEntry::IMPayloadType::kNestingExit:
            return value;
        case PayloadEntry::IMPayloadType::kAttribute:
            value[formatter.Reset().AddFormat("ATTR(%u/%u)", entry.GetClusterId(), entry.GetAttributeId()).c_str()] =
                "<NOT_DECODED>";
            break;
        case PayloadEntry::IMPayloadType::kCommand:
            value[formatter.Reset().AddFormat("CMD(%u/%u)", entry.GetClusterId(), entry.GetCommandId()).c_str()] = "<NOT_DECODED>";
            continue;
        case PayloadEntry::IMPayloadType::kEvent:
            value[formatter.Reset().AddFormat("EVNT(%u/%u)", entry.GetClusterId(), entry.GetEventId()).c_str()] = "<NOT_DECODED>";
            continue;
        default:
            nameBuilder.ComputeNextUniqueName(entry.GetName(), value);
            value[nameBuilder.c_str()] = entry.GetValueText();
            break;
        }
    }
    return value;
}

#endif

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

void DecodePayloadData(::Json::Value & value, chip::ByteSpan payload, Protocols::Id protocolId, uint8_t messageType)
{
    value["size"] = static_cast<::Json::Value::UInt>(payload.size());

#if MATTER_LOG_JSON_DECODE_HEX
    char hex_buffer[4096];
    if (chip::Encoding::BytesToUppercaseHexString(payload.data(), payload.size(), hex_buffer, sizeof(hex_buffer)) == CHIP_NO_ERROR)
    {
        value["hex"] = hex_buffer;
    }
#endif // MATTER_LOG_JSON_DECODE_HEX

#if MATTER_LOG_JSON_DECODE_FULL

    // As PayloadDecoder is quite large (large strings buffers), we place it in heap
    auto decoder = chip::Platform::MakeUnique<PayloadDecoderType>(PayloadDecoderInitParams()
                                                                      .SetProtocolDecodeTree(chip::TLVMeta::protocols_meta)
                                                                      .SetClusterDecodeTree(chip::TLVMeta::clusters_meta)
                                                                      .SetProtocol(protocolId)
                                                                      .SetMessageType(messageType));

    decoder->StartDecoding(payload);

    value["decoded"] = GetPayload(*decoder);
#endif // MATTER_LOG_JSON_DECODE_FULL
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

void JsonBackend::TraceCounter(const char * label)
{
    std::string counterId = std::string(label);
    if (mCounters.find(counterId) == mCounters.end())
    {
        mCounters[counterId] = 1;
    }
    else
    {
        mCounters[counterId]++;
    }
    ::Json::Value value;
    value["event"] = "TraceCounter";
    value["label"] = label;
    value["count"] = mCounters[counterId];

    // Output the counter event
    OutputValue(value);
}

void JsonBackend::LogMetricEvent(const MetricEvent & event)
{
    ::Json::Value value;

    value["label"] = event.key();

    using ValueType = MetricEvent::Value::Type;
    switch (event.ValueType())
    {
    case ValueType::kInt32:
        value["value"] = event.ValueInt32();
        break;
    case ValueType::kUInt32:
        value["value"] = event.ValueUInt32();
        break;
    case ValueType::kChipErrorCode:
        value["value"] = event.ValueErrorCode();
        break;
    case ValueType::kUndefined:
        value["value"] = ::Json::Value();
        break;
    default:
        value["value"] = "UNKNOWN";
        break;
    }

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
    DecodePayloadData(value["payload"], info.payload, info.payloadHeader->GetProtocolID(), info.payloadHeader->GetMessageType());

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
    DecodePayloadData(value["payload"], info.payload, info.payloadHeader->GetProtocolID(), info.payloadHeader->GetMessageType());

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

        result["supports_tcp_client"] = info.result->supportsTcpClient;
        result["supports_tcp_server"] = info.result->supportsTcpServer;
        result["address"]             = address_buff;

        result["mrp"]["idle_retransmit_timeout_ms"]   = info.result->mrpRemoteConfig.mIdleRetransTimeout.count();
        result["mrp"]["active_retransmit_timeout_ms"] = info.result->mrpRemoteConfig.mActiveRetransTimeout.count();
        result["mrp"]["active_threshold_time_ms"]     = info.result->mrpRemoteConfig.mActiveThresholdTime.count();

        result["isICDOperatingAsLIT"] = info.result->isICDOperatingAsLIT;

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

    std::error_code ec;
    std::filesystem::path filePath(path);
    // Create directories if they don't exist
    if (!std::filesystem::create_directories(filePath.remove_filename(), ec))
    {
        return CHIP_ERROR_POSIX(ec.value());
    }

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
        // For pretty-printing, output each log line individually.
        std::string data_string = output.str();
        chip::StringSplitter splitter(data_string.c_str(), '\n');

        chip::CharSpan line;
        while (splitter.Next(line))
        {
            ChipLogProgress(Automation, "%.*s", static_cast<int>(line.size()), line.data());
        }
    }
}

} // namespace Json
} // namespace Tracing
} // namespace chip
