/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object
 *          for Telink platform.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#include <platform/telink/ThreadStackManagerImpl.h>

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
    mRadioBlocked               = false;
    mReadyToAttach              = false;
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

CHIP_ERROR
ThreadStackManagerImpl::_AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                               NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback)
{
    CHIP_ERROR result = CHIP_NO_ERROR;

    if (mRadioBlocked)
    {
        /* On Telink platform it's not possible to rise Thread network when its used by BLE,
           so just mark that it's provisioned and rise Thread after BLE disconnect */
        result = SetThreadProvision(dataset.AsByteSpan());
        if (result == CHIP_NO_ERROR)
        {
            mReadyToAttach = true;
            callback->OnResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);
        }
    }
    else
    {
        result =
            Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::_AttachToThreadNetwork(dataset, callback);
    }
    return result;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(NetworkCommissioning::ThreadDriver::ScanCallback * callback)
{
    mpScanCallback = callback;

    /* On Telink platform it's not possible to rise Thread network when its used by BLE,
       so Thread networks scanning performed before start BLE and also available after switch into Thread */
    if (mRadioBlocked)
    {
        if (mpScanCallback != nullptr)
        {
            DeviceLayer::SystemLayer().ScheduleLambda([this]() {
                mpScanCallback->OnFinished(NetworkCommissioning::Status::kSuccess, CharSpan(), &mScanResponseIter);
                mpScanCallback = nullptr;
            });
        }
    }
    else
    {
        return Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::_StartThreadScan(mpScanCallback);
    }

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::Finalize(void)
{
    otInstanceFinalize(openthread_get_default_instance());
}

} // namespace DeviceLayer
} // namespace chip
