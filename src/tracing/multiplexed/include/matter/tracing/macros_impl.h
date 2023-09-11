/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
