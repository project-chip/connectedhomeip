/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/UnitTestUtils.h>

// Platform specific includes for test_utils
#include <platform/CHIPDeviceBuildConfig.h>
#if CHIP_DEVICE_LAYER_TARGET_EFR32 || CHIP_DEVICE_LAYER_TARGET_AMEBA
#include <FreeRTOS.h>
#include <task.h>
#else
#include <time.h>
#include <unistd.h>
#endif

namespace chip {
namespace test_utils {

#if CHIP_DEVICE_LAYER_TARGET_EFR32 || CHIP_DEVICE_LAYER_TARGET_AMEBA

namespace {

constexpr uint64_t TicksToMillis(uint32_t ticks)
{
    const uint64_t seconds      = ticks / configTICK_RATE_HZ;
    const uint64_t millis_ticks = ticks - (seconds * configTICK_RATE_HZ);
    const uint64_t millis       = (((millis_ticks * 1000) + (configTICK_RATE_HZ / 2)) / configTICK_RATE_HZ);
    return (seconds * 1000) + millis;
}

} // namespace

uint64_t TimeMonotonicMillis()
{
    return TicksToMillis(xTaskGetTickCount());
}

void SleepMillis(uint64_t millisecs)
{
    uint32_t ticks = static_cast<uint32_t>(millisecs / portTICK_PERIOD_MS);
    vTaskDelay(ticks == 0 ? 1 : ticks); // delay at least 1 tick
}

void SleepMicros(uint64_t microsecs)
{
    // FreeRTOS currently only sleep for intervals of 1ms
    SleepMillis((microsecs + 500) / 1000);
}

#else

void SleepMicros(uint64_t microsecs)
{
    usleep(static_cast<useconds_t>(microsecs));
}

void SleepMillis(uint64_t millisecs)
{
    SleepMicros(millisecs * 1000);
}

uint64_t TimeMonotonicMillis()
{
    return static_cast<uint64_t>(time(nullptr) * 1000);
}

#endif

} // namespace test_utils
} // namespace chip
