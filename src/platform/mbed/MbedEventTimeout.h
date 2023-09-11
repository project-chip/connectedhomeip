/* mbed Microcontroller Library
 * SPDX-FileCopyrightText: 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <chrono>
#include <platform/Callback.h>

namespace chip {
namespace DeviceLayer {

/**
 * The special mbed timeout implementation as a separate compilation block.
 * It is fix of sleep() function declaration conflict.
 * sleep() is declarated in unistd.h (toolchain) and in mbed_power_mgmt.h
 * which is included by Timeout.h
 */
class MbedEventTimeout
{
public:
    /**
     * Attach a function to be called by the Timeout, specifying the interval in microseconds
     *
     *  @param func pointer to the function to be called
     *  @param t the time between calls in micro-seconds
     *
     */
    static void AttachTimeout(mbed::Callback<void()> func, std::chrono::microseconds t);
    static void DetachTimeout();
};

} // namespace DeviceLayer
} // namespace chip
