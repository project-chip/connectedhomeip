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

#include <memory>
#include <string>
#include <vector>

namespace chip {
namespace CommandLineApp {

namespace {
using ::chip::Tracing::ScopedRegistration;
using ::chip::Tracing::LogJson::LogJsonBackend;

// currently supported backends
LogJsonBackend log_json_backend;

// ScopedRegistration ensures register/unregister is met, as long
// as the vector is cleared (and we do so when stopping tracing).
std::vector<std::unique_ptr<ScopedRegistration>> tracing_backends;

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
                tracing_backends.push_back(std::make_unique<ScopedRegistration>(log_json_backend));
            }
        }
        else
        {
            ChipLogError(AppServer, "Unknown trace destination: '%s'", std::string(value.data(), value.size()).c_str());
        }
    }
}

void StopTracing()
{
    tracing_backends.clear();
}

} // namespace CommandLineApp
} // namespace chip
