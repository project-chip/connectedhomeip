/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for the Qorvo QPG platforms.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "FreeRTOS.h"
#include "qvCHIP.h"
#include "timers.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the platform.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown() {}
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBufferHandle pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBufferHandle pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBufferHandle pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum class Flags : uint16_t
    {
        kAsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kAdvertisingEnabled       = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled   = 0x0004, /**< The application has enabled fast advertising. */
        kAdvertising              = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kAdvertisingRefreshNeeded = 0x0010, /**< The advertising state/configuration state in the BLE layer needs to be updated. */
        kDeviceNameSet            = 0x0020, /**< The device name has been set. */
        kRestartAdvertising = 0x0040, /**< The advertising will be restarted when stop advertising confirmation is received and this
                                            flag is set*/
        kEnablingAdvertising = 0x0080, /**< The BLE controller is setting up Advertisements. */
    };

    enum
    {
        kMaxConnections              = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength         = 20, // TODO: right-size this
        kMaxAdvertisementDataSetSize = 31
    };

    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    uint16_t mNumGAPCons;
    uint16_t mSubscribedConIds[kMaxConnections];
    qvCHIP_Ble_Callbacks_t appCbacks;

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertisingData(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);

    void HandleDmMsg(qvCHIP_Ble_DmEvt_t * pDmEvt);
    void HandleAttMsg(qvCHIP_Ble_AttEvt_t * pAttEvt);

    CHIP_ERROR MapBLEError(int bleErr) const;
    /* Callbacks from BLE stack*/
    static void ExternalCbHandler(qvCHIP_Ble_MsgHdr_t * pMsg);
    static void HandleTXCharRead(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset, qvCHIP_Ble_Attr_t * pAttr) {}
    static void HandleRXCharWrite(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset, uint16_t len,
                                  uint8_t * pValue, qvCHIP_Ble_Attr_t * pAttr);
    void HandleTXCharCCCDWrite(qvCHIP_Ble_AttsCccEvt_t * event);

    static void DriveBLEState(intptr_t arg);

    /* Handlers for stack events */
    static void _handleTXCharCCCDWrite(qvCHIP_Ble_AttsCccEvt_t * event);
    static void BleAdvTimeoutHandler(TimerHandle_t xTimer);
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR PrepareAdditionalData();
#endif /* CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING */
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr(void)
{
    return BLEManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the platform.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

inline bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kAdvertising);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
