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
#include <vector>

#include "transport/TraceMessage.h"
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
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
    ~TraceOutput() { UnregisterAllStreams(); }

    void RegisterStream(TraceStream * stream)
    {
        std::lock_guard<std::mutex> guard(mLock);
        mStreams.push_back(stream);
    }

    void UnregisterAllStreams()
    {
        for (auto stream : mStreams)
        {
            delete stream;
        }
        mStreams.clear();
    }

    void StartEvent(const std::string & label)
    {
        std::lock_guard<std::mutex> guard(mLock);
        for (auto stream : mStreams)
        {
            stream->StartEvent(label);
        }
    }

    void AddField(const std::string & tag, const std::string & data)
    {
        std::lock_guard<std::mutex> guard(mLock);
        for (auto stream : mStreams)
        {
            stream->AddField(tag, data);
        }
    }

    void FinishEvent()
    {
        std::lock_guard<std::mutex> guard(mLock);
        for (auto stream : mStreams)
        {
            stream->FinishEvent();
        }
    }

    bool HasStreamAvailable() const
    {
        std::lock_guard<std::mutex> guard(mLock);
        return (mStreams.size());
    }

private:
    mutable std::mutex mLock;
    std::vector<TraceStream *> mStreams;
};

TraceOutput gTraceOutputs;

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
    return AsJsonString(convBuf);
}

std::string AsJsonBool(bool isTrue)
{
    return isTrue ? "true"s : "false"s;
}

std::string AsJsonHexString(const uint8_t * buf, size_t bufLen)
{
    // Avoid hex conversion that would fail on empty
    if (bufLen == 0)
    {
        return AsJsonString("");
    }

    // Fill a buffer long enough for the hex conversion, that will be overwritten
    std::vector<char> hexBuf(2 * bufLen, '\0');

    CHIP_ERROR status = Encoding::BytesToLowercaseHexBuffer(buf, bufLen, hexBuf.data(), hexBuf.size());

    // Static conditions exist that should ensure never failing. Catch failure in an assert.
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(Support, "Unexpected failure: %" CHIP_ERROR_FORMAT, status.Format());
        VerifyOrDie(status == CHIP_NO_ERROR);
    }

    return AsJsonString(std::string(hexBuf.data(), hexBuf.size()));
}

std::string PacketHeaderToJson(const PacketHeader * packetHeader)
{
    std::string jsonBody;

    uint32_t messageCounter = packetHeader->GetMessageCounter();
    jsonBody += AsFirstJsonKey("msg_counter", std::to_string(messageCounter));

    const Optional<NodeId> & sourceNodeId = packetHeader->GetSourceNodeId();
    if (sourceNodeId.HasValue())
    {
        jsonBody += AsNextJsonKey("source_node_id", std::to_string(sourceNodeId.Value()));
    }

    uint16_t sessionId                  = packetHeader->GetSessionId();
    const Optional<GroupId> & groupId   = packetHeader->GetDestinationGroupId();
    const Optional<NodeId> & destNodeId = packetHeader->GetDestinationNodeId();
    if (packetHeader->IsValidGroupMsg())
    {
        if (groupId.HasValue())
        {
            jsonBody += AsNextJsonKey("group_id", std::to_string(groupId.Value()));
        }

        jsonBody += AsNextJsonKey("group_key_hash", std::to_string(sessionId));
    }
    else if (destNodeId.HasValue())
    {
        jsonBody += AsNextJsonKey("dest_node_id", std::to_string(destNodeId.Value()));
    }

    jsonBody += AsNextJsonKey("session_id", std::to_string(sessionId));

    uint8_t messageFlags = packetHeader->GetMessageFlags();
    jsonBody += AsNextJsonKey("msg_flags", std::to_string(messageFlags));

    uint8_t securityFlags = packetHeader->GetSecurityFlags();
    jsonBody += AsNextJsonKey("security_flags", std::to_string(securityFlags));

    return jsonBody;
}

std::string PayloadHeaderToJson(const PayloadHeader * payloadHeader)
{

    std::string jsonBody;

    uint8_t exchangeFlags = payloadHeader->GetExhangeFlags();
    jsonBody += AsFirstJsonKey("exchange_flags", std::to_string(exchangeFlags));

    uint16_t exchangeId = payloadHeader->GetExchangeID();
    jsonBody += AsNextJsonKey("exchange_id", std::to_string(exchangeId));

    uint32_t protocolId = payloadHeader->GetProtocolID().ToFullyQualifiedSpecForm();
    jsonBody += AsNextJsonKey("protocol_id", std::to_string(protocolId));

    uint8_t protocolOpcode = payloadHeader->GetMessageType();
    jsonBody += AsNextJsonKey("protocol_opcode", std::to_string(protocolOpcode));

    bool isInitiator = payloadHeader->IsInitiator();
    jsonBody += AsNextJsonKey("is_initiator", AsJsonBool(isInitiator));

    bool needsAck = payloadHeader->NeedsAck();
    jsonBody += AsNextJsonKey("is_ack_requested", AsJsonBool(needsAck));

    const Optional<uint32_t> & acknowledgedMessageCounter = payloadHeader->GetAckMessageCounter();
    if (acknowledgedMessageCounter.HasValue())
    {
        jsonBody += AsNextJsonKey("acknowledged_msg_counter", std::to_string(acknowledgedMessageCounter.Value()));
    }

    return jsonBody;
}

std::string PreparedSecureMessageDataToJson(const TracePreparedSecureMessageData * data, const std::string & peerAddressKey)
{
    const System::PacketBuffer * packetBuffer = data->packetBuffer->operator->();
    std::string jsonBody                      = "{";
    jsonBody += AsFirstJsonKey(peerAddressKey, AsJsonString(data->peerAddress));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_size", std::to_string(packetBuffer->DataLength()));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_hex", AsJsonHexString(packetBuffer->Start(), packetBuffer->DataLength()));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("buffer_ptr", std::to_string(reinterpret_cast<std::uintptr_t>(packetBuffer)));
    jsonBody += "}";

    return jsonBody;
}

void SecureMessageSentHandler(const TraceSecureMessageSentData * eventData)
{
    if (!gTraceOutputs.HasStreamAvailable())
    {
        return;
    }

    std::string jsonBody = "{";
    jsonBody += PacketHeaderToJson(eventData->packetHeader);
    jsonBody += ", ";
    jsonBody += PayloadHeaderToJson(eventData->payloadHeader);
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_size", std::to_string(eventData->packetSize));
    jsonBody += ", ";
    jsonBody += AsFirstJsonKey("payload_hex", AsJsonHexString(eventData->packetPayload, eventData->packetSize));
    jsonBody += "}";

    gTraceOutputs.StartEvent(std::string{ kTraceMessageEvent } + "." + kTraceMessageSentDataFormat);
    gTraceOutputs.AddField("json", jsonBody);
    gTraceOutputs.FinishEvent();
}

void SecureMessageReceivedHandler(const TraceSecureMessageReceivedData * eventData)
{
    if (!gTraceOutputs.HasStreamAvailable())
    {
        return;
    }

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

    gTraceOutputs.StartEvent(std::string{ kTraceMessageEvent } + "." + kTraceMessageReceivedDataFormat);
    gTraceOutputs.AddField("json", jsonBody);
    gTraceOutputs.FinishEvent();

    // Note that `eventData->session` is currently ignored.
}

void PreparedMessageSentHandler(const TracePreparedSecureMessageData * eventData)
{
    if (!gTraceOutputs.HasStreamAvailable())
    {
        return;
    }

    gTraceOutputs.StartEvent(std::string{ kTraceMessageEvent } + "." + kTracePreparedMessageSentDataFormat);
    gTraceOutputs.AddField("json", PreparedSecureMessageDataToJson(eventData, "destination"));
    gTraceOutputs.FinishEvent();
}

void PreparedMessageReceivedHandler(const TracePreparedSecureMessageData * eventData)
{
    if (!gTraceOutputs.HasStreamAvailable())
    {
        return;
    }

    gTraceOutputs.StartEvent(std::string{ kTraceMessageEvent } + "." + kTracePreparedMessageReceivedDataFormat);
    gTraceOutputs.AddField("json", PreparedSecureMessageDataToJson(eventData, "source"));
    gTraceOutputs.FinishEvent();
}

void TraceHandler(const char * type, const void * data, size_t size)
{
    if ((std::string{ type } == kTracePreparedMessageReceivedDataFormat) && (size == sizeof(TracePreparedSecureMessageData)))
    {
        PreparedMessageReceivedHandler(reinterpret_cast<const TracePreparedSecureMessageData *>(data));
    }
    else if ((std::string{ type } == kTracePreparedMessageSentDataFormat) && (size == sizeof(TracePreparedSecureMessageData)))
    {
        PreparedMessageSentHandler(reinterpret_cast<const TracePreparedSecureMessageData *>(data));
    }
    else if ((std::string{ type } == kTraceMessageSentDataFormat) && (size == sizeof(TraceSecureMessageSentData)))
    {
        SecureMessageSentHandler(reinterpret_cast<const TraceSecureMessageSentData *>(data));
    }
    else if ((std::string{ type } == kTraceMessageReceivedDataFormat) && (size == sizeof(TraceSecureMessageReceivedData)))
    {
        SecureMessageReceivedHandler(reinterpret_cast<const TraceSecureMessageReceivedData *>(data));
    }
}

} // namespace

void AddTraceStream(TraceStream * stream)
{
    gTraceOutputs.RegisterStream(stream);
}

void InitTrace()
{
    SetTransportTraceHook(TraceHandler);
}

void DeInitTrace()
{
    gTraceOutputs.UnregisterAllStreams();
}

} // namespace trace
} // namespace chip
