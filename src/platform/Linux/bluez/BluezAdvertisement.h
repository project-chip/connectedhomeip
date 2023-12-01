/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#pragma once

#include <stdint.h>

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BluezEndpoint;

class BluezAdvertisement
{
public:
    BluezAdvertisement() = default;
    ~BluezAdvertisement() { Shutdown(); }

    CHIP_ERROR Init(const BluezEndpoint & aEndpoint, ChipAdvType aAdvType, const char * aAdvUUID, uint32_t aAdvDurationMs);
    void Shutdown();

    /// Start BLE advertising.
    ///
    /// BLE advertising is started asynchronously. Application will be notified of
    /// completion via a call to BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete().
    CHIP_ERROR Start();

    /// Stop BLE advertising.
    ///
    /// BLE advertising is stopped asynchronously. Application will be notified of
    /// completion via a call to BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete().
    CHIP_ERROR Stop();

private:
    BluezLEAdvertisement1 * CreateLEAdvertisement();
    gboolean BluezLEAdvertisement1Release(BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInv);

    CHIP_ERROR InitImpl();

    void StartDone(GObject * aObject, GAsyncResult * aResult);
    CHIP_ERROR StartImpl();

    void StopDone(GObject * aObject, GAsyncResult * aResult);
    CHIP_ERROR StopImpl();

    // Objects (interfaces) used by LE advertisement
    GDBusObjectManagerServer * mpRoot = nullptr;
    BluezAdapter1 * mpAdapter         = nullptr;
    BluezLEAdvertisement1 * mpAdv     = nullptr;

    bool mIsInitialized = false;
    bool mIsAdvertising = false;

    Ble::ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    char * mpAdvPath     = nullptr;
    char * mpAdapterName = nullptr;
    char * mpAdvUUID     = nullptr;
    ChipAdvType mAdvType;
    uint16_t mAdvDurationMs = 0;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
