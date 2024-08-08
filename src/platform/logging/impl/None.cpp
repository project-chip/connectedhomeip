/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <platform/logging/LogV.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char *, uint8_t, const char *, va_list)
{
    // This backend discards all log messages. This is useful when all log output
    // is routed via `SetLogRedirectCallback()` and/or platform logging
    // integration at the log macro level (`CHIP_SYSTEM_CONFIG_PLATFORM_LOG`).
}

} // namespace Platform
} // namespace Logging
} // namespace chip
