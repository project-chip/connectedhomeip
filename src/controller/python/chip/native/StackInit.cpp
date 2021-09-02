/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <errno.h>
#include <pthread.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

namespace {

pthread_t sPlatformMainThread;
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
uint32_t sBluetoothAdapterId = 0;
#endif

void * PlatformMainLoop(void *)
{
    ChipLogProgress(DeviceLayer, "Platform main loop started.");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "Platform main loop completed.");
    return nullptr;
}

} // namespace

extern "C" {

static_assert(std::is_same<uint32_t, chip::ChipError::StorageType>::value, "python assumes CHIP_ERROR maps to c_uint32");

chip::ChipError::StorageType pychip_BLEMgrImpl_ConfigureBle(uint32_t bluetoothAdapterId)
{
#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    sBluetoothAdapterId = bluetoothAdapterId;
    CHIP_ERROR err =
        chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(/* BLE adapter ID */ bluetoothAdapterId, /* BLE central */ true);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err.AsInteger());
#endif
    return CHIP_NO_ERROR.AsInteger();
}

void pychip_native_init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: memory init failed: %s", chip::ErrorStr(err));
    }

#if CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    // By default, Linux device is configured as a BLE peripheral while the controller needs a BLE central.
    err = chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(/* BLE adapter ID */ sBluetoothAdapterId, /* BLE central */ true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to configure BLE central:  %s", chip::ErrorStr(err));
    }
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: platform init failed: %s", chip::ErrorStr(err));
    }
    int result   = pthread_create(&sPlatformMainThread, nullptr, PlatformMainLoop, nullptr);
    int tmpErrno = errno;

    if (result != 0)
    {
        ChipLogError(DeviceLayer, "Failed to initialize CHIP stack: pthread_create failed: %s", strerror(tmpErrno));
    }
}
}
