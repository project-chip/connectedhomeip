/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *      This is an internal header file that defines the interface to a platform-supplied
 *      function for retrieving the current system time.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <lib/support/DLLUtil.h>
#include <lib/support/TimeUtils.h>
#include <system/SystemError.h>

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <sys/time.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include <zephyr/net/socket.h>
#endif

#include <chrono>
#include <stdint.h>

#if CHIP_DEVICE_LAYER_TARGET_DARWIN || CHIP_DEVICE_LAYER_TARGET_LINUX
#define CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK 1
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN || CHIP_DEVICE_LAYER_TARGET_LINUX

#ifndef CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK
#define CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK 0
#endif

namespace chip {
namespace System {

namespace Clock {

/*
 * We use `std::chrono::duration` for clock types to provide type safety. But unlike the predefined std types
 * (`std::chrono::milliseconds` et al), CHIP uses unsigned base types, and types are explicity sized, with
 * smaller-size types available for members and arguments where appropriate.
 *
 * Most conversions are handled by the types transparently. To convert with possible loss of information, use
 * `std::chrono::duration_cast<>()`.
 */

using Microseconds64 = std::chrono::duration<uint64_t, std::micro>;
using Microseconds32 = std::chrono::duration<uint32_t, std::micro>;

using Milliseconds64 = std::chrono::duration<uint64_t, std::milli>;
using Milliseconds32 = std::chrono::duration<uint32_t, std::milli>;
using Milliseconds16 = std::chrono::duration<uint16_t, std::milli>;

using Seconds64 = std::chrono::duration<uint64_t>;
using Seconds32 = std::chrono::duration<uint32_t>;
using Seconds16 = std::chrono::duration<uint16_t>;

inline constexpr Seconds16 kZero{ 0 };

namespace Literals {

constexpr Microseconds64 operator""_us(unsigned long long int us)
{
    return Microseconds64(us);
}
constexpr Microseconds64 operator""_us64(unsigned long long int us)
{
    return Microseconds64(us);
}
constexpr Microseconds32 operator""_us32(unsigned long long int us)
{
    return Microseconds32(us);
}

constexpr Milliseconds64 operator""_ms(unsigned long long int ms)
{
    return Milliseconds64(ms);
}
constexpr Milliseconds64 operator""_ms64(unsigned long long int ms)
{
    return Milliseconds64(ms);
}
constexpr Milliseconds32 operator""_ms32(unsigned long long int ms)
{
    return Milliseconds32(ms);
}
constexpr Milliseconds16 operator""_ms16(unsigned long long int ms)
{
    return Milliseconds16(ms);
}

constexpr Seconds64 operator""_s(unsigned long long int s)
{
    return Seconds64(s);
}
constexpr Seconds64 operator""_s64(unsigned long long int s)
{
    return Seconds64(s);
}
constexpr Seconds32 operator""_s32(unsigned long long int s)
{
    return Seconds32(s);
}
constexpr Seconds16 operator""_s16(unsigned long long int s)
{
    return Seconds16(s);
}

} // namespace Literals

/**
 * Type for System time stamps.
 */
using Timestamp = Milliseconds64;

/**
 * Type for System time offsets (i.e. `StartTime()` duration).
 *
 * It is required of platforms that time stamps from `GetMonotonic…()` have the high bit(s) zero,
 * so the sum of a `Milliseconds64` time stamp and `Milliseconds32` offset will never overflow.
 */
using Timeout = Milliseconds32;

class ClockBase
{
public:
    virtual ~ClockBase() = default;

    /**
     * Returns a monotonic system time.
     *
     * This function returns an elapsed time since an arbitrary, platform-defined epoch.
     * The value returned is guaranteed to be ever-increasing (i.e. never wrapping or decreasing) between
     * reboots of the system.  Additionally, the underlying time source is guaranteed to tick
     * continuously during any system sleep modes that do not entail a restart upon wake.
     *
     * Although some platforms may choose to return a value that measures the time since boot for the
     * system, applications must *not* rely on this.
     *
     * WARNING: *** It is up to each platform to ensure that GetMonotonicTimestamp can be
     *              called safely in a re-entrant way from multiple contexts if making use
     *              of this method from the application, outside the Matter stack execution
     *              serialization context. ***
     */
    virtual Timestamp GetMonotonicTimestamp();

    /**
     * Returns a monotonic system time in units of microseconds, from the platform.
     *
     * This function returns an elapsed time in microseconds since an arbitrary, platform-defined epoch.
     * The value returned MUST BE guaranteed to be ever-increasing (i.e. never wrapping or decreasing) until
     * reboot of the system.  Additionally, the underlying time source is guaranteed to tick
     * continuously during any system sleep modes that do not entail a restart upon wake.
     *
     * Although some platforms may choose to return a value that measures the time since boot for the
     * system, applications must *not* rely on this.
     *
     * Applications must not rely on the time returned by GetMonotonicMicroseconds64() actually having
     * granularity finer than milliseconds.
     *
     * Platform implementations *must* use the same epoch for GetMonotonicMicroseconds64() and GetMonotonicMilliseconds64().
     *
     * Platforms *must* use an epoch such that the upper bit of a value returned by GetMonotonicMicroseconds64() is zero
     * for the expected operational life of the system.
     *
     * @returns Elapsed time in microseconds since an arbitrary, platform-defined epoch.
     */
    virtual Microseconds64 GetMonotonicMicroseconds64() = 0;

    /**
     * Returns a monotonic system time in units of microseconds, from the platform.
     *
     * This function returns an elapsed time in milliseconds since an arbitrary, platform-defined epoch.
     * The value returned MUST BE guaranteed to be ever-increasing (i.e. never wrapping or decreasing) until
     * reboot of the system.  Additionally, the underlying time source is guaranteed to tick
     * continuously during any system sleep modes that do not entail a restart upon wake.
     *
     * Although some platforms may choose to return a value that measures the time since boot for the
     * system, applications must *not* rely on this.
     *
     * Platform implementations *must* use the same epoch for GetMonotonicMicroseconds64() and GetMonotonicMilliseconds64().
     * (As a consequence of this, and the requirement for GetMonotonicMicroseconds64() to return high bit zero, values
     * returned by GetMonotonicMilliseconds64() will have the high ten bits zero.)
     *
     * @returns             Elapsed time in milliseconds since an arbitrary, platform-defined epoch.
     */
    virtual Milliseconds64 GetMonotonicMilliseconds64() = 0;

    /**
     * @brief
     *   Platform-specific function for getting the current real (civil) time in microsecond Unix time
     *   format.
     *
     * This function is expected to return the local platform's notion of current real time, expressed
     * as a Unix time value scaled to microseconds.  The underlying clock is required to tick at a
     * rate of least at whole seconds (values of 1,000,000), but may tick faster.
     *
     * On those platforms that are capable of tracking real time, GetClock_RealTime() must return the
     * error CHIP_ERROR_REAL_TIME_NOT_SYNCED whenever the system is unsynchronized with real time.
     *
     * Platforms that are incapable of tracking real time should not implement the GetClock_RealTime()
     * function, thereby forcing link-time failures of features that depend on access to real time.
     * Alternatively, such platforms may supply an implementation of GetClock_RealTime() that returns
     * the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
     *
     * This function is expected to be thread-safe on any platform that employs threading.
     *
     * @note
     *   This function is reserved for internal use by the System Clock.  Users of the System
     *   Clock should call System::Clock::GetClock_RealTime().
     *
     * @param[out] aCurTime                  The current time, expressed as Unix time scaled to microseconds.
     *
     * @retval #CHIP_NO_ERROR       If the method succeeded.
     * @retval #CHIP_ERROR_REAL_TIME_NOT_SYNCED
     *                                      If the platform is capable of tracking real time, but is
     *                                      is currently unsynchronized.
     * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                                      If the platform is incapable of tracking real time.
     */
    virtual CHIP_ERROR GetClock_RealTime(Microseconds64 & aCurTime) = 0;

    /**
     * @brief
     *   Platform-specific function for getting the current real (civil) time in millisecond Unix time
     *   format.
     *
     * This function is expected to return the local platform's notion of current real time, expressed
     * as a Unix time value scaled to milliseconds.
     *
     * See the documentation for GetClock_RealTime() for details on the expected behavior.
     *
     * @note
     *   This function is reserved for internal use by the System Clock.  Users of the System
     *   Clock should call System::Clock::GetClock_RealTimeMS().
     *
     * @param[out] aCurTime                  The current time, expressed as Unix time scaled to milliseconds.
     *
     * @retval #CHIP_NO_ERROR       If the method succeeded.
     * @retval #CHIP_ERROR_REAL_TIME_NOT_SYNCED
     *                                      If the platform is capable of tracking real time, but is
     *                                      is currently unsynchronized.
     * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                                      If the platform is incapable of tracking real time.
     */
    virtual CHIP_ERROR GetClock_RealTimeMS(Milliseconds64 & aCurTime) = 0;

    /**
     * @brief
     *   Platform-specific function for setting the current real (civil) time.
     *
     * This function set the local platform's notion of current real time.  The new current
     * time is expressed as a Unix time value scaled to microseconds.
     *
     * Once set, underlying platform clock is expected to track real time with a granularity of at least whole
     * seconds.
     *
     * On platforms that support tracking real time, the SetClock_RealTime() function must return the error
     * CHIP_ERROR_ACCESS_DENIED if the calling application does not have the privilege to set the
     * current time.
     *
     * Platforms that are incapable of tracking real time, or do not offer the ability to set real time,
     * should not implement the SetClock_RealTime() function, thereby forcing link-time failures of features
     * that depend on setting real time.  Alternatively, such platforms may supply an implementation of
     * SetClock_RealTime() that returns the error CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.
     *
     * This function is expected to be thread-safe on any platform that employs threading.
     *
     * @note
     *   This function is reserved for internal use by the System Clock.  Users of the System
     *   Clock should call System::Clock::GetClock_RealTimeMS().
     *
     * @param[in] aNewCurTime                The new current time, expressed as Unix time scaled to microseconds.
     *
     * @retval #CHIP_NO_ERROR       If the method succeeded.
     * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
     *                                      If the platform is incapable of tracking real time.
     * @retval #CHIP_ERROR_ACCESS_DENIED
     *                                      If the calling application does not have the privilege to set the
     *                                      current time.
     */
    virtual CHIP_ERROR SetClock_RealTime(Microseconds64 aNewCurTime) = 0;

protected:
    uint64_t mLastTimestamp = 0;
};

// Currently we have a single implementation class, ClockImpl, whose members are implemented in build-specific files.
class ClockImpl : public ClockBase
{
public:
    ~ClockImpl() override = default;
    Microseconds64 GetMonotonicMicroseconds64() override;
    Milliseconds64 GetMonotonicMilliseconds64() override;
    CHIP_ERROR GetClock_RealTime(Microseconds64 & aCurTime) override;
    CHIP_ERROR GetClock_RealTimeMS(Milliseconds64 & aCurTime) override;
    CHIP_ERROR SetClock_RealTime(Microseconds64 aNewCurTime) override;
};

namespace Internal {

// This should only be used via SystemClock() below.
extern ClockBase * gClockBase;

inline void SetSystemClockForTesting(Clock::ClockBase * clock)
{
    Clock::Internal::gClockBase = clock;
}

// Provide a mock implementation for use by unit tests.
class MockClock : public ClockImpl
{
public:
    Microseconds64 GetMonotonicMicroseconds64() override { return mSystemTime; }
    Milliseconds64 GetMonotonicMilliseconds64() override { return std::chrono::duration_cast<Milliseconds64>(mSystemTime); }
    CHIP_ERROR GetClock_RealTime(Microseconds64 & aCurTime) override
    {
        aCurTime = mRealTime;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetClock_RealTimeMS(Milliseconds64 & aCurTime) override
    {
        aCurTime = std::chrono::duration_cast<Milliseconds64>(mRealTime);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SetClock_RealTime(Microseconds64 aNewCurTime) override
    {
        mRealTime = aNewCurTime;
        return CHIP_NO_ERROR;
    }

    void SetMonotonic(Milliseconds64 timestamp)
    {
        mSystemTime = timestamp;
#if CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK
        __atomic_store_n(&mLastTimestamp, timestamp.count(), __ATOMIC_SEQ_CST);
#endif // CHIP_DEVICE_LAYER_USE_ATOMICS_FOR_CLOCK
    }

    void AdvanceMonotonic(Milliseconds64 increment) { mSystemTime += increment; }
    void AdvanceRealTime(Milliseconds64 increment) { mRealTime += increment; }

    Microseconds64 mSystemTime = Clock::kZero;
    Microseconds64 mRealTime   = Clock::kZero;
};

} // namespace Internal

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS
Microseconds64 TimevalToMicroseconds(const timeval & in);
void ToTimeval(Microseconds64 in, timeval & out);
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Clock

inline Clock::ClockBase & SystemClock()
{
    return *Clock::Internal::gClockBase;
}

} // namespace System
} // namespace chip
