/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "tracing/enabled_features.h"

#include <tracing/json/json_tracing.h>

#if ENABLE_PERFETTO_TRACING
#include <tracing/perfetto/file_output.h>      // nogncheck
#include <tracing/perfetto/perfetto_tracing.h> // nogncheck
#endif

/// A string with supported command line tracing targets
/// to be pretty-printed in help strings if needed
#if ENABLE_PERFETTO_TRACING
#define SUPPORTED_COMMAND_LINE_TRACING_TARGETS "json:log, json:<path>, perfetto, perfetto:<path>"
#else
#define SUPPORTED_COMMAND_LINE_TRACING_TARGETS "json:log, json:<path>"
#endif

namespace chip {
namespace CommandLineApp {

class TracingSetup
{
public:
    TracingSetup() = default;
    ~TracingSetup() { StopTracing(); }

    /// Enable tracing based on the given command line argument
    /// like "json:log" or "json:/tmp/foo.txt,perfetto" or similar
    ///
    /// Single arguments as well as comma separated ones are accepted.
    ///
    /// Calling this method multiple times is ok and will enable each of
    /// the given tracing modules if not already enabled.
    void EnableTracingFor(const char * cliArg);

    /// If EnableTracingFor is called, this MUST be called as well
    /// to unregister tracing backends
    void StopTracing();

private:
    ::chip::Tracing::Json::JsonBackend mJsonBackend;

#if ENABLE_PERFETTO_TRACING
    chip::Tracing::Perfetto::FileTraceOutput mPerfettoFileOutput;
    chip::Tracing::Perfetto::PerfettoBackend mPerfettoBackend;
#endif
};

} // namespace CommandLineApp
} // namespace chip
