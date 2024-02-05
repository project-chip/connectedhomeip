/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    Copyright (c) 2023 NXP
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
 *          Provides the SMU2 namespace for K32W1 platform using the NXP SDK.
 *          This namespace implements all the necessary function to allocate
 *          OpenThread buffers from SMU2 region.
 */

#include "SMU2Manager.h"
#include <platform/CHIPDeviceLayer.h>

using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

namespace chip::SMU2 {
namespace {

static const uint32_t AREA_START = (0x489C5380U);
static const uint32_t AREA_END   = (0x489C87FFU);
static const uint32_t AREA_SIZE  = (AREA_END - AREA_START);

uint8_t mAreaId = 0;

PersistentStorageDelegate * mStorage = nullptr;

memAreaCfg_t mAreaDescriptor;
bool mDeviceCommissioned = false;
bool mUseAllocator       = false;

StorageKeyName GetSMU2AllocatorKey()
{
    return StorageKeyName::FromConst("nxp/ot-smu2");
}

void ResetBLEController()
{
    VerifyOrDie(BLEMgrImpl().ResetController() == CHIP_NO_ERROR);
}

void RegisterArea(void)
{
    mem_status_t st = kStatus_MemSuccess;

    memset((void *) AREA_START, 0x00, AREA_SIZE);

    mAreaDescriptor.start_address = (void *) AREA_START;
    mAreaDescriptor.end_address   = (void *) AREA_END;

    st = MEM_RegisterExtendedArea(&mAreaDescriptor, &mAreaId, AREA_FLAGS_POOL_NOT_SHARED);
    VerifyOrDie(st == kStatus_MemSuccess);
}

void UnregisterArea(void)
{
    mem_status_t st = kStatus_MemSuccess;

    st = MEM_UnRegisterExtendedArea(mAreaId);
    VerifyOrDie(st == kStatus_MemSuccess);
    mAreaId = 0;

    memset((void *) AREA_START, 0x00, AREA_SIZE);
}

void EventHandler(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kCommissioningComplete: {
        mDeviceCommissioned = true;
        break;
    }

    case DeviceEventType::kCHIPoBLEConnectionClosed: {
        if (mDeviceCommissioned)
        {
            mUseAllocator = true;
            mStorage->SyncSetKeyValue(GetSMU2AllocatorKey().KeyName(), (void *) &mUseAllocator, (uint16_t) sizeof(mUseAllocator));
            ResetBLEController();
            RegisterArea();
        }
        break;
    }
    }
}

} // anonymous namespace

CHIP_ERROR Init(PersistentStorageDelegate * storage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t size  = (uint16_t) sizeof(mUseAllocator);
    mStorage       = storage;

    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INCORRECT_STATE);

    PlatformMgr().AddEventHandler(EventHandler, reinterpret_cast<intptr_t>(nullptr));

    err = mStorage->SyncGetKeyValue(GetSMU2AllocatorKey().KeyName(), (void *) &mUseAllocator, size);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        mUseAllocator = false;
        err           = mStorage->SyncSetKeyValue(GetSMU2AllocatorKey().KeyName(), (void *) &mUseAllocator, size);
    }
    ReturnErrorOnFailure(err);

    if (mUseAllocator)
    {
        RegisterArea();
    }

    return err;
}

CHIP_ERROR Deactivate(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mUseAllocator)
    {
        mUseAllocator = false;
        err           = mStorage->SyncDeleteKeyValue(GetSMU2AllocatorKey().KeyName());
        ReturnErrorOnFailure(err);

        UnregisterArea();
        ResetBLEController();
    }

    return CHIP_NO_ERROR;
}

void * Allocate(size_t size)
{
    size_t smu2Size = 0;
    if (mAreaId)
    {
        smu2Size = MEM_GetFreeHeapSizeByAreaId(mAreaId);
        if (size > smu2Size)
        {
            mAreaId = 0;
        }
    }

    return MEM_BufferAllocWithId(size, mAreaId);
}

} // namespace chip::SMU2
