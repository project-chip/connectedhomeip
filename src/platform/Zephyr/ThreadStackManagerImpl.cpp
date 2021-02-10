/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          Zephyr platforms.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp>
#include <platform/Zephyr/ThreadStackManagerImpl.h>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    return GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(openthread_get_default_instance());
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    // Intentionally empty.
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_LockThreadStack()
{
    openthread_api_mutex_lock(openthread_get_default_context());
}

bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    // There's no openthread_api_mutex_try_lock() in Zephyr, so until it's contributed we must use the low-level API
    return k_mutex_lock(&openthread_get_default_context()->api_lock, K_NO_WAIT) == 0;
}

void ThreadStackManagerImpl::_UnlockThreadStack()
{
    openthread_api_mutex_unlock(openthread_get_default_context());
}

void ThreadStackManagerImpl::_ProcessThreadActivity()
{
    // Intentionally empty.
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStart()
{
    // Intentionally empty.
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStop()
{
    // Intentionally empty.
}

} // namespace DeviceLayer
} // namespace chip
