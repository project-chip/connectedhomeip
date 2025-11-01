/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/ConnectivityManager.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <algorithm>
#include <cstdlib>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <credentials/CHIPCert.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include <platform/Linux/WiFiPAFDriver.h>
#include <platform/Linux/WiFiPAFDriverDbus.h>
#include <wifipaf/WiFiPAFLayer.h>

using namespace ::chip::DeviceLayer;
using namespace ::chip::WiFiPAF;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR WiFiPAFDriverDbus::Publish(std::unique_ptr<uint16_t[]> freq_list, uint16_t freq_list_len)
{
    GAutoPtr<GError> err;
    guint publish_id;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_MAX_ADVERTISING_TIMEOUT_SECS;
    unsigned int freq                             = CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL;
    unsigned int ssi_len                          = sizeof(struct PAFPublishSSI);

    // Add the freq_list:
    GVariant * freq_array_variant =
        g_variant_new_fixed_array(G_VARIANT_TYPE_UINT16, freq_list.get(), freq_list_len, sizeof(guint16));
    if (freq_array_variant == nullptr)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: freq_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }

    // Construct the SSI
    PAFPublishSSI PafPublish_ssi = BuildSSI();
    VerifyOrDie(DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(PafPublish_ssi.DevInfo) == CHIP_NO_ERROR);

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
    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);
    wpa_supplicant_1_interface_call_nanpublish_sync(mWpaSupplicant->iface.get(), args, &publish_id, nullptr, &err.GetReceiver());

    g_signal_connect(
        mWpaSupplicant->iface.get(), "nanreplied",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, WiFiPAFDriverDbus * self) { return self->OnReplied(obj); }),
        this);

    g_signal_connect(mWpaSupplicant->iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, WiFiPAFDriverDbus * self) {
                         return self->OnNanReceive(obj);
                     }),
                     this);
    g_signal_connect(mWpaSupplicant->iface.get(), "nanpublish-terminated",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_publish_id, gchar * reason,
                                    WiFiPAFDriverDbus * self) { return self->OnNanPublishTerminated(term_publish_id, reason); }),
                     this);

    ChipLogProgress(DeviceLayer, "WiFi-PAF: publish_id: [%u], freq: %u", publish_id, freq);

    return _Publish(publish_id);
}

CHIP_ERROR WiFiPAFDriverDbus::CancelPublish(uint32_t PublishId)
{
    GAutoPtr<GError> err;

    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel publish_id: %d ! ", PublishId);
    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);

    VerifyOrReturnError(mWpaSupplicant->iface, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "WiFi-PAF: Skip D-Bus 'cancel publish' call since wpa_supplicant is not ready"));

    gboolean result =
        wpa_supplicant_1_interface_call_nancancel_publish_sync(mWpaSupplicant->iface.get(), PublishId, nullptr, &err.GetReceiver());

    // TODO #40814: make sure that the callers do check the return values. This doesn't seem to be happening now.
    VerifyOrReturnError(
        result, CHIP_ERROR_INTERNAL,
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to Cancel Publish with Error: %s", err ? err->message : "unknown error"));

    return CHIP_NO_ERROR;
}

/*
    NAN-USD Service Protocol Type: ref: Table 58 of Wi-Fi Aware Specificaiton
*/
void WiFiPAFDriverDbus::OnDiscoveryResult(GVariant * discov_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnDiscoveryResult");
    uint32_t subscribe_id;
    uint32_t peer_publish_id;
    uint8_t peer_addr[6];
    uint32_t srv_proto_type;

    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);
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
    size_t bufferLen;
    value = g_variant_lookup_value(discov_info, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);
    auto ssibuf      = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssibuf);

    std::array<uint8_t, 6> peer_addr_array = { peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5] };

    _OnDiscoveryResult(subscribe_id, peer_publish_id, srv_proto_type, peer_addr_array, pPublishSSI);
}

void WiFiPAFDriverDbus::OnReplied(GVariant * reply_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnReplied");
    uint32_t publish_id;
    uint32_t peer_subscribe_id;
    uint8_t peer_addr[6];
    uint32_t srv_proto_type;

    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);
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
    size_t bufferLen;
    value = g_variant_lookup_value(reply_info, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);
    auto ssibuf      = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssibuf);

    std::array<uint8_t, 6> peer_addr_array = { peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5] };

    _OnReplied(peer_subscribe_id, publish_id, srv_proto_type, peer_addr_array, pPublishSSI);
}

void WiFiPAFDriverDbus::OnNanReceive(GVariant * obj)
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
    value = g_variant_lookup_value(obj, "ssi", G_VARIANT_TYPE_BYTESTRING);
    dataValue.reset(value);

    size_t bufferLen;
    auto rxbuf = g_variant_get_fixed_array(dataValue.get(), &bufferLen, sizeof(uint8_t));

    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(rxbuf, bufferLen);

    _OnNanReceive(RxInfo, std::move(buf));
}

void WiFiPAFDriverDbus::OnNanPublishTerminated(guint public_id, gchar * reason)
{
    _OnNanPublishTerminated(public_id, reason);
}

void WiFiPAFDriverDbus::OnNanSubscribeTerminated(guint subscribe_id, gchar * reason)
{
    _OnNanSubscribeTerminated(subscribe_id, reason);
}

CHIP_ERROR WiFiPAFDriverDbus::Subscribe(const uint16_t & connDiscriminator, uint16_t mApFreq)
{
    ChipLogProgress(Controller, "WiFi-PAF: Try to subscribe the NAN-USD devices");

    guint subscribe_id;
    GAutoPtr<GError> err;
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    uint8_t is_active                             = 1;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_DISCOVERY_TIMEOUT_SECS;
    unsigned int freq                             = (mApFreq == 0) ? CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL : mApFreq;
    unsigned int ssi_len                          = sizeof(struct PAFPublishSSI);

    PAFPublishSSI PafPublish_ssi = BuildSSI();
    PafPublish_ssi.DevInfo       = connDiscriminator;

    GVariant * ssi_array_variant = g_variant_new_fixed_array(G_VARIANT_TYPE_BYTE, &PafPublish_ssi, ssi_len, sizeof(guint8));
    if (ssi_array_variant == nullptr)
    {
        ChipLogProgress(DeviceLayer, "WiFi-PAF: ssi_array_variant is NULL ");
        return CHIP_ERROR_INTERNAL;
    }

    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);
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
    wpa_supplicant_1_interface_call_nansubscribe_sync(mWpaSupplicant->iface.get(), args, &subscribe_id, nullptr,
                                                      &err.GetReceiver());

    g_signal_connect(mWpaSupplicant->iface.get(), "nandiscovery-result",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, WiFiPAFDriverDbus * self) {
                         return self->OnDiscoveryResult(obj);
                     }),
                     this);

    g_signal_connect(mWpaSupplicant->iface.get(), "nanreceive",
                     G_CALLBACK(+[](WpaSupplicant1Interface * proxy, GVariant * obj, WiFiPAFDriverDbus * self) {
                         return self->OnNanReceive(obj);
                     }),
                     this);

    g_signal_connect(
        mWpaSupplicant->iface.get(), "nansubscribe-terminated",
        G_CALLBACK(+[](WpaSupplicant1Interface * proxy, guint term_subscribe_id, gchar * reason, WiFiPAFDriverDbus * self) {
            return self->OnNanSubscribeTerminated(term_subscribe_id, reason);
        }),
        this);

    ChipLogProgress(DeviceLayer, "WiFi-PAF: subscribe_id: [%u], freq: %u", subscribe_id, freq);

    return _Subscribe(subscribe_id, connDiscriminator);
}

CHIP_ERROR WiFiPAFDriverDbus::CancelSubscribe(uint32_t SubscribeId)
{
    GAutoPtr<GError> err;

    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel subscribe_id: %d ! ", SubscribeId);
    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);
    wpa_supplicant_1_interface_call_nancancel_subscribe_sync(mWpaSupplicant->iface.get(), SubscribeId, nullptr, &err.GetReceiver());
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFDriverDbus::Send(const WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msgBuf)
{
    ChipLogProgress(Controller, "WiFi-PAF: sending PAF Follow-up packets, (%lu)", msgBuf->DataLength());

    if (msgBuf.IsNull())
    {
        ChipLogError(Controller, "WiFi-PAF: Invalid Packet (%lu)", msgBuf->DataLength());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Ensure outgoing message fits in a single contiguous packet buffer, as currently required by the
    // message fragmentation and reassembly engine.
    if (msgBuf->HasChainedBuffer())
    {
        msgBuf->CompactHead();
        if (msgBuf->HasChainedBuffer())
        {
            ChipLogError(Controller, "WiFi-PAF: Outbound message too big (%lu), skip temporally", msgBuf->DataLength());
            return CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
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
    std::lock_guard<std::mutex> lock(*mWpaSupplicantMutex);

    GVariantBuilder builder;
    GVariant * args = nullptr;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);
    g_variant_builder_add(&builder, "{sv}", "handle", g_variant_new_uint32(TxInfo.id));
    g_variant_builder_add(&builder, "{sv}", "req_instance_id", g_variant_new_uint32(TxInfo.peer_id));
    g_variant_builder_add(&builder, "{sv}", "peer_addr", g_variant_new_string(peer_mac));
    g_variant_builder_add(&builder, "{sv}", "ssi", ssi_array_variant);
    args = g_variant_builder_end(&builder);
    gboolean result =
        wpa_supplicant_1_interface_call_nantransmit_sync(mWpaSupplicant->iface.get(), args, nullptr, &err.GetReceiver());
    if (!result)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to send message: %s", err == nullptr ? "unknown error" : err->message);
    }
    ChipLogProgress(Controller, "WiFi-PAF: Outbound message (%lu) done", msgBuf->DataLength());

    return _Send(TxInfo, result);
}

CHIP_ERROR WiFiPAFDriverDbus::Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    return _Shutdown(id, role);
}

} // namespace DeviceLayer
} // namespace chip
