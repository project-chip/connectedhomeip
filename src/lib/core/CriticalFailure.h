/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>

namespace chip {
/**
 * This class represents a critical failure.
 *
 * Although similar to ChipError, this class is meant to represent a low-level failure
 * which, on most platforms, would be catastrophic and as such there is no need
 * for the caller to do handling. Some examples include MemoryInit() and ScheduleTimer(...).
 *
 * This results in less branching on such platforms, with error-handling code being optimized away.
 *
 * On platforms that want to handle the failure instead of aborting, they have the option of doing so
 * by defining CALLER_HANDLES_CRITICAL_FAILURE.
 *
 * Note that the divergence in [[nodiscard]] is purposeful: the expectation is that long-term
 * we probably want the abort behavior on all platforms, and as such don't need to clutter
 * the callsite with handling the failure; in the interim, however, we enforce handling
 * of the failure when we compile with CALLER_HANDLES_CRITICAL_FAILURE.
 *
 * If this divergence creates significant friction, e.g. in continuous integration, we may want
 * to consistently annotate with [[nodiscard]], even in the case where no handling is necessary.
 */
#if CALLER_HANDLES_CRITICAL_FAILURE
class [[nodiscard]] CriticalFailure
{
    CHIP_ERROR error;

public:
    constexpr CriticalFailure(CHIP_ERROR err) : error(err) {}

    template <class F>
    __attribute__((always_inline)) inline bool Handle(F && fn)
    {
        if (!::chip::ChipError::IsSuccess(error))
        {
            fn(error);
            return false;
        }
        return true;
    }

    __attribute__((always_inline)) inline CHIP_ERROR GetError() const { return error; }
#else
class CriticalFailure
{
public:
    __attribute__((always_inline)) inline CriticalFailure(CHIP_ERROR err) { SuccessOrDie(err); }

    template <class F>
    __attribute__((always_inline)) inline bool Handle(F && fn)
    {
        return true;
    }

    __attribute__((always_inline)) inline CHIP_ERROR GetError() const { return CHIP_NO_ERROR; }
#endif

    __attribute__((always_inline)) inline operator CHIP_ERROR() const { return GetError(); }
    __attribute__((always_inline)) inline bool operator==(const ChipError & other) const { return GetError() == other; }
    __attribute__((always_inline)) inline bool operator!=(const ChipError & other) const { return GetError() != other; }
};
} // namespace chip

__attribute__((always_inline)) inline bool operator==(const chip::ChipError & lhs, const chip::CriticalFailure & rhs)
{
    return lhs == rhs.GetError();
}

__attribute__((always_inline)) inline bool operator!=(const chip::ChipError & lhs, const chip::CriticalFailure & rhs)
{
    return lhs != rhs.GetError();
}
