/*
 *   Copyright (c) 2021 Project CHIP Authors
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
#include "TraceHandlers.h"

#include <mutex>
#include <stdint.h>
#include <string>

#include "pw_trace/trace.h"
#include "pw_trace_chip/trace_chip.h"
#include "transport/TraceMessage.h"
#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>

// For `s` std::string literal suffix
using namespace std::string_literals;

namespace chip {
namespace trace {

namespace {

// Handles the output from the trace handlers.
class TraceOutput
{
public:
    ~TraceOutput() { DeleteStream(); }

    void SetStream(TraceStream * stream)
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            delete mStream;
            mStream = nullptr;
        }
        mStream = stream;
    }

    void DeleteStream() { SetStream(nullptr); }

    void StartEvent(const std::string & label)
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            mStream->StartEvent(label);
        }
    }

    void AddField(const std::string & tag, const std::string & data)
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            mStream->AddField(tag, data);
        }
    }

    void FinishEvent()
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            mStream->FinishEvent();
        }
    }

private:
    std::mutex mLock;
    TraceStream * mStream = nullptr;
};

struct TraceHandlerContext
{
    TraceOutput * sink;
};

TraceOutput gTraceOutput;
TraceHandlerContext gTraceHandlerContext = { .sink = &gTraceOutput };

std::string AsJsonKey(const std::string & key, const std::string & value, bool prepend_comma)
{
    return (prepend_comma ? ", "s : ""s) + "\""s + key + "\": " + value;
}

std::string AsFirstJsonKey(const std::string & key, const std::string & value)
{
    return AsJsonKey(key, value, /* prepend_comma = */ false);
}

std::string AsNextJsonKey(const std::string & key, const std::string & value)
{
    return AsJsonKey(key, value, /* prepend_comma = */ true);
}

std::string AsJsonString(const std::string & value)
{
    return "\""s + value + "\""s;
}

std::string AsJsonString(const Transport::PeerAddress * peerAddress)
{
    char convBuf[Transport::PeerAddress::kMaxToStringSize] = { 0 };

    peerAddress->ToString(convBuf);
    return convBuf;
}

std::string AsJsonBool(bool isTrue)
{
    return isTrue ? "true"s : "false"s;
}

std::string AsJsonHexString(const uint8_t * buf, size_t bufLen)
{
    // Fill a string long enough for the hex conversion, that will be overwritten
    std::string hexBuf(2 * bufLen, '0');

    Encoding::BytesToLowercaseHexBuffer(buf, bufLen, hexBuf.data(), hexBuf.size());

    return AsJsonString(hexBuf);
}

std::string PacketHeaderToJson(const PacketHeader * packetHeader)
{
    std::string jsonBody;

    uint32_t messageCounter               = packetHeader->GetMessageCounter();
    const Optional<NodeId> & sourceNodeId = packetHeader->GetSourceNodeId();
    const Optional<NodeId> & destNodeId   = packetHeader->GetDestinationNodeId();
    const Optional<GroupId> & groupId     = packetHeader->GetDestinationGroupId();
    uint16_t sessionId                    = packetHeader->GetSessionId();
    uint8_t messageFlags                  = packetHeader->GetMessageFlags();
    uint8_t securityFlags                 = packetHeader->GetSecurityFlags();

    jsonBody += AsFirstJsonKey("msg_counter", std::to_string(messageCounter));

    if (sourceNodeId.HasValue())
    {
        jsonBody += AsNextJsonKey("source_node_id", std::to_string(sourceNodeId.Value()));
    }

    if (packetHeader->IsValidGroupMsg() && groupId.HasValue())
    {
        jsonBody += AsNextJsonKey("group_id", std::to_string(groupId.Value()));
        jsonBody += AsNextJsonKey("group_key_hash", std::to_string(sessionId));
    }
    else if (destNodeId.HasValue())
    {
        jsonBody += AsNextJsonKey("dest_node_id", std::to_string(destNodeId.Value()));
    }

    jsonBody += AsNextJsonKey("session_id", std::to_string(sessionId));
    jsonBody += AsNextJsonKey("msg_flags", std::to_string(messageFlags));
    jsonBody += AsNextJsonKey("security_flags", std::to_string(securityFlags));

    return jsonBody;
}

std::string PayloadHeaderToJson(const PayloadHeader * payloadHeader)
{
    uint16_t exchangeId                                   = payloadHeader->GetExchangeID();
    uint32_t protocolId                                   = payloadHeader->GetProtocolID().ToFullyQualifiedSpecForm();
    uint8_t protocolOpcode                                = payloadHeader->GetMessageType();
    const Optional<uint32_t> & acknowledgedMessageCounter = payloadHeader->GetAckMessageCounter();
    bool isInitiator                                      = payloadHeader->IsInitiator();
    bool needsAck                                         = payloadHeader->NeedsAck();
    uint8_t exchangeFlags                                 = payloadHeader->GetExhangeFlags();

    std::string jsonBody;

    jsonBody += AsFirstJsonKey("exchange_flags", std::to_string(exchangeFlags));
    jsonBody += AsNextJsonKey("exchange_id", std::to_string(exchangeId));
    jsonBody += AsNextJsonKey("protocol_id", std::to_string(protocolId));
    jsonBody += AsNextJsonKey("protocol_opcode", std::to_string(protocolOpcode));
    jsonBody += AsNextJsonKey("is_initiator", AsJsonBool(isInitiator));
    jsonBody += AsNextJsonKey("is_ack_requested", AsJsonBool(needsAck));

    if (acknowledgedMessageCounter.HasValue())
    {
        jsonBody += AsNextJsonKey("acknowledged_msg_counter", std::to_string(acknowledgedMessageCounter.Value()));
    }

    return jsonBody;
}

bool SecureMessageSentHandler(const TraceEventFields & eventFields, TraceHandlerContext * context)
{
    if (eventFields.dataSize != sizeof(TraceSecureMessageSentData))
    {
        return false;
    }

    const auto * eventData = reinterpret_cast<const TraceSecureMessageSentData *>(eventFields.dataBuffer);
    std::string jsonBody   = "{";
    jsonBody += PacketHeaderToJson(eventData->packetHeader);
    jsonBody += ", ";
    jsonBody += PayloadHeaderToJson(eventData->payloadHeader);
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_size", std::to_string(eventData->packetSize));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_hex", AsJsonHexString(eventData->packetPayload, eventData->packetSize));
    jsonBody += "}";

    TraceOutput * sink = context->sink;
    sink->StartEvent(std::string{ kTraceMessageEvent } + "." + kTraceMessageSentDataFormat);
    sink->AddField("json", jsonBody);
    sink->FinishEvent();

    return true;
}

bool SecureMessageReceivedHandler(const TraceEventFields & eventFields, TraceHandlerContext * context)
{
    if (eventFields.dataSize != sizeof(TraceSecureMessageReceivedData))
    {
        return false;
    }

    const auto * eventData = reinterpret_cast<const TraceSecureMessageReceivedData *>(eventFields.dataBuffer);

    std::string jsonBody = "{";
    jsonBody += AsFirstJsonKey("peer_address", AsJsonString(eventData->peerAddress));
    jsonBody += ", ";
    jsonBody += PacketHeaderToJson(eventData->packetHeader);
    jsonBody += ", ";
    jsonBody += PayloadHeaderToJson(eventData->payloadHeader);
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_size", std::to_string(eventData->packetSize));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_hex", AsJsonHexString(eventData->packetPayload, eventData->packetSize));
    jsonBody += "}";

    TraceOutput * sink = context->sink;
    sink->StartEvent(std::string{ kTraceMessageEvent } + "." + kTraceMessageReceivedDataFormat);
    sink->AddField("json", jsonBody);
    sink->FinishEvent();

    // Note that `eventData->session` is currently ignored.

    return true;
}

// TODO: Framework this into a registry of handlers for different message types.
bool TraceDefaultHandler(const TraceEventFields & eventFields, void * context)
{
    TraceHandlerContext * ourContext = reinterpret_cast<TraceHandlerContext *>(context);
    if (ourContext == nullptr)
    {
        return false;
    }

    if (std::string{ eventFields.dataFormat } == kTraceMessageSentDataFormat)
    {
        return SecureMessageSentHandler(eventFields, ourContext);
    }
    else if (std::string{ eventFields.dataFormat } == kTraceMessageReceivedDataFormat)
    {
        return SecureMessageReceivedHandler(eventFields, ourContext);
    }

    return false;
}

} // namespace

void SetTraceStream(TraceStream * stream)
{
    gTraceOutput.SetStream(stream);
}

void InitTrace()
{
    void * context = &gTraceHandlerContext;
    RegisterTraceHandler(TraceDefaultHandler, context);
}

void DeInitTrace()
{
    UnregisterAllTraceHandlers();
    gTraceOutput.DeleteStream();
}

} // namespace trace
} // namespace chip
