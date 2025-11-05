/*
 * Small portable wrapper for strerror/strerror_r usage.
 * Provides PlatformStrError(int err, char *buf, size_t bufSize)
 * which returns buf on success or nullptr on failure.
 */
#ifndef SYSTEM_STRERROR_UTIL_H
#define SYSTEM_STRERROR_UTIL_H

#include <stddef.h>
#include <string.h>
#include <type_traits>

namespace chip {
namespace System {

inline const char * PlatformStrError(int err, char * buf, size_t bufSize)
{
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 200112L && !defined(_GNU_SOURCE)
    // POSIX/XSI variant: int strerror_r(int, char*, size_t)
    int r = strerror_r(err, buf, bufSize);
    if (r == 0)
        return buf;
    return nullptr;
#elif defined(_GNU_SOURCE) || defined(__GLIBC__) || defined(__USE_XOPEN2K)
    // The actual signature of strerror_r differs (GNU vs XSI). Use decltype
    // and constexpr-if to handle whichever return type is declared.
    using ret_t = decltype(strerror_r(err, buf, bufSize));
    if constexpr (std::is_same_v<ret_t, int>)
    {
        int r = strerror_r(err, buf, bufSize);
        if (r == 0)
            return buf;
        return nullptr;
    }
    else
    {
        char * s = strerror_r(err, buf, bufSize);
        if (s != nullptr)
        {
            if (s != buf)
            {
                strncpy(buf, s, bufSize - 1);
                buf[bufSize - 1] = '\0';
            }
            return buf;
        }
        return nullptr;
    }
#else
    // Fallback: use strerror() and copy result into buffer.
    const char * s = strerror(err);
    if (s != nullptr)
    {
        strncpy(buf, s, bufSize - 1);
        buf[bufSize - 1] = '\0';
        return buf;
    }
    return nullptr;
#endif
}

} // namespace System
} // namespace chip

#endif // SYSTEM_STRERROR_UTIL_H
