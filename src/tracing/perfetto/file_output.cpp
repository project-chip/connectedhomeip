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
#include "file_output.h"

#include <lib/support/CodeUtils.h>
#include <system/SystemError.h>

#include <perfetto.h>

#include <errno.h>
#include <fcntl.h>

namespace chip {
namespace Tracing {
namespace Perfetto {

CHIP_ERROR FileTraceOutput::Open(const char * file_name)
{
    VerifyOrReturnError(file_name != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*file_name != '\0', CHIP_ERROR_INVALID_ARGUMENT);

    // Close any existing files
    Close();

    // Create a trace file and start sending data to it
    mTraceFileId = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0640);
    if (mTraceFileId < 0)
    {
        mTraceFileId = kInvalidFileId;
        return CHIP_ERROR_POSIX(errno);
    }

    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(1024);

    auto * ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event"); // corresponds to TRACE_EVENT

    mTracingSession = perfetto::Tracing::NewTrace();
    mTracingSession->Setup(cfg, mTraceFileId);
    mTracingSession->StartBlocking();

    return CHIP_NO_ERROR;
}

void FileTraceOutput::Close()
{
    if (mTracingSession)
    {
        mTracingSession->FlushBlocking();
        mTracingSession->StopBlocking();
        mTracingSession.reset();
    }

    if (mTraceFileId != kInvalidFileId)
    {
        close(mTraceFileId);
        mTraceFileId = kInvalidFileId;
    }
}

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
