/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <matter/tracing/build_config.h>

#include <tracing/macros.h>

#ifdef MATTER_TRACING_ENABLED

#ifdef MATTER_TRACE_NONE

#define MATTER_TRACE_SCOPE(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#elif defined(MATTER_TRACE_MULTIPLEXED)

namespace chip {
namespace Tracing {

/// Convenience class for RAII for scoped tracing
///
/// Usage:
///   {
///      ::chip::Tracing::Scoped scope("label", "group");
///      // TRACE_BEGIN called here
///
///      // ... add code here
///
///   } // TRACE_END called here

class Scoped
{
public:
    inline Scoped(const char * label, const char * group) : mLabel(label), mGroup(group) { MATTER_TRACE_BEGIN(label, group); }
    inline ~Scoped() { MATTER_TRACE_END(mLabel, mGroup); }

private:
    const char * mLabel;
    const char * mGroup;
};

} // namespace Tracing
} // namespace chip

#define _CONCAT_IMPL(a, b) a##b
#define _MACRO_CONCAT(a, b) _CONCAT_IMPL(a, b)

/// convenience macro to create a tracing scope
///
/// Usage:
///   {
///      MATTER_TRACE_SCOPE(::chip::Tracing::Scope::CASESession_SendSigma1);
///      // TRACE_BEGIN called here
///
///      // ... add code here
///
///   } // TRACE_END called here
#define MATTER_TRACE_SCOPE(label, group) ::chip::Tracing::Scoped _MACRO_CONCAT(_trace_scope, __COUNTER__)(label, group)

#else
// backends MUST provide a config for this
#include <matter/tracing/macros_impl.h>
#endif

#else // ifdef MATTER_TRACING_ENABLED

#define MATTER_TRACE_SCOPE(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)

#endif
