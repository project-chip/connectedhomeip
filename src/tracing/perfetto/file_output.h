/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/core/CHIPError.h>

#include <perfetto.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

/// Registers a perfetto data source to output to a file.
class FileTraceOutput
{
public:
    FileTraceOutput() = default;
    ~FileTraceOutput() { Close(); }

    /// Starts a new tracing output, closing the existing one
    CHIP_ERROR Open(const char * file_name);

    /// Closes the tracing/session (assuming it is open)
    void Close();

private:
    static constexpr int kInvalidFileId = -1;

    int mTraceFileId = kInvalidFileId;
    std::unique_ptr<perfetto::TracingSession> mTracingSession;
};

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
