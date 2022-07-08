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

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

static_assert(std::is_same<uint32_t, chip::ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

extern "C" {

chip::ChipError::StorageType pychip_CommonStackInit()
{
    ReturnErrorOnFailure(chip::Platform::MemoryInit().AsInteger());
    ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgr().InitChipStack().AsInteger());
    return CHIP_NO_ERROR.AsInteger();
}

void pychip_CommonStackShutdown()
{
#if 0 //
      // We cannot actually call this because the destructor for the MdnsContexts singleton on Darwin only gets called
      // on termination of the program, and that unfortunately makes a bunch of Platform::MemoryFree calls.
      //
    chip::Platform::MemoryShutdown();
#endif
}
};
