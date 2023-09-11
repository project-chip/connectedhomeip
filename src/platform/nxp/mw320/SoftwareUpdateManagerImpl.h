/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/internal/GenericSoftwareUpdateManagerImpl.h>
#include <platform/internal/GenericSoftwareUpdateManagerImpl_BDX.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the SoftwareUpdateManager singleton object for the
 * NXP MW320 platforms.
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
 * that are specific to the MW320 platform.
 */
inline SoftwareUpdateManagerImpl & SoftwareUpdateMgrImpl(void)
{
    return SoftwareUpdateManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
