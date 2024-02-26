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

#include <tracing/metric_event.h>
#include <tracing/perfetto/perfetto_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/StringBuilder.h>
#include <matter/tracing/macros_impl.h>
#include <perfetto.h>
#include <transport/TracingStructs.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

void PerfettoBackend::LogMessageReceived(MessageReceivedInfo & info)
{
    const char * messageType = "UNKNOWN";
    switch (info.messageType)
    {
    case IncomingMessageType::kGroupMessage:
        messageType = "Group";
        break;
    case IncomingMessageType::kSecureUnicast:
        messageType = "Secure";
        break;
    case IncomingMessageType::kUnauthenticated:
        messageType = "Unauthenticated";
        break;
    }

    TRACE_EVENT_INSTANT(              //
        "Matter", "Message Received", //
        "message_type", messageType   //
    );
}

void PerfettoBackend::LogMessageSend(MessageSendInfo & info)
{
    const char * messageType = "UNKNOWN";
    switch (info.messageType)
    {
    case OutgoingMessageType::kGroupMessage:
        messageType = "Group";
        break;
    case OutgoingMessageType::kSecureSession:
        messageType = "Secure";
        break;
    case OutgoingMessageType::kUnauthenticated:
        messageType = "Unauthenticated";
        break;
    }

    TRACE_EVENT_INSTANT(            //
        "Matter", "Message Send",   //
        "message_type", messageType //
    );
}

void PerfettoBackend::LogNodeLookup(NodeLookupInfo & info)
{
    TRACE_EVENT_INSTANT(                                                          //
        "Matter", "NodeLookup",                                                   //
        "node_id", info.request->GetPeerId().GetNodeId(),                         //
        "compressed_fabric_id", info.request->GetPeerId().GetCompressedFabricId() //
    );
}

void PerfettoBackend::LogNodeDiscovered(NodeDiscoveredInfo & info)
{
    char address_buff[chip::Transport::PeerAddress::kMaxToStringSize];
    info.result->address.ToString(address_buff);

    switch (info.type)
    {
    case chip::Tracing::DiscoveryInfoType::kIntermediateResult:
        TRACE_EVENT_INSTANT(                                              //
            "Matter", "NodeDiscovered Intermediate",                      //
            "node_id", info.peerId->GetNodeId(),                          //
            "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
            "address", address_buff                                       //
        );
        break;
    case chip::Tracing::DiscoveryInfoType::kResolutionDone:
        TRACE_EVENT_INSTANT(                                              //
            "Matter", "NodeDiscovered Final",                             //
            "node_id", info.peerId->GetNodeId(),                          //
            "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
            "address", address_buff                                       //
        );
        break;
    case chip::Tracing::DiscoveryInfoType::kRetryDifferent:
        TRACE_EVENT_INSTANT(                                              //
            "Matter", "NodeDiscovered Retry Different",                   //
            "node_id", info.peerId->GetNodeId(),                          //
            "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
            "address", address_buff                                       //
        );
        break;
    }
}

void PerfettoBackend::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info)
{
    TRACE_EVENT_INSTANT(                                              //
        "Matter", "Discovery Failed",                                 //
        "node_id", info.peerId->GetNodeId(),                          //
        "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
        "error", chip::ErrorStr(info.error)                           //
    );
}

void PerfettoBackend::LogMetricEvent(const MetricEvent & event)
{
    using ValueType = MetricEvent::Value::Type;
    switch (event.ValueType())
    {
    case ValueType::kInt32:
        TRACE_EVENT_INSTANT("Matter", event.key(), "value", event.ValueInt32());
        break;
    case ValueType::kUInt32:
        TRACE_EVENT_INSTANT("Matter", event.key(), "value", event.ValueUInt32());
        break;
    case ValueType::kChipErrorCode:
        TRACE_EVENT_INSTANT("Matter", event.key(), "error", event.ValueErrorCode());
        break;
    case ValueType::kUndefined:
        TRACE_EVENT_INSTANT("Matter", event.key());
        break;
    default:
        TRACE_EVENT_INSTANT("Matter", event.key(), "type", "UNKNOWN");
        break;
    }
}

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
