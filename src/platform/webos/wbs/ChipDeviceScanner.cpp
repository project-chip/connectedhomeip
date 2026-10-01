/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ChipDeviceScanner.h"

#include <algorithm>
#include <cstring>
#include <errno.h>
#include <lib/support/SafeInt.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <luna-service2++/handle.hpp>
#include "lsrequester.h"

#define CHIP_BLE_BASE_SERVICE_UUID_STRING "-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_SERVICE_PREFIX_LENGTH 8
#define CHIP_BLE_BASE_SERVICE_PREFIX "0000"
#define CHIP_BLE_UUID_SERVICE_SHORT_STRING "fff6"
#define CHIP_BLE_UUID_SERVICE_SHORT 0xfff6

#define CHIP_BLE_UUID_SERVICE_STRING                                                                                               \
    CHIP_BLE_BASE_SERVICE_PREFIX CHIP_BLE_UUID_SERVICE_SHORT_STRING CHIP_BLE_BASE_SERVICE_UUID_STRING

#define API_BLUETOOTH_ADAPTER_GETSTATUS "luna://com.webos.service.bluetooth2/adapter/getStatus"
#define API_BLUETOOTH_ADAPTER_SETSTATE "luna://com.webos.service.bluetooth2/adapter/setState"
//#define API_BLUETOOTH_ADAPTER_START_DISCOVERY "luna://com.webos.service.bluetooth2/adapter/startDiscovery"
//#define API_BLUETOOTH_ADAPTER_CANCEL_DISCOVERY "luna://com.webos.service.bluetooth2/adapter/cancelDiscovery"
//#define API_BLUETOOTH_LE_INTERNAL_STARTSCAN "luna://com.webos.service.bluetooth2/le/internal/startScan"
#define API_BLUETOOTH_LE_STARTSCAN "luna://com.webos.service.bluetooth2/le/startScan"

#define PARAM_BLANK "{}"

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Max CHIPoBLE service data payload carried in a single AD structure (see ReportDevice()).
constexpr size_t kMaxServiceDataLength = 32;

static bool _HexToBytes(const std::string & octetString, uint8_t * dataBytes, size_t dataBytesSize)
{
    uint8_t buffer[kMaxServiceDataLength];
    size_t argLen = octetString.length();
    if (argLen == 0 || argLen > sizeof(buffer) * 2)
    {
        return false;
    }

    size_t octetCount = chip::Encoding::HexToBytes(octetString.c_str(), argLen, buffer, sizeof(buffer));

    if (octetCount == 0)
    {
        return false;
    }
    // Never copy more than the destination can hold (service data may carry extra bytes).
    memcpy(dataBytes, buffer, std::min(octetCount, dataBytesSize));

    return true;
}

/// Retrieve CHIP device identification info from the device advertising data
bool WbsGetChipDeviceInfo(const pbnjson::JValue & aDevice, chip::Ble::ChipBLEDeviceIdentificationInfo & aDeviceInfo)
{
    VerifyOrReturnError(aDevice.hasKey("serviceData") == true, false);
    bool bChipDevice = false;
// origin > remove
    // if (aDevice.hasKey("serviceUuid") == true)
    // {
    //     for (int i = 0 ; i < aDevice["serviceUuid"].arraySize() ; ++i)
    //     {
    //         if (aDevice["serviceUuid"][i].asString().compare(CHIP_BLE_UUID_SERVICE_SHORT_STRING) == 0)
    //         {
    //             bChipDevice = true;
    //             break;
    //         }
    //     }
    // }

    if (aDevice.hasKey("serviceDataUuid") == true)
    {
        if (aDevice["serviceDataUuid"].asString().compare(CHIP_BLE_UUID_SERVICE_SHORT_STRING) == 0)
        {
            bChipDevice = true;
        }
    }

    VerifyOrReturnError(bChipDevice == true, false);
    VerifyOrReturnError( _HexToBytes(aDevice["serviceData"].asString(),
                reinterpret_cast<uint8_t *>(&aDeviceInfo), sizeof(aDeviceInfo)) == true, false);

    return bChipDevice;
}

} // namespace

CHIP_ERROR ChipDeviceScanner::Init(ChipDeviceScannerDelegate * delegate)
{
    // Make this function idempotent by shutting down previously initialized state if any.
    Shutdown();

//    mAdapter.reset(reinterpret_cast<BluezAdapter1 *>(g_object_ref(adapter)));
    mDelegate = delegate;

    mScannerState = ChipDeviceScannerState::INITIALIZED;

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::Shutdown()
{
    if(mScannerState == ChipDeviceScannerState::UNINITIALIZED) {
        mLeInternalStartScanToken = LSMESSAGE_TOKEN_INVALID;
        return;
    }
    //VerifyOrReturn(mScannerState != ChipDeviceScannerState::UNINITIALIZED);

    TEMPORARY_RETURN_IGNORED StopScan();

    // Release resources on the glib thread. This is necessary because the D-Bus manager client
    // object handles D-Bus signals. Otherwise, we might face a race when the manager object is
    // released during a D-Bus signal being processed.
    TEMPORARY_RETURN_IGNORED PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) {
//            self->mAdapter.reset();
            return CHIP_NO_ERROR;
        },
        this);

    mScannerState = ChipDeviceScannerState::UNINITIALIZED;
}

CHIP_ERROR ChipDeviceScanner::StartScan()
{
    assertChipStackLockedByCurrentThread();

    //VerifyOrReturnError(mScannerState != ChipDeviceScannerState::SCANNING, CHIP_ERROR_INCORRECT_STATE);
    if(mScannerState == ChipDeviceScannerState::SCANNING || mLeInternalStartScanToken != LSMESSAGE_TOKEN_INVALID) {
        ChipLogProgress(Ble, "StartScan() : already scanning! > stop.");
        if(mLeInternalStartScanToken != LSMESSAGE_TOKEN_INVALID) {
            LsRequester *lsRequester = LsRequester::getInstance();
            lsRequester->lsCallCancel(mLeInternalStartScanToken);
            mLeInternalStartScanToken = LSMESSAGE_TOKEN_INVALID;
        }
        mScannerState = ChipDeviceScannerState::INITIALIZED;
    }

//    mCancellable.reset(g_cancellable_new());
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StartScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan start: %" CHIP_ERROR_FORMAT, err.Format());
        mDelegate->OnScanComplete();
        return err;
    }

    mScannerState = ChipDeviceScannerState::SCANNING;
    ChipLogDetail(Ble, "ChipDeviceScanner has started scanning!");

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    assertChipStackLockedByCurrentThread();

    //VerifyOrReturnError(mScannerState == ChipDeviceScannerState::SCANNING, CHIP_NO_ERROR);
    if(mScannerState != ChipDeviceScannerState::SCANNING) {
        ChipLogDetail(Ble, "StopScan() : ChipDeviceScanner status is not scanning");
        if(mLeInternalStartScanToken == LSMESSAGE_TOKEN_INVALID) {
            ChipLogProgress(Ble, "StopScan() : ChipDeviceScanner is not scanning > skip stopScan");
            return CHIP_NO_ERROR;
        }
        else {
            ChipLogDetail(Ble, "scanner is working");
        }
    }

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StopScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan stop: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    // Stop scanning and return to initialization state
    mScannerState = ChipDeviceScannerState::INITIALIZED;

    ChipLogDetail(Ble, "ChipDeviceScanner has stopped scanning!");

    mDelegate->OnScanComplete();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScanImpl()
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue responsePayload;

    ret = lsRequester->lsCallCancel(mLeInternalStartScanToken);
    if(ret != true) {
        ChipLogProgress(DeviceLayer, "StopScanImpl error  : lsRequester->lsCallCancel");
        //return CHIP_ERROR_INTERNAL; //do not need check
    }
    else {
        ChipLogDetail(DeviceLayer, "StopScanImpl lsRequester->lsCallCancel end.");
    }
    mLeInternalStartScanToken = LSMESSAGE_TOKEN_INVALID;

//    ret = lsRequester->lsCallSync(API_BLUETOOTH_ADAPTER_CANCEL_DISCOVERY, PARAM_BLANK, responsePayload, 30);
//    if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
//        return CHIP_ERROR_INTERNAL;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StartScanImpl()
{
    bool ret = false;
    LsRequester *lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue serviceDataParam = pbnjson::JObject();
    pbnjson::JValue dataJArray = pbnjson::JArray();
    pbnjson::JValue maskJArray = pbnjson::JArray();
    pbnjson::JValue responsePayload;

    // ret = lsRequester->lsCallSync(API_BLUETOOTH_ADAPTER_START_DISCOVERY, lunaParam.stringify().c_str(), responsePayload);
    // if(ret != true || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    //     return CHIP_ERROR_INTERNAL;

    dataJArray.append(0);
    maskJArray.append(0);
    serviceDataParam.put("uuid","fff6");
    serviceDataParam.put("data",dataJArray);
    serviceDataParam.put("mask",maskJArray);
    lunaParam.put("serviceData", serviceDataParam);
    lunaParam.put("subscribe", true);

    if(mLeInternalStartScanToken != LSMESSAGE_TOKEN_INVALID) {
        ChipLogProgress(DeviceLayer, "StartScanImpl() : scanner is working > try to stop");
       lsRequester->lsCallCancel(mLeInternalStartScanToken);
       mLeInternalStartScanToken = LSMESSAGE_TOKEN_INVALID;
    }
    // LSMessageToken ulToken = LSMESSAGE_TOKEN_INVALID;

    // ret = lsRequester->lsSubscribe(API_BLUETOOTH_LE_INTERNAL_STARTSCAN, lunaParam.stringify().c_str(), this, OnLeDeviceScanned, &ulToken);
    // ChipLogDetail(DeviceLayer, "[%lu]Call %s '%s'", ulToken, API_BLUETOOTH_LE_INTERNAL_STARTSCAN, lunaParam.stringify().c_str());
    // mLeInternalStartScanToken = ulToken;
    ret = lsRequester->lsSubscribe(API_BLUETOOTH_LE_STARTSCAN, lunaParam.stringify().c_str(), this, OnLeDeviceScanned, &mLeInternalStartScanToken);
    ChipLogDetail(DeviceLayer, "[%lu]Call %s '%s'", mLeInternalStartScanToken, API_BLUETOOTH_LE_STARTSCAN, lunaParam.stringify().c_str());

    VerifyOrReturnError(ret == true, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "StartScanImpl ret: %d", ret));

    ChipLogProgress(DeviceLayer, "Scan started");

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::ReportDevice(const pbnjson::JValue & device)
{
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo = {};

    pbnjson::JValue bleDevice = pbnjson::JObject();
    bleDevice = device;

    //["scanRecord"] : luna://com.webos.service.bluetooth2/le/internal/startScan result
    pbnjson::JValueArrayElement scanRecordDataJObj = device["scanRecord"];
    ssize_t scanRecordDataJSize = scanRecordDataJObj.arraySize();

    uint8_t aScanRecord[256] = {0,};
    uint8_t* payload = &aScanRecord[0];
    // Clamp to the local buffer (out-of-bounds write otherwise).
    if (scanRecordDataJSize > static_cast<ssize_t>(sizeof(aScanRecord)))
        scanRecordDataJSize = static_cast<ssize_t>(sizeof(aScanRecord));
    if (scanRecordDataJSize < 0)
        scanRecordDataJSize = 0;
    const size_t total_len = static_cast<size_t>(scanRecordDataJSize);

    for(ssize_t j = 0; j < scanRecordDataJSize; j++)
    {
        int32_t v = scanRecordDataJObj[j].asNumber<int32_t>();
        if (chip::CanCastTo<uint8_t>(v))
            aScanRecord[j] = static_cast<uint8_t>(v);
    }
    uint8_t adv_length = 0;
    uint8_t adv_type = 0;
    size_t sizeConsumed = 0; // uint8_t would wrap at 256
    bool finished = (total_len == 0);

    while(!finished) {
        adv_length = *payload;
        payload++;
        sizeConsumed += 1 + adv_length;
        // Stop if the AD structure runs past the scan record (out-of-bounds read otherwise).
        if (sizeConsumed > total_len)
            break;

        if (adv_length != 0)
        {
            adv_type = *payload;
            payload++;
            adv_length--;

            switch(adv_type) {
                case 0x09 /* BLE_AD_TYPE_NAME_CMPL */: {   // Adv Data Type: 0x09
                    std::string nameStr(reinterpret_cast<char*>(payload), adv_length);
                    //ChipLogDetail(DeviceLayer, "Type: name : %s",nameStr.c_str());
                    bleDevice.put("name", nameStr.c_str());
                    break;
                } // BLE_AD_TYPE_NAME_CMPL

                case 0x03 /* BLE_AD_TYPE_16SRV_CMPL */:
                case 0x02 /* BLE_AD_TYPE_16SRV_PART */: {   // Adv Data Type: 0x02
                    pbnjson::JValue serviceUuidArray = pbnjson::JArray();
                    for ( int var = 0 ; var < adv_length/2; ++var )
                    {
                        uint16_t serviceUuid = ( *(payload + var * 2 + 1) << 8) | *(payload + var * 2);
                        char serviceUuidStr[4 + 1] = "";
                        //ChipLogDetail(DeviceLayer, "Type: serviceUuid 16 : %u", serviceUuid);
                        TEMPORARY_RETURN_IGNORED chip::Encoding::Uint16ToHex(serviceUuid, serviceUuidStr, sizeof(serviceUuidStr), chip::Encoding::HexFlags::kNullTerminate);

                        serviceUuidArray.append(std::string(serviceUuidStr));
                        bleDevice.put("serviceUuid", serviceUuidArray);
                    }
                    break;
                } // BLE_AD_TYPE_16SRV_PART

                case 0x16 /* BLE_AD_TYPE_SERVICE_DATA */: {  // Adv Data Type: 0x16 (Service Data) - 2 byte UUID
                    if (adv_length < 2) {
                        //ChipLogError(DeviceLayer, "Length too small for BLE_AD_TYPE_SERVICE_DATA");
                        break;
                    }
                    uint16_t serviceDataUuid = ( *(payload+1) << 8) | *payload;
                    char serviceDataUuidStr[4 + 1] = "";

                    if (serviceDataUuid != CHIP_BLE_UUID_SERVICE_SHORT)
                        break;

                    //ChipLogDetail(DeviceLayer, "Type: serviceData 16 : %u", serviceDataUuid );

                    TEMPORARY_RETURN_IGNORED chip::Encoding::Uint16ToHex(serviceDataUuid, serviceDataUuidStr, sizeof(serviceDataUuidStr), chip::Encoding::HexFlags::kNullTerminate);
                    bleDevice.put("serviceDataUuid", std::string(serviceDataUuidStr));
                    if (adv_length > 2) {
                        char serviceDataStr[kMaxServiceDataLength * 2 + 1] = "";
                        TEMPORARY_RETURN_IGNORED chip::Encoding::BytesToLowercaseHexString(payload + 2, adv_length - 2, &serviceDataStr[0], MATTER_ARRAY_SIZE(serviceDataStr));
                        bleDevice.put("serviceData", std::string(serviceDataStr));
                    }
                    break;
                } //BLE_AD_TYPE_SERVICE_DATA

                default: {
                    //ChipLogDetail(DeviceLayer, "Type: 0x%.2x, adv_length: %d", adv_type, adv_length);
                    break;
                }
            }
            payload += adv_length;
        }

        if (sizeConsumed >= static_cast<size_t>(total_len))
            finished = true;
    }


    if (!WbsGetChipDeviceInfo(bleDevice, deviceInfo))
    {
        ChipLogDetail(Ble, "Device %s does not look like a CHIP device.", device["address"].asString().c_str());
        return;
    }

    mDelegate->OnDeviceScanned(bleDevice, deviceInfo);
}

bool ChipDeviceScanner::OnLeDeviceScanned(LSHandle * sh, LSMessage * message, void * userData)
{
    ChipDeviceScanner * self = (ChipDeviceScanner *) userData;

    LS::Message response(message);
    pbnjson::JValue responsePayload;
    ChipLogDetail(DeviceLayer, "receiveMessage = %s", response.getPayload());

    responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    if (responsePayload["returnValue"].asBool() == true)
    {
        if(responsePayload.hasKey("adapterAddress") == true) {
            ChipLogProgress(DeviceLayer, "LeStartScan success");
        }
        else if(responsePayload.hasKey("devices") == true) {
            pbnjson::JValueArrayElement devicesDataJObj = responsePayload["devices"];
            ssize_t devicesDataSize = devicesDataJObj.arraySize();

            for (ssize_t i = 0; i < devicesDataSize; ++i)
            {
                self->ReportDevice(devicesDataJObj[i]);
            }
        }
    }
    else
    {
        ChipLogDetail(DeviceLayer, "LeStartScan failure");
    }

    return true;
}

void ChipDeviceScanner::RemoveDevice(const pbnjson::JValue & device)
{
    ChipLogError(Ble, "RemoveDevice: Not implemented");

}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
