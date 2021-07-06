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
 *      Defines the chip Device Layer TimeSyncManager object.
 *
 */

#pragma once

namespace chip {
namespace Profiles {
namespace StatusReporting {
class StatusReport;
}
} // namespace Profiles
} // namespace chip

namespace chip {
namespace DeviceLayer {

class PlatformManagerImpl;
namespace Internal {
template <class>
class GenericPlatformManagerImpl;
template <class>
class GenericPlatformManagerImpl_FreeRTOS;
} // namespace Internal

/**
 * Manages time synchronization for chip Devices.
 */
class TimeSyncManager final
{
public:
    // ===== Members that define the public interface of the TimeSyncManager

    enum TimeSyncMode
    {
        kTimeSyncMode_NotSupported = 0,
        kTimeSyncMode_Disabled     = 1,
        kTimeSyncMode_Service      = 2,

        kTimeSyncMode_Max,
    };

    TimeSyncMode GetMode();
    CHIP_ERROR SetMode(TimeSyncMode newMode);

    uint32_t GetSyncInterval();
    void SetSyncInterval(uint32_t intervalSec);

    bool IsTimeSynchronized();

private:
    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl;
    template <class>
    friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    friend TimeSyncManager & TimeSyncMgr();

    static TimeSyncManager sInstance;

    CHIP_ERROR Init();
    void OnPlatformEvent(const ChipDeviceEvent * event);

    // ===== Private members for use by this class only.

    uint64_t mLastSyncTimeMS; // in monotonic time
#if CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
    ::chip::Binding * mTimeSyncBinding;
#endif
    uint32_t mSyncIntervalSec;
    TimeSyncMode mMode;

    void DriveTimeSync();
    void CancelTimeSync();
    void ApplySynchronizedTime(uint64_t syncedRealTimeUS);
    void TimeSyncFailed(CHIP_ERROR reason, chip::Profiles::StatusReporting::StatusReport * statusReport);

#if CHIP_DEVICE_CONFIG_ENABLE_CHIP_TIME_SERVICE_TIME_SYNC
    static void TimeServiceSync_HandleBindingEvent(void * appState, ::chip::Binding::EventType event,
                                                   const ::chip::Binding::InEventParam & inParam,
                                                   ::chip::Binding::OutEventParam & outParam);
    static void TimeServiceSync_HandleSyncComplete(void * context, CHIP_ERROR result, int64_t syncedRealTimeUS);
#endif

    static void DriveTimeSync(::chip::System::Layer * layer, void * appState, ::CHIP_ERROR err);

protected:
    // Construction/destruction limited to subclasses.
    TimeSyncManager()  = default;
    ~TimeSyncManager() = default;

    // No copy, move or assignment.
    TimeSyncManager(const TimeSyncManager &)  = delete;
    TimeSyncManager(const TimeSyncManager &&) = delete;
    TimeSyncManager & operator=(const TimeSyncManager &) = delete;
};

inline TimeSyncManager::TimeSyncMode TimeSyncManager::GetMode()
{
    return mMode;
}

inline uint32_t TimeSyncManager::GetSyncInterval()
{
    return mSyncIntervalSec;
}

/**
 * Returns a reference to the TimeSyncManager singleton object.
 */
inline TimeSyncManager & TimeSyncMgr()
{
    return TimeSyncManager::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
