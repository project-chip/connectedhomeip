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

#include <platform/logging/LogV.h>

namespace chip {
namespace Logging {
namespace Platform {

void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    // ChipPlatformLog expands to an os_log call directly (see Logging.h), so
    // we don't need to do anything further here. However his function and the
    // call to it still exist because of scenarios where a different logging
    // backend (usually stdio) is swapped in at link time, e.g. for unit tests.
}

} // namespace Platform
} // namespace Logging
} // namespace chip
