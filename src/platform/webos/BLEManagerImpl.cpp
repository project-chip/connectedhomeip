/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          for webOS platforms.
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

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    return err;
}

CHIP_ERROR BLEManagerImpl::_Shutdown()
{
    // ensure scan resources are cleared (e.g. timeout timers)
    mDeviceScanner.reset();

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

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent) {}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const {}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return true;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return true;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    return true;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBufferHandle pBuf)
{
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    return true;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBufferHandle pBuf)
{
    ChipLogError(Ble, "SendReadRequest: Not implemented");
    return true;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId)
{
    ChipLogError(Ble, "SendReadRBluezonse: Not implemented");
    return true;
}

void BLEManagerImpl::HandleNewConnection(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error) {}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed) {}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len) {}

void BLEManagerImpl::HandleRXCharWrite(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len) {}

void BLEManagerImpl::CHIPoBluez_ConnectionClosed(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::HandleTXCharCCCDWrite(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::HandleTXComplete(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::DriveBLEState() {}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
}

void BLEManagerImpl::InitiateScan(BleScanState scanType) {}

void BLEManagerImpl::CleanScanConfig() {}

void BLEManagerImpl::InitiateScan(intptr_t arg) {}

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const uint16_t connDiscriminator) {}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate) {}

void BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate) {}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate) {}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate) {}

void BLEManagerImpl::OnDeviceScanned(BluezDevice1 * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) {}

void BLEManagerImpl::OnScanComplete() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
