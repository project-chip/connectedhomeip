/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Logging {

/// Provides the ability to add tracing code to methods. Will do
/// ChipLog tracing if CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING is set
/// and will be optimized away by the compiler if the define is not set
///
/// Typical usage:
///     LogTracer tracer(__func__);
///
///     VerifyOrReturnError(somecondition, tracer.LogError(CHIP_ERROR_INVALID_ARGUMENT));
///     VerifyOrReturnError(otherCondition, tracer.LogError(CHIP_ERROR_INVALID_ARGUMENT, "othercondition"));
///
///     ReturnTracedErrorOnFailure(foo.Bar(), tracer);
///     ReturnTracedErrorOnFailure(foo.Baz(), tracer);
///
///     return CHIP_NO_ERROR;
class LogTracer
{
#if CHIP_CONFIG_ENABLE_FUNCT_ERROR_LOGGING
public:
    LogTracer(const char * name) : mName(name) {}
    CHIP_ERROR LogError(CHIP_ERROR err)
    {
        ChipLogError(NotSpecified, "%s in %s", ErrorStr(err), mName);
        return err;
    }

    CHIP_ERROR LogError(CHIP_ERROR err, const char * context)
    {
        ChipLogError(NotSpecified, "%s/%s in %s", ErrorStr(err), context, mName);
        return err;
    }

private:
    const char * mName;

#else
public:
    LogTracer(const char *) {}
    constexpr CHIP_ERROR LogError(CHIP_ERROR err) { return err; }
    constexpr CHIP_ERROR LogError(CHIP_ERROR err, const char *) { return err; }
#endif
};

} // namespace Logging
} // namespace chip

#define ReturnTracedErrorOnFailure(expr, tracer)                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        auto __err = (expr);                                                                                                       \
        if (!::chip::ChipError::IsSuccess(__err))                                                                                  \
        {                                                                                                                          \
            return tracer.LogError(__err);                                                                                         \
        }                                                                                                                          \
    } while (false)
