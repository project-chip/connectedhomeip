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
 *          Provides an implementation of the BLEManager singleton object
 *          for Linux platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <new>
#include <platform/internal/BLEManager.h>

#include <cassert>
#include <type_traits>
#include <utility>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "bluez/Helper.h"

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static constexpr System::Clock::Timeout kNewConnectionScanTimeout = System::Clock::Seconds16(10);
static constexpr System::Clock::Timeout kConnectTimeout           = System::Clock::Seconds16(10);

const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR BLEManagerImpl::_Shutdown()
{
    // ensure scan resources are cleared (e.g. timeout timers)
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

uint16_t BLEManagerImpl::_NumConnections()
{
    return 0;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::StartBLEAdvertising()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::StopBLEAdvertising()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return false;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    return false;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBufferHandle pBuf)
{
    ChipLogError(Ble, "SendReadRequest: Not implemented");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId)
{
    ChipLogError(Ble, "SendReadRBluezonse: Not implemented");
    return false;
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
