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
 *          for mbed platforms.
 */

#include <inttypes.h>
#include <stdint.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/mbed/BLEManagerImpl.h>

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

// Show BLE status with LEDs
#define _BLEMGRIMPL_USE_LEDS 0

/* Undefine the BLE_ERROR_NOT_IMPLEMENTED macro provided by CHIP's
 * src/ble/BleError.h to avoid a name conflict with Mbed-OS ble_error_t
 * enum value. For the enum values, see:
 * mbed-os/connectivity/FEATURE_BLE/include/ble/common/blecommon.h
 */
#undef BLE_ERROR_NOT_IMPLEMENTED
// mbed-os headers
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"
#include "platform/Span.h"

#if _BLEMGRIMPL_USE_LEDS
#include "drivers/DigitalOut.h"
#endif

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {
const UUID ShortUUID_CHIPoBLEService(0xFEAF);
// RX = BleLayer::CHIP_BLE_CHAR_1_ID
const UUID LongUUID_CHIPoBLEChar_RX("18EE2EF5-263D-4559-959F-4F9C429F9D11");
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
// TX = BleLayer::CHIP_BLE_CHAR_2_ID
const UUID LongUUID_CHIPoBLEChar_TX("18EE2EF5-263D-4559-959F-4F9C429F9D12");
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };
} // namespace

#if _BLEMGRIMPL_USE_LEDS
#include "drivers/DigitalOut.h"
// LED1 -- toggle on every call to ble::BLE::processEvents()
// LED2 -- on when ble::BLE::init() callback completes
// LED3 -- on when advertising
mbed::DigitalOut led1(LED1, 1);
mbed::DigitalOut led2(LED2, 1);
mbed::DigitalOut led3(LED3, 1);
#endif

struct ConnectionInfo
{
    struct ConnStatus
    {
        ble::connection_handle_t connHandle;
        uint16_t attMtuSize;
    };

    ConnectionInfo()
    {
        for (size_t i = 0; i < kMaxConnections; i++)
        {
            mConnStates[i].connHandle = kInvalidHandle;
            mConnStates[i].attMtuSize = 0;
        }
    }

    CHIP_ERROR setStatus(ble::connection_handle_t conn_handle, uint16_t mtu_size)
    {
        size_t new_i = kMaxConnections;
        for (size_t i = 0; i < kMaxConnections; i++)
        {
            if (mConnStates[i].connHandle == conn_handle)
            {
                mConnStates[i].attMtuSize = mtu_size;
                return CHIP_NO_ERROR;
            }
            else if (mConnStates[i].connHandle == kInvalidHandle && i < new_i)
            {
                new_i = i;
            }
        }
        // Handle not found, has to be added.
        if (new_i == kMaxConnections)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mConnStates[new_i].connHandle = conn_handle;
        mConnStates[new_i].attMtuSize = mtu_size;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR clearStatus(ble::connection_handle_t conn_handle)
    {
        for (size_t i = 0; i < kMaxConnections; i++)
        {
            if (mConnStates[i].connHandle == conn_handle)
            {
                mConnStates[i].connHandle = kInvalidHandle;
                mConnStates[i].attMtuSize = 0;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ConnStatus getStatus(ble::connection_handle_t conn_handle) const
    {
        for (size_t i = 0; i < kMaxConnections; i++)
        {
            if (mConnStates[i].connHandle == conn_handle)
            {
                return mConnStates[i];
            }
        }
        return { kInvalidHandle, 0 };
    }

    uint16_t getMTU(ble::connection_handle_t conn_handle) const { return getStatus(conn_handle).attMtuSize; }

private:
    const size_t kMaxConnections = BLE_LAYER_NUM_BLE_ENDPOINTS;
    ConnStatus mConnStates[BLE_LAYER_NUM_BLE_ENDPOINTS];
    const ble::connection_handle_t kInvalidHandle = 0xf00d;
};

static ConnectionInfo sConnectionInfo;

#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class GapEventHandler : private mbed::NonCopyable<GapEventHandler>, public ble::Gap::EventHandler
{
    void onScanRequestReceived(const ble::ScanRequestEvent & event)
    {
        // Requires enable action from setScanRequestNotification(true).
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    /* Called when advertising starts.
     */
    void onAdvertisingStart(const ble::AdvertisingStartEvent & event)
    {
#if _BLEMGRIMPL_USE_LEDS
        led3 = 0;
#endif
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);

        BLEManagerImpl & ble_manager = BLEMgrImpl();
        ble_manager.mFlags.Set(ble_manager.kFlag_Advertising);
        ble_manager.mFlags.Clear(ble_manager.kFlag_AdvertisingRefreshNeeded);

        // Post a CHIPoBLEAdvertisingChange(Started) event.
        ChipDeviceEvent chip_event;
        chip_event.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        chip_event.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
        PlatformMgrImpl().PostEvent(&chip_event);

        PlatformMgr().ScheduleWork(ble_manager.DriveBLEState, 0);
    }

    /* Called when advertising ends.
     *
     * Advertising ends when the process timeout or if it is stopped by the
     * application or if the local device accepts a connection request.
     */
    void onAdvertisingEnd(const ble::AdvertisingEndEvent & event)
    {
#if _BLEMGRIMPL_USE_LEDS
        led3 = 1;
#endif
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);

        BLEManagerImpl & ble_manager = BLEMgrImpl();
        ble_manager.mFlags.Set(ble_manager.kFlag_Advertising);

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        ChipDeviceEvent chip_event;
        chip_event.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        chip_event.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
        PlatformMgrImpl().PostEvent(&chip_event);

        if (event.isConnected())
        {
            ble_manager.mFlags.Set(ble_manager.kFlag_AdvertisingRefreshNeeded);
            ChipLogDetail(DeviceLayer, "Restarting advertising to allow more connections.");
        }
        PlatformMgr().ScheduleWork(ble_manager.DriveBLEState, 0);
    }

    /* Called when connection attempt ends or an advertising device has been
     * connected.
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent & event)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);

        ble_error_t mbed_err         = event.getStatus();
        BLEManagerImpl & ble_manager = BLEMgrImpl();

        if (mbed_err == BLE_ERROR_NONE)
        {
            const ble::address_t & peer_addr = event.getPeerAddress();
            ChipLogProgress(DeviceLayer, "BLE connection established with %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX", peer_addr[5],
                            peer_addr[4], peer_addr[3], peer_addr[2], peer_addr[1], peer_addr[0]);
            ble_manager.mGAPConns++;
            CHIP_ERROR err = sConnectionInfo.setStatus(event.getConnectionHandle(), BLE_GATT_MTU_SIZE_DEFAULT);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Unable to store connection status, error: %s ", ErrorStr(err));
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "BLE connection failed, mbed-os error: %d", mbed_err);
        }
        ChipLogProgress(DeviceLayer, "Current number of connections: %" PRIu16 "/%d", ble_manager.NumConnections(),
                        ble_manager.kMaxConnections);

        // The connection established event is propagated when the client has subscribed to
        // the TX characteristic.
    }

    void onUpdateConnectionParametersRequest(const ble::UpdateConnectionParametersRequestEvent & event)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    void onConnectionParametersUpdateComplete(const ble::ConnectionParametersUpdateCompleteEvent & event)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    void onReadPhy(ble_error_t status, ble::connection_handle_t connectionHandle, ble::phy_t txPhy, ble::phy_t rxPhy)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    void onPhyUpdateComplete(ble_error_t status, ble::connection_handle_t connectionHandle, ble::phy_t txPhy, ble::phy_t rxPhy)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    /* Called when a connection has been disconnected.
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent & event)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);

        const ble::disconnection_reason_t & reason = event.getReason();
        BLEManagerImpl & ble_manager               = BLEMgrImpl();

        if (ble_manager.NumConnections())
        {
            ble_manager.mGAPConns--;
        }
        CHIP_ERROR err = sConnectionInfo.clearStatus(event.getConnectionHandle());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to clear connection status, error: %s ", ErrorStr(err));
        }

        ChipDeviceEvent chip_event;
        chip_event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        chip_event.CHIPoBLEConnectionError.ConId = event.getConnectionHandle();
        switch (reason.value())
        {
        case ble::disconnection_reason_t::REMOTE_USER_TERMINATED_CONNECTION:
            chip_event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case ble::disconnection_reason_t::LOCAL_HOST_TERMINATED_CONNECTION:
            chip_event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            chip_event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        PlatformMgrImpl().PostEvent(&chip_event);

        ChipLogProgress(DeviceLayer, "BLE connection terminated, mbed-os reason: %d", reason.value());
        ChipLogProgress(DeviceLayer, "Current number of connections: %" PRIu16 "/%d", ble_manager.NumConnections(),
                        ble_manager.kMaxConnections);

        // Force a reconfiguration of advertising in case we switched to non-connectable mode when
        // the BLE connection was established.
        ble_manager.mFlags.Set(ble_manager.kFlag_AdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(ble_manager.DriveBLEState, 0);
    }

    void onDataLengthChange(ble::connection_handle_t connectionHandle, uint16_t txSize, uint16_t rxSize)
    {
        ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__);
    }

    void onPrivacyEnabled() { ChipLogDetail(DeviceLayer, "GAP %s", __FUNCTION__); }
};

#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
struct CHIPService : public ble::GattServer::EventHandler
{
    CHIPService() {}
    CHIPService(const CHIPService &) = delete;
    CHIPService & operator=(const CHIPService &) = delete;

    CHIP_ERROR init(ble::BLE & ble_interface)
    {
        ChipLogDetail(DeviceLayer, "GATT %s", __FUNCTION__);

        if (mCHIPoBLEChar_RX != nullptr || mCHIPoBLEChar_TX != nullptr)
        {
            return CHIP_NO_ERROR;
        }

        mCHIPoBLEChar_RX = new GattCharacteristic(LongUUID_CHIPoBLEChar_RX, nullptr, 0, BLE_GATT_MTU_SIZE_DEFAULT,
                                                  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);

        mCHIPoBLEChar_TX = new GattCharacteristic(LongUUID_CHIPoBLEChar_TX, nullptr, 0, BLE_GATT_MTU_SIZE_DEFAULT,
                                                  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);

        // Setup callback
        mCHIPoBLEChar_RX->setWriteAuthorizationCallback(this, &CHIPService::onWriteAuth);

        GattCharacteristic * chipoble_gatt_characteristics[] = { mCHIPoBLEChar_RX, mCHIPoBLEChar_TX };
        auto num_characteristics = sizeof chipoble_gatt_characteristics / sizeof chipoble_gatt_characteristics[0];
        GattService chipoble_gatt_service(ShortUUID_CHIPoBLEService, chipoble_gatt_characteristics, num_characteristics);

        auto mbed_err = ble_interface.gattServer().addService(chipoble_gatt_service);
        if (mbed_err != BLE_ERROR_NONE)
        {
            ChipLogError(DeviceLayer, "Unable to add GATT service, mbed-os err: %d", mbed_err);
            return CHIP_ERROR_INTERNAL;
        }

        // Store the attribute handles in the class so they are reused in
        // callbacks to discriminate events.
        mRxHandle = mCHIPoBLEChar_RX->getValueHandle();
        mTxHandle = mCHIPoBLEChar_TX->getValueHandle();
        // There is a single descriptor in the characteristic, CCCD is at index 0
        mTxCCCDHandle = mCHIPoBLEChar_TX->getDescriptor(0)->getHandle();
        ChipLogDetail(DeviceLayer, "char handles: rx=%d, tx=%d, cccd=%d", mRxHandle, mTxHandle, mTxCCCDHandle);

        ble_interface.gattServer().setEventHandler(this);
        return CHIP_NO_ERROR;
    }

    // Write authorization callback
    void onWriteAuth(GattWriteAuthCallbackParams * params)
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params->connHandle, params->handle);
        if (params->handle == mRxHandle)
        {
            ChipLogDetail(DeviceLayer, "Received BLE packet on RX");

            // Allocate a buffer, copy the data. They will be passed into the event
            auto buf = System::PacketBufferHandle::NewWithData(params->data, params->len);
            if (buf.IsNull())
            {
                params->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_REQUEST_REJECTED;
                ChipLogError(DeviceLayer, "Dropping packet, not enough memory");
                return;
            }

            params->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;

            ChipDeviceEvent chip_event;
            chip_event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
            chip_event.CHIPoBLEWriteReceived.ConId = params->connHandle;
            chip_event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
            PlatformMgrImpl().PostEvent(&chip_event);
        }
        else
        {
            params->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_HANDLE;
        }
    }

    // overrides of GattServerEvent Handler
    void onAttMtuChange(ble::connection_handle_t connectionHandle, uint16_t attMtuSize) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s", __FUNCTION__);
        CHIP_ERROR err = sConnectionInfo.setStatus(connectionHandle, attMtuSize);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Unable to store connection status, error: %s ", ErrorStr(err));
        }
    }

    void onDataSent(const GattDataSentCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.attHandle);

        // FIXME: ACK hack
#if (defined(MBED_CONF_APP_USE_GATT_INDICATION_ACK_HACK) && (MBED_CONF_APP_USE_GATT_INDICATION_ACK_HACK != 0))
        onConfirmationReceived(params);
#endif
    }

    void onDataWritten(const GattWriteCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.handle);
    }

    void onDataRead(const GattReadCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.handle);
    }

    void onShutdown(const ble::GattServer & server) override { ChipLogDetail(DeviceLayer, "GATT %s", __FUNCTION__); }

    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.attHandle);
        if (params.attHandle == mTxCCCDHandle)
        {
            ChipLogDetail(DeviceLayer, "Updates enabled on TX CCCD");
            ChipDeviceEvent chip_event;
            chip_event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
            chip_event.CHIPoBLESubscribe.ConId = params.connHandle;
            PlatformMgrImpl().PostEvent(&chip_event);
        }
    }

    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.attHandle);
        if (params.attHandle == mTxCCCDHandle)
        {
            ChipLogDetail(DeviceLayer, "Updates disabled on TX CCCD");
            ChipDeviceEvent chip_event;
            chip_event.Type                      = DeviceEventType::kCHIPoBLEUnsubscribe;
            chip_event.CHIPoBLEUnsubscribe.ConId = params.connHandle;
            PlatformMgrImpl().PostEvent(&chip_event);
        }
    }

    void onConfirmationReceived(const GattConfirmationReceivedCallbackParams & params) override
    {
        ChipLogDetail(DeviceLayer, "GATT %s, connHandle=%d, attHandle=%d", __FUNCTION__, params.connHandle, params.attHandle);
        if (params.attHandle == mTxHandle)
        {
            ChipLogDetail(DeviceLayer, "Confirmation received for TX transfer");
            ChipDeviceEvent chip_event;
            chip_event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
            chip_event.CHIPoBLEIndicateConfirm.ConId = params.connHandle;
            PlatformMgrImpl().PostEvent(&chip_event);
        }
    }

    ble::attribute_handle_t getTxHandle() const { return mTxHandle; }
    ble::attribute_handle_t getTxCCCDHandle() const { return mTxCCCDHandle; }
    ble::attribute_handle_t getRxHandle() const { return mRxHandle; }

    GattCharacteristic * mCHIPoBLEChar_RX = nullptr;
    GattCharacteristic * mCHIPoBLEChar_TX = nullptr;
    ble::attribute_handle_t mRxHandle     = 0;
    ble::attribute_handle_t mTxCCCDHandle = 0;
    ble::attribute_handle_t mTxHandle     = 0;
};

#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class SecurityManagerEventHandler : private mbed::NonCopyable<SecurityManagerEventHandler>,
                                    public ble::SecurityManager::EventHandler
{
    void pairingRequest(ble::connection_handle_t connectionHandle) override
    {
        ChipLogDetail(DeviceLayer, "SM %s, connHandle=%d", __FUNCTION__, connectionHandle);
        ble::SecurityManager & security_mgr = ble::BLE::Instance().securityManager();

        auto mbed_err = security_mgr.acceptPairingRequest(connectionHandle);
        if (mbed_err == BLE_ERROR_NONE)
        {
            ChipLogProgress(DeviceLayer, "Pairing request authorized.");
        }
        else
        {
            ChipLogError(DeviceLayer, "Pairing request not authorized, mbed-os err: %d", mbed_err);
        }
    }

    void pairingResult(ble::connection_handle_t connectionHandle, SecurityManager::SecurityCompletionStatus_t result) override
    {
        ChipLogDetail(DeviceLayer, "SM %s, connHandle=%d", __FUNCTION__, connectionHandle);
        if (result == SecurityManager::SEC_STATUS_SUCCESS)
        {
            ChipLogProgress(DeviceLayer, "Pairing successful.");
        }
        else
        {
            ChipLogError(DeviceLayer, "Pairing failed, status: 0x%X.", result);
        }
    }

    void linkEncryptionResult(ble::connection_handle_t connectionHandle, ble::link_encryption_t result) override
    {
        ChipLogDetail(DeviceLayer, "SM %s, connHandle=%d", __FUNCTION__, connectionHandle);
        if (result == ble::link_encryption_t::NOT_ENCRYPTED)
        {
            ChipLogDetail(DeviceLayer, "Link NOT_ENCRYPTED.");
        }
        else if (result == ble::link_encryption_t::ENCRYPTION_IN_PROGRESS)
        {
            ChipLogDetail(DeviceLayer, "Link ENCRYPTION_IN_PROGRESS.");
        }
        else if (result == ble::link_encryption_t::ENCRYPTED)
        {
            ChipLogDetail(DeviceLayer, "Link ENCRYPTED.");
        }
        else if (result == ble::link_encryption_t::ENCRYPTED_WITH_MITM)
        {
            ChipLogDetail(DeviceLayer, "Link ENCRYPTED_WITH_MITM.");
        }
        else if (result == ble::link_encryption_t::ENCRYPTED_WITH_SC_AND_MITM)
        {
            ChipLogDetail(DeviceLayer, "Link ENCRYPTED_WITH_SC_AND_MITM.");
        }
        else
        {
            ChipLogDetail(DeviceLayer, "Link encryption status UNKNOWN.");
        }
    }
};

BLEManagerImpl BLEManagerImpl::sInstance;
static GapEventHandler sMbedGapEventHandler;
static CHIPService sCHIPService;
static SecurityManagerEventHandler sSecurityManagerEventHandler;

/* Initialize the mbed-os BLE subsystem. Register the BLE event processing
 * callback to the system event queue. Register the BLE initialization complete
 * callback that handles the rest of the setup commands. Register the BLE GAP
 * event handler.
 */
CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    ble_error_t mbed_err = BLE_ERROR_NONE;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = BitFlags<Flags>(CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? kFlag_AdvertisingEnabled : 0);
    mGAPConns    = 0;

    ble::BLE & ble_interface = ble::BLE::Instance();

    ble_interface.gap().setEventHandler(&sMbedGapEventHandler);
    err = sCHIPService.init(ble_interface);
    SuccessOrExit(err);

    ble_interface.onEventsToProcess(FunctionPointerWithContext<ble::BLE::OnEventsToProcessCallbackContext *>{
        [](ble::BLE::OnEventsToProcessCallbackContext * context) { PlatformMgr().ScheduleWork(DoBLEProcessing, 0); } });

    mbed_err = ble_interface.init([](ble::BLE::InitializationCompleteCallbackContext * context) {
        BLEMgrImpl().HandleInitComplete(context->error == BLE_ERROR_NONE);
    });
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

exit:
    return err;
}

/* Process all the events from the mbed-os BLE subsystem.
 */
void BLEManagerImpl::DoBLEProcessing(intptr_t arg)
{
#if _BLEMGRIMPL_USE_LEDS
    led1 = !led1;
#endif
    ble::BLE::Instance().processEvents();
}

/* This is the mbed-os BLE subsystem init complete callback. Initialize the
 * BLELayer and update the state based on the flags.
 */
void BLEManagerImpl::HandleInitComplete(bool no_error)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    ble_error_t mbed_err = BLE_ERROR_NONE;

    ble::Gap & gap                      = ble::BLE::Instance().gap();
    ble::SecurityManager & security_mgr = ble::BLE::Instance().securityManager();
    ble::own_address_type_t addr_type;
    ble::address_t addr;

    VerifyOrExit(no_error, err = CHIP_ERROR_INTERNAL);

    gap.getAddress(addr_type, addr);
    ChipLogDetail(DeviceLayer, "Device address: %02X:%02X:%02X:%02X:%02X:%02X", addr[5], addr[4], addr[3], addr[2], addr[1],
                  addr[0]);

    mbed_err = security_mgr.init(
        /*bool enableBonding             */ false,
        /*bool requireMITM               */ true,
        /*SecurityIOCapabilities_t iocaps*/ SecurityManager::IO_CAPS_NONE,
        /*const Passkey_t passkey        */ nullptr,
        /*bool signing                   */ true,
        /*const char *dbFilepath         */ nullptr);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    mbed_err = security_mgr.setPairingRequestAuthorisation(true);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);
    security_mgr.setSecurityManagerEventHandler(&sSecurityManagerEventHandler);

    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
#if _BLEMGRIMPL_USE_LEDS
    led2 = 0;
#endif

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManager init error: %s ", ErrorStr(err));
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service.");
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    BLEMgrImpl().DriveBLEState();
}

/* Update the advertising state based on the flags.
 */
void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the CHIP task is running.
    if (!mFlags.Has(kFlag_AsyncInitCompleted))
    {
        mFlags.Set(kFlag_AsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            mFlags.Clear(kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        mFlags.Has(kFlag_AdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (_NumConnections() == 0)
#endif
    )
    {
        // Start/re-start advertising if not already advertising, or if the
        // advertising state needs to be refreshed.
        if (!mFlags.Has(kFlag_Advertising) || mFlags.Has(kFlag_AdvertisingRefreshNeeded))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }
    // Otherwise, stop advertising if currently active.
    else
    {
        err = StopAdvertising();
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFlag_FastAdvertisingEnabled, true);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Set(Flags::kFlag_AdvertisingEnabled, false);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mFlags.Set(Flags::kFlag_AdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

/* Build the advertising data and start advertising.
 */
CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    ble_error_t mbed_err = BLE_ERROR_NONE;

    ble::Gap & gap = ble::BLE::Instance().gap();
    ble::AdvertisingDataBuilder adv_data_builder(mAdvertisingDataBuffer);

    ChipBLEDeviceIdentificationInfo dev_id_info;

    // Advertise CONNECTABLE if we haven't reached the maximum number of connections.
    uint16_t num_conns = _NumConnections();
    bool connectable   = (num_conns < kMaxConnections);
    ble::advertising_type_t adv_type =
        connectable ? ble::advertising_type_t::CONNECTABLE_UNDIRECTED : ble::advertising_type_t::SCANNABLE_UNDIRECTED;

    // Advertise in fast mode if not fully provisioned and there are no CHIPoBLE connections, or
    // if the application has expressly requested fast advertising.
    ble::adv_interval_t adv_interval = (num_conns == 0 && !ConfigurationMgr().IsFullyProvisioned())
        ? ble::adv_interval_t(CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX)
        : ble::adv_interval_t(CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX);
    // minInterval and maxInterval are equal for CHIP.
    ble::AdvertisingParameters adv_params(adv_type, adv_interval, adv_interval);

    // Restart advertising if already active.
    if (gap.isAdvertisingActive(ble::LEGACY_ADVERTISING_HANDLE))
    {
        mbed_err = gap.stopAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);
        ChipLogDetail(DeviceLayer, "Advertising already active. Restarting.");
    }

    mbed_err = gap.setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_params);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    mbed_err =
        adv_data_builder.setFlags(ble::adv_data_flags_t::BREDR_NOT_SUPPORTED | ble::adv_data_flags_t::LE_GENERAL_DISCOVERABLE);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    if (!mFlags.Has(kFlag_UseCustomDeviceName))
    {
        uint16_t discriminator;
        SuccessOrExit(err = ConfigurationMgr().GetSetupDiscriminator(discriminator));
        memset(mDeviceName, 0, kMaxDeviceNameLength);
        snprintf(mDeviceName, kMaxDeviceNameLength, "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
    }
    mbed_err = adv_data_builder.setName(mDeviceName);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    dev_id_info.Init();
    SuccessOrExit(ConfigurationMgr().GetBLEDeviceIdentificationInfo(dev_id_info));
    mbed_err = adv_data_builder.setServiceData(
        ShortUUID_CHIPoBLEService, mbed::make_Span<const uint8_t>(reinterpret_cast<uint8_t *>(&dev_id_info), sizeof dev_id_info));
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    mbed_err = gap.setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE, adv_data_builder.getAdvertisingData());
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    adv_data_builder.clear();
    adv_data_builder.setLocalServiceList(mbed::make_Span<const UUID>(&ShortUUID_CHIPoBLEService, 1));
    mbed_err = gap.setAdvertisingScanResponse(ble::LEGACY_ADVERTISING_HANDLE, adv_data_builder.getAdvertisingData());
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    mbed_err = gap.startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

    ChipLogDetail(DeviceLayer, "Advertising started, type: 0x%x (%sconnectable), interval: [%d:%d] ms, device name: %s)",
                  adv_params.getType(), connectable ? "" : "non-", adv_params.getMinPrimaryInterval().valueInMs(),
                  adv_params.getMaxPrimaryInterval().valueInMs(), mDeviceName);

exit:
    if (mbed_err != BLE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "StartAdvertising mbed-os error: %d", mbed_err);
    }
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    ble_error_t mbed_err = BLE_ERROR_NONE;

    ble::Gap & gap = ble::BLE::Instance().gap();

    if (!gap.isAdvertisingActive(ble::LEGACY_ADVERTISING_HANDLE))
    {
        ChipLogDetail(DeviceLayer, "No need to stop. Advertising inactive.");
        return err;
    }
    mbed_err = gap.stopAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

exit:
    if (mbed_err != BLE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "StopAdvertising mbed-os error: %d", mbed_err);
    }
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(kFlag_AdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "SetAdvertisingEnabled(%s)", val ? "true" : "false");

        mFlags.Set(kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(kFlag_FastAdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "SetFastAdvertisingEnabled(%s)", val ? "true" : "false");

        mFlags.Set(kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(strlen(mDeviceName) < bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    strcpy(buf, mDeviceName);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != nullptr && deviceName[0] != '\0')
    {
        VerifyOrExit(strlen(deviceName) < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
        strcpy(mDeviceName, deviceName);
        mFlags.Set(kFlag_UseCustomDeviceName);
        ChipLogDetail(DeviceLayer, "Device name set to: %s", deviceName);
    }
    else
    {
        mDeviceName[0] = '\0';
        mFlags.Clear(kFlag_UseCustomDeviceName);
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return mGAPConns;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;

        ChipLogDetail(DeviceLayer, "_OnPlatformEvent kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgrImpl().PostEvent(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogDetail(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogDetail(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        ChipLogDetail(DeviceLayer, "_OnPlatformEvent kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        ChipLogDetail(DeviceLayer, "_OnPlatformEvent kCHIPoBLEIndicateConfirm");
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    default:
        ChipLogDetail(DeviceLayer, "_OnPlatformEvent default:  event->Type = 0x%x", event->Type);
        break;
    }
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // no-op
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection, connHandle=%d", conId);

    ble::Gap & gap = ble::BLE::Instance().gap();

    ble_error_t mbed_err = gap.disconnect(conId, ble::local_disconnection_reason_t::USER_TERMINATION);
    return mbed_err == BLE_ERROR_NONE;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return sConnectionInfo.getMTU(conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle pBuf)
{
    ChipLogDetail(DeviceLayer, "BlePlatformDelegate %s", __FUNCTION__);

    CHIP_ERROR err       = CHIP_NO_ERROR;
    ble_error_t mbed_err = BLE_ERROR_NONE;

    ble::GattServer & gatt_server = ble::BLE::Instance().gattServer();
    ble::attribute_handle_t att_handle;

    // No need to do anything fancy here. Only 3 handles are used in this impl.
    if (UUIDsMatch(charId, &ChipUUID_CHIPoBLEChar_TX))
    {
        att_handle = sCHIPService.getTxHandle();
    }
    else if (UUIDsMatch(charId, &ChipUUID_CHIPoBLEChar_RX))
    {
        // TODO does this make sense?
        att_handle = sCHIPService.getRxHandle();
    }
    else
    {
        // TODO handle error with chipConnection::SendMessage as described
        // in the BlePlatformDelegate.h.
        ChipLogError(DeviceLayer, "Send indication failed, invalid charID.");
        return false;
    }

    ChipLogDetail(DeviceLayer,
                  "Sending indication for CHIPoBLE characteristic "
                  "(connHandle=%d, attHandle=%d, data_len=%" PRIu16 ")",
                  conId, att_handle, pBuf->DataLength());

    mbed_err = gatt_server.write(att_handle, pBuf->Start(), pBuf->DataLength(), false);
    VerifyOrExit(mbed_err == BLE_ERROR_NONE, err = CHIP_ERROR_INTERNAL);

exit:
    if (mbed_err != BLE_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Send indication failed, mbed-os error: %d", mbed_err);
    }
    return err == CHIP_NO_ERROR;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
