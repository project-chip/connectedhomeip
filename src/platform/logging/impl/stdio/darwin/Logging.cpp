/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>
#include <lib/support/EnforceFormat.h>
#include <lib/support/logging/Constants.h>
#include <platform/logging/LogV.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

namespace chip {
namespace Logging {
namespace Platform {

void ENFORCE_FORMAT(3, 0) LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    timeval time;
    gettimeofday(&time, nullptr);
    long ms = (time.tv_sec * 1000) + (time.tv_usec / 1000);

    uint64_t ktid;
    pthread_threadid_np(nullptr, &ktid);

    char formattedMsg[CHIP_CONFIG_LOG_MESSAGE_MAX_SIZE];
    int32_t prefixLen =
        snprintf(formattedMsg, sizeof(formattedMsg), "[%ld] [%lld:%lld] [%s] ", ms, (long long) getpid(), (long long) ktid, module);
    if (prefixLen < 0)
    {
        // This should not happen
        return;
    }

    if (static_cast<size_t>(prefixLen) >= sizeof(formattedMsg))
    {
        prefixLen = sizeof(formattedMsg) - 1;
    }

    vsnprintf(formattedMsg + prefixLen, sizeof(formattedMsg) - static_cast<size_t>(prefixLen), msg, v);

    switch (category)
    {
    case kLogCategory_Error:
        printf("\033[1;31m%s\033[0m\n", formattedMsg);
        break;
    case kLogCategory_Progress:
        printf("\033[0;32m%s\033[0m\n", formattedMsg);
        break;
    case kLogCategory_Detail:
        printf("\033[0;34m%s\033[0m\n", formattedMsg);
        break;
    }
}

} // namespace Platform
} // namespace Logging
} // namespace chip
