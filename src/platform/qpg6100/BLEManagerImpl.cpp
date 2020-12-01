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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the QPG6100 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

const uint16_t UUID16_CHIPoBLEService = 0xFEAF;

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? kFlag_AdvertisingEnabled : 0;
    mNumGAPCons  = 0;
    for (int i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    appCbacks.stackCback    = ExternalCbHandler;
    appCbacks.chrReadCback  = HandleTXCharRead;
    appCbacks.chrWriteCback = HandleRXCharWrite;
    appCbacks.cccCback      = HandleTXCharCCCDWrite;
    qvCHIP_BleInit(&appCbacks);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);
exit:
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::Init() complete");

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

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (GetFlag(mFlags, kFlag_FastAdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    CHIP_ERROR err;

    err = qvCHIP_BleGetDeviceName(buf, bufSize);

    return err;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (devName != nullptr && devName[0] != 0)
    {
        err = qvCHIP_BleSetDeviceName(devName);
    }

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    // Platform specific events
    case DeviceEventType::kCHIPoBLETXCharWriteEvent:
        HandleTXComplete(event);
        break;

    // Generic CHIP events
    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:

        // If CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable CHIPoBLE advertising if the device
        // is now fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising state to be refreshed to reflect new provisioning state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

        DriveBLEState();

        break;

    default:
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent default:  event->Type = %d", event->Type);
        break;
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    err = qvCHIP_BleCloseConnection(conId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "qvCHIP_BleCloseConnection() failed: %s", ErrorStr(err));
    }

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return qvCHIP_BleGetMTU(conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    bool isRxHandle;
    uint16_t cId;
    uint16_t dataLen = data->DataLength();

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogDetail(DeviceLayer, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, dataLen);

    isRxHandle = UUIDsMatch(&chipUUID_CHIPoBLEChar_RX, charId);
    cId        = qvCHIP_BleGetHandle(isRxHandle);

    qvCHIP_TxData(conId, cId, dataLen, data->Start());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Nothing to do
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the CHIP task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        GetFlag(mFlags, kFlag_AdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (mNumGAPCons == 0)
#endif
    )
    {
        // Start/re-start BLE advertising if not already advertising, or if the
        // advertising state of the underlying stack needs to be refreshed.
        if (!GetFlag(mFlags, kFlag_Advertising) || GetFlag(mFlags, kFlag_AdvertisingRefreshNeeded))
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

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "CHIPoBLE start advertising");

    return qvCHIP_BleStartAdvertising();
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err;

    err = qvCHIP_BleStopAdvertising();
    SuccessOrExit(err);

    // Transition to the not Advertising state...
    if (GetFlag(mFlags, kFlag_Advertising))
    {
        ClearFlag(mFlags, kFlag_Advertising);

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

        // Directly inform the ThreadStackManager that CHIPoBLE advertising has stopped.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ThreadStackMgr().OnCHIPoBLEAdvertisingStop();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            PlatformMgr().PostEvent(&advChange);
        }
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] != BLE_CONNECTION_UNINITIALIZED)
        {
            numCons++;
        }
    }
    return numCons;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::HandleTXCharRead(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset,
                                      qvCHIP_Ble_Attr_t * pAttr)
{
    uint8_t rsp = 0;

    ChipLogProgress(DeviceLayer, "Read request received for CHIPoBLE TX characteristic (con %u)", connId);

    // Send a zero-length response.
    qvCHIP_TxData(connId, handle, 0, &rsp);
}

void BLEManagerImpl::HandleRXCharWrite(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset, uint16_t len,
                                       uint8_t * pValue, qvCHIP_Ble_Attr_t * pAttr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE RX characteristic (con %u, len %u)", connId, len);

    // Copy the data to a PacketBuffer.
    PacketBufferHandle buf = PacketBuffer::New(0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), pValue, len);
    buf->SetDataLength(len);
    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = connId;
        event.CHIPoBLEWriteReceived.Data  = buf.Release_ForNow();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(qvCHIP_Ble_AttsCccEvt_t * pEvt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool indicationsEnabled;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE TX characteristic CCCD (con %u, len %u)", pEvt->hdr.param, 0);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = (pEvt->value != 0);

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = (indicationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = pEvt->hdr.param;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXComplete(const ChipDeviceEvent * event)
{
    // TODO -
}

/* Process DM Messages */
void BLEManagerImpl::HandleDmMsg(qvCHIP_Ble_DmEvt_t * pDmEvt)
{
    switch (pDmEvt->hdr.event)
    {
    case QVCHIP_DM_RESET_CMPL_IND: {
        break;
    }
    case QVCHIP_DM_SCAN_START_IND: {
        break;
    }
    case QVCHIP_DM_SCAN_STOP_IND: {
        break;
    }
    case QVCHIP_DM_ADV_START_IND: {
        if (pDmEvt->hdr.status != QVCHIP_HCI_SUCCESS)
        {
            ChipLogError(DeviceLayer, "QVCHIP_DM_ADV_START_IND error: %d", (int) pDmEvt->advSetStart.hdr.status);
            ExitNow();
        }

        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        // Transition to the Advertising state...
        if (!GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

            SetFlag(sInstance.mFlags, kFlag_Advertising, true);

            // Post a CHIPoBLEAdvertisingChange(Started) event.
            {
                ChipDeviceEvent advChange;
                advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
                PlatformMgr().PostEvent(&advChange);
            }
        }
        break;
    }
    case QVCHIP_DM_ADV_STOP_IND: {
        if (pDmEvt->advSetStop.status != QVCHIP_HCI_SUCCESS)
        {
            ChipLogError(DeviceLayer, "QVCHIP_DM_ADV_STOP_IND error: %d", (int) pDmEvt->advSetStop.status);
            ExitNow();
        }

        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            ClearFlag(sInstance.mFlags, kFlag_Advertising);

            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

            // Directly inform the ThreadStackManager that CHIPoBLE advertising has stopped.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
            ThreadStackMgr().OnCHIPoBLEAdvertisingStop();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

            // Post a CHIPoBLEAdvertisingChange(Stopped) event.
            {
                ChipDeviceEvent advChange;
                advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
                PlatformMgr().PostEvent(&advChange);
            }
        }
        break;
    }
    case QVCHIP_DM_CONN_OPEN_IND: {
        ChipLogProgress(DeviceLayer, "BLE GATT connection established (con %u)", pDmEvt->connOpen.hdr.param);

        // Allocate a connection state record for the new connection.
        mNumGAPCons++;

        // Receiving a connection stops the advertising processes.  So force a refresh of the advertising
        // state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    case QVCHIP_DM_CONN_CLOSE_IND: {
        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", pDmEvt->connClose.hdr.param,
                        pDmEvt->connClose.reason);

        // If this was a CHIPoBLE connection, release the associated connection state record
        // and post an event to deliver a connection error to the CHIPoBLE layer.
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = pDmEvt->connClose.hdr.param;
        switch (pDmEvt->connClose.reason)
        {
        case QVCHIP_HCI_ERR_REMOTE_TERMINATED:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case QVCHIP_HCI_ERR_LOCAL_TERMINATED:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        PlatformMgr().PostEvent(&event);

        // Force a refresh of the advertising state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    case QVCHIP_DM_CONN_UPDATE_IND: {
        break;
    }
    default: {
        break;
    }
    }

exit:
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

/* Process ATT Messages */
void BLEManagerImpl::HandleAttMsg(qvCHIP_Ble_AttEvt_t * pAttEvt)
{
    switch (pAttEvt->hdr.event)
    {
    case QVCHIP_ATT_MTU_UPDATE_IND: {
        break;
    }
    case QVCHIP_ATTC_FIND_BY_TYPE_VALUE_RSP:
    case QVCHIP_ATTC_READ_BY_GROUP_TYPE_RSP:
    case QVCHIP_ATTC_READ_BY_TYPE_RSP:
    case QVCHIP_ATTC_FIND_INFO_RSP:
        break;
    case QVCHIP_ATTC_READ_RSP:
        break;
    case QVCHIP_ATTC_WRITE_RSP:
    case QVCHIP_ATTC_WRITE_CMD_RSP:
    case QVCHIP_ATTC_PREPARE_WRITE_RSP:
        break;
    case QVCHIP_ATTC_EXECUTE_WRITE_RSP:
        break;
    case QVCHIP_ATTC_HANDLE_VALUE_NTF:
    case QVCHIP_ATTC_HANDLE_VALUE_IND:
        break;
    default: {
        break;
    }
    }
}

void BLEManagerImpl::ExternalCbHandler(qvCHIP_Ble_MsgHdr_t * pMsg)
{
    if (pMsg != nullptr)
    {
        PlatformMgr().LockChipStack();

        /* Process advertising/scanning and connection-related messages */
        if (pMsg->event >= QVCHIP_DM_CBACK_START && pMsg->event <= QVCHIP_DM_CBACK_END)
        {
            ChipLogProgress(DeviceLayer, "DM event %d: status %d", pMsg->event, pMsg->status);
            sInstance.HandleDmMsg((qvCHIP_Ble_DmEvt_t *) pMsg);
        }
        /* Process attribute-related messages */
        else if (pMsg->event >= QVCHIP_ATT_CBACK_START && pMsg->event <= QVCHIP_ATT_CBACK_END)
        {
            ChipLogProgress(DeviceLayer, "ATT event %d: status %d", pMsg->event, pMsg->status);
            sInstance.HandleAttMsg((qvCHIP_Ble_AttEvt_t *) pMsg);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "Unknown event %d", pMsg->event);
        }

        PlatformMgr().UnlockChipStack();
    }
}

CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    uint16_t freeIndex = kMaxConnections;

    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            return CHIP_NO_ERROR;
        }
        else if (mSubscribedConIds[i] == BLE_CONNECTION_UNINITIALIZED && i < freeIndex)
        {
            freeIndex = i;
        }
    }

    if (freeIndex < kMaxConnections)
    {
        mSubscribedConIds[freeIndex] = conId;
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_NO_MEMORY;
    }
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
            return true;
        }
    }
    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    if (conId != BLE_CONNECTION_UNINITIALIZED)
    {
        for (uint16_t i = 0; i < kMaxConnections; i++)
        {
            if (mSubscribedConIds[i] == conId)
            {
                return true;
            }
        }
    }
    return false;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
