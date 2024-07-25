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
#pragma once

#include <fstream>
#include <string>
#include <tracing/backend.h>
#include <unordered_map>

namespace Json {
class Value;
}

namespace chip {
namespace Tracing {
namespace Json {

/// A Backend that outputs data to chip logging.
///
/// Structured data is formatted as json strings.
///
/// THREAD SAFETY:
///    class assumes that ChipLog* is thread_safe (generally
///    we ChipLog* everywhere, so that condition seems to be met).
class JsonBackend : public ::chip::Tracing::Backend
{
public:
    JsonBackend() = default;
    ~JsonBackend();

    // Start tracing output to the given file
    CHIP_ERROR OpenFile(const char * path);

    // Close if an output file is open
    void CloseFile();

    void TraceBegin(const char * label, const char * group) override;
    void TraceEnd(const char * label, const char * group) override;
    void TraceInstant(const char * label, const char * group) override;
    void TraceCounter(const char * label) override;
    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;
    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
    void LogMetricEvent(const MetricEvent &) override;
    void Close() override { CloseFile(); }

private:
    /// Does the actual write of the value
    void OutputValue(::Json::Value & value);

    std::unordered_map<std::string, int> mCounters;

    // Output file if writing to a file. If closed, writing
    // to ChipLog*
    std::fstream mOutputFile;
    bool mFirstRecord = true;
};

} // namespace Json
} // namespace Tracing
} // namespace chip
