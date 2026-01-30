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
    if ((pPafInfo->id == subscribe_id) && (pPafInfo->peer_id != UINT32_MAX))
    {
        // Reentrance, depends on wpa_supplicant behaviors
        ChipLogError(DeviceLayer, "WiFi-PAF: DiscoveryResult, reentrance, subscribe_id: %u ", subscribe_id);
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

    g_signal_connect(mWpaSupplicant.iface.get(), "nandiscovery-result",
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

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF

} // namespace DeviceLayer
} // namespace chip
