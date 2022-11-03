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

/**
 *    @file
 *          Provides an implementation of the BLEManager object
 *          for Telink platform.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

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
    // Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown();
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void);

    // Members that implement virtual methods on BlePlatformDelegate.

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

    // Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // Private members reserved for use by this class only.

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
    };

    enum
    {
        kMaxConnections              = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength         = 16,
        kMaxAdvertisementDataSetSize = 31,
        kMaxRxDataBuffSize           = 20,
        kMaxTxDataBuffSize           = 20
    };

    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    uint16_t mGAPConns;
    bool mSubscribedConns[kMaxConnections];
    uint8_t mAdvDataBuf[kMaxAdvertisementDataSetSize];
    uint8_t mScanRespDataBuf[kMaxAdvertisementDataSetSize];
    uint8_t mRxDataBuff[kMaxRxDataBuffSize];
    uint8_t mTxDataBuff[kMaxRxDataBuffSize];
    bool ThreadConnectivityReady;

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertisingData(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);

    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleDisconnectRequest(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharComplete(const ChipDeviceEvent * event);
    CHIP_ERROR HandleBleConnectionClosed(const ChipDeviceEvent * event);

    /*
        @todo WORKAROUND: Due to abscense of non-cuncurrent mode in Matter
        we are emulating connection to Thread with this events and manually
        disconnect BLE ass soon as OperationalNetworkEnabled occures.
        This functionality shall be removed as soon as non-cuncurrent mode
        would be implemented
     */
    CHIP_ERROR HandleThreadStateChange(const ChipDeviceEvent * event);
    CHIP_ERROR HandleOperationalNetworkEnabled(const ChipDeviceEvent * event);

    /* Callbacks from BLE stack*/
    static void DriveBLEState(intptr_t arg);

    static void HandleBLEAdvertisementIntervalChange(System::Layer * layer, void * param);

    /* Handlers for stack events */
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);

    /* Other init functions */
    CHIP_ERROR _InitStack(void);
    void _InitGatt(void);
    CHIP_ERROR _InitGap(void);

public:
    static int RxWriteCallback(uint16_t connHandle, void * p);
    static int TxCccWriteCallback(uint16_t connHandle, void * p);
    static void ConnectCallback(uint8_t event, uint8_t * data, int len);
    static void DisconnectCallback(uint8_t event, uint8_t * data, int len);
    static int GapEventHandler(uint32_t event, uint8_t * data, int size);

    /* Switch to IEEE802154 interface. @todo: remove to other module? */
    void SwitchToIeee802154(void);

    /* BLE thread entry */
    static void BleEntry(void *, void *, void *);
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
