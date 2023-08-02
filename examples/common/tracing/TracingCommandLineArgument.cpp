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
#include <tracing/json/json_tracing.h>
#include <tracing/registry.h>

#if ENABLE_PERFETTO_TRACING
#include <tracing/perfetto/event_storage.h>     // nogncheck
#include <tracing/perfetto/simple_initialize.h> // nogncheck
#endif

#include <memory>
#include <string>

namespace chip {
namespace CommandLineApp {

namespace {

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

} // namespace

void TracingSetup::EnableTracingFor(const char * cliArg)
{
    chip::StringSplitter splitter(cliArg, ',');
    chip::CharSpan value;

    while (splitter.Next(value))
    {
        if (StartsWith(value, "json:"))
        {
            std::string fileName(value.data() + 5, value.size() - 5);

            if (fileName != "log")
            {
                CHIP_ERROR err = mJsonBackend.OpenFile(fileName.c_str());
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Failed to open json trace output: %" CHIP_ERROR_FORMAT, err.Format());
                }
            }
            else
            {
                mJsonBackend.CloseFile(); // just in case, ensure no file output
            }
            chip::Tracing::Register(mJsonBackend);
        }
#if ENABLE_PERFETTO_TRACING
        else if (value.data_equal(CharSpan::fromCharString("perfetto")))
        {
            chip::Tracing::Perfetto::Initialize(perfetto::kSystemBackend);
            chip::Tracing::Perfetto::RegisterEventTrackingStorage();
            chip::Tracing::Register(mPerfettoBackend);
        }
        else if (StartsWith(value, "perfetto:"))
        {
            std::string fileName(value.data() + 9, value.size() - 9);

            chip::Tracing::Perfetto::Initialize(perfetto::kInProcessBackend);
            chip::Tracing::Perfetto::RegisterEventTrackingStorage();

            CHIP_ERROR err = mPerfettoFileOutput.Open(fileName.c_str());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to open perfetto trace output: %" CHIP_ERROR_FORMAT, err.Format());
            }
            chip::Tracing::Register(mPerfettoBackend);
        }
#endif // ENABLE_PERFETTO_TRACING
        else
        {
            ChipLogError(AppServer, "Unknown trace destination: '%s'", std::string(value.data(), value.size()).c_str());
        }
    }
}

void TracingSetup::StopTracing()
{
#if ENABLE_PERFETTO_TRACING
    chip::Tracing::Perfetto::FlushEventTrackingStorage();
    mPerfettoFileOutput.Close();
    chip::Tracing::Unregister(mPerfettoBackend);

#endif

    chip::Tracing::Unregister(mJsonBackend);
}

} // namespace CommandLineApp
} // namespace chip
