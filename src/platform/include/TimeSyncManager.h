/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *      Defines the Weave Device Layer TimeSyncManager object.
 *
 */

#ifndef TIME_SYNC_MANAGER_H
#define TIME_SYNC_MANAGER_H

namespace nl {
namespace Weave {
namespace Profiles {
namespace StatusReporting {
class StatusReport;
}
}
}
}

namespace nl {
namespace Weave {
namespace DeviceLayer {

class PlatformManagerImpl;
namespace Internal {
extern WEAVE_ERROR InitServiceDirectoryManager(void);
template<class> class GenericPlatformManagerImpl;
template<class> class GenericPlatformManagerImpl_FreeRTOS;
}

/**
 * Manages time synchronization for Weave Devices.
 */
class TimeSyncManager final
{
public:

    // ===== Members that define the public interface of the TimeSyncManager

    enum TimeSyncMode
    {
        kTimeSyncMode_NotSupported          = 0,
        kTimeSyncMode_Disabled              = 1,
        kTimeSyncMode_Service               = 2,

        kTimeSyncMode_Max,
    };

    TimeSyncMode GetMode();
    WEAVE_ERROR SetMode(TimeSyncMode newMode);

    uint32_t GetSyncInterval();
    void SetSyncInterval(uint32_t intervalSec);

    bool IsTimeSynchronized();

private:

    // ===== Members for internal use by the following friends.

    friend class PlatformManagerImpl;
    template<class> friend class Internal::GenericPlatformManagerImpl;
    template<class> friend class Internal::GenericPlatformManagerImpl_FreeRTOS;
    friend WEAVE_ERROR Internal::InitServiceDirectoryManager();
    friend TimeSyncManager & TimeSyncMgr(void);

    static TimeSyncManager sInstance;

    WEAVE_ERROR Init();
    void OnPlatformEvent(const WeaveDeviceEvent * event);
#if WEAVE_DEVICE_CONFIG_ENABLE_SERVICE_DIRECTORY_TIME_SYNC
    static void MarkServiceDirRequestStart();
    static void ProcessServiceDirTimeData(uint64_t timeQueryReceiptMsec, uint32_t timeProcessMsec);
#endif

    // ===== Private members for use by this class only.

    uint64_t mLastSyncTimeMS; // in monotonic time
#if WEAVE_DEVICE_CONFIG_ENABLE_SERVICE_DIRECTORY_TIME_SYNC
    uint64_t mServiceDirTimeSyncStartUS;
#endif
#if WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC
    ::nl::Weave::Binding * mTimeSyncBinding;
#endif
    uint32_t mSyncIntervalSec;
    TimeSyncMode mMode;

    void DriveTimeSync();
    void CancelTimeSync();
    void ApplySynchronizedTime(uint64_t syncedRealTimeUS);
    void TimeSyncFailed(WEAVE_ERROR reason, nl::Weave::Profiles::StatusReporting::StatusReport * statusReport);

#if WEAVE_DEVICE_CONFIG_ENABLE_WEAVE_TIME_SERVICE_TIME_SYNC
    static void TimeServiceSync_HandleBindingEvent(void * appState, ::nl::Weave::Binding::EventType event,
            const ::nl::Weave::Binding::InEventParam & inParam, ::nl::Weave::Binding::OutEventParam & outParam);
    static void TimeServiceSync_HandleSyncComplete(void * context, WEAVE_ERROR result, int64_t syncedRealTimeUS);
#endif

    static void DriveTimeSync(::nl::Weave::System::Layer * layer, void * appState, ::nl::Weave::System::Error err);

protected:

    // Construction/destruction limited to subclasses.
    TimeSyncManager() = default;
    ~TimeSyncManager() = default;

    // No copy, move or assignment.
    TimeSyncManager(const TimeSyncManager &) = delete;
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
inline TimeSyncManager & TimeSyncMgr(void)
{
    return TimeSyncManager::sInstance;
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // TIME_SYNC_MANAGER_H
