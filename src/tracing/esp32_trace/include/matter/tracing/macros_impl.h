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

/* Ensure we do not have double tracing macros defined */
#if defined(MATTER_TRACE_BEGIN)
#error "Tracing macros seem to be double defined"
#endif

#include <tracing/registry.h>

// This gets forwarded to the multiplexed instance
#define MATTER_TRACE_BEGIN(label, group) ::chip::Tracing::Internal::Begin(label, group)
#define MATTER_TRACE_END(label, group) ::chip::Tracing::Internal::End(label, group)
#define MATTER_TRACE_INSTANT(label, group) ::chip::Tracing::Internal::Instant(label, group)
#define MATTER_TRACE_COUNTER(label) ::chip::Tracing::Internal::Counter(label)

namespace chip {
namespace Tracing {
namespace Insights {
class Scoped
{
public:
    inline Scoped(const char * label, const char * group) : mLabel(label), mGroup(group) { MATTER_TRACE_BEGIN(label, group); }
    inline ~Scoped() { MATTER_TRACE_END(mLabel, mGroup); }

private:
    const char * mLabel;
    const char * mGroup;
};
} // namespace Insights
} // namespace Tracing
} // namespace chip
#define _CONCAT_IMPL(a, b) a##b
#define _MACRO_CONCAT(a, b) _CONCAT_IMPL(a, b)

#define MATTER_TRACE_SCOPE(label, group) ::chip::Tracing::Insights::Scoped _MACRO_CONCAT(_trace_scope, __COUNTER__)(label, group)
