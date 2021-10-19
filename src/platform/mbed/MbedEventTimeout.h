/* mbed Microcontroller Library
 * Copyright (c) 2021 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
