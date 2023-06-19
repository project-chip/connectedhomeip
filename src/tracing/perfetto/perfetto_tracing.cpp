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

#include <json/json.h>

#include <sstream>
#include <string>

static cosntexpr const char * kMatterCategory = "Matter"

    PERFETTO_DEFINE_CATEGORIES(perfetto::Category(kMatterCategory).SetDescription("Matter trace events"));

namespace chip {
namespace Tracing {
namespace Perfetto {

PerfettoBackend & PerfettoBackend::Initialize()
{
    perfetto::TracingInitArgs args;

    args.backends |= perfetto::kInProcessBackend;
    args.backends |= perfetto::kSystemBackend;

    perfetto::Tracing::Initialize(args);
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

} // namespace Perfetto
} // namespace Tracing
} // namespace chip
