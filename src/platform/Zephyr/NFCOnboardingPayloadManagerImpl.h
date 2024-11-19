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
 *      Platform-specific NFCOnboardingPayloadManager implementation for Zephyr RTOS.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace chip {
namespace DeviceLayer {

class NFCOnboardingPayloadManagerImpl final : public NFCOnboardingPayloadManager
{
    friend class NFCOnboardingPayloadManager;

private:
    // ===== Members that implement the NFCOnboardingPayloadManager internal interface.

    CHIP_ERROR _Init();
    CHIP_ERROR _StartTagEmulation(const char * payload, size_t payloadLength);
    CHIP_ERROR _StopTagEmulation();
    bool _IsTagEmulationStarted() const { return mIsStarted; };

    // ===== Members for internal use by this class.

    constexpr static uint8_t kNdefBufferSize = 128;

    uint8_t mNdefBuffer[kNdefBufferSize];
    bool mIsStarted;

    // ===== Members for internal use by the following friends.

    friend NFCOnboardingPayloadManager & NFCOnboardingPayloadMgr();
    friend NFCOnboardingPayloadManagerImpl & NFCOnboardingPayloadMgrImpl();

    static NFCOnboardingPayloadManagerImpl sInstance;
};

inline NFCOnboardingPayloadManager & NFCOnboardingPayloadMgr()
{
    return NFCOnboardingPayloadManagerImpl::sInstance;
}

inline NFCOnboardingPayloadManagerImpl & NFCOnboardingPayloadMgrImpl()
{
    return NFCOnboardingPayloadManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
