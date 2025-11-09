/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *          Provides an implementation of the ThreadStackManager object for
 *          ESP32 platform.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ThreadStackManager.h>

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

#include "lib/core/CHIPError.h"
#include <lib/support/CodeUtils.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <system/SystemLayerImpl.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    CHIP_ERROR error;

    SuccessOrExit(error = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(nullptr));

    static_cast<chip::System::LayerImpl &>(SystemLayer()).ConfigureOpenThread(OTInstance());

exit:
    return error;
}

void ThreadStackManagerImpl::_LockThreadStack() {}
bool ThreadStackManagerImpl::_TryLockThreadStack()
{
    return true;
}
void ThreadStackManagerImpl::_UnlockThreadStack() {}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
void ThreadStackManagerImpl::_WaitOnSrpClearAllComplete() {}

void ThreadStackManagerImpl::_NotifySrpClearAllComplete() {}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

} // namespace DeviceLayer
} // namespace chip
