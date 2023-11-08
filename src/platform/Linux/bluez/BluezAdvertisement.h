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

#pragma once

#include <stdint.h>

#include <gio/gio.h>
#include <glib.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct BLEAdvConfig;
struct BluezEndpoint;

class BluezAdvertisement
{
public:
    struct Configuration
    {
        char * mpBleName;
        uint32_t mAdapterId;
        uint8_t mMajor;
        uint8_t mMinor;
        uint16_t mVendorId;
        uint16_t mProductId;
        uint64_t mDeviceId;
        uint8_t mPairingStatus;
        ChipAdvType mType;
        uint16_t mDurationMs;
        const char * mpAdvertisingUUID;
    };

    BluezAdvertisement() = default;
    ~BluezAdvertisement() { Shutdown(); }

    CHIP_ERROR Init(BluezEndpoint * apEndpoint, const Configuration & aConfig);
    void Shutdown();

    CHIP_ERROR Start();
    CHIP_ERROR Stop();

private:
    CHIP_ERROR ConfigureBluezAdv(const Configuration & aConfig);

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

    bool mIsInitialized      = false;
    char * mpAdvPath         = nullptr;
    char * mpAdapterName     = nullptr;
    char * mpAdvertisingUUID = nullptr;
    chip::Ble::ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    ChipAdvType mType;
    uint16_t mDurationMs;
    bool mIsAdvertising = false;

    // Allow BluezConnection to access our private members
    friend class BluezConnection;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
