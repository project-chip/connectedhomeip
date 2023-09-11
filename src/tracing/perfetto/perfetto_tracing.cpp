/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <tracing/perfetto/perfetto_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/StringBuilder.h>
#include <transport/TracingStructs.h>

#include <matter/tracing/macros_impl.h>
#include <perfetto.h>

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

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
