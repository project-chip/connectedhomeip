/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for Tizen platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/PlatformManager.h>
#include <platform/Tizen/DeviceInstanceInfoProviderImpl.h>
#include <platform/Tizen/DiagnosticDataProviderImpl.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    ReturnErrorOnFailure(Internal::PosixConfig::Init());

    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack());

    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
