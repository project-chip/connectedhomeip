/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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

#include <lib/support/logging/Constants.h>

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#if defined(__APPLE__)
#include <pthread.h>
#include <unistd.h>
#elif defined(__gnu_linux__)
#include <sys/syscall.h>
#include <unistd.h>
#endif

namespace chip {
namespace Logging {
namespace Platform {

// TODO: investigate whether pw_chrono or pw_log could be used here
void LogV(const char * module, uint8_t category, const char * msg, va_list v)
{
    // Stdout needs to be locked, because it's printed in pieces
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    flockfile(stdout);
#endif

    switch (category)
    {
    case kLogCategory_Error:
        printf("\033[1;31m");
        break;
    case kLogCategory_Progress:
        printf("\033[0;32m");
        break;
    case kLogCategory_Detail:
        printf("\033[0;34m");
        break;
    }

#if defined(__APPLE__) || defined(__gnu_linux__)
    timespec ts;
    timespec_get(&ts, TIME_UTC);
    printf("[%lld.%03ld] ", static_cast<long long>(ts.tv_sec), static_cast<long>(ts.tv_nsec / 1000000));
#endif

#if defined(__APPLE__)
    uint64_t ktid;
    pthread_threadid_np(nullptr, &ktid);
    printf("[%lld:%lld] ", static_cast<long long>(getpid()), static_cast<long long>(ktid));
#elif defined(__gnu_linux__) && !defined(__NuttX__)
    // TODO: change to getpid() and gettid() after glib upgrade
    printf("[%lld:%lld] ", static_cast<long long>(syscall(SYS_getpid)), static_cast<long long>(syscall(SYS_gettid)));
#endif

    printf("[%s] ", module);
    vprintf(msg, v);
    printf("\033[0m\n");
    fflush(stdout);

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    funlockfile(stdout);
#endif
}

} // namespace Platform
} // namespace Logging
} // namespace chip
