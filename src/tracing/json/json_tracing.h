/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <fstream>
#include <tracing/backend.h>

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
    void LogMessageSend(MessageSendInfo &) override;
    void LogMessageReceived(MessageReceivedInfo &) override;
    void LogNodeLookup(NodeLookupInfo &) override;
    void LogNodeDiscovered(NodeDiscoveredInfo &) override;
    void LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo &) override;
    void Close() override { CloseFile(); }

private:
    /// Does the actual write of the value
    void OutputValue(::Json::Value & value);

    // Output file if writing to a file. If closed, writing
    // to ChipLog*
    std::fstream mOutputFile;
    bool mFirstRecord = true;
};

} // namespace Json
} // namespace Tracing
} // namespace chip
