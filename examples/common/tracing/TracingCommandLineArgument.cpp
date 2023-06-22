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
#include <TracingCommandLineArgument.h>

#include <lib/support/StringSplitter.h>
#include <lib/support/logging/CHIPLogging.h>
#include <tracing/log_json/log_json_tracing.h>
#include <tracing/registry.h>

#if ENABLE_PERFETTO_TRACING
#include <tracing/perfetto/event_storage.h>     // nogncheck
#include <tracing/perfetto/file_output.h>       // nogncheck
#include <tracing/perfetto/perfetto_tracing.h>  // nogncheck
#include <tracing/perfetto/simple_initialize.h> // nogncheck
#endif

#include <memory>
#include <string>

namespace chip {
namespace CommandLineApp {

namespace {
using ::chip::Tracing::ScopedRegistration;
using ::chip::Tracing::LogJson::LogJsonBackend;

// currently supported backends
LogJsonBackend log_json_backend;

#if ENABLE_PERFETTO_TRACING

using ::chip::Tracing::Perfetto::FileTraceOutput;
using ::chip::Tracing::Perfetto::PerfettoBackend;

FileTraceOutput perfetto_file_output;
PerfettoBackend perfetto_backend;

bool StartsWith(CharSpan argument, const char * prefix)
{
    const size_t prefix_len = strlen(prefix);
    if (argument.size() < prefix_len)
    {
        return false;
    }

    argument.reduce_size(prefix_len);
    return argument.data_equal(CharSpan(prefix, prefix_len));
}

#endif

} // namespace

void EnableTracingFor(const char * cliArg)
{
    chip::StringSplitter splitter(cliArg, ',');
    chip::CharSpan value;

    while (splitter.Next(value))
    {
        if (value.data_equal(CharSpan::fromCharString("log")))
        {
            if (!log_json_backend.IsInList())
            {
                chip::Tracing::Register(log_json_backend);
            }
        }
#if ENABLE_PERFETTO_TRACING
        else if (value.data_equal(CharSpan::fromCharString("perfetto")))
        {
            if (!perfetto_backend.IsInList())
            {
                chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
                chip::Tracing::Perfetto::RegisterEventTrackingStorage();
                chip::Tracing::Register(perfetto_backend);
            }
        }
        else if (StartsWith(value, "perfetto:"))
        {
            if (!perfetto_backend.IsInList())
            {
                std::string fileName(value.data() + 9, value.size() - 9);

                chip::Tracing::Perfetto::Initialize(perfetto::kInProcessBackend);
                chip::Tracing::Perfetto::RegisterEventTrackingStorage();

                CHIP_ERROR err = perfetto_file_output.Open(fileName.c_str());
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Failed to open perfetto trace output: %" CHIP_ERROR_FORMAT, err.Format());
                }
                chip::Tracing::Register(perfetto_backend);
            }
        }
#endif // ENABLE_PERFETTO_TRACING
        else
        {
            ChipLogError(AppServer, "Unknown trace destination: '%s'", std::string(value.data(), value.size()).c_str());
        }
    }
}

void StopTracing()
{
#if ENABLE_PERFETTO_TRACING
    chip::Tracing::Perfetto::FlushEventTrackingStorage();
    perfetto_file_output.Close();
    if (perfetto_backend.IsInList())
    {
        chip::Tracing::Unregister(perfetto_backend);
    }

#endif

    if (log_json_backend.IsInList())
    {
        chip::Tracing::Unregister(log_json_backend);
    }
}

} // namespace CommandLineApp
} // namespace chip
