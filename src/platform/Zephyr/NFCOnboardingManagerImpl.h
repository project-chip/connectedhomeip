/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      Platform-specific NFCOnboardingManager implementation for Zephyr RTOS.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace chip {
namespace DeviceLayer {

class NFCOnboardingManagerImpl final : public NFCOnboardingManager
{
    friend class NFCOnboardingManager;

private:
    // ===== Members that implement the NFCOnboardingManager internal interface.

    CHIP_ERROR _Init();
    CHIP_ERROR _StartTagEmulation(const char * payload, size_t payloadLength);
    CHIP_ERROR _StopTagEmulation();
    bool _IsTagEmulationStarted() const { return mIsStarted; };

    // ===== Members for internal use by this class.

    constexpr static uint8_t kNdefBufferSize = 128;

    uint8_t mNdefBuffer[kNdefBufferSize];
    bool mIsStarted;

    // ===== Members for internal use by the following friends.

    friend NFCOnboardingManager & NFCOnboardingMgr();
    friend NFCOnboardingManagerImpl & NFCOnboardingMgrImpl();

    static NFCOnboardingManagerImpl sInstance;
};

inline NFCOnboardingManager & NFCOnboardingMgr()
{
    return NFCOnboardingManagerImpl::sInstance;
}

inline NFCOnboardingManagerImpl & NFCOnboardingMgrImpl()
{
    return NFCOnboardingManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
