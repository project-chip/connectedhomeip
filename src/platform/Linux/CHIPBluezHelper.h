/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/*
 *  Copyright (c) 2016-2019, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *    @file
 *          Provides Bluez dbus implementation for BLE
 */

#pragma once

#include "BLEManagerImpl.h"
#include <stdbool.h>
#include <stdint.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "ble/CHIPBleServiceData.h"
#include "platform/CHIPDeviceConfig.h"
#include "platform/Linux/dbus/bluez/DbusBluez.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define BLUEZ_ADDRESS_SIZE 6 ///< BLE address size (in bytes)
#define BLUEZ_PATH "/org/bluez"
#define BLUEZ_INTERFACE "org.bluez"
#define ADAPTER_INTERFACE BLUEZ_INTERFACE ".Adapter1"
#define PROFILE_INTERFACE BLUEZ_INTERFACE ".GattManager1"
#define ADVERTISING_MANAGER_INTERFACE BLUEZ_INTERFACE ".LEAdvertisingManager1"
#define SERVICE_INTERFACE BLUEZ_INTERFACE ".GattService1"
#define CHARACTERISTIC_INTERFACE BLUEZ_INTERFACE ".GattCharacteristic1"
#define ADVERTISING_INTERFACE BLUEZ_INTERFACE ".LEAdvertisement1"
#define DEVICE_INTERFACE BLUEZ_INTERFACE ".Device1"

#define CHIP_PLAT_BLE_UUID_C1_STRING "18ee2ef5-263d-4559-959f-4f9c429f9d11"
#define CHIP_PLAT_BLE_UUID_C2_STRING "18ee2ef5-263d-4559-959f-4f9c429f9d12"
#define CHIP_PLAT_BLE_UUID_C3_STRING "64630238-8772-45F2-B87D-748A83218F04"

#define CHIP_BLE_BASE_SERVICE_UUID_STRING "-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_SERVICE_PREFIX_LENGTH 8
#define CHIP_BLE_BASE_SERVICE_PREFIX "0000"
#define CHIP_BLE_UUID_SERVICE_SHORT_STRING "fffb"

#define CHIP_BLE_UUID_SERVICE_STRING                                                                                               \
    CHIP_BLE_BASE_SERVICE_PREFIX CHIP_BLE_UUID_SERVICE_SHORT_STRING CHIP_BLE_BASE_SERVICE_UUID_STRING

#define BLUEZ_ADV_TYPE_FLAGS 0x01
#define BLUEZ_ADV_TYPE_SERVICE_DATA 0x16

#define BLUEZ_ADV_FLAGS_LE_LIMITED (1 << 0)
#define BLUEZ_ADV_FLAGS_LE_DISCOVERABLE (1 << 1)
#define BLUEZ_ADV_FLAGS_EDR_UNSUPPORTED (1 << 2)
#define BLUEZ_ADV_FLAGS_LE_EDR_CONTROLLER (1 << 3)
#define BLUEZ_ADV_FLAGS_LE_EDR_HOST (1 << 4)

#define CHAR_TO_BLUEZ(c) (static_cast<uint8_t>(((c) <= '9') ? (c) - '0' : tolower((c)) - 'a' + 10))

enum BluezAddressType
{
    BLUEZ_ADDRESS_TYPE_PUBLIC                        = 0, ///< Bluetooth public device address.
    BLUEZ_ADDRESS_TYPE_RANDOM_STATIC                 = 1, ///< Bluetooth random static address.
    BLUEZ_ADDRESS_TYPE_RANDOM_PRIVATE_RESOLVABLE     = 2, ///< Bluetooth random private resolvable address.
    BLUEZ_ADDRESS_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE = 3, ///< Bluetooth random private non-resolvable address.
};

struct BluezAddress
{
    BluezAddressType mType;               ///< Bluetooth device address type.
    uint8_t mAddress[BLUEZ_ADDRESS_SIZE]; ///< A 48-bit address of Bluetooth device in LSB format.
};

struct IOChannel
{
    GIOChannel * mpChannel;
    guint mWatch;
};

struct CHIPIdInfo
{
    uint8_t mMajor;
    uint8_t mMinor;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint64_t mDeviceId;
    uint8_t mPairingStatus;
} __attribute__((packed));

struct CHIPServiceData
{
    uint8_t mDataBlock0Len;
    uint8_t mDataBlock0Type;
    CHIPIdInfo mIdInfo;
} __attribute__((packed));

struct BluezDiscoveryRequest
{
    uint16_t mDiscriminator;
    bool mAutoConnect;
};

struct BluezEndpoint
{
    char * mpOwningName; // Bus owning name

    // Adapter properties
    char * mpAdapterName;
    char * mpAdapterAddr;

    // Paths for objects published by this service
    char * mpRootPath;
    char * mpAdvPath;
    char * mpServicePath;

    // Objects (interfaces) subscibed to by this service
    GDBusObjectManager * mpObjMgr;
    BluezAdapter1 * mpAdapter;
    BluezDevice1 * mpDevice;

    // Objects (interfaces) published by this service
    GDBusObjectManagerServer * mpRoot;
    BluezGattService1 * mpService;
    BluezGattCharacteristic1 * mpC1;
    BluezGattCharacteristic1 * mpC2;
    // additional data characteristics
    BluezGattCharacteristic1 * mpC3;

    // map device path to the connection
    GHashTable * mpConnMap;
    uint32_t mNodeId;
    bool mIsNotify;
    bool mIsCentral;
    char * mpAdvertisingUUID;
    chip::Ble::ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    ChipAdvType mType;  ///< Advertisement type.
    uint16_t mDuration; ///< Advertisement interval (in ms).
    bool mIsAdvertising;
    char * mpPeerDevicePath;

    // Discovery settings
    BluezDiscoveryRequest mDiscoveryRequest = {};
};

struct BluezConnection
{
    char * mpPeerAddress;
    BluezDevice1 * mpDevice;
    BluezGattService1 * mpService;
    BluezGattCharacteristic1 * mpC1;
    BluezGattCharacteristic1 * mpC2;
    // additional data characteristics
    BluezGattCharacteristic1 * mpC3;

    bool mIsNotify;
    uint16_t mMtu;
    struct IOChannel mC1Channel;
    struct IOChannel mC2Channel;
    BluezEndpoint * mpEndpoint;
};

struct ConnectionDataBundle
{
    BluezConnection * mpConn;
    GVariant * mpVal;
};

CHIP_ERROR InitBluezBleLayer(bool aIsCentral, char * apBleAddr, BLEAdvConfig & aBleAdvConfig, void *& apEndpoint);
bool BluezRunOnBluezThread(int (*aCallback)(void *), void * apClosure);
bool SendBluezIndication(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
bool CloseBluezConnection(BLE_CONNECTION_OBJECT apConn);
CHIP_ERROR StartBluezAdv(BluezEndpoint * apEndpoint);
CHIP_ERROR StopBluezAdv(BluezEndpoint * apEndpoint);
CHIP_ERROR BluezGattsAppRegister(BluezEndpoint * apEndpoint);
CHIP_ERROR BluezAdvertisementSetup(BluezEndpoint * apEndpoint);

CHIP_ERROR StartDiscovery(BluezEndpoint * apEndpoint, BluezDiscoveryRequest aRequest = {});
CHIP_ERROR StopDiscovery(BluezEndpoint * apEndpoint);

CHIP_ERROR ConnectDevice(BluezDevice1 * apDevice);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
