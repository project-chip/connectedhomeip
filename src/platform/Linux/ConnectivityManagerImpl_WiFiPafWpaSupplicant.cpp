/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    Copyright (c) 2025 NXP
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

#include <mutex>

#include <stdint.h>

#include <glib.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformManager.h>

#include "ConnectivityManagerImpl.h"

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
using namespace ::chip::WiFiPAF;
#endif

namespace chip {
namespace DeviceLayer {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
static const char srv_name[] = "_matterc._udp";
/*
    NAN-USD Service Protocol Type: ref: Table 58 of Wi-Fi Aware Specificaiton
*/
#define NAN_PUBLISH_SSI_TAG " ssi="

#pragma pack(push, 1)
struct PAFPublishSSI
{
    uint8_t DevOpCode;
    uint16_t DevInfo;
    uint16_t ProductId;
    uint16_t VendorId;
};

enum nan_service_protocol_type
{
    NAN_SRV_PROTO_BONJOUR    = 1,
    NAN_SRV_PROTO_GENERIC    = 2,
    NAN_SRV_PROTO_CSA_MATTER = 3,
};
#pragma pack(pop)

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFPublish(ConnectivityManager::WiFiPAFAdvertiseParam & InArgs)
{
    CHIP_ERROR result = StartWiFiManagementSync();
    VerifyOrReturnError(result == CHIP_NO_ERROR, result);

    GAutoPtr<GError> err;
    guint publish_id;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_MAX_ADVERTISING_TIMEOUT_SECS;
    unsigned int freq                             = CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL;
    unsigned int ssi_len                          = sizeof(struct PAFPublishSSI);

    // Add the freq_list:
    GVariant * freq_array_variant =
        g_variant_new_fixed_array(G_VARIANT_TYPE_UINT16, InArgs.freq_list.get(), InArgs.freq_list_len, sizeof(guint16));
    if (freq_array_variant == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: freq_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }

    // Construct the SSI
    struct PAFPublishSSI PafPublish_ssi;

    PafPublish_ssi.DevOpCode = 0;
    SuccessOrDie(DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(PafPublish_ssi.DevInfo));
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(PafPublish_ssi.ProductId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.ProductId = 0;
    }
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(PafPublish_ssi.VendorId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.VendorId = 0;
    }
    GVariant * ssi_array_variant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &PafPublish_ssi, ssi_len, sizeof(guint8));
    if (ssi_array_variant == nullptr)
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: ssi_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }
    GVariantBuilder builder;
    GVariant * args = nullptr;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "srv_name", g_variant_new_string(srv_name));
    g_variant_builder_add(&builder, "{sv}", "srv_proto_type", g_variant_new_byte(srv_proto_type));
    g_variant_builder_add(&builder, "{sv}", "ttl", g_variant_new_uint16(ttl));
    g_variant_builder_add(&builder, "{sv}", "freq", g_variant_new_uint16(freq));
    g_variant_builder_add(&builder, "{sv}", "ssi", ssi_array_variant);
    g_variant_builder_add(&builder, "{sv}", "freq_list", freq_array_variant);
    args = g_variant_builder_end(&builder);
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    wpa_supplicant_1_interface_call_nanpublish_sync(mWpaSupplicant.iface.get(), args, &publish_id, nullptr, &err.GetReceiver());

    ChipLogProgress(DeviceLayer, "WiFi-PAF: publish_id: %u ! ", publish_id);
    WiFiPAFSession sessionInfo  = { .role = WiFiPafRole::kWiFiPafRole_Publisher, .id = publish_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    ReturnErrorOnFailure(WiFiPafLayer.AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo));
    InArgs.publish_id = publish_id;

    g_signal_connect(mWpaSupplicant.iface.get(), "nanreplied",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->OnReplied(obj);
                     }),
                     this);

    g_signal_connect(mWpaSupplicant.iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->OnNanReceive(obj);
                     }),
                     this);
    g_signal_connect(
        mWpaSupplicant.iface.get(), "nanpublish-terminated",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_publish_id, gchar * reason, ConnectivityManagerImpl * self) {
            return self->OnNanPublishTerminated(term_publish_id, reason);
        }),
        this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelPublish(uint32_t PublishId)
{
    GAutoPtr<GError> err;

    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel publish_id: %d ! ", PublishId);
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant.iface, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "WiFi-PAF: Skip D-Bus 'cancel publish' call since wpa_supplicant is not ready"));

    gboolean result =
        wpa_supplicant_1_interface_call_nancancel_publish_sync(mWpaSupplicant.iface.get(), PublishId, nullptr, &err.GetReceiver());

    // TODO #40814: make sure that the callers do check the return values. This doesn't seem to be happening now.
    VerifyOrReturnError(
        result, CHIP_ERROR_INTERNAL,
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to Cancel Publish with Error: %s", err ? err->message : "unknown error"));

    return CHIP_NO_ERROR;
}

/*
    NAN-USD Service Protocol Type: ref: Table 58 of Wi-Fi Aware Specificaiton
*/
void ConnectivityManagerImpl::OnDiscoveryResult(GVariant * discov_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnDiscoveryResult");
    uint32_t subscribe_id;
    uint32_t peer_publish_id;
    uint8_t peer_addr[6];
    uint32_t srv_proto_type;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    if (g_variant_n_children(discov_info) == 0)
    {
        return;
    }

    /*
        Read the data from dbus
    */
    GAutoPtr<GVariant> dataValue;
    GVariant * value;

    value = g_variant_lookup_value(discov_info, "subscribe_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &subscribe_id);
    value = g_variant_lookup_value(discov_info, "publish_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &peer_publish_id);

    char addr_str[20];
    char * paddr;
    value = g_variant_lookup_value(discov_info, "peer_addr", G_VARIANT_TYPE_STRING);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "&s", &paddr);
    strncpy(addr_str, paddr, sizeof(addr_str));
    sscanf(addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &peer_addr[0], &peer_addr[1], &peer_addr[2], &peer_addr[3],
           &peer_addr[4], &peer_addr[5]);

    value = g_variant_lookup_value(discov_info, "srv_proto_type", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &srv_proto_type);

    // Read the ssi
    gsize bufferLen;
    value = g_variant_lookup_value(discov_info, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);
    auto ssibuf      = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssibuf);
    GetWiFiPAF()->SetWiFiPAFState(WiFiPAF::State::kConnected);

    /*
        Error Checking
    */
    WiFiPAFSession sessionInfo  = { .discriminator = pPublishSSI->DevInfo };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    if (pPafInfo == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, no valid session with discriminator: %u", pPublishSSI->DevInfo);
        return;
    }
    if (pPafInfo->peer_id != UINT32_MAX)
    {
        // Session already established — ignore all subsequent discovery events,
        // including those from a stale background-scan subscribe whose cancel has
        // not yet taken effect in wpa_supplicant.
        if (pPafInfo->id == subscribe_id)
        {
            ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, reentrance, subscribe_id: %u", subscribe_id);
        }
        else
        {
            ChipLogDetail(DeviceLayer,
                          "WiFi-PAF: DiscoveryResult, session active (peer_id=%u), ignoring stale subscribe_id=%u",
                          pPafInfo->peer_id, subscribe_id);
        }
        return;
    }
    if (srv_proto_type != nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, Incorrect Protocol Type: %u, exp: %u", srv_proto_type,
                     nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER);
        return;
    }

    /*
        Set the PAF session information
    */
    ChipLogProgress(DeviceLayer, "WiFi-PAF: DiscoveryResult, set PafInfo, whose nodeId: %" PRIu64, pPafInfo->nodeId);
    ChipLogProgress(DeviceLayer, "\t (subscribe_id, peer_publish_id): (%u, %u)", subscribe_id, peer_publish_id);
    ChipLogProgress(DeviceLayer, "\t peer_addr: [%02x:%02x:%02x:%02x:%02x:%02x]", peer_addr[0], peer_addr[1], peer_addr[2],
                    peer_addr[3], peer_addr[4], peer_addr[5]);
    ChipLogProgress(DeviceLayer, "\t DevInfo: %x", pPublishSSI->DevInfo);

    pPafInfo->role    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber;
    pPafInfo->id      = subscribe_id;
    pPafInfo->peer_id = peer_publish_id;
    memcpy(pPafInfo->peer_addr, peer_addr, sizeof(uint8_t) * 6);

    // Disconnect the discovery signal handler so that continued NAN publisher
    // activity on the commissionee side does not flood OnDiscoveryResult at
    // ~100/s after the session is established.  The wpa_supplicant subscribe
    // slot itself is intentionally left active — NAN Follow-up frames require
    // an active subscribe in order to be transmitted by wpa_supplicant.
    if (mConnectDiscoverySignalId != 0)
    {
        g_signal_handler_disconnect(mWpaSupplicant.iface.get(), mConnectDiscoverySignalId);
        mConnectDiscoverySignalId = 0;
        ChipLogProgress(DeviceLayer, "WiFi-PAF: OnDiscoveryResult: disconnected discovery signal handler");
    }

    /*
        Indicate the connection event
    */
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFConnected };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, pPafInfo, sizeof(chip::WiFiPAF::WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnReplied(GVariant * reply_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnReplied");
    uint32_t publish_id;
    uint32_t peer_subscribe_id;
    uint8_t peer_addr[6];
    uint32_t srv_proto_type;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    if (g_variant_n_children(reply_info) == 0)
    {
        return;
    }

    /*
        Read the data from dbus
    */
    GAutoPtr<GVariant> dataValue;
    GVariant * value;

    value = g_variant_lookup_value(reply_info, "publish_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &publish_id);
    value = g_variant_lookup_value(reply_info, "subscribe_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &peer_subscribe_id);

    char addr_str[20];
    char * paddr;
    value = g_variant_lookup_value(reply_info, "peer_addr", G_VARIANT_TYPE_STRING);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "&s", &paddr);
    strncpy(addr_str, paddr, sizeof(addr_str));
    sscanf(addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &peer_addr[0], &peer_addr[1], &peer_addr[2], &peer_addr[3],
           &peer_addr[4], &peer_addr[5]);

    value = g_variant_lookup_value(reply_info, "srv_proto_type", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &srv_proto_type);

    // Read the ssi
    gsize bufferLen;
    value = g_variant_lookup_value(reply_info, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);
    auto ssibuf      = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssibuf);

    /*
        Error Checking
    */
    uint16_t SetupDiscriminator;
    TEMPORARY_RETURN_IGNORED DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(SetupDiscriminator);
    if ((pPublishSSI->DevInfo != SetupDiscriminator) || (srv_proto_type != nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER))
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: OnReplied, mismatched discriminator, got %u, ours: %u", pPublishSSI->DevInfo,
                        SetupDiscriminator);
        return;
    }
    WiFiPAFSession sessionInfo  = { .id = publish_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccSessionId, sessionInfo);
    if (pPafInfo == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: OnReplied, no valid session with publish_id: %d", publish_id);
        return;
    }
    if ((pPafInfo->role == WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher) && (pPafInfo->peer_id == peer_subscribe_id) &&
        !memcmp(pPafInfo->peer_addr, peer_addr, sizeof(uint8_t) * 6))
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: OnReplied, reentrance, publish_id: %u ", publish_id);
        return;
    }
    /*
        Set the PAF session information
    */
    ChipLogProgress(DeviceLayer, "WiFi-PAF: OnReplied, set PafInfo, whose nodeId: %" PRIu64, pPafInfo->nodeId);
    ChipLogProgress(DeviceLayer, "\t (publish_id, peer_subscribe_id): (%u, %u)", publish_id, peer_subscribe_id);
    ChipLogProgress(DeviceLayer, "\t peer_addr: [%02x:%02x:%02x:%02x:%02x:%02x]", peer_addr[0], peer_addr[1], peer_addr[2],
                    peer_addr[3], peer_addr[4], peer_addr[5]);
    ChipLogProgress(DeviceLayer, "\t DevInfo: %x", pPublishSSI->DevInfo);

    pPafInfo->role    = WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher;
    pPafInfo->id      = publish_id;
    pPafInfo->peer_id = peer_subscribe_id;
    memcpy(pPafInfo->peer_addr, peer_addr, sizeof(uint8_t) * 6);
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.HandleTransportConnectionInitiated(*pPafInfo);
}

void ConnectivityManagerImpl::OnNanReceive(GVariant * obj)
{
    if (g_variant_n_children(obj) == 0)
    {
        return;
    }
    // Read the rx_info
    WiFiPAF::WiFiPAFSession RxInfo;
    GAutoPtr<GVariant> dataValue;
    GVariant * value;
    value = g_variant_lookup_value(obj, "id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &RxInfo.id);

    value = g_variant_lookup_value(obj, "peer_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &RxInfo.peer_id);

    char addr_str[20];
    char * paddr;
    value = g_variant_lookup_value(obj, "peer_addr", G_VARIANT_TYPE_STRING);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "&s", &paddr);
    strncpy(addr_str, paddr, sizeof(addr_str));
    sscanf(addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &RxInfo.peer_addr[0], &RxInfo.peer_addr[1], &RxInfo.peer_addr[2],
           &RxInfo.peer_addr[3], &RxInfo.peer_addr[4], &RxInfo.peer_addr[5]);

    // Read the rx_data
    gsize bufferLen;
    System::PacketBufferHandle buf;

    value = g_variant_lookup_value(obj, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);

    auto rxbuf = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    ChipLogProgress(DeviceLayer, "WiFi-PAF: wpa_supplicant: nan-rx: [len: %" G_GSIZE_FORMAT "]", bufferLen);
    buf = System::PacketBufferHandle::NewWithData(rxbuf, bufferLen);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event{ .Type                 = DeviceEventType::kCHIPoWiFiPAFReceived,
                           .CHIPoWiFiPAFReceived = { .Data = std::move(buf).UnsafeRelease() } };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &RxInfo, sizeof(WiFiPAF::WiFiPAFSession));
    PlatformMgr().PostEventOrDie(&event);
}

void ConnectivityManagerImpl::OnNanPublishTerminated(guint public_id, gchar * reason)
{
    ChipLogProgress(Controller, "WiFi-PAF: Publish terminated (%u, %s)", public_id, reason);
    WiFiPAFSession sessionInfo  = { .id = public_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
}

void ConnectivityManagerImpl::OnNanSubscribeTerminated(guint subscribe_id, gchar * reason)
{
    ChipLogProgress(Controller, "WiFi-PAF: Subscription terminated (%u, %s)", subscribe_id, reason);
    WiFiPAFSession sessionInfo  = { .id = subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
    /*
        Indicate the connection event
    */
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFCancelConnect };
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSubscribe(const uint16_t & connDiscriminator, void * appState,
                                                      OnConnectionCompleteFunct onSuccess, OnConnectionErrorFunct onError)
{
    CHIP_ERROR result = StartWiFiManagementSync();
    VerifyOrReturnError(result == CHIP_NO_ERROR, result);

    ChipLogProgress(Controller, "WiFi-PAF: Try to subscribe the NAN-USD devices");

    guint subscribe_id;
    GAutoPtr<GError> err;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    uint8_t is_active                             = 1;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_DISCOVERY_TIMEOUT_SECS;
    unsigned int freq                             = (mApFreq == 0) ? CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL : mApFreq;
    unsigned int ssi_len                          = sizeof(struct PAFPublishSSI);
    struct PAFPublishSSI PafPublish_ssi;

    mAppState                = appState;
    PafPublish_ssi.DevOpCode = 0;
    PafPublish_ssi.DevInfo   = connDiscriminator;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(PafPublish_ssi.ProductId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.ProductId = 0;
    }
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(PafPublish_ssi.VendorId) != CHIP_NO_ERROR)
    {
        PafPublish_ssi.VendorId = 0;
    }
    GVariant * ssi_array_variant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &PafPublish_ssi, ssi_len, sizeof(guint8));
    if (ssi_array_variant == nullptr)
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: ssi_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    GVariantBuilder builder;
    GVariant * args = nullptr;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "srv_name", g_variant_new_string(srv_name));
    g_variant_builder_add(&builder, "{sv}", "srv_proto_type", g_variant_new_byte(srv_proto_type));
    g_variant_builder_add(&builder, "{sv}", "active", g_variant_new_boolean(is_active));
    g_variant_builder_add(&builder, "{sv}", "ttl", g_variant_new_uint16(ttl));
    g_variant_builder_add(&builder, "{sv}", "freq", g_variant_new_uint16(freq));
    g_variant_builder_add(&builder, "{sv}", "ssi", ssi_array_variant);
    args = g_variant_builder_end(&builder);
    wpa_supplicant_1_interface_call_nansubscribe_sync(mWpaSupplicant.iface.get(), args, &subscribe_id, nullptr, &err.GetReceiver());

    ChipLogProgress(DeviceLayer, "WiFi-PAF: subscribe_id: [%u], freq: %u", subscribe_id, freq);
    mOnPafSubscribeComplete = onSuccess;
    mOnPafSubscribeError    = onError;

    WiFiPAFSession sessionInfo  = { .discriminator = PafPublish_ssi.DevInfo };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    auto pPafInfo               = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccDisc, sessionInfo);
    if (pPafInfo != nullptr)
    {
        pPafInfo->id   = subscribe_id;
        pPafInfo->role = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber;
    }

    mConnectDiscoverySignalId = g_signal_connect(
        mWpaSupplicant.iface.get(), "nandiscovery-result",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
            return self->OnDiscoveryResult(obj);
        }),
        this);

    g_signal_connect(mWpaSupplicant.iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->OnNanReceive(obj);
                     }),
                     this);

    g_signal_connect(
        mWpaSupplicant.iface.get(), "nansubscribe-terminated",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_subscribe_id, gchar * reason, ConnectivityManagerImpl * self) {
            return self->OnNanSubscribeTerminated(term_subscribe_id, reason);
        }),
        this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelSubscribe(uint32_t SubscribeId)
{
    GAutoPtr<GError> err;

    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel subscribe_id: %d ! ", SubscribeId);
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    wpa_supplicant_1_interface_call_nancancel_subscribe_sync(mWpaSupplicant.iface.get(), SubscribeId, nullptr, &err.GetReceiver());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFCancelIncompleteSubscribe()
{
    mOnPafSubscribeComplete = nullptr;
    mOnPafSubscribeError    = nullptr;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFSend(const WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msgBuf)
{
    ChipLogProgress(Controller, "WiFi-PAF: sending PAF Follow-up packets, (%zu)", msgBuf->DataLength());
    CHIP_ERROR ret = CHIP_NO_ERROR;

    if (msgBuf.IsNull())
    {
        ChipLogError(Controller, "WiFi-PAF: Invalid Packet (%zu)", msgBuf->DataLength());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Ensure outgoing message fits in a single contiguous packet buffer, as currently required by the
    // message fragmentation and reassembly engine.
    if (msgBuf->HasChainedBuffer())
    {
        msgBuf->CompactHead();

        if (msgBuf->HasChainedBuffer())
        {
            ret = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
            ChipLogError(Controller, "WiFi-PAF: Outbound message too big (%zu), skip temporally", msgBuf->DataLength());
            return ret;
        }
    }

    //  Send the packets
    GAutoPtr<GError> err;
    gchar peer_mac[18];

    snprintf(peer_mac, sizeof(peer_mac), "%02x:%02x:%02x:%02x:%02x:%02x", TxInfo.peer_addr[0], TxInfo.peer_addr[1],
             TxInfo.peer_addr[2], TxInfo.peer_addr[3], TxInfo.peer_addr[4], TxInfo.peer_addr[5]);
    GVariant * ssi_array_variant =
        g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, msgBuf->Start(), msgBuf->DataLength(), sizeof(guint8));
    if (ssi_array_variant == nullptr)
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: ssi_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);

    GVariantBuilder builder;
    GVariant * args = nullptr;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "handle", g_variant_new_uint32(TxInfo.id));
    g_variant_builder_add(&builder, "{sv}", "req_instance_id", g_variant_new_uint32(TxInfo.peer_id));
    g_variant_builder_add(&builder, "{sv}", "peer_addr", g_variant_new_string(peer_mac));
    g_variant_builder_add(&builder, "{sv}", "ssi", ssi_array_variant);
    args = g_variant_builder_end(&builder);
    gboolean result =
        wpa_supplicant_1_interface_call_nantransmit_sync(mWpaSupplicant.iface.get(), args, nullptr, &err.GetReceiver());
    if (!result)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to send message: %s", err == nullptr ? "unknown error" : err->message);
    }
    ChipLogProgress(Controller, "WiFi-PAF: Outbound message (%zu) done", msgBuf->DataLength());

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoWiFiPAFWriteDone };
    memcpy(&event.CHIPoWiFiPAFReceived.SessionInfo, &TxInfo, sizeof(chip::WiFiPAF::WiFiPAFSession));
    event.CHIPoWiFiPAFReceived.result = result;
    PlatformMgr().PostEventOrDie(&event);
    return ret;
}

CHIP_ERROR ConnectivityManagerImpl::_WiFiPAFShutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    switch (role)
    {
    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Publisher:
        return _WiFiPAFCancelPublish(id);
    case WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber:
        return _WiFiPAFCancelSubscribe(id);
    }
    return CHIP_ERROR_INTERNAL;
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

// ExtendedData is optional, See core R1.4.2 5.4.2.6.3
// The 7 bytes are mandatory: <8-bits, Device OpCode>, <16-bits, Device Information>,
// <16-bits, Vendor ID>, <16-bits, Product ID>
#define PAF_MANDATORY_PUBLISH_LENGTH 7

static uint16_t FreqToBand(uint32_t freq)
{
    // 2.4 GHz: channels 1–13 (2412–2472 MHz) plus channel 14 (2484 MHz).
    if ((freq >= 2412 && freq <= 2472) || freq == 2484)
        return static_cast<uint16_t>(0x0001u); // k2g4
    // 5 GHz: main range plus the two additional channels above the main range.
    if ((freq >= 5035 && freq <= 5945) || freq == 5960 || freq == 5980)
        return static_cast<uint16_t>(0x0004u); // k5g
    return 0;
}

// Context for dispatching background-scan discovery results onto the CHIP event loop.
namespace {
struct BgScanWorkCtx
{
    chip::DeviceLayer::ConnectivityManagerImpl::BgScanDiscoveryCallback cb;
    void * cbCtx;
    chip::DeviceLayer::NanPeerInfo peer;
};
} // namespace

void ConnectivityManagerImpl::WiFiPAFDisconnectPublishReceiveHandler()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    if (!mWpaSupplicant.iface)
        return;

    // Disconnect all "nanreceive" handlers on the interface that were registered
    // with this ConnectivityManagerImpl as user-data.  This removes the handler
    // added by _WiFiPAFPublish so that a subsequent _WiFiPAFSubscribe call
    // registers exactly one handler and packets are not delivered twice.
    guint sig = g_signal_lookup("nanreceive", G_OBJECT_TYPE(mWpaSupplicant.iface.get()));
    g_signal_handlers_disconnect_matched(mWpaSupplicant.iface.get(),
                                         static_cast<GSignalMatchType>(G_SIGNAL_MATCH_ID | G_SIGNAL_MATCH_DATA),
                                         sig, 0, nullptr, nullptr, this);
}

void ConnectivityManagerImpl::DisconnectScanSignals()
{
    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    if (!mWpaSupplicant.iface)
        return;

    GObject * obj = G_OBJECT(mWpaSupplicant.iface.get());
    for (gulong & id : mScanSignalIds)
    {
        if (id != 0)
        {
            g_signal_handler_disconnect(obj, id);
            id = 0;
        }
    }
}

// Scan for Matter PAF devices, but don't connect
void ConnectivityManagerImpl::ScanDiscoveryResult(GVariant * discov_info)
{
    uint32_t peer_publish_id;
    uint8_t peer_addr[6];
    uint32_t srv_proto_type;
    uint32_t subscribe_id;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    if (g_variant_n_children(discov_info) == 0)
    {
        return;
    }

    // Read the data from dbus
    GAutoPtr<GVariant> dataValue;
    GVariant * value;

    value = g_variant_lookup_value(discov_info, "subscribe_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &subscribe_id);
    ChipLogDetail(DeviceLayer, "ScanDiscoveryResult: subscribe_id=%u", subscribe_id);

    // Ignore results from orphaned wpa_supplicant subscriptions (e.g. from
    // previous process runs that weren't cancelled). Only accept results from
    // the currently active scan subscription.
    {
        uint32_t expected = (mBgScanCb != nullptr) ? mBgScanSubscribeId : mActiveScanSubscribeId;
        if (expected == 0 || subscribe_id != expected)
        {
            ChipLogDetail(DeviceLayer, "ScanDiscoveryResult: ignoring stale subscribe_id=%u (expected=%u)",
                          subscribe_id, expected);
            return;
        }
    }

    value = g_variant_lookup_value(discov_info, "publish_id", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &peer_publish_id);
    char addr_str[20];
    char * paddr;
    value = g_variant_lookup_value(discov_info, "peer_addr", G_VARIANT_TYPE_STRING);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "&s", &paddr);
    strncpy(addr_str, paddr, sizeof(addr_str));
    sscanf(addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &peer_addr[0], &peer_addr[1], &peer_addr[2], &peer_addr[3],
           &peer_addr[4], &peer_addr[5]);
    value = g_variant_lookup_value(discov_info, "srv_proto_type", G_VARIANT_TYPE_UINT32);
    dataValue.reset(value);
    g_variant_get(dataValue.get(), "u", &srv_proto_type);

    // Read the variable length Extended data from the SSI, as per R1.5 Section 5.4.2.6.3
    size_t bufferLen;
    value = g_variant_lookup_value(discov_info, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);
    auto ssibuf      = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssibuf);

    NanPeerInfo peer;
    peer.vid           = pPublishSSI->VendorId;
    peer.pid           = pPublishSSI->ProductId;
    peer.opcode        = pPublishSSI->DevOpCode;
    peer.discriminator = pPublishSSI->DevInfo;
    std::memcpy(peer.mac, peer_addr, sizeof(peer_addr));
    if (bufferLen > PAF_MANDATORY_PUBLISH_LENGTH)
    {
        const auto * bytes = static_cast<const uint8_t *>(ssibuf);
        peer.storage.assign(bytes + PAF_MANDATORY_PUBLISH_LENGTH, bytes + bufferLen);
        peer.hasExtendedData = true;
    }

    // Set WiFiBand from the frequency used when the scan was started.
    peer.band = FreqToBand(mScanFreq);

    // The bg-scan callback uses SystemLayer and cluster APIs which require
    // the CHIP stack lock.  ScanDiscoveryResult runs on a GLib/dbus thread,
    // so schedule the callback on the CHIP event loop instead of calling directly.
    if (mBgScanCb != nullptr)
    {
        auto * workCtx      = new BgScanWorkCtx{ mBgScanCb, mBgScanCbCtx, peer };
        CHIP_ERROR schedErr = chip::DeviceLayer::PlatformMgr().ScheduleWork(
            +[](intptr_t arg) {
                auto * w = reinterpret_cast<BgScanWorkCtx *>(arg);
                w->cb(w->cbCtx, w->peer);
                delete w;
            },
            reinterpret_cast<intptr_t>(workCtx));
        if (schedErr != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ScanDiscoveryResult: ScheduleWork failed: %" CHIP_ERROR_FORMAT, schedErr.Format());
            delete workCtx;
        }
    }

    // Do not log if already present
    auto [it, inserted] = mNanScanPeers.insert(peer);
    if (!inserted)
    {
        return;
    }

    ChipLogProgress(DeviceLayer, "Discovered Device: %s() Subscribe_id:%u peer_publish_id:%u srv_proto_type:%u",
        __func__, subscribe_id, peer_publish_id, srv_proto_type);
    ChipLogProgress(DeviceLayer, "\tDiscriminator:%u Opcode:%u PID:0x%X VID:0x%X",
        pPublishSSI->DevInfo, pPublishSSI->DevOpCode, pPublishSSI->ProductId, pPublishSSI->VendorId);
    ChipLogProgress(DeviceLayer, "\tpeer_addr: [%02x:%02x:%02x:%02x:%02x:%02x]",
        peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5]);
    ChipLogProgress(DeviceLayer, "\tSSI Buffer Len:%lu", bufferLen);

    if (peer.hasExtendedData && !peer.storage.empty())
    {
        chip::ByteSpan s(peer.storage.data(), peer.storage.size());
        constexpr size_t kMaxBytesToPrint = 20;
        const size_t bytesToPrint         = (s.size() > kMaxBytesToPrint) ? kMaxBytesToPrint : s.size();
        char hexBuf[2 * kMaxBytesToPrint + 1] = { 0 };
        CHIP_ERROR err = chip::Encoding::BytesToHex(s.data(), bytesToPrint, hexBuf, sizeof(hexBuf),
                                                    chip::Encoding::HexFlags::kUppercase);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "\tExtendedData (%lu bytes, showing %lu): %s",
                            static_cast<unsigned long>(s.size()),
                            static_cast<unsigned long>(bytesToPrint),
                            hexBuf);
        }
        else
        {
            ChipLogError(DeviceLayer, "\tExtendedData, BytesToHex failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "\tNo Extended Data");
    }
}

void ConnectivityManagerImpl::ScanNanReceive(GVariant * obj)
{
    ChipLogError(DeviceLayer, "Commissioning Proxy: Unexpected ScanNanReceive");
}

void ConnectivityManagerImpl::ScanNanSubscribeTerminated(guint subscribe_id, gchar * reason)
{
    ChipLogProgress(DeviceLayer, "Commissioning Proxy: Subscription terminated (%u, %s)", subscribe_id, reason);
    WiFiPAFSession sessionInfo  = { .id = subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
}

CHIP_ERROR ConnectivityManagerImpl::WiFiPAFScan(uint8_t scanMaxTime, PafScanResultsCallback cb, void * cbContext)
{
    // Cannot start a one-shot scan while a background scan is running.
    VerifyOrReturnError(mBgScanCb == nullptr, CHIP_ERROR_BUSY);

    mScanCb        = cb;
    mScanCbContext = cbContext;

    CHIP_ERROR result = StartWiFiManagementSync();
    VerifyOrReturnError(result == CHIP_NO_ERROR, result);

    ChipLogProgress(DeviceLayer, "WiFiPAFScan: starting one-shot NAN discovery (maxTime=%us)", scanMaxTime);

    guint subscribe_id;
    GAutoPtr<GError> err;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    uint8_t is_active  = false; // passive subscribe = discovery mode
    unsigned int ttl   = CHIP_DEVICE_CONFIG_WIFIPAF_DISCOVERY_TIMEOUT_SECS;
    unsigned int freq  = (mApFreq == 0) ? CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL : mApFreq;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    mScanFreq = static_cast<uint32_t>(freq);
    GVariantBuilder builder;
    GVariant * args = nullptr;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "srv_name", g_variant_new_string(srv_name));
    g_variant_builder_add(&builder, "{sv}", "srv_proto_type", g_variant_new_byte(srv_proto_type));
    g_variant_builder_add(&builder, "{sv}", "active", g_variant_new_boolean(is_active));
    g_variant_builder_add(&builder, "{sv}", "ttl", g_variant_new_uint16(ttl));
    g_variant_builder_add(&builder, "{sv}", "freq", g_variant_new_uint16(freq));
    g_variant_builder_add(&builder, "{sv}", "discovery_only", g_variant_new_boolean(TRUE));
    args = g_variant_builder_end(&builder);
    wpa_supplicant_1_interface_call_nansubscribe_sync(mWpaSupplicant.iface.get(), args, &subscribe_id, nullptr, &err.GetReceiver());

    if (err.get() != nullptr)
    {
        ChipLogError(DeviceLayer, "WiFiPAFScan: nansubscribe failed: %s", err->message);
        ChipLogError(DeviceLayer, "WiFiPAFScan: Check wpa_supplicant supports discovery_only flag");
        mScanCb        = nullptr;
        mScanCbContext = nullptr;
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "WiFiPAFScan: subscribe_id=%u freq=%u", subscribe_id, freq);
    mActiveScanSubscribeId     = static_cast<uint32_t>(subscribe_id);
    WiFiPAFSession sessionInfo = { .role = WiFiPafRole::kWiFiPafRole_Subscriber, .id = subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    ReturnErrorOnFailure(WiFiPafLayer.AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo));
    auto pPafInfo = WiFiPafLayer.GetPAFInfo(PafInfoAccess::kAccSessionId, sessionInfo);
    if (pPafInfo != nullptr)
    {
        pPafInfo->id   = subscribe_id;
        pPafInfo->role = WiFiPAF::WiFiPafRole::kWiFiPafRole_Subscriber;
    }

    mScanSignalIds[0] = g_signal_connect(mWpaSupplicant.iface.get(), "nandiscovery-result",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->ScanDiscoveryResult(obj);
                     }),
                     this);
    mScanSignalIds[1] = g_signal_connect(mWpaSupplicant.iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->ScanNanReceive(obj);
                     }),
                     this);
    mScanSignalIds[2] = g_signal_connect(
        mWpaSupplicant.iface.get(), "nansubscribe-terminated",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_subscribe_id, gchar * reason, ConnectivityManagerImpl * self) {
            return self->ScanNanSubscribeTerminated(term_subscribe_id, reason);
        }),
        this);

    ChipLogProgress(DeviceLayer, "WiFiPAFScan: timeout in %d seconds on subscribe_id %u", scanMaxTime, subscribe_id);
    auto * ctx = new ScanTimerCtx{ this, subscribe_id };
    SystemLayer().StartTimer(System::Clock::Milliseconds32(scanMaxTime * 1000),
        +[](System::Layer *, void * context) {
            auto * timerCtx = static_cast<ScanTimerCtx *>(context);
            timerCtx->self->FinishWiFiPAFScan(timerCtx);
            delete timerCtx;
        }, ctx);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::FinishWiFiPAFScan(ScanTimerCtx * ctx)
{
    ChipLogProgress(DeviceLayer, "FinishWiFiPAFScan: subscribe_id %u", ctx->subscribe_id);
    DisconnectScanSignals();
    TEMPORARY_RETURN_IGNORED _WiFiPAFCancelSubscribe(ctx->subscribe_id);
    TEMPORARY_RETURN_IGNORED _WiFiPAFCancelIncompleteSubscribe();

    WiFiPAFSession sessionInfo  = { .role = WiFiPafRole::kWiFiPafRole_Subscriber, .id = ctx->subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    TEMPORARY_RETURN_IGNORED WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);

    std::vector<NanPeerInfo> results;
    results.reserve(mNanScanPeers.size());
    for (auto & p : mNanScanPeers)
        results.push_back(p);
    mNanScanPeers.clear();

    if (mScanCb != nullptr)
        mScanCb(mScanCbContext, results);

    mScanCb                = nullptr;
    mScanCbContext         = nullptr;
    mActiveScanSubscribeId = 0;
    mScanFreq              = 0;
}

CHIP_ERROR ConnectivityManagerImpl::WiFiPAFStartBackgroundScan(BgScanDiscoveryCallback cb, void * cbCtx)
{
    VerifyOrReturnError(cb != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mScanCb == nullptr, CHIP_ERROR_BUSY);

    // Already running — update callback and return success.
    if (mBgScanCb != nullptr)
    {
        mBgScanCb    = cb;
        mBgScanCbCtx = cbCtx;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR result = StartWiFiManagementSync();
    VerifyOrReturnError(result == CHIP_NO_ERROR, result);

    ChipLogProgress(DeviceLayer, "WiFiPAFStartBackgroundScan: starting continuous NAN discovery");

    guint subscribe_id;
    GAutoPtr<GError> err;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    uint8_t is_active  = false; // passive subscribe = discovery mode
    unsigned int ttl   = 0;    // 0 = infinite in wpa_supplicant
    unsigned int freq  = (mApFreq == 0) ? CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL : mApFreq;

    std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
    mScanFreq = static_cast<uint32_t>(freq);
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "srv_name", g_variant_new_string(srv_name));
    g_variant_builder_add(&builder, "{sv}", "srv_proto_type", g_variant_new_byte(srv_proto_type));
    g_variant_builder_add(&builder, "{sv}", "active", g_variant_new_boolean(is_active));
    g_variant_builder_add(&builder, "{sv}", "ttl", g_variant_new_uint16(ttl));
    g_variant_builder_add(&builder, "{sv}", "freq", g_variant_new_uint16(freq));
    g_variant_builder_add(&builder, "{sv}", "discovery_only", g_variant_new_boolean(TRUE));
    GVariant * args = g_variant_builder_end(&builder);
    wpa_supplicant_1_interface_call_nansubscribe_sync(mWpaSupplicant.iface.get(), args, &subscribe_id, nullptr,
                                                      &err.GetReceiver());

    if (err.get() != nullptr)
    {
        ChipLogError(DeviceLayer, "WiFiPAFStartBackgroundScan: nansubscribe failed: %s", err->message);
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(DeviceLayer, "WiFiPAFStartBackgroundScan: subscribe_id=%u freq=%u", subscribe_id, freq);

    WiFiPAFSession sessionInfo  = { .role = WiFiPafRole::kWiFiPafRole_Subscriber, .id = subscribe_id };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    ReturnErrorOnFailure(WiFiPafLayer.AddPafSession(PafInfoAccess::kAccSessionId, sessionInfo));

    mScanSignalIds[0] = g_signal_connect(mWpaSupplicant.iface.get(), "nandiscovery-result",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->ScanDiscoveryResult(obj);
                     }),
                     this);
    mScanSignalIds[1] = g_signal_connect(mWpaSupplicant.iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, ConnectivityManagerImpl * self) {
                         return self->ScanNanReceive(obj);
                     }),
                     this);
    mScanSignalIds[2] = g_signal_connect(
        mWpaSupplicant.iface.get(), "nansubscribe-terminated",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_subscribe_id, gchar * reason, ConnectivityManagerImpl * self) {
            return self->ScanNanSubscribeTerminated(term_subscribe_id, reason);
        }),
        this);

    mBgScanCb          = cb;
    mBgScanCbCtx       = cbCtx;
    mBgScanSubscribeId = static_cast<uint32_t>(subscribe_id);

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::WiFiPAFStopBackgroundScan()
{
    if (mBgScanCb == nullptr)
        return;

    ChipLogProgress(DeviceLayer, "WiFiPAFStopBackgroundScan: stopping subscribe_id=%u", mBgScanSubscribeId);

    DisconnectScanSignals();
    {
        std::lock_guard<std::mutex> lock(mWpaSupplicantMutex);
        mScanFreq = 0;
    }

    CHIP_ERROR cancelErr = _WiFiPAFCancelSubscribe(mBgScanSubscribeId);
    if (cancelErr != CHIP_NO_ERROR)
        ChipLogDetail(DeviceLayer, "WiFiPAFStopBackgroundScan: CancelSubscribe: %" CHIP_ERROR_FORMAT, cancelErr.Format());

    WiFiPAFSession sessionInfo  = { .role = WiFiPafRole::kWiFiPafRole_Subscriber, .id = mBgScanSubscribeId };
    WiFiPAFLayer & WiFiPafLayer = WiFiPAFLayer::GetWiFiPAFLayer();
    CHIP_ERROR rmErr            = WiFiPafLayer.RmPafSession(PafInfoAccess::kAccSessionId, sessionInfo);
    if (rmErr != CHIP_NO_ERROR)
        ChipLogDetail(DeviceLayer, "WiFiPAFStopBackgroundScan: RmPafSession: %" CHIP_ERROR_FORMAT, rmErr.Format());

    mBgScanCb          = nullptr;
    mBgScanCbCtx       = nullptr;
    mBgScanSubscribeId = 0;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONING_PROXY

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

} // namespace DeviceLayer
} // namespace chip
