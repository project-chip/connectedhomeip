/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Platform-specific NFCManager implementation for Zephyr RTOS.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace chip {
namespace DeviceLayer {

class NFCManagerImpl final : public NFCManager
{
    friend class NFCManager;

private:
    // ===== Members that implement the NFCManager internal interface.

    CHIP_ERROR _Init();
    CHIP_ERROR _StartTagEmulation(const char * payload, size_t payloadLength);
    CHIP_ERROR _StopTagEmulation();
    bool _IsTagEmulationStarted() const { return mIsStarted; };

    // ===== Members for internal use by this class.

    constexpr static uint8_t kNdefBufferSize = 128;

    uint8_t mNdefBuffer[kNdefBufferSize];
    bool mIsStarted;

    // ===== Members for internal use by the following friends.

    friend NFCManager & NFCMgr();
    friend NFCManagerImpl & NFCMgrImpl();

    static NFCManagerImpl sInstance;
};

inline NFCManager & NFCMgr()
{
    return NFCManagerImpl::sInstance;
}

inline NFCManagerImpl & NFCMgrImpl()
{
    return NFCManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
