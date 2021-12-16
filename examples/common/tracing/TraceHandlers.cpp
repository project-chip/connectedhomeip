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
#include <lib/support/logging/CHIPLogging.h>

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

    void Stream(const std::string & tag, const std::string & data)
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            mStream->Stream(tag, data);
        }
    }

    void Handler(const std::string & label)
    {
        std::lock_guard<std::mutex> guard(mLock);
        if (mStream)
        {
            mStream->Handler(label);
        }
    }

private:
    std::mutex mLock;
    TraceStream * mStream = nullptr;
};

TraceOutput output;

// TODO: Framework this into a registry of handlers for different message types.
bool TraceDefaultHandler(const TraceEventFields & trace)
{
    if (strcmp(trace.dataFormat, kTraceSecureMessageDataFormat) != 0 || trace.dataSize != sizeof(TraceSecureMessageData))
    {
        return false;
    }

    const TraceSecureMessageData * msg = reinterpret_cast<const TraceSecureMessageData *>(trace.dataBuffer);
    output.Handler("Default");
    output.Stream("ExchangeId", std::to_string(msg->payloadHeader->GetExchangeID()));
    output.Stream("MsgType", std::to_string(msg->payloadHeader->GetMessageType()));
    output.Stream("MessageCounter", std::to_string(msg->packetHeader->GetMessageCounter()));
    output.Stream("PacketSize", std::to_string(msg->packetSize));

    return true;
}

} // namespace

void SetTraceStream(TraceStream * stream)
{
    output.SetStream(stream);
}

void InitTrace()
{
    RegisterTraceHandler(TraceDefaultHandler);
}

void DeInitTrace()
{
    UnregisterAllTraceHandlers();
    output.DeleteStream();
}

} // namespace trace
} // namespace chip
