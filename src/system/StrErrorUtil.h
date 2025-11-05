/*
 * Small portable wrapper for strerror/strerror_r usage.
 * Provides PlatformStrError(int err, char *buf, size_t bufSize)
 * which returns buf on success or nullptr on failure.
 */
#ifndef SYSTEM_STRERROR_UTIL_H
#define SYSTEM_STRERROR_UTIL_H

#include <stddef.h>
#include <string.h>

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
#elif defined(_GNU_SOURCE) || defined(__GLIBC__)
    // GNU variant: char *strerror_r(int, char*, size_t)
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
