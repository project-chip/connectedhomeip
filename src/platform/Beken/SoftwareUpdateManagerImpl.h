/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/internal/GenericSoftwareUpdateManagerImpl.h>
#include <platform/internal/GenericSoftwareUpdateManagerImpl_BDX.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the SoftwareUpdateManager singleton object for the
 * Beken platform.
 */
class SoftwareUpdateManagerImpl final : public SoftwareUpdateManager,
                                        public Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>,
                                        public Internal::GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>
{
    // Allow the SoftwareUpdateManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class SoftwareUpdateManager;

    // Allow the GenericSoftwareUpdateManagerImpl base class to access helper methods
    // and types defined on this class.
    friend class Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>;

    // Allow the GenericSoftwareUpdateManagerImpl_BDX base class to access helper methods
    // and types defined on this class.
    friend class Internal::GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>;

public:
    // ===== Members for internal use by the following friends.

    friend ::chip::DeviceLayer::SoftwareUpdateManager & SoftwareUpdateMgr(void);
    friend SoftwareUpdateManagerImpl & SoftwareUpdateMgrImpl(void);

    static SoftwareUpdateManagerImpl sInstance;

private:
    // ===== Members that implement the SoftwareUpdateManager abstract interface.

    CHIP_ERROR _Init(void);
};

/**
 * Returns a reference to the public interface of the SoftwareUpdateManager singleton object.
 *
 * Internal components should use this to access features of the SoftwareUpdateManager object
 * that are common to all platforms.
 */
inline SoftwareUpdateManager & SoftwareUpdateMgr(void)
{
    return SoftwareUpdateManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the SoftwareUpdateManager singleton object.
 *
 * Internal components can use this to gain access to features of the SoftwareUpdateManager
 * that are specific to the Beken platform.
 */
inline SoftwareUpdateManagerImpl & SoftwareUpdateMgrImpl(void)
{
    return SoftwareUpdateManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
