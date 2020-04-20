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
 *          Provides an implementation of the PlatformManager object
 *          for the ESP32 platform.
 */

#ifndef PLATFORM_MANAGER_IMPL_H
#define PLATFORM_MANAGER_IMPL_H

#include <Weave/DeviceLayer/FreeRTOS/GenericPlatformManagerImpl_FreeRTOS.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the ESP32 platform.
 */
class PlatformManagerImpl final
    : public PlatformManager,
      public Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
    friend Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

public:

    // ===== Platform-specific members that may be accessed directly by the application.

    WEAVE_ERROR InitLwIPCoreLock(void);
    static esp_err_t HandleESPSystemEvent(void * ctx, system_event_t * event);

private:

    // ===== Methods that implement the PlatformManager abstract interface.

    WEAVE_ERROR _InitWeaveStack(void);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);

    static PlatformManagerImpl sInstance;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * Weave applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * Weave applications can use this to gain access to features of the PlatformManager
 * that are specific to the ESP32 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // PLATFORM_MANAGER_IMPL_H
