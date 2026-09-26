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

#include "WbsGattServer.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#include "LsRequester.h"
#include "WbsConnection.h"
#include <luna-service2++/handle.hpp>

#define API_BLUETOOTH_GATT_OPENSERVER "luna://com.webos.service.bluetooth2/gatt/openServer"
#define API_BLUETOOTH_GATT_CLOSESERVER "luna://com.webos.service.bluetooth2/gatt/closeServer"
#define API_BLUETOOTH_GATT_ADDSERVICE "luna://com.webos.service.bluetooth2/gatt/addService"
#define API_BLUETOOTH_GATT_REMOVESERVICE "luna://com.webos.service.bluetooth2/gatt/removeService"
#define API_BLUETOOTH_GATT_MONITORCHARACTERISTIC "luna://com.webos.service.bluetooth2/gatt/monitorCharacteristic"
#define API_BLUETOOTH_GATT_READDESCRIPTOR "luna://com.webos.service.bluetooth2/gatt/readDescriptorValue"

// Matches the UUIDs already used for the central/client role in WbsConnection.cpp - the same
// CHIPoBLE service is now also exposed locally for the peripheral role.
#define CHIP_BLE_GATT_SERVICE "0000fff6-0000-1000-8000-00805f9b34fb"
#define CHIP_BLE_GATT_CHAR_WRITE "18ee2ef5-263d-4559-959f-4f9c429f9d11"
#define CHIP_BLE_GATT_CHAR_READ "18ee2ef5-263d-4559-959f-4f9c429f9d12"
#define CHIP_BLE_GATT_CCCD "00002902-0000-1000-8000-00805f9b34fb"

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR WbsGattServer::Init()
{
    VerifyOrReturnValue(!mIsOpen, CHIP_NO_ERROR, ChipLogDetail(DeviceLayer, "GATT server already open"));
    return PlatformMgrImpl().GLibMatterContextInvokeSync(InitImpl, this);
}

CHIP_ERROR WbsGattServer::InitImpl(WbsGattServer * self)
{
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue responsePayload;

    bool ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_OPENSERVER, "{}", responsePayload);
    VerifyOrReturnError(ret && responsePayload.hasKey(STR_RETURN_VALUE) && responsePayload[STR_RETURN_VALUE].asBool(),
                        CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "gatt/openServer failed"));
    VerifyOrReturnError(responsePayload.hasKey("serverId"), CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "gatt/openServer response missing serverId"));
    self->mServerId = responsePayload["serverId"].asString();

    pbnjson::JValue rxProps = pbnjson::JObject();
    rxProps.put("write", true);
    pbnjson::JValue rxPerms = pbnjson::JObject();
    rxPerms.put("write", true);
    pbnjson::JValue rxBytes = pbnjson::JArray();
    rxBytes.append(0);
    pbnjson::JValue rxValue = pbnjson::JObject();
    rxValue.put("bytes", rxBytes);
    pbnjson::JValue rxChar = pbnjson::JObject();
    rxChar.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
    rxChar.put("properties", rxProps);
    rxChar.put("permissions", rxPerms);
    rxChar.put("value", rxValue);

    // TX: we indicate response data back to the commissioner; the CCCD descriptor lets it
    // subscribe (enable indications) the same way a standard BLE central would.
    pbnjson::JValue txProps = pbnjson::JObject();
    txProps.put("indicate", true);
    pbnjson::JValue txPerms = pbnjson::JObject();
    txPerms.put("read", true);
    pbnjson::JValue txBytes = pbnjson::JArray();
    txBytes.append(0);
    pbnjson::JValue txValue = pbnjson::JObject();
    txValue.put("bytes", txBytes);

    pbnjson::JValue cccdBytes = pbnjson::JArray();
    cccdBytes.append(0);
    cccdBytes.append(0);
    pbnjson::JValue cccdValue = pbnjson::JObject();
    cccdValue.put("bytes", cccdBytes);
    pbnjson::JValue cccd = pbnjson::JObject();
    cccd.put("descriptor", std::string(CHIP_BLE_GATT_CCCD));
    cccd.put("value", cccdValue);
    pbnjson::JValue descriptors = pbnjson::JArray();
    descriptors.append(cccd);

    pbnjson::JValue txChar = pbnjson::JObject();
    txChar.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));
    txChar.put("properties", txProps);
    txChar.put("permissions", txPerms);
    txChar.put("value", txValue);
    txChar.put("descriptors", descriptors);

    pbnjson::JValue characteristics = pbnjson::JArray();
    characteristics.append(rxChar);
    characteristics.append(txChar);

    pbnjson::JValue addServiceParam = pbnjson::JObject();
    addServiceParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    addServiceParam.put("type", std::string("primary"));
    addServiceParam.put("includes", pbnjson::JArray());
    addServiceParam.put("characteristics", characteristics);

    ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_ADDSERVICE, addServiceParam.stringify().c_str(), responsePayload);
    VerifyOrReturnError(ret && responsePayload.hasKey(STR_RETURN_VALUE) && responsePayload[STR_RETURN_VALUE].asBool(),
                        CHIP_ERROR_INTERNAL, {
                            ChipLogError(DeviceLayer, "gatt/addService failed");
                            RollbackInitImpl(self);
                        });

    self->mIsOpen = true;
    ChipLogDetail(DeviceLayer, "WbsGattServer: CHIPoBLE service registered (serverId=%s)", self->mServerId.c_str());

    // BLE_CONNECTION_OBJECT (see BlePlatformConfig.h) is always a WbsConnection*. Model the
    // (single) inbound peripheral-role connection accepted by this GATT server as one too, so
    // BLEManagerImpl::SubscribeCharacteristic/SendIndication/CloseConnection - which all just call
    // through to conId->Method() - route correctly for it. See WbsConnection::ConfigureAsServerRole().
    self->mPeerConnection = chip::Platform::New<WbsConnection>();
    self->mPeerConnection->ConfigureAsServerRole(self->mServerId);

    {
        pbnjson::JValue monitorParam = pbnjson::JObject();
        monitorParam.put("serverId", self->mServerId);
        monitorParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
        monitorParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_WRITE));
        monitorParam.put("subscribe", true);

        bool subscribed = lsRequester->lsSubscribe(API_BLUETOOTH_GATT_MONITORCHARACTERISTIC, monitorParam.stringify().c_str(), self,
                                                   OnRxCharacteristicChanged, &self->mRxMonitorToken);
        if (!subscribed)
        {
            ChipLogError(DeviceLayer, "WbsGattServer: failed to subscribe RX characteristic monitor");
            RollbackInitImpl(self);
            return CHIP_ERROR_INTERNAL;
        }
    }

    self->mCccdIndicateEnabled = false;
    DeviceLayer::SystemLayer().StartTimer(kCccdPollInterval, HandleCccdPollTimer, self);

    return CHIP_NO_ERROR;
}

bool WbsGattServer::OnRxCharacteristicChanged(LSHandle * sh, LSMessage * message, void * ctx)
{
    auto * self = static_cast<WbsGattServer *>(ctx);
    LS::Message response(message);
    pbnjson::JValue responsePayload = pbnjson::JDomParser::fromString(response.getPayload());

    VerifyOrExit(responsePayload["returnValue"].asBool() == true,
                 ChipLogError(DeviceLayer, "WbsGattServer: RX monitor error: %s", responsePayload["errorText"].asString().c_str()));

    if (responsePayload.hasKey("changed"))
    {
        pbnjson::JValueArrayElement bytesDataJObj = responsePayload["changed"]["value"]["bytes"];
        ssize_t bytesDataJSize                    = bytesDataJObj.arraySize();

        System::PacketBufferHandle buf = System::PacketBufferHandle::New(static_cast<size_t>(bytesDataJSize), 0);
        VerifyOrExit(!buf.IsNull(), ChipLogError(DeviceLayer, "WbsGattServer: failed to allocate RX buffer"));

        for (ssize_t i = 0; i < bytesDataJSize; ++i)
        {
            int32_t v = 0;
            bytesDataJObj[i].asNumber<int32_t>(v);
            if (chip::CanCastTo<uint8_t>(v))
                *(buf->Start() + i) = static_cast<uint8_t>(v);
        }
        buf->SetDataLength(static_cast<uint16_t>(bytesDataJSize));

        BLEManagerImpl::HandleRXCharWrite(self->mPeerConnection, buf->Start(), buf->DataLength());
    }

exit:
    return true;
}

void WbsGattServer::HandleCccdPollTimer(chip::System::Layer * layer, void * appState)
{
    auto * self = static_cast<WbsGattServer *>(appState);

    // Init()/Shutdown() run on the Matter event loop, same as this timer callback, so there is no
    // race between mIsOpen going false and this reschedule check.
    VerifyOrReturn(self->mIsOpen);

    PlatformMgrImpl().GLibMatterContextInvokeSync(PollCccdImpl, self);
    DeviceLayer::SystemLayer().StartTimer(kCccdPollInterval, HandleCccdPollTimer, self);
}

CHIP_ERROR WbsGattServer::PollCccdImpl(WbsGattServer * self)
{
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue lunaParam = pbnjson::JObject();
    pbnjson::JValue responsePayload;

    lunaParam.put("serverId", self->mServerId);
    lunaParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lunaParam.put("characteristic", std::string(CHIP_BLE_GATT_CHAR_READ));
    lunaParam.put("descriptor", std::string(CHIP_BLE_GATT_CCCD));

    bool ret = lsRequester->lsCallSync(API_BLUETOOTH_GATT_READDESCRIPTOR, lunaParam.stringify().c_str(), responsePayload);
    // A transient failure here (e.g. no central connected yet, so the descriptor read fails) is
    // not fatal - just try again on the next poll tick rather than tearing down the BLE service.
    VerifyOrReturnError(ret && responsePayload.hasKey(STR_RETURN_VALUE) && responsePayload[STR_RETURN_VALUE].asBool(),
                        CHIP_NO_ERROR);

    pbnjson::JValueArrayElement bytesArray = responsePayload["value"]["value"]["bytes"];
    bool indicateEnabled                   = false;
    if (bytesArray.arraySize() >= 1)
    {
        int32_t v = 0;
        bytesArray[0].asNumber<int32_t>(v);
        // CCCD bit 1 (0x02) is the "enable indications" bit (bit 0 / 0x01 is "enable notifications").
        indicateEnabled = (v & 0x02) != 0;
    }

    if (indicateEnabled != self->mCccdIndicateEnabled)
    {
        self->mCccdIndicateEnabled = indicateEnabled;
        self->mPeerConnection->SetNotifyAcquired(indicateEnabled);
        ChipLogProgress(DeviceLayer, "WbsGattServer: remote %s indications on TX", indicateEnabled ? "enabled" : "disabled");
        BLEManagerImpl::HandleTXCharCCCDWrite(self->mPeerConnection);
    }

    return CHIP_NO_ERROR;
}

void WbsGattServer::Shutdown()
{
    VerifyOrReturn(mIsOpen);
    // Stop rescheduling before anything else. HandleCccdPollTimer only checks mIsOpen from the
    // Matter event loop thread, which is also the thread Shutdown() is required to run on (see the
    // header doc comment on Init()), so flipping the flag here is enough to prevent a poll already
    // in flight from rescheduling itself again after this call returns.
    mIsOpen = false;
    DeviceLayer::SystemLayer().CancelTimer(HandleCccdPollTimer, this);
    PlatformMgrImpl().GLibMatterContextInvokeSync(ShutdownImpl, this);
}

// Undo a partially completed InitImpl() once gatt/openServer has succeeded. Deliberately does not
// depend on mIsOpen or Shutdown(): ShutdownImpl() already releases every resource InitImpl() may
// have acquired (RX monitor token, CHIPoBLE service, serverId, mPeerConnection) and tolerates the
// ones that were never acquired (removeService on an unregistered service is simply rejected).
void WbsGattServer::RollbackInitImpl(WbsGattServer * self)
{
    ShutdownImpl(self);
}

CHIP_ERROR WbsGattServer::ShutdownImpl(WbsGattServer * self)
{
    LsRequester * lsRequester = LsRequester::getInstance();
    pbnjson::JValue responsePayload;

    if (self->mRxMonitorToken != LSMESSAGE_TOKEN_INVALID)
    {
        lsRequester->lsCallCancel(self->mRxMonitorToken);
        self->mRxMonitorToken = LSMESSAGE_TOKEN_INVALID;
    }

    pbnjson::JValue removeParam = pbnjson::JObject();
    removeParam.put("service", std::string(CHIP_BLE_GATT_SERVICE));
    lsRequester->lsCallSync(API_BLUETOOTH_GATT_REMOVESERVICE, removeParam.stringify().c_str(), responsePayload);

    pbnjson::JValue closeParam = pbnjson::JObject();
    closeParam.put("serverId", self->mServerId);
    lsRequester->lsCallSync(API_BLUETOOTH_GATT_CLOSESERVER, closeParam.stringify().c_str(), responsePayload);

    chip::Platform::Delete(self->mPeerConnection);
    self->mPeerConnection      = nullptr;
    self->mCccdIndicateEnabled = false;

    self->mIsOpen = false;
    self->mServerId.clear();
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
