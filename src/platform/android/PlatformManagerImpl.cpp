/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          Provides an implementation of the PlatformManager object
 *          for Android platforms.
 */

#include <platform/android/PlatformManagerImpl.h>

#include <platform/android/AndroidConfig.h>

namespace chip {
namespace DeviceLayer {

PlatformManager & PlatformMgr()
{
    static PlatformManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR PlatformManagerImpl::InitChipStackInner()
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::AndroidConfig::Init();
    SuccessOrExit(err);

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_POSIX::InitChipStackInner();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace DeviceLayer
} // namespace chip
