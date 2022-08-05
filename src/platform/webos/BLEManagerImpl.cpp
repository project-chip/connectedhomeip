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
#include <platform/CommissionableDataProvider.h>
#include <platform/internal/BLEManager.h>

#include <cassert>
#include <iomanip>
#include <type_traits>
#include <utility>

#include "MainLoop.h"
#include <pbnjson.h>
#include <pbnjson.hpp>

using namespace pbnjson;

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static constexpr unsigned kNewConnectionScanTimeoutMs   = 10000;
static constexpr System::Clock::Timeout kConnectTimeout = System::Clock::Seconds16(10);

const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };
#define CHIP_BLE_GATT_SERVICE "0000fff6-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_GATT_CHAR_WRITE "18ee2ef5-263d-4559-959f-4f9c429f9d11"
#define CHIP_BLE_GATT_CHAR_READ "18ee2ef5-263d-4559-959f-4f9c429f9d12"

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void HandleIncomingBleConnection(BLEEndPoint * bleEP)
{
    ChipLogProgress(DeviceLayer, "con rcvd");
}

void BLEManagerImpl::InitConnectionData(void)
{
    /* Initialize Hashmap */
    if (!mConnectionMap)
    {
        mConnectionMap = g_hash_table_new(g_str_hash, g_str_equal);
        ChipLogProgress(DeviceLayer, "GATT Connection HashMap created");
    }
}

gboolean BLEManagerImpl::_BleInitialize(void * userData)
{
    if (sInstance.mFlags.Has(Flags::kWebOSBLELayerInitialized))
    {
        ChipLogProgress(DeviceLayer, "BLE Already Initialized");
        return true;
    }

    sInstance.InitConnectionData();

    // Should add BT callback

    sInstance.mFlags.Set(Flags::kWebOSBLELayerInitialized);
    ChipLogProgress(DeviceLayer, "BLE Initialized");
    sInstance.mMainContext = g_main_context_get_thread_default();
    return true;
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    bool ret;

    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);

    memset(mDeviceName, 0, sizeof(mDeviceName));

    OnChipBleConnectReceived = HandleIncomingBleConnection;

    ret = MainLoop::Instance().Init(_BleInitialize);
    VerifyOrExit(ret != false, err = CHIP_ERROR_INTERNAL);

    ret = MainLoop::Instance().StartLSMainLoop();
    VerifyOrExit(ret != false, err = CHIP_ERROR_INTERNAL);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    // ensure scan resources are cleared (e.g. timeout timers)
    mDeviceScanner.reset();
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, false);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != nullptr && deviceName[0] != 0)
    {
        VerifyOrExit(strlen(deviceName) < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kUseCustomDeviceName);
    }
    else
    {
        uint16_t discriminator;
        SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
        mFlags.Clear(Flags::kUseCustomDeviceName);
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections()
{
    uint16_t numCons = 0;
    return numCons;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mIsCentral     = aIsCentral;

    return err;
}

CHIP_ERROR BLEManagerImpl::StartBLEAdvertising()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::StopBLEAdvertising()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        {
            ChipDeviceEvent connectionEvent;
            connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEventOrDie(&connectionEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;
    case DeviceEventType::kServiceProvisioningChange:
        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        mFlags.Clear(Flags::kAdvertisingConfigured);

        DriveBLEState();
        break;
    default:
        HandlePlatformSpecificBLEEvent(event);
        break;
    }
}

void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool controlOpComplete = false;
    ChipLogDetail(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);
    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformWebOSBLECentralConnected:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionComplete(mBLEScanConfig.mAppState,
                                                        apEvent->Platform.BLECentralConnected.mConnection);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformWebOSBLECentralConnectFailed:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnectFailed.mError);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformWebOSBLEWriteComplete:
        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX);
        break;
    case DeviceEventType::kPlatformWebOSBLESubscribeOpComplete:
        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                    &ChipUUID_CHIPoBLEChar_TX);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                      &ChipUUID_CHIPoBLEChar_TX);
        break;
    case DeviceEventType::kPlatformWebOSBLEIndicationReceived:
        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;
    case DeviceEventType::kPlatformWebOSBLEPeripheralAdvConfiguredComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        sInstance.mFlags.Set(Flags::kAdvertisingConfigured).Clear(Flags::kControlOpInProgress);
        controlOpComplete = true;
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising config complete");
        break;
    case DeviceEventType::kPlatformWebOSBLEPeripheralAdvStartComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStartComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);

        if (!sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Set(Flags::kAdvertising);
        }

        break;
    case DeviceEventType::kPlatformWebOSBLEPeripheralAdvStopComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStopComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);

        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Clear(Flags::kAdvertising);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        }
        break;
    case DeviceEventType::kPlatformWebOSBLEPeripheralRegisterAppComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralRegisterAppComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        mFlags.Set(Flags::kAppRegistered);
        controlOpComplete = true;
        break;
    default:
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        sInstance.mFlags.Clear(Flags::kControlOpInProgress);
    }

    if (controlOpComplete)
    {
        mFlags.Clear(Flags::kControlOpInProgress);
        DriveBLEState();
    }
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return 20;
}

bool BLEManagerImpl::gattMonitorCharateristicsCb(LSHandle * sh, LSMessage * message, void * userData)
{
    BLEConnection * conn = nullptr;
    conn                 = (BLEConnection *) userData;

    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return true;

    const char * payload = jvalue_tostring(parsedObj, input_schema);

    pbnjson::JValue jvalue = pbnjson::JDomParser::fromString(std::string(payload));

    if (jvalue.hasKey("returnValue"))
    {
        ChipLogProgress(DeviceLayer, "gattMonitorCharateristicsCb payload returnValue is %d", jvalue["returnValue"].asBool());

        if (jvalue["changed"]["value"].hasKey("bytes") == true)
        {
            ChipLogProgress(DeviceLayer, "received read value is %s", jvalue["changed"]["value"].stringify().c_str());

            pbnjson::JValue value = jvalue["changed"]["value"];

            uint8_t * values = (uint8_t *) malloc(sizeof(uint8_t) * value["bytes"].arraySize());

            for (int i = 0; i < value["bytes"].arraySize(); i++)
            {
                values[i] = value["bytes"][i].asNumber<int32_t>();
            }

            sInstance.HandleTXCharChanged(conn, values, value["bytes"].arraySize());
        }
    }

    return true;
}

bool BLEManagerImpl::gattWriteDescriptorValueCb(LSHandle * sh, LSMessage * message, void * userData)
{
    BLEConnection * conn = nullptr;
    conn                 = (BLEConnection *) userData;

    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return false;

    const char * payload = jvalue_tostring(parsedObj, input_schema);

    ChipLogProgress(DeviceLayer, "gattWriteDescriptorValueCb payload is %s", payload);
    sInstance.HandleSubscribeOpComplete(conn, true);

    return true;
}

bool BLEManagerImpl::SubscribeCharacteristicToWebOS(void * bleConnObj, const uint8_t * svcId, const uint8_t * charId)
{
    pbnjson::JValue valueForMonitor = pbnjson::JObject();

    valueForMonitor.put("clientId", std::string(mClientId));
    valueForMonitor.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    pbnjson::JValue bytesJArray = pbnjson::JArray();
    bytesJArray.append(std::string(CHIP_BLE_GATT_CHAR_READ));

    valueForMonitor.put("characteristics", bytesJArray);
    valueForMonitor.put("subscribe", true);

    int ret = 0;

    if (mLSHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "LS handle is null");
        return false;
    }

    ret = LSCall(mLSHandle, "luna://com.webos.service.bluetooth2/gatt/monitorCharacteristics", valueForMonitor.stringify().c_str(),
                 gattMonitorCharateristicsCb, bleConnObj, NULL, NULL);

    sleep(2);

    pbnjson::JValue valueForDescriptor = pbnjson::JObject();
    valueForDescriptor.put("clientId", std::string(mClientId));
    valueForDescriptor.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    valueForDescriptor.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));

    valueForDescriptor.put("descriptor", std::string("00002902-0000-1000-8000-00805f9b34fb"));

    bool subscribe = true; // current is true.

    pbnjson::JValue valueParam               = pbnjson::JObject();
    pbnjson::JValue bytesForDescriptorJArray = pbnjson::JArray(); // "bytes": [ ]
    if (subscribe)
    {
        bytesForDescriptorJArray.append(2);
        bytesForDescriptorJArray.append(0);
    }
    else
    {
        bytesForDescriptorJArray.append(0);
        bytesForDescriptorJArray.append(0);
    }

    valueParam.put("bytes", bytesForDescriptorJArray);
    valueForDescriptor.put("value", valueParam);

    ChipLogProgress(Ble, "SubscribeCharacteristicToWebOS Param : valueForDescriptor  %s", valueForDescriptor.stringify().c_str());

    ret = LSCall(mLSHandle, "luna://com.webos.service.bluetooth2/gatt/writeDescriptorValue", valueForDescriptor.stringify().c_str(),
                 gattWriteDescriptorValueCb, bleConnObj, NULL, NULL);

    if (ret != 1)
        return false;

    return true;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    bool result = false;
    result      = SubscribeCharacteristicToWebOS(conId, static_cast<const uint8_t *>(svcId->bytes),
                                            static_cast<const uint8_t *>(charId->bytes));
    return result;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(Ble, "UnsubscribeCharacteristic: Not implemented");
    return true;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogError(Ble, "CloseConnection: Not implemented");
    return true;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBufferHandle pBuf)
{
    ChipLogError(Ble, "SendIndication: Not implemented");
    return true;
}

bool BLEManagerImpl::gattWriteValueCb(LSHandle * sh, LSMessage * message, void * userData)
{
    BLEConnection * conn = nullptr;
    conn                 = (BLEConnection *) userData;

    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return true;

    const char * payload = jvalue_tostring(parsedObj, input_schema);

    ChipLogProgress(DeviceLayer, "gattWriteValueCb payload is %s", payload);
    sInstance.HandleWriteComplete(conn);

    return true;
}

bool BLEManagerImpl::SendWriteRequestToWebOS(void * bleConnObj, const uint8_t * svcId, const uint8_t * charId, const uint8_t * pBuf,
                                             uint32_t pBufDataLen)
{
    BLEConnection * conn = (BLEConnection *) bleConnObj;
    std::ostringstream cvt;

    for (int i = 0; i < (int) pBufDataLen; i++)
    {
        cvt << std::hex << std::setfill('0') << std::setw(2) << (int) pBuf[i];
    }

    pbnjson::JValue values = pbnjson::JObject();
    values.put("clientId", std::string(mClientId));
    values.put("data", cvt.str().c_str());

    ChipLogProgress(Ble, "SendWriteRequestToWebOS Param : value %s", values.stringify().c_str());

    std::string clientId  = values["clientId"].asString();
    std::string valueType = "bytes";
    std::string value     = values["data"].asString();

    pbnjson::JValue param      = pbnjson::JObject();
    pbnjson::JValue valueParam = pbnjson::JObject();
    param.put("clientId", clientId);
    param.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    param.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));

    if (valueType == "byte")
    {
        valueParam.put("bytes", pbnjson::JArray{ std::stoi(value) });
    }
    else if (valueType == "bytes")
    {
        pbnjson::JValue bytesJArray = JArray(); // "bytes": [ ]
        std::vector<char> bytes;
        for (int i = 0; i < (int) value.length(); i += 2)
        {
            std::string byteString = value.substr(i, 2);
            char c                 = (char) strtol(byteString.c_str(), NULL, 16);
            bytesJArray.append(c);
        }
        valueParam.put("bytes", bytesJArray);
    }
    else if (valueType == "string")
    {
        valueParam.put("string", value);
    }
    else
    {
    }
    param.put("value", valueParam);

    ChipLogProgress(Ble, "SendWriteRequestToWebOS Param : param  %s", param.stringify().c_str());

    int ret = 0;

    if (mLSHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "LS handle is null");
        return false;
    }

    ret = LSCall(mLSHandle, "luna://com.webos.service.bluetooth2/gatt/writeCharacteristicValue", param.stringify().c_str(),
                 gattWriteValueCb, conn, NULL, NULL);

    VerifyOrExit(ret == 1, ChipLogError(DeviceLayer, "Failed to write characteristic . ret [%d]", ret));

exit:
    if (ret != 1)
        return false;

    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    bool result = false;
    result = SendWriteRequestToWebOS(conId, static_cast<const uint8_t *>(svcId->bytes), static_cast<const uint8_t *>(charId->bytes),
                                     pBuf->Start(), pBuf->DataLength());

    return result;
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

void BLEManagerImpl::AddConnectionData(const char * remoteAddr)
{
    BLEConnection * conn;
    ChipLogProgress(DeviceLayer, "AddConnectionData for [%s]", remoteAddr);

    if (!g_hash_table_lookup(mConnectionMap, remoteAddr))
    {
        ChipLogProgress(DeviceLayer, "Not Found in Map");
        conn           = (BLEConnection *) g_malloc0(sizeof(BLEConnection));
        conn->peerAddr = g_strdup(remoteAddr);

        if (sInstance.mIsCentral)
        {
            g_hash_table_insert(mConnectionMap, (gpointer) conn->peerAddr, conn);
            ChipLogProgress(DeviceLayer, "New Connection Added for [%s]", remoteAddr);
            HandleNewConnection(conn);
        }
        else
        {
            /* Local Device is BLE Peripheral Role, assume remote is CHIP Central */
            conn->isChipDevice = true;
            g_hash_table_insert(mConnectionMap, (gpointer) conn->peerAddr, conn);
            ChipLogProgress(DeviceLayer, "New Connection Added for [%s]", remoteAddr);
        }
    }
}

void BLEManagerImpl::HandleConnectionEvent(bool connected, const char * remoteAddress)
{
    if (connected)
    {
        ChipLogProgress(DeviceLayer, "Device Connected [%s]", remoteAddress);
        AddConnectionData(remoteAddress);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device DisConnected [%s]", remoteAddress);
    }
}

void BLEManagerImpl::HandleNewConnection(BLE_CONNECTION_OBJECT conId)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                     = DeviceEventType::kPlatformWebOSBLECentralConnected;
        event.Platform.BLECentralConnected.mConnection = conId;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformWebOSBLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = error;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformWebOSBLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed)
{
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformWebOSBLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = subscribed;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(value, len);

    ChipLogDetail(DeviceLayer, "Indication received, conn = %p", conId);

    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformWebOSBLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = conId;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);

exit:
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "HandleTXCharChanged() failed: %s", ErrorStr(err));
}

void BLEManagerImpl::HandleRXCharWrite(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(value, len);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kCHIPoBLEWriteReceived;
        ChipLogProgress(Ble, "Write request received debug %p", conId);
        event.CHIPoBLEWriteReceived.ConId = conId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEventOrDie(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::HandleTXComplete(BLE_CONNECTION_OBJECT conId)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);
        ExitNow();
    }

    // If there's already a control operation in progress, wait until it completes.
    VerifyOrExit(!mFlags.Has(Flags::kControlOpInProgress), /* */);

    // Initializes the Bluez BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kWebOSBLELayerInitialized))
    {
        mFlags.Set(Flags::kWebOSBLELayerInitialized);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
}

void BLEManagerImpl::InitiateScan(BleScanState scanType)
{
    DriveBLEState();

    if (scanType == BleScanState::kNotScanning)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "Invalid scan type requested");
        return;
    }

    mDeviceScanner               = Internal::ChipDeviceScanner::Create(this);
    mBLEScanConfig.mBleScanState = scanType;

    if (!mDeviceScanner)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INTERNAL);
        ChipLogError(Ble, "Failed to create a BLE device scanner");
        return;
    }

    CHIP_ERROR err = mDeviceScanner->StartChipScan(kNewConnectionScanTimeoutMs);

    if (err != CHIP_NO_ERROR)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to start a BLE can: %s", chip::ErrorStr(err));
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
        return;
    }

    ChipLogError(DeviceLayer, "BLE Scan Initiation Successful");
}

void BLEManagerImpl::CleanScanConfig()
{
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::InitiateScan(intptr_t arg)
{
    sInstance.InitiateScan(static_cast<BleScanState>(arg));
}

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator)
{
    mBLEScanConfig.mDiscriminator = connDiscriminator;
    mBLEScanConfig.mAppState      = appState;

    // Scan initiation performed async, to ensure that the BLE subsystem is initialized.
    PlatformMgr().ScheduleWork(InitiateScan, static_cast<intptr_t>(BleScanState::kScanForDiscriminator));
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                                 = DeviceEventType::kPlatformWebOSBLEPeripheralRegisterAppComplete;
    event.Platform.BLEPeripheralRegisterAppComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralRegisterAppComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformWebOSBLEPeripheralAdvConfiguredComplete;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                              = DeviceEventType::kPlatformWebOSBLEPeripheralAdvStartComplete;
    event.Platform.BLEPeripheralAdvStartComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStartComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                             = DeviceEventType::kPlatformWebOSBLEPeripheralAdvStopComplete;
    event.Platform.BLEPeripheralAdvStopComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStopComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::OnChipScanComplete(void)
{
    if (mBLEScanConfig.mBleScanState != BleScanState::kScanForDiscriminator &&
        mBLEScanConfig.mBleScanState != BleScanState::kScanForAddress)
    {
        ChipLogProgress(DeviceLayer, "Scan complete notification without an active scan.");
        return;
    }

    ChipLogError(DeviceLayer, "Scan Completed with Timeout: Notify Upstream.");
    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_TIMEOUT);
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::OnScanComplete()
{
    if (mBLEScanConfig.mBleScanState != BleScanState::kScanForDiscriminator &&
        mBLEScanConfig.mBleScanState != BleScanState::kScanForAddress)
    {
        ChipLogProgress(Ble, "Scan complete notification without an active scan.");
        return;
    }

    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_TIMEOUT);
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

bool BLEManagerImpl::gattGetServiceCb(LSHandle * sh, LSMessage * message, void * userData)
{
    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return true;

    const char * payload = jvalue_tostring(parsedObj, input_schema);

    ChipLogProgress(DeviceLayer, "gattGetServiceCb payload is %s", payload);

    sInstance.HandleConnectionEvent(true, sInstance.mRemoteAddress);

    return true;
}

bool BLEManagerImpl::gattConnectCb(LSHandle * sh, LSMessage * message, void * userData)
{
    jvalue_ref parsedObj     = { 0 };
    jschema_ref input_schema = jschema_parse(j_cstr_to_buffer("{}"), DOMOPT_NOOPT, NULL);

    if (!input_schema)
        return false;

    JSchemaInfo schemaInfo;
    jschema_info_init(&schemaInfo, input_schema, NULL, NULL);
    parsedObj = jdom_parse(j_cstr_to_buffer(LSMessageGetPayload(message)), DOMOPT_NOOPT, &schemaInfo);
    jschema_release(&input_schema);

    if (jis_null(parsedObj))
        return true;

    const char * payload = jvalue_tostring(parsedObj, input_schema);

    ChipLogProgress(DeviceLayer, "gattConnectCb payload is %s", payload);

    jvalue_ref clientIdObj = { 0 };
    if (jobject_get_exists(parsedObj, J_CSTR_TO_BUF("clientId"), &clientIdObj))
    {
        raw_buffer clientId_buf = jstring_get(clientIdObj);
        char * clientId         = g_strdup(clientId_buf.m_str);
        jstring_free_buffer(clientId_buf);

        printf("clientId : %s \n", clientId);

        if (sInstance.mClientId != nullptr)
        {
            g_free(sInstance.mClientId);
        }
        sInstance.mClientId = g_strdup(clientId);

        g_free(clientId);
    }

    jvalue_ref addressObj = { 0 };
    if (jobject_get_exists(parsedObj, J_CSTR_TO_BUF("address"), &addressObj))
    {
        raw_buffer address_buf = jstring_get(addressObj);
        char * address         = g_strdup(address_buf.m_str);
        jstring_free_buffer(address_buf);

        printf("address : %s \n", address);

        if (sInstance.mRemoteAddress != nullptr)
        {
            g_free(sInstance.mRemoteAddress);
        }
        sInstance.mRemoteAddress = g_strdup(address);

        g_free(address);
    }

    if (sInstance.mLSHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "LS handle is null");
        return false;
    }
    sleep(1);
    char ls2Param[100];
    snprintf(ls2Param, 100, "{\"address\":\"%s\"}", sInstance.mRemoteAddress);

    ChipLogProgress(DeviceLayer, "getService: Addr [%s]", sInstance.mRemoteAddress);

    int ret = 0;

    ret = LSCall(sInstance.mLSHandle, "luna://com.webos.service.bluetooth2/gatt/getServices", ls2Param, gattGetServiceCb, NULL,
                 NULL, NULL);

    VerifyOrExit(ret == 1, ChipLogError(DeviceLayer, "Failed to get GATT service . ret [%d]", ret));

exit:
    if (ret != 1)
        sInstance.HandleConnectFailed(CHIP_ERROR_INTERNAL);

    return true;
}

gboolean BLEManagerImpl::ConnectChipThing(gpointer userData)
{
    int ret = 0;

    if (sInstance.mLSHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "LS handle is null");
        return false;
    }

    char * address = (char *) userData;
    ChipLogProgress(DeviceLayer, "ConnectRequest: Addr [%s]", address);

    char ls2Param[100];
    snprintf(ls2Param, 100, "{\"address\":\"%s\"}", address);

    ret = LSCall(sInstance.mLSHandle, "luna://com.webos.service.bluetooth2/gatt/connect", ls2Param, gattConnectCb, userData, NULL,
                 NULL);

    VerifyOrExit(ret == 1, ChipLogError(DeviceLayer, "Failed to create GATT client. ret [%d]", ret));

    ChipLogProgress(DeviceLayer, "GATT Connect Issued");
exit:
    if (ret != 1)
        sInstance.HandleConnectFailed(CHIP_ERROR_INTERNAL);

    g_free(address);
    return G_SOURCE_REMOVE;
}

void BLEManagerImpl::ConnectHandler(const char * address)
{
    ChipLogProgress(DeviceLayer, "Try to connect New device scanned: %s", address);

    mLSHandle = MainLoop::Instance().mLSHandle;

    if (mLSHandle == nullptr)
    {
        ChipLogError(DeviceLayer, "LS handle is null");
        return;
    }

    GSource * idleSource;

    idleSource = g_idle_source_new();
    g_source_set_callback(idleSource, ConnectChipThing, g_strdup(address), nullptr);
    g_source_set_priority(idleSource, G_PRIORITY_HIGH_IDLE);
    g_source_attach(idleSource, sInstance.mMainContext);
    g_source_unref(idleSource);
}

void BLEManagerImpl::OnChipDeviceScanned(char * address)
{
    ChipLogProgress(DeviceLayer, "New device scanned: %s", address);

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        ChipLogProgress(DeviceLayer, "Device discriminator match. Attempting to connect.");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        if (strcmp(address, "e4:5f:01:20:49:c0") == 0)
        {
            ChipLogProgress(DeviceLayer, "SSH Ignore : %s", address);
            return;
        }
        ChipLogProgress(DeviceLayer, "Device address match. Attempting to connect.");
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknown discovery type. Ignoring scanned device.");
        return;
    }

    /* Set CHIP Connecting state */
    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
    mDeviceScanner->StopChipScan();

    /* Initiate Connect */
    ConnectHandler(address);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
