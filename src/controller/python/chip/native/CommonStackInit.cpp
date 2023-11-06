/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      Implementation of the native methods expected by the Python
 *      version of Chip Device Manager.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#include <controller/python/chip/native/PyChipError.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;

extern "C" {

struct __attribute__((packed)) PyCommonStackInitParams
{
    uint32_t mBluetoothAdapterId;
};

/**
 * Function to artifically cause a crash to happen
 * that can be used in place of os.exit() in Python so that
 * when run through GDB, you'll get a backtrace of what happened.
 */
void pychip_CauseCrash()
{
    uint8_t * ptr = nullptr;
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference): Intentionally trying to cause crash.
    *ptr = 0;
}

PyChipError pychip_CommonStackInit(const PyCommonStackInitParams * aParams)
{
    PyReturnErrorOnFailure(ToPyChipError(Platform::MemoryInit()));

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    PyReturnErrorOnFailure(
        ToPyChipError(DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(aParams->mBluetoothAdapterId, /* BLE central */ true)));
#endif

    PyReturnErrorOnFailure(ToPyChipError(DeviceLayer::PlatformMgr().InitChipStack()));

    return ToPyChipError(CHIP_NO_ERROR);
}

void pychip_CommonStackShutdown()
{
#if 0 //
      // We cannot actually call this because the destructor for the MdnsContexts singleton on Darwin only gets called
      // on termination of the program, and that unfortunately makes a bunch of Platform::MemoryFree calls.
      //
    Platform::MemoryShutdown();
#endif
}
};
