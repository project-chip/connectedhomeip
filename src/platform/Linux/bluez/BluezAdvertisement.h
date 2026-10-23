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
#include <utility>

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>

#include "BluezEndpoint.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BluezAdvertisement
{
public:
    using ServiceDataFlags = uint16_t;
    // Minimum and maximum advertising intervals in units of 0.625ms.
    using AdvertisingIntervals = std::pair<uint16_t, uint16_t>;

    static constexpr ServiceDataFlags kServiceDataNone                 = 0;
    static constexpr ServiceDataFlags kServiceDataExtendedAnnouncement = 1 << 0;

    BluezAdvertisement(BluezEndpoint & aEndpoint) : mEndpoint(aEndpoint) {}
    ~BluezAdvertisement() { Shutdown(); }

    CHIP_ERROR Init(BluezAdapter1 * apAdapter, const char * aAdvUUID, const char * aAdvName);
    CHIP_ERROR SetupServiceData(ServiceDataFlags aFlags);
    CHIP_ERROR SetIntervals(AdvertisingIntervals aAdvIntervals);
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
    ///
    /// It is also possible that the advertising is released by BlueZ. In that case,
    /// the application will be notified by BLEManagerImpl::NotifyBLEPeripheralAdvReleased().
    CHIP_ERROR Stop();

private:
    BluezLEAdvertisement1 * CreateLEAdvertisement();
    gboolean BluezLEAdvertisement1Release(BluezLEAdvertisement1 * aAdv, GDBusMethodInvocation * aInv);

    CHIP_ERROR InitImpl();

    void StartDone(GObject * aObject, GAsyncResult * aResult);
    CHIP_ERROR StartImpl();

    void StopDone(GObject * aObject, GAsyncResult * aResult);
    CHIP_ERROR StopImpl();

    BluezEndpoint & mEndpoint;
    GAutoPtr<BluezAdapter1> mAdapter;
    GAutoPtr<BluezLEAdvertisement1> mAdv;

    bool mIsInitialized = false;
    bool mIsAdvertising = false;

    char mAdvPath[64] = ""; // D-Bus path of the advertisement object
    char mAdvUUID[64] = ""; // UUID of the service to be advertised
    char mAdvName[32] = "";
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
