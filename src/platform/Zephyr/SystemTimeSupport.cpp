/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides implementations of the CHIP System Layer platform
 *          time/clock functions based on the Zephyr tick counter.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <system/SystemError.h>

#include <zephyr.h>

#if !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

namespace chip {
namespace System {
namespace Platform {
namespace Layer {

static uint64_t sBootTimeUS = 0;

uint64_t GetClock_Monotonic(void)
{
    return k_ticks_to_us_floor64(k_uptime_ticks());
}

uint64_t GetClock_MonotonicMS(void)
{
    return k_uptime_get();
}

uint64_t GetClock_MonotonicHiRes(void)
{
    return GetClock_Monotonic();
}

Error GetClock_RealTime(uint64_t & curTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = sBootTimeUS + GetClock_Monotonic();
    return CHIP_SYSTEM_NO_ERROR;
}

Error GetClock_RealTimeMS(uint64_t & curTime)
{
    if (sBootTimeUS == 0)
    {
        return CHIP_SYSTEM_ERROR_REAL_TIME_NOT_SYNCED;
    }
    curTime = (sBootTimeUS + GetClock_Monotonic()) / 1000;
    return CHIP_SYSTEM_NO_ERROR;
}

Error SetClock_RealTime(uint64_t newCurTime)
{
    // FIXME: make thread-safe or update comment in SystemClock.h
    uint64_t timeSinceBootUS = GetClock_Monotonic();
    if (newCurTime > timeSinceBootUS)
    {
        sBootTimeUS = newCurTime - timeSinceBootUS;
    }
    else
    {
        sBootTimeUS = 0;
    }
    return CHIP_SYSTEM_NO_ERROR;
}

} // namespace Layer
} // namespace Platform
} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
