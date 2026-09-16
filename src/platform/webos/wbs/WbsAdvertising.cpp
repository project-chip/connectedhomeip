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

#include "WbsAdvertising.h"

#include <ble/Ble.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ConfigurationManager.h>

#include "LsRequester.h"
#include <luna-service2++/handle.hpp>

#define API_BLUETOOTH_LE_CONFIGUREADVERTISEMENT "luna://com.webos.service.bluetooth2/le/configureAdvertisement"
#define API_BLUETOOTH_LE_STARTADVERTISING "luna://com.webos.service.bluetooth2/le/startAdvertising"
#define API_BLUETOOTH_LE_STOPADVERTISING "luna://com.webos.service.bluetooth2/le/stopAdvertising"

// Short-form 16-bit CHIPoBLE service UUID, matches CHIP_BLE_UUID_SERVICE_SHORT_STRING in WbsDeviceScanner.cpp.
#define CHIP_BLE_UUID_SERVICE_SHORT_STRING "fff6"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR WbsAdvertising::Start()
{
    VerifyOrReturnValue(!mIsAdvertising, CHIP_NO_ERROR, ChipLogDetail(DeviceLayer, "BLE advertising already started"));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(StartImpl, this);
}

CHIP_ERROR WbsAdvertising::StartImpl(WbsAdvertising * self)
{
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue responsePayload;

    Ble::ChipBLEDeviceIdentificationInfo deviceInfo;
    ReturnErrorOnFailure(ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceInfo));

    pbnjson::JValue configParam  = pbnjson::JObject();
    pbnjson::JValue service      = pbnjson::JObject();
    pbnjson::JValue serviceArray = pbnjson::JArray();
    pbnjson::JValue dataArray    = pbnjson::JArray();

    const uint8_t * infoBytes = reinterpret_cast<const uint8_t *>(&deviceInfo);
    for (size_t i = 0; i < sizeof(deviceInfo); ++i)
    {
        dataArray.append(infoBytes[i]);
    }
    service.put("uuid", std::string(CHIP_BLE_UUID_SERVICE_SHORT_STRING));
    service.put("data", dataArray);
    serviceArray.append(service);
    configParam.put("services", serviceArray);

    bool ret = lsRequester->lsCallSync(API_BLUETOOTH_LE_CONFIGUREADVERTISEMENT, configParam.stringify().c_str(), responsePayload);
    VerifyOrReturnError(ret && responsePayload.hasKey(STR_RETURN_VALUE) && responsePayload[STR_RETURN_VALUE].asBool(),
                        CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "le/configureAdvertisement failed"));

    pbnjson::JValue startParam = pbnjson::JObject();
    startParam.put("subscribe", true);

    ret = lsRequester->lsSubscribe(API_BLUETOOTH_LE_STARTADVERTISING, startParam.stringify().c_str(), self,
                                   OnAdvertisingStatusChanged, &self->mSubscribeToken);
    VerifyOrReturnError(ret, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "le/startAdvertising failed"));

    return CHIP_NO_ERROR;
}

bool WbsAdvertising::OnAdvertisingStatusChanged(LSHandle * sh, LSMessage * message, void * ctx)
{
    auto * self = static_cast<WbsAdvertising *>(ctx);
    LS::Message response(message);
    pbnjson::JValue responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    if (!responsePayload["returnValue"].asBool())
    {
        ChipLogError(DeviceLayer, "le/startAdvertising subscription error: %s", responsePayload["errorText"].asString().c_str());
        self->mIsAdvertising = false;
        BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(CHIP_ERROR_INTERNAL);
        return true;
    }

    if (responsePayload.hasKey("advertiserId"))
    {
        self->mAdvertiserId = responsePayload["advertiserId"].asNumber<int32_t>();
    }

    bool subscribed = responsePayload.hasKey("subscribed") && responsePayload["subscribed"].asBool();
    if (subscribed && !self->mIsAdvertising)
    {
        self->mIsAdvertising = true;
        BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(CHIP_NO_ERROR);
    }
    else if (!subscribed && self->mIsAdvertising)
    {
        // The advertiser subscription was cancelled by the system, i.e. advertising was
        // stopped outside of our control. Mirrors BlueZ's "advertisement released" case.
        self->mIsAdvertising = false;
        self->mAdvertiserId  = -1;
        BLEManagerImpl::NotifyBLEPeripheralAdvReleased();
    }

    return true;
}

CHIP_ERROR WbsAdvertising::Stop()
{
    VerifyOrReturnValue(mIsAdvertising || mSubscribeToken != LSMESSAGE_TOKEN_INVALID, CHIP_NO_ERROR,
                        ChipLogDetail(DeviceLayer, "BLE advertising already stopped"));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(StopImpl, this);
}

CHIP_ERROR WbsAdvertising::StopImpl(WbsAdvertising * self)
{
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue stopParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    if (self->mAdvertiserId >= 0)
    {
        stopParam.put("advertiserId", self->mAdvertiserId);
    }

    bool ret = lsRequester->lsCallSync(API_BLUETOOTH_LE_STOPADVERTISING, stopParam.stringify().c_str(), responsePayload);

    if (self->mSubscribeToken != LSMESSAGE_TOKEN_INVALID)
    {
        lsRequester->lsCallCancel(self->mSubscribeToken);
        self->mSubscribeToken = LSMESSAGE_TOKEN_INVALID;
    }
    self->mIsAdvertising = false;
    self->mAdvertiserId  = -1;

    if (!ret || !responsePayload.hasKey(STR_RETURN_VALUE) || !responsePayload[STR_RETURN_VALUE].asBool())
    {
        ChipLogError(DeviceLayer, "le/stopAdvertising failed");
        BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(CHIP_ERROR_INTERNAL);
        return CHIP_ERROR_INTERNAL;
    }

    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(CHIP_NO_ERROR);
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
