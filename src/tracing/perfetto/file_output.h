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
