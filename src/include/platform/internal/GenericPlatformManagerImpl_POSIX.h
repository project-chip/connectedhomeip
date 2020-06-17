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
 *          Provides an generic implementation of PlatformManager features
 *          for use on Linux platforms.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_POSIX_H
#define GENERIC_PLATFORM_MANAGER_IMPL_POSIX_H

#include <platform/internal/GenericPlatformManagerImpl.h>

#include <map>
#include <memory>
#include <queue>

#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <dbus/dbus.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of PlatformManager features that works on any OSAL platform.
 *
 * This template contains implementations of select features from the PlatformManager abstract
 * interface that are suitable for use on OSAL-based platforms.  It is intended to be inherited
 * (directly or indirectly) by the PlatformManagerImpl class, which also appears as the template's
 * ImplClass parameter.
 */
template <class ImplClass>
class GenericPlatformManagerImpl_POSIX : public GenericPlatformManagerImpl<ImplClass>
{
public:
    // ===== Public methods
    DBusConnection & GetSystemDBusConnection();

protected:
    // Members for select loop
    int mMaxFd;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;
    struct timeval mNextTimeout;

    // OS-specific members (pthread)
    pthread_mutex_t mChipStackLock;
    std::queue<ChipDeviceEvent> mChipEventQueue;

    pthread_t mChipTask;
    pthread_attr_t mChipTaskAttr;
    struct sched_param mChipTaskSchedParam;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR
    _InitChipStack();
    void _LockChipStack(void);
    bool _TryLockChipStack(void);
    void _UnlockChipStack(void);
    void _PostEvent(const ChipDeviceEvent * event);
    void _RunEventLoop(void);
    CHIP_ERROR _StartEventLoopTask(void);
    CHIP_ERROR _StartChipTimer(uint32_t durationMS);

private:
    // ===== Private members for use by this class only.

    inline ImplClass * Impl() { return static_cast<ImplClass *>(this); }

    void SysUpdate();
    void SysProcess();
    static void SysOnEventSignal(void * arg);

    void ProcessDeviceEvents();

    static void * EventLoopTaskMain(void * arg);

    static dbus_bool_t AddDBusWatch(struct DBusWatch * aWatch, void * aContext);
    static void RemoveDBusWatch(struct DBusWatch * aWatch, void * aContext);
    static void ToggleDBusWatch(struct DBusWatch * aWatch, void * aContext);

    void UpdateDBusFdSet();
    void ProcessDBus();

    struct DBusConnectionDeleter
    {
        void operator()(DBusConnection * aConnection) { dbus_connection_unref(aConnection); }
    };
    using UniqueDBusConnection = std::unique_ptr<DBusConnection, DBusConnectionDeleter>;
    UniqueDBusConnection mDBusConnection;
    /**
     * This map is used to track DBusWatch-es.
     *
     */
    using WatchMap = std::map<DBusWatch *, bool>;
    WatchMap mWatches;
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_POSIX_H
