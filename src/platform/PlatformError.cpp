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

#include <lib/core/CHIPConfig.h>
#include <platform/PlatformError.h>

#include <lib/core/ErrorStr.h>

#include <string.h>
#include <mutex>
#define CHIP_HAVE_STD_MUTEX 1

namespace chip {
namespace Platform {

namespace Internal {
#if CHIP_CONFIG_ERROR_SOURCE && CHIP_CONFIG_ERROR_STD_SOURCE_LOCATION
::chip::ChipError MapPlatformError(int aError, std::source_location location)
{
    return (aError == 0 ? CHIP_NO_ERROR
                        : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError), location));
}
#elif CHIP_CONFIG_ERROR_SOURCE
::chip::ChipError MapPlatformError(int aError, const char * file, unsigned int line)
{
    return (aError == 0 ? CHIP_NO_ERROR
                        : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError), file, line));
}
#else
::chip::ChipError MapPlatformError(int aError)
{
    return (aError == 0 ? CHIP_NO_ERROR : CHIP_ERROR(ChipError::Range::kPlatform, static_cast<ChipError::ValueType>(aError)));
}
#endif

const char * DescribePlatformError(CHIP_ERROR aError)
{
    const int lError = static_cast<int>(aError.GetValue());
#if CHIP_SYSTEM_CONFIG_THREAD_LOCAL_STORAGE
    static thread_local char errBuf[128];
#else
    static char errBuf[128];
#endif // CHIP_SYSTEM_CONFIG_THREAD_LOCAL_STORAGE
    // Try the POSIX/XSI strerror_r variant (int strerror_r(int, char*, size_t)).
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L && !defined(_GNU_SOURCE)
    if (strerror_r(lError, errBuf, sizeof(errBuf)) == 0)
    {
        return errBuf;
    }
#elif defined(_GNU_SOURCE) || defined(__GLIBC__)
    {
        // GNU variant: char *strerror_r(int, char*, size_t)
        char * s = strerror_r(lError, errBuf, sizeof(errBuf));
        if (s != nullptr)
        {
            if (s != errBuf)
            {
                strncpy(errBuf, s, sizeof(errBuf) - 1);
                errBuf[sizeof(errBuf) - 1] = '\0';
            }
            return errBuf;
        }
    }
#else
    // No strerror_r declared: fall back to strerror(). To make this safe in
    // multi-threaded builds, serialize access to strerror() if std::mutex is
    // available; otherwise do an unprotected copy (best-effort).
#  ifdef CHIP_HAVE_STD_MUTEX
    static std::mutex s_strerror_mutex;
    std::lock_guard<std::mutex> lock(s_strerror_mutex);
#  endif
    {
        const char * s = strerror(lError);
        if (s != nullptr)
        {
            strncpy(errBuf, s, sizeof(errBuf) - 1);
            errBuf[sizeof(errBuf) - 1] = '\0';
            return errBuf;
        }
    }
#endif

    return "Unknown platform error";
}

void RegisterPlatformErrorFormatter()
{
    static ErrorFormatter sPlatformErrorFormatter = { FormatPlatformError, nullptr };
    static bool sRegistered                       = false;
    if (sRegistered)
        return;
    RegisterErrorFormatter(&sPlatformErrorFormatter);
    sRegistered = true;
}

bool FormatPlatformError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    if (err.IsRange(ChipError::Range::kPlatform))
    {
        const char * desc =
#if CHIP_CONFIG_SHORT_ERROR_STR
            nullptr;
#else
            DescribePlatformError(err);
#endif
        FormatError(buf, bufSize, "Platform", err, desc);
        return true;
    }

    return false;
}

} // namespace Internal
} // namespace Platform
} // namespace chip
