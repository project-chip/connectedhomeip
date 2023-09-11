/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          Zephyr platforms.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/Zephyr/ThreadStackManagerImpl.h>

#include <inet/UDPEndPointImpl.h>
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::Inet;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    otInstance * const instance = openthread_get_default_instance();

    ReturnErrorOnFailure(GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(instance));

    UDPEndPointImplSockets::SetJoinMulticastGroupHandler([](InterfaceId, const IPAddress & address) {
        const otIp6Address otAddress = ToOpenThreadIP6Address(address);

        ThreadStackMgr().LockThreadStack();
        const auto otError = otIp6SubscribeMulticastAddress(openthread_get_default_instance(), &otAddress);
        ThreadStackMgr().UnlockThreadStack();

        return MapOpenThreadError(otError);
    });

    UDPEndPointImplSockets::SetLeaveMulticastGroupHandler([](InterfaceId, const IPAddress & address) {
        const otIp6Address otAddress = ToOpenThreadIP6Address(address);

        ThreadStackMgr().LockThreadStack();
        const auto otError = otIp6UnsubscribeMulticastAddress(openthread_get_default_instance(), &otAddress);
        ThreadStackMgr().UnlockThreadStack();

        return MapOpenThreadError(otError);
    });

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

} // namespace DeviceLayer
} // namespace chip
