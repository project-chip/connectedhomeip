/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

/* Ensure we do not have double tracing macros defined */
#if defined(MATTER_TRACE_BEGIN)
#error "Tracing macros seem to be double defined"
#endif

namespace Insights {
class ESP32Backend
{
public:
    ESP32Backend(const char * str, ...);
    ~ESP32Backend();

private:
    const char * mlabel;
    const char * mgroup;
};
} // namespace Insights

#define MATTER_TRACE_SCOPE(...)                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        Insights::ESP32Backend backend(__VA_ARGS__);                                                                               \
    } while (0)

#define _MATTER_TRACE_DISABLE(...)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
    } while (false)

#define MATTER_TRACE_BEGIN(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_END(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
#define MATTER_TRACE_INSTANT(...) _MATTER_TRACE_DISABLE(__VA_ARGS__)
