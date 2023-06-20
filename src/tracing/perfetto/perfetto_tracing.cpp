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

#include <tracing/perfetto/perfetto_tracing.h>

#include <lib/address_resolve/TracingStructs.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/StringBuilder.h>
#include <transport/TracingStructs.h>

#include <perfetto.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

static constexpr const char * kMatterCategory = "Matter";

PERFETTO_DEFINE_CATEGORIES(perfetto::Category(kMatterCategory).SetDescription("Matter trace events"));

PERFETTO_TRACK_EVENT_STATIC_STORAGE();

namespace chip {
namespace Tracing {
namespace Perfetto {

PerfettoBackend & PerfettoBackend::Init(const char * output_name)
{
    perfetto::TracingInitArgs args;

    if ((output_name != nullptr) && (output_name[0] != '\0'))
    {
        OpenTracingFile(output_name);
        args.backends = perfetto::kInProcessBackend;
    }
    else
    {
        args.backends = perfetto::kSystemBackend;
    }

    perfetto::Tracing::Initialize(args);
    perfetto::TrackEvent::Register();

    return *this;
}

PerfettoBackend::~PerfettoBackend()
{
    CloseTracingFile();
}

void PerfettoBackend::OpenTracingFile(const char * name)
{
    VerifyOrDie(name != nullptr);
    CloseTracingFile();

    mTraceFileId = open(name, O_RDWR | O_CREAT | O_TRUNC, 0640);
    if (mTraceFileId < 0)
    {
        ChipLogError(Automation, "Failed to open logging file '%s': %s", name, strerror(errno));
        mTraceFileId = kInvalidFileId;
    }
}

void PerfettoBackend::CloseTracingFile()
{
    if (mTraceFileId != kInvalidFileId)
    {
        close(mTraceFileId);
        mTraceFileId = kInvalidFileId;
    }
}

void PerfettoBackend::Open()
{
    if (mTraceFileId == kInvalidFileId)
    {
        return;
    }

    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(1024);

    auto * ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event"); // corresponds to TRACE_EVENT

    mTracingSession = perfetto::Tracing::NewTrace();
    mTracingSession->Setup(cfg, mTraceFileId);
    mTracingSession->StartBlocking();
}

void PerfettoBackend::Close()
{
    if (!mTracingSession)
    {
        return;
    }

    perfetto::TrackEvent::Flush();

    mTracingSession->FlushBlocking();
    mTracingSession->StopBlocking();
    mTracingSession.reset();
    CloseTracingFile();
}

void PerfettoBackend::TraceBegin(const char * label, const char * group)
{
    // We generally expect macros to be used using
    //
    // matter_trace_config = "${chip_root}/src/tracing/perfetto:tracing"
    //
    // However perfetto can also do non-const string tracing (just likely
    // slower)
    TRACE_EVENT_BEGIN(kMatterCategory, perfetto::StaticString(label), "class_name", perfetto::StaticString(group));
}

void PerfettoBackend::TraceEnd(const char * label, const char * group)
{
    TRACE_EVENT_END(kMatterCategory);

    // Force a trace flush to work around perfetto caching issues.
    PERFETTO_INTERNAL_ADD_EMPTY_EVENT();
}

void PerfettoBackend::TraceInstant(const char * label, const char * group)
{
    TRACE_EVENT_INSTANT(kMatterCategory, perfetto::StaticString(label), "class_name", perfetto::StaticString(group));
}

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

    TRACE_EVENT_INSTANT(                     //
        kMatterCategory, "Message Received", //
        "message_type", messageType          //
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

    TRACE_EVENT_INSTANT(                 //
        kMatterCategory, "Message Send", //
        "message_type", messageType      //
    );
}

void PerfettoBackend::LogNodeLookup(NodeLookupInfo & info)
{
    TRACE_EVENT_INSTANT(                                                          //
        kMatterCategory, "NodeLookup",                                            //
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
            kMatterCategory, "NodeDiscovered Intermediate",               //
            "node_id", info.peerId->GetNodeId(),                          //
            "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
            "address", address_buff                                       //
        );
        break;
    case chip::Tracing::DiscoveryInfoType::kResolutionDone:
        TRACE_EVENT_INSTANT(                                              //
            kMatterCategory, "NodeDiscovered Final",                      //
            "node_id", info.peerId->GetNodeId(),                          //
            "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
            "address", address_buff                                       //
        );
        break;
    case chip::Tracing::DiscoveryInfoType::kRetryDifferent:
        TRACE_EVENT_INSTANT(                                              //
            kMatterCategory, "NodeDiscovered Retry Different",            //
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
        kMatterCategory, "Discovery Failed",                          //
        "node_id", info.peerId->GetNodeId(),                          //
        "compressed_fabric_id", info.peerId->GetCompressedFabricId(), //
        "error", chip::ErrorStr(info.error)                           //
    );
}

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
