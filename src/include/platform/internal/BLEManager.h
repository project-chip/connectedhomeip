/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Defines the abstract interface for the Device Layer's
 *          internal BLEManager object.
 */

#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BLEManagerImpl;

/**
 * Provides control over CHIPoBLE services and connectivity for a chip device.
 *
 * BLEManager defines the abstract interface of a singleton object that provides
 * control over CHIPoBLE services and connectivity for a chip device.  BLEManager
 * is an internal object that is used by other components with the chip Device
 * Layer, but is not directly accessible to the application.
 */
class BLEManager
{
    using ImplClass = BLEManagerImpl;

public:
    // ===== Members that define the internal interface of the BLEManager

    using CHIPoBLEServiceMode = ConnectivityManager::CHIPoBLEServiceMode;

    CHIP_ERROR Init(void);
    CHIPoBLEServiceMode GetCHIPoBLEServiceMode(void);
    CHIP_ERROR SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val);
    bool IsAdvertisingEnabled(void);
    CHIP_ERROR SetAdvertisingEnabled(bool val);
    bool IsFastAdvertisingEnabled(void);
    CHIP_ERROR SetFastAdvertisingEnabled(bool val);
    bool IsAdvertising(void);
    CHIP_ERROR GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR SetDeviceName(const char * deviceName);
    uint16_t NumConnections(void);
    void OnPlatformEvent(const ChipDeviceEvent * event);
    chip::Ble::BleLayer * GetBleLayer(void) const;

protected:
    // Construction/destruction limited to subclasses.
    BLEManager()  = default;
    ~BLEManager() = default;

    // No copy, move or assignment.
    BLEManager(const BLEManager &)  = delete;
    BLEManager(const BLEManager &&) = delete;
    BLEManager & operator=(const BLEManager &) = delete;
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
extern BLEManager & BLEMgr(void);

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * chip applications can use this to gain access to features of the BLEManager
 * that are specific to the selected platform.
 */
extern BLEManagerImpl & BLEMgrImpl(void);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the BLEManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_BLEMANAGERIMPL_HEADER
#include EXTERNAL_BLEMANAGERIMPL_HEADER
#else
#define BLEMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/BLEManagerImpl.h>
#include BLEMANAGERIMPL_HEADER
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

inline CHIP_ERROR BLEManager::Init(void)
{
    return static_cast<ImplClass *>(this)->_Init();
}

inline BLEManager::CHIPoBLEServiceMode BLEManager::GetCHIPoBLEServiceMode(void)
{
    return static_cast<ImplClass *>(this)->_GetCHIPoBLEServiceMode();
}

inline CHIP_ERROR BLEManager::SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    return static_cast<ImplClass *>(this)->_SetCHIPoBLEServiceMode(val);
}

inline bool BLEManager::IsAdvertisingEnabled(void)
{
    return static_cast<ImplClass *>(this)->_IsAdvertisingEnabled();
}

inline CHIP_ERROR BLEManager::SetAdvertisingEnabled(bool val)
{
    return static_cast<ImplClass *>(this)->_SetAdvertisingEnabled(val);
}

inline bool BLEManager::IsFastAdvertisingEnabled(void)
{
    return static_cast<ImplClass *>(this)->_IsFastAdvertisingEnabled();
}

inline CHIP_ERROR BLEManager::SetFastAdvertisingEnabled(bool val)
{
    return static_cast<ImplClass *>(this)->_SetFastAdvertisingEnabled(val);
}

inline bool BLEManager::IsAdvertising(void)
{
    return static_cast<ImplClass *>(this)->_IsAdvertising();
}

inline CHIP_ERROR BLEManager::GetDeviceName(char * buf, size_t bufSize)
{
    return static_cast<ImplClass *>(this)->_GetDeviceName(buf, bufSize);
}

inline CHIP_ERROR BLEManager::SetDeviceName(const char * deviceName)
{
    return static_cast<ImplClass *>(this)->_SetDeviceName(deviceName);
}

inline uint16_t BLEManager::NumConnections(void)
{
    return static_cast<ImplClass *>(this)->_NumConnections();
}

inline void BLEManager::OnPlatformEvent(const ChipDeviceEvent * event)
{
    return static_cast<ImplClass *>(this)->_OnPlatformEvent(event);
}

inline chip::Ble::BleLayer * BLEManager::GetBleLayer(void) const
{
    return static_cast<const ImplClass *>(this)->_GetBleLayer();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#endif // BLE_MANAGER_H
