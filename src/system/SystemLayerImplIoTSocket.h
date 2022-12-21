/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file declares an implementation of System::Layer using select().
 */

#pragma once

#include <atomic>
#include <inet/EndPointStateIoTSocket.h>
#include <lib/support/ObjectLifeCycle.h>
#include <system/SystemLayer.h>
#include <system/SystemMutex.h>
#include <system/SystemTimer.h>

namespace chip {
namespace System {

class LayerImplOpenIoTSDK : public LayerOpenIoTSDK
{
public:
    LayerImplOpenIoTSDK();
    ~LayerImplOpenIoTSDK() { VerifyOrDie(mLayerState.Destroy()); }

    // Layer overrides.
    CHIP_ERROR Init() override;
    void Shutdown() override;
    bool IsInitialized() const override { return mLayerState.IsInitialized(); }
    CHIP_ERROR StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState) override;
    void CancelTimer(TimerCompleteCallback onComplete, void * appState) override;
    CHIP_ERROR ScheduleWork(TimerCompleteCallback onComplete, void * appState) override;

public:
    // Platform implementation.
    CHIP_ERROR HandlePlatformTimer(void);

private:
    friend class PlatformEventing;

    CHIP_ERROR StartPlatformTimer(System::Clock::Timeout aDelay);

    TimerPool<TimerList::Node> mTimerPool;
    TimerList mTimerList;
    bool mHandlingTimerComplete; // true while handling any timer completion
    ObjectLifeCycle mLayerState;

    /* socket */
public:
    CHIP_ERROR EnableSelectCallback(chip::Inet::EndPointStateIoTSocket * endpoint, bool read, bool write) override;
    CHIP_ERROR DisableSelectCallback(chip::Inet::EndPointStateIoTSocket * endpoint, bool read, bool write) override;

    void Signal() override;
    CHIP_ERROR WaitForEvents() override;
    void HandleEvents() override;

private:
    typedef void * SelectMask;
    enum SelectMaskType
    {
        ReadMask = 0,     // used to store the user requested set
        WriteMask,        // used to store the user requested set
        ExceptionMask,    // used to store the user requested set
        ReadMaskOut,      // contains the result of select
        WriteMaskOut,     // contains the result of select
        ExceptionMaskOut, // contains the result of select
        SelectMaskTypeMax
    };

    CHIP_ERROR EnableSignalSocket();

    /* Return the individual mask from the memory block storing all masks */
    SelectMask GetSelectMask(SelectMaskType mask) { return (SelectMask *) ((char *) mMaskMemory + mMaskSize * (size_t) mask); };

    /* Return true if any of the bits are set in the any of the result masks */
    bool IsResultSelectMaskSet()
    {
        uint32_t * current   = (uint32_t *) GetSelectMask(ReadMaskOut);
        const uint32_t * end = (uint32_t *) GetSelectMask(SelectMaskTypeMax);
        while (current < end)
        {
            if (*current)
            {
                return true;
            }
            current++;
        }
        return false;
    };

    static constexpr int kSocketMax = (INET_CONFIG_ENABLE_TCP_ENDPOINT ? INET_CONFIG_NUM_TCP_ENDPOINTS : 0) +
        (INET_CONFIG_ENABLE_UDP_ENDPOINT ? INET_CONFIG_NUM_UDP_ENDPOINTS : 0);

    chip::Inet::EndPointStateIoTSocket * mSelectEndpoints[kSocketMax];
    SelectMask mMaskMemory        = nullptr;
    size_t mSelectEndpointsNumber = 0;

    static std::atomic<int32_t> mSignalSocket;
    static Mutex mSelectMutex;
    static size_t mMaskSize;
    Mutex mSignalMutex;
    osEventFlagsId_t mSignalFlags = nullptr;
};

using LayerImpl = LayerImplOpenIoTSDK;

} // namespace System
} // namespace chip
