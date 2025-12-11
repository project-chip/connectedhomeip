/* @@@LICENSE
 *
 * Copyright (c) 2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#include "WbsDeviceScanner.h"

#include <errno.h>
#include <lib/support/BytesToHex.h>
#include <lib/core/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <luna-service2++/handle.hpp>

#define CHIP_BLE_BASE_SERVICE_UUID_STRING "-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_SERVICE_PREFIX_LENGTH 8
#define CHIP_BLE_BASE_SERVICE_PREFIX "0000"
#define CHIP_BLE_UUID_SERVICE_SHORT_STRING "fff6"
#define CHIP_BLE_UUID_SERVICE_LONG_STRING "0000FFF6-0000-1000-8000-00805F9B34FB"

#define CHIP_BLE_UUID_SERVICE_STRING                                                                                               \
    CHIP_BLE_BASE_SERVICE_PREFIX CHIP_BLE_UUID_SERVICE_SHORT_STRING CHIP_BLE_BASE_SERVICE_UUID_STRING

#define API_BLUETOOTH_ADAPTER_GETSTATUS "luna://com.webos.service.bluetooth2/adapter/getStatus"
#define API_BLUETOOTH_ADAPTER_SETSTATE "luna://com.webos.service.bluetooth2/adapter/setState"
#define API_BLUETOOTH_ADAPTER_START_DISCOVERY "luna://com.webos.service.bluetooth2/adapter/startDiscovery"
#define API_BLUETOOTH_ADAPTER_CANCEL_DISCOVERY "luna://com.webos.service.bluetooth2/adapter/cancelDiscovery"
#define API_BLUETOOTH_LE_INTERNAL_STARTSCAN "luna://com.webos.service.bluetooth2/le/internal/startScan"

#define PARAM_BLANK "{}"
#define ArraySize(a) (sizeof(a) / sizeof((a)[0]))

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static bool _HexToBytes(std::string octetString, uint8_t * dataBytes)
{
    chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;
    size_t argLen = octetString.length();
    if (!buffer.Calloc(argLen))
    {
        return false;
    }

    size_t octetCount = chip::Encoding::HexToBytes(octetString.c_str(), argLen, buffer.Get(), argLen);

    if (octetCount == 0)
    {
        return false;
    }
    memcpy(dataBytes, buffer.Get(), octetCount);

    return true;
}

/// Retrieve CHIP device identification info from the device advertising data
bool WbsGetChipDeviceInfo(const pbnjson::JValue & aDevice, chip::Ble::ChipBLEDeviceIdentificationInfo & aDeviceInfo)
{
    VerifyOrReturnError(aDevice.hasKey("serviceData") == true, false);
    bool bChipDevice = false;

    if (aDevice.hasKey("serviceUuid") == true)
    {
        for (int i = 0; i < aDevice["serviceUuid"].arraySize(); ++i)
        {
            if (aDevice["serviceUuid"][i].asString().compare(CHIP_BLE_UUID_SERVICE_SHORT_STRING) == 0)
            {
                bChipDevice = true;
                break;
            }
        }
    }

    if (aDevice.hasKey("serviceDataUuid") == true)
    {
        if (aDevice["serviceDataUuid"].asString().compare(CHIP_BLE_UUID_SERVICE_SHORT_STRING) == 0)
        {
            bChipDevice = true;
        }
    }

    VerifyOrReturnError(bChipDevice == true, false);
    VerifyOrReturnError(_HexToBytes(aDevice["serviceData"].asString(), reinterpret_cast<uint8_t *>(&aDeviceInfo)) == true, false);

    return bChipDevice;
}

} // namespace

CHIP_ERROR WbsDeviceScanner::Init(WbsDeviceScannerDelegate * delegate)
{
    // Make this function idempotent by shutting down previously initialized state if any.
    Shutdown();

    //    mAdapter.reset(reinterpret_cast<BluezAdapter1 *>(g_object_ref(adapter)));
    mDelegate = delegate;

    mScannerState = WbsDeviceScannerState::INITIALIZED;

    return CHIP_NO_ERROR;
}

void WbsDeviceScanner::Shutdown()
{
    VerifyOrReturn(mScannerState != WbsDeviceScannerState::UNINITIALIZED);

    StopScan();

    // Release resources on the glib thread. This is necessary because the D-Bus manager client
    // object handles D-Bus signals. Otherwise, we might face a race when the manager object is
    // released during a D-Bus signal being processed.
    PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WbsDeviceScanner * self) {
            //            self->mAdapter.reset();
            return CHIP_NO_ERROR;
        },
        this);

    mScannerState = WbsDeviceScannerState::UNINITIALIZED;
}

CHIP_ERROR WbsDeviceScanner::StartScan()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState != WbsDeviceScannerState::SCANNING, CHIP_ERROR_INCORRECT_STATE);

    //    mCancellable.reset(g_cancellable_new());
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WbsDeviceScanner * self) { return self->StartScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan start: %" CHIP_ERROR_FORMAT, err.Format());
        mDelegate->OnScanComplete();
        return err;
    }

    mScannerState = WbsDeviceScannerState::SCANNING;
    ChipLogDetail(Ble, "ChipDeviceScanner has started scanning!");

    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsDeviceScanner::StopScan()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState == WbsDeviceScannerState::SCANNING, CHIP_NO_ERROR);

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WbsDeviceScanner * self) { return self->StopScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan stop: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    // Stop scanning and return to initialization state
    mScannerState = WbsDeviceScannerState::INITIALIZED;

    ChipLogDetail(Ble, "ChipDeviceScanner has stopped scanning!");

    mDelegate->OnScanComplete();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsDeviceScanner::StopScanImpl()
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();

    ret = lsRequester->lsCallCancel(mLeInternalStartScanToken);
    if (ret != true)
    {
        ChipLogError(Ble, "ChipDeviceScanner StopScanImpl lsCallCancel Error");
        return CHIP_ERROR_INTERNAL;
    }
    lsRequester->restart();
    ChipLogError(Ble, "ChipDeviceScanner StopScanImpl Success");
    return CHIP_NO_ERROR;
}

CHIP_ERROR WbsDeviceScanner::StartScanImpl()
{
    bool ret                  = false;
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("subscribe", true);
    pbnjson::JValue serviceUuid = pbnjson::JObject();
    serviceUuid.put("uuid", CHIP_BLE_UUID_SERVICE_LONG_STRING);
    lunaParam.put("serviceUuid", serviceUuid);

    LSMessageToken ulToken = LSMESSAGE_TOKEN_INVALID;

    ret = lsRequester->lsSubscribe(API_BLUETOOTH_LE_INTERNAL_STARTSCAN, lunaParam.stringify().c_str(), this, OnLeDeviceScanned,
                                   &ulToken);
    ChipLogDetail(DeviceLayer, "[%lu]Call %s '%s'", ulToken, API_BLUETOOTH_LE_INTERNAL_STARTSCAN, lunaParam.stringify().c_str());
    mLeInternalStartScanToken = ulToken;
    VerifyOrReturnError(ret == true, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "StartScanImpl ret: %d", ret));

    ChipLogProgress(DeviceLayer, "Scan started(API_BLUETOOTH_LE_INTERNAL_STARTSCAN)");

    return CHIP_NO_ERROR;
}

bool WbsDeviceScanner::OnLeDeviceScanned(LSHandle * sh, LSMessage * message, void * userData)
{
    WbsDeviceScanner * self = (WbsDeviceScanner *) userData;

    LS::Message response(message);
    pbnjson::JValue responsePayload;
    std::string responseStr(response.getPayload());
    ChipLogDetail(DeviceLayer, "receiveMessage = %s", response.getPayload());

    responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    if (responsePayload["returnValue"].asBool() == true)
    {
        if (responsePayload.hasKey("adapterAddress") == true)
        {
            ChipLogDetail(DeviceLayer, "LeStartScan success");
        }
        else if (responsePayload.hasKey("devices") == true)
        {
            pbnjson::JValue value = pbnjson::JDomParser::fromString(responseStr);

            pbnjson::JValueArrayElement devicesDataJObj = value["devices"];
            ssize_t devicesDataSize                     = devicesDataJObj.arraySize();

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

void WbsDeviceScanner::ReportDevice(const pbnjson::JValue & device)
{
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    pbnjson::JValue bleDevice = pbnjson::JObject();
    bleDevice                 = device;

    //["scanRecord"] : luna://com.webos.service.bluetooth2/le/internal/startScan result
    pbnjson::JValueArrayElement scanRecordDataJObj = device["scanRecord"];
    ssize_t scanRecordDataJSize                    = scanRecordDataJObj.arraySize();

    uint8_t aScanRecord[256] = {
        0,
    };
    uint8_t * payload = &aScanRecord[0];
    ssize_t total_len = scanRecordDataJObj.arraySize();

    for (ssize_t j = 0; j < scanRecordDataJSize; j++)
    {
        int32_t v = scanRecordDataJObj[j].asNumber<int32_t>();
        if (chip::CanCastTo<uint8_t>(v))
            aScanRecord[j] = static_cast<uint8_t>(v);
    }
    uint8_t adv_length   = 0;
    uint8_t adv_type     = 0;
    uint8_t sizeConsumed = 0;
    bool finished        = false;

    while (!finished)
    {
        adv_length = *payload;
        payload++;
        sizeConsumed += 1 + adv_length;

        if (adv_length != 0)
        {
            adv_type = *payload;
            payload++;
            adv_length--;

            switch (adv_type)
            {
            // BLE_AD_TYPE_NAME_CMPL
            case 0x09: { // Adv Data Type: 0x09
                std::string nameStr(reinterpret_cast<char *>(payload), adv_length);
                // ChipLogDetail(DeviceLayer, "Type: name : %s",nameStr.c_str());
                bleDevice.put("name", nameStr.c_str());
                break;
            } // BLE_AD_TYPE_NAME_CMPL
            // BLE_AD_TYPE_16SRV_CMPL
            case 0x03:
            // BLE_AD_TYPE_16SRV_PART
            case 0x02: { // Adv Data Type: 0x02
                pbnjson::JValue serviceUuidArray = pbnjson::JArray();
                for (int var = 0; var < adv_length / 2; ++var)
                {
                    uint16_t serviceUuid       = (*(payload + var * 2 + 1) << 8) | *(payload + var * 2);
                    char serviceUuidStr[4 + 1] = "";
                    // ChipLogDetail(DeviceLayer, "Type: serviceUuid 16 : %u", serviceUuid);
                    chip::Encoding::Uint16ToHex(serviceUuid, serviceUuidStr, sizeof(serviceUuidStr),
                                                chip::Encoding::HexFlags::kNullTerminate);

                    serviceUuidArray.append(std::string(serviceUuidStr));
                    bleDevice.put("serviceUuid", serviceUuidArray);
                }
                break;
            } // BLE_AD_TYPE_16SRV_PART
            // BLE_AD_TYPE_SERVICE_DATA
            case 0x16: { // Adv Data Type: 0x16 (Service Data) - 2 byte UUID
                if (adv_length < 2)
                {
                    // ChipLogError(DeviceLayer, "Length too small for BLE_AD_TYPE_SERVICE_DATA");
                    break;
                }
                uint16_t serviceDataUuid       = (*(payload + 1) << 8) | *payload;
                char serviceDataUuidStr[4 + 1] = "";

                // ChipLogDetail(DeviceLayer, "Type: serviceData 16 : %u", serviceDataUuid );

                chip::Encoding::Uint16ToHex(serviceDataUuid, serviceDataUuidStr, sizeof(serviceDataUuidStr),
                                            chip::Encoding::HexFlags::kNullTerminate);
                bleDevice.put("serviceDataUuid", std::string(serviceDataUuidStr));
                if (adv_length > 2)
                {
                    char serviceDataStr[64 + 1] = "";
                    chip::Encoding::BytesToLowercaseHexString(payload + 2, adv_length - 2, &serviceDataStr[0],
                                                              ArraySize(serviceDataStr));
                    bleDevice.put("serviceData", std::string(serviceDataStr));
                }
                break;
            } // BLE_AD_TYPE_SERVICE_DATA

            default: {
                char buffer[256] = {
                    0,
                };
                chip::Encoding::BytesToLowercaseHexString(payload, adv_length, &buffer[0], ArraySize(buffer));
                // ChipLogDetail(DeviceLayer, "Type: 0x%.2x, adv_length: %d, data: %s", adv_type, adv_length, buffer);
                break;
            }
            }
            payload += adv_length;
        }

        if (sizeConsumed >= total_len)
            finished = true;
    }

    if (!WbsGetChipDeviceInfo(bleDevice, deviceInfo))
    {
        // ChipLogDetail(Ble, "Device %s does not look like a CHIP device.", device["address"].asString().c_str());
        return;
    }

    mBleChipDevice = chip::Platform::New<BLEChipDevice>(bleDevice, deviceInfo);
    mDelegate->OnDeviceScanned(mBleChipDevice->mBleDevice, mBleChipDevice->mDeviceInfo);
    chip::Platform::Delete(mBleChipDevice);
}

void WbsDeviceScanner::RemoveDevice(const pbnjson::JValue & device)
{
    ChipLogError(Ble, "RemoveDevice: Not implemented");
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
