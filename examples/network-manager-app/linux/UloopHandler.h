/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <lib/core/CHIPError.h>
#include <system/SystemLayer.h>

extern "C" struct uloop_fd;

namespace chip {

/** Integrates uloop into the SystemLayer event loop. */
class UloopHandler final : private System::EventLoopHandler
{
public:
    /**
     * Starts driving uloop via the SystemLayer event loop.
     * May be called multiple times.
     * Each successful call to Register() must be balanced by a call to Unregister().
     */
    static CHIP_ERROR Register();
    static void Unregister();

private:
    constexpr UloopHandler() = default;

    System::Clock::Timestamp PrepareEvents(System::Clock::Timestamp now) override;
    void HandleEvents() override;

    static void UloopFdSet(struct uloop_fd * fd, unsigned int events);

    static UloopHandler gInstance;
    static int gRegistered;
};

} // namespace chip
