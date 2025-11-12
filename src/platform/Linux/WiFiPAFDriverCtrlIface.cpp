/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    Copyright (c) 2025 NXP
 *    Copyright (c) 2025 Cable Television Laboratories, Inc.
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
#include <iomanip>
#include <new>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <charconv>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

extern "C" {
#include "wpa_ctrl.h"
}

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <credentials/CHIPCert.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>
#endif

#include <platform/Linux/WiFiPAFDriver.h>
#include <platform/Linux/WiFiPAFDriverCtrlIface.h>
#include <wifipaf/WiFiPAFLayer.h>

using namespace ::chip::DeviceLayer;
using namespace ::chip::WiFiPAF;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR StringToUInt32(std::string input, uint32_t & output)
{
    auto [ptr, ec] = std::from_chars(input.data(), input.data() + input.size(), output);

    if (ec != std::errc())
    {
        ChipLogError(DeviceLayer, "Error converting std::string %s to uint32_t", input.c_str());
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

std::string GetEventType(const std::string & reply)
{
    auto pos = reply.find('>');
    if (pos == std::string::npos)
    {
        return "";
    }

    auto space = reply.find(' ', pos + 1);

    std::string event;
    if (space == std::string::npos)
    {
        event = reply.substr(pos + 1);
    }
    else
    {
        event = reply.substr(pos + 1, space - (pos + 1));
    }

    return event;
}

std::unordered_map<std::string, std::string> GetKeyValueFromWpaCtrlReply(const std::string & reply)
{
    std::regex re(R"((\w+)=([^\s]+))");
    std::smatch match;

    std::unordered_map<std::string, std::string> kv;

    auto begin = std::sregex_iterator(reply.begin(), reply.end(), re);
    auto end   = std::sregex_iterator();

    for (auto it = begin; it != end; ++it)
    {
        kv[(*it)[1]] = (*it)[2];
    }

    return kv;
}

WiFiPAFDriverCtrlIface::~WiFiPAFDriverCtrlIface()
{
    wpa_ctrl_close(ctrl);
    wpa_ctrl_close(cmd_ctrl);
}

gboolean WiFiPAFDriverCtrlIface::OnWiFiManagerFdActivity(GIOChannel * source, GIOCondition condition, gpointer data)
{
    ChipLogProgress(DeviceLayer, "Activity on WiFiPAFDriver file descriptor");

    auto self = static_cast<WiFiPAFDriverCtrlIface *>(data);

    while (wpa_ctrl_pending(self->ctrl) != 0)
    {
        char reply[4096];
        size_t reply_len = sizeof(reply);
        if (wpa_ctrl_recv(self->ctrl, reply, &reply_len) != 0)
        {
            ChipLogProgress(DeviceLayer, "Error on wpa_ctrl_recv");
            continue;
        }
        auto reply_str = std::string(reply, reply_len);
        ChipLogProgress(DeviceLayer, "Received: %s", reply_str.c_str());

        auto event_str = GetEventType(reply_str);
        if ("NAN-DISCOVERY-RESULT" == event_str)
        {
            self->OnDiscoveryResult(GetKeyValueFromWpaCtrlReply(reply_str));
        }
        else if ("NAN-REPLIED" == event_str)
        {
            self->OnReplied(GetKeyValueFromWpaCtrlReply(reply_str));
        }
        else if ("NAN-RECEIVE" == event_str)
        {
            self->OnNanReceive(GetKeyValueFromWpaCtrlReply(reply_str));
        }
        else if ("NAN-PUBLISH-TERMINATED" == event_str)
        {
            self->OnNanPublishTerminated(GetKeyValueFromWpaCtrlReply(reply_str));
        }
        else if ("NAN-SUBSCRIBE-TERMINATED" == event_str)
        {
            self->OnNanSubscribeTerminated(GetKeyValueFromWpaCtrlReply(reply_str));
        }
        else
        {
            ChipLogProgress(Controller, "Unknown wpa_ctrl event: %s", event_str.c_str());
        }
    }
    return TRUE;
}

CHIP_ERROR WiFiPAFDriverCtrlIface::Initialize()
{
    ChipLogProgress(DeviceLayer, "Using iface path: %s", kCtrlIfacePath.c_str());
    ctrl = wpa_ctrl_open(kCtrlIfacePath.c_str());

    auto fd               = wpa_ctrl_get_fd(ctrl);
    GIOChannel * ch       = g_io_channel_unix_new(fd);
    GSource * watchSource = g_io_create_watch(ch, G_IO_IN);
    g_source_set_callback(watchSource, G_SOURCE_FUNC(OnWiFiManagerFdActivity), (gpointer) this, nullptr);
    g_io_channel_set_close_on_unref(ch, TRUE);
    g_io_channel_set_encoding(ch, nullptr, nullptr);
    PlatformMgrImpl().GLibMatterContextAttachSource(watchSource);

    gio_channel = ch;
    gsource     = watchSource;

    if (wpa_ctrl_attach(ctrl) != 0)
    {
        ChipLogProgress(DeviceLayer, "Error attaching to control interface");
        return CHIP_ERROR_INTERNAL;
    }

    cmd_ctrl = wpa_ctrl_open(kCtrlIfacePath.c_str());

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFDriverCtrlIface::Publish(std::unique_ptr<uint16_t[]> freq_list, uint16_t freq_list_len)
{
    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_MAX_ADVERTISING_TIMEOUT_SECS;
    unsigned int freq                             = CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL;

    // Construct the SSI
    PAFPublishSSI PafPublish_ssi = BuildSSI();
    VerifyOrDie(DeviceLayer::GetCommissionableDataProvider()->GetSetupDiscriminator(PafPublish_ssi.DevInfo) == CHIP_NO_ERROR);

    std::ostringstream oss;
    oss << "NAN_PUBLISH "
        << "service_name=" << srv_name << " srv_proto_type=" << srv_proto_type << " ttl=" << ttl << " freq=" << freq;

    oss << " ssi=" << std::hex << std::setfill('0');
    uint8_t * ssi_array = reinterpret_cast<uint8_t *>(&PafPublish_ssi);
    for (size_t i = 0; i < sizeof(PafPublish_ssi); i++)
    {
        oss << std::setw(2) << static_cast<int>(ssi_array[i]);
    }

    oss << " freq_list=";
    for (size_t i = 0; i < freq_list_len; ++i)
    {
        if (i > 0)
        {
            oss << ",";
        }
        oss << freq_list[i];
    }

    std::string cmd_str = oss.str();
    const char * cmd    = cmd_str.c_str();
    size_t cmd_len      = std::strlen(cmd);

    char reply[4096];
    size_t reply_len = sizeof(reply);

    std::lock_guard<std::mutex> lock(mCtrlIfaceCmdMutex);
    if (wpa_ctrl_request(cmd_ctrl, cmd, cmd_len, reply, &reply_len, nullptr) != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: error sending NAN-PUBLISH");
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t publish_id;
    ReturnErrorOnFailure(StringToUInt32(reply, publish_id));

    ChipLogProgress(DeviceLayer, "WiFi-PAF: publish_id: [%u], freq: %u", publish_id, freq);

    CHIP_ERROR ret = _Publish(publish_id);

    return ret;
}

CHIP_ERROR WiFiPAFDriverCtrlIface::CancelPublish(uint32_t PublishId)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel publish_id: %d ! ", PublishId);

    std::ostringstream oss;
    oss << "NAN_CANCEL_PUBLISH publish_id=" << PublishId;
    std::string cmd_str = oss.str();
    const char * cmd    = cmd_str.c_str();
    size_t cmd_len      = std::strlen(cmd);

    char reply[4096];
    size_t reply_len = sizeof(reply);

    std::lock_guard<std::mutex> lock(mCtrlIfaceCmdMutex);
    if (wpa_ctrl_request(cmd_ctrl, cmd, cmd_len, reply, &reply_len, nullptr) != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to send message");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

/*
    NAN-USD Service Protocol Type: ref: Table 58 of Wi-Fi Aware Specificaiton
*/
void WiFiPAFDriverCtrlIface::OnDiscoveryResult(std::unordered_map<std::string, std::string> discov_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnDiscoveryResult");

    uint32_t subscribe_id;
    uint32_t peer_publish_id;
    uint32_t srv_proto_type;

    ReturnOnFailure(StringToUInt32(discov_info["subscribe_id"], subscribe_id));
    ReturnOnFailure(StringToUInt32(discov_info["publish_id"], peer_publish_id));
    ReturnOnFailure(StringToUInt32(discov_info["srv_proto_type"], srv_proto_type));

    std::string peer_addr_str = discov_info["address"];
    uint8_t peer_addr[6];
    sscanf(peer_addr_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &peer_addr[0], &peer_addr[1], &peer_addr[2], &peer_addr[3],
           &peer_addr[4], &peer_addr[5]);
    std::array<uint8_t, 6> peer_addr_array = { peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5] };

    std::string ssi_str = discov_info["ssi"];
    size_t ssi_buf_len  = ssi_str.size() / 2;
    std::vector<uint8_t> ssi_buf;
    ssi_buf.resize(ssi_buf_len);
    if (chip::Encoding::HexToBytes(ssi_str.c_str(), ssi_str.size(), ssi_buf.data(), ssi_buf_len) != ssi_buf_len)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: error converting SSI to byte array");
        return;
    }
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssi_buf.data());

    _OnDiscoveryResult(subscribe_id, peer_publish_id, srv_proto_type, peer_addr_array, pPublishSSI);
}

void WiFiPAFDriverCtrlIface::OnReplied(std::unordered_map<std::string, std::string> reply_info)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnReplied");

    uint32_t peer_subscribe_id;
    uint32_t publish_id;
    uint32_t srv_proto_type;

    ReturnOnFailure(StringToUInt32(reply_info["subscribe_id"], peer_subscribe_id));
    ReturnOnFailure(StringToUInt32(reply_info["publish_id"], publish_id));
    ReturnOnFailure(StringToUInt32(reply_info["srv_proto_type"], srv_proto_type));

    std::string peer_addr_str = reply_info["address"];
    uint8_t peer_addr[6];
    sscanf(peer_addr_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &peer_addr[0], &peer_addr[1], &peer_addr[2], &peer_addr[3],
           &peer_addr[4], &peer_addr[5]);

    std::string ssi_str = reply_info["ssi"];
    size_t ssi_buf_len  = ssi_str.size() / 2;
    std::vector<uint8_t> ssi_buf;
    ssi_buf.resize(ssi_buf_len);
    if (chip::Encoding::HexToBytes(ssi_str.c_str(), ssi_str.size(), ssi_buf.data(), ssi_buf_len) != ssi_buf_len)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: error converting SSI to byte array");
        return;
    }
    auto pPublishSSI = reinterpret_cast<const PAFPublishSSI *>(ssi_buf.data());

    std::array<uint8_t, 6> peer_addr_array = { peer_addr[0], peer_addr[1], peer_addr[2], peer_addr[3], peer_addr[4], peer_addr[5] };

    _OnReplied(peer_subscribe_id, publish_id, srv_proto_type, peer_addr_array, pPublishSSI);
}

void WiFiPAFDriverCtrlIface::OnNanReceive(std::unordered_map<std::string, std::string> received_data)
{
    ChipLogProgress(Controller, "WiFi-PAF: OnNanReceive");

    WiFiPAF::WiFiPAFSession RxInfo;
    ReturnOnFailure(StringToUInt32(received_data["id"], RxInfo.id));
    ReturnOnFailure(StringToUInt32(received_data["peer_instance_id"], RxInfo.peer_id));

    std::string peer_addr_str = received_data["address"];
    sscanf(peer_addr_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &RxInfo.peer_addr[0], &RxInfo.peer_addr[1], &RxInfo.peer_addr[2],
           &RxInfo.peer_addr[3], &RxInfo.peer_addr[4], &RxInfo.peer_addr[5]);

    std::string ssi_str = received_data["ssi"];
    size_t ssi_buf_len  = ssi_str.size() / 2;
    std::vector<uint8_t> ssi_buf;
    ssi_buf.resize(ssi_buf_len);
    if (chip::Encoding::HexToBytes(ssi_str.c_str(), ssi_str.size(), ssi_buf.data(), ssi_buf_len) != ssi_buf_len)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: error converting SSI to byte array");
        return;
    }

    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(ssi_buf.data(), ssi_buf_len);

    _OnNanReceive(RxInfo, std::move(buf));
}

void WiFiPAFDriverCtrlIface::OnNanPublishTerminated(std::unordered_map<std::string, std::string> publish_terminated_data)
{
    uint32_t publish_id;
    ReturnOnFailure(StringToUInt32(publish_terminated_data["publish_id"], publish_id));

    _OnNanPublishTerminated(publish_id, publish_terminated_data["reason"].c_str());
}

void WiFiPAFDriverCtrlIface::OnNanSubscribeTerminated(std::unordered_map<std::string, std::string> subscribe_terminated_data)
{
    uint32_t subscribe_id;
    ReturnOnFailure(StringToUInt32(subscribe_terminated_data["subscribe_id"], subscribe_id));

    _OnNanSubscribeTerminated(subscribe_id, subscribe_terminated_data["reason"].c_str());
}

CHIP_ERROR WiFiPAFDriverCtrlIface::Subscribe(const uint16_t & connDiscriminator, uint16_t mApFreq)
{
    ChipLogProgress(Controller, "WiFi-PAF: Try to subscribe the NAN-USD devices");

    enum nan_service_protocol_type srv_proto_type = nan_service_protocol_type::NAN_SRV_PROTO_CSA_MATTER;
    uint32_t is_active                            = 1;
    unsigned int ttl                              = CHIP_DEVICE_CONFIG_WIFIPAF_DISCOVERY_TIMEOUT_SECS;
    unsigned int freq                             = (mApFreq == 0) ? CHIP_DEVICE_CONFIG_WIFIPAF_24G_DEFAUTL_CHNL : mApFreq;
    PAFPublishSSI PafPublish_ssi                  = BuildSSI();
    PafPublish_ssi.DevInfo                        = connDiscriminator;

    std::ostringstream oss;

    oss << "NAN_SUBSCRIBE "
        << "service_name=" << srv_name << " srv_proto_type=" << srv_proto_type << " active=" << is_active << " ttl=" << ttl
#if !CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF_HOSTAPD
        << " freq=" << freq
#endif
        << " ssi=" << std::hex << std::setfill('0') // hex output, zero-padded
        << std::setw(2) << static_cast<int>(PafPublish_ssi.DevOpCode) << std::setw(4) << htons(PafPublish_ssi.DevInfo)
        << std::setw(4) << htons(PafPublish_ssi.ProductId) << std::setw(4) << htons(PafPublish_ssi.VendorId);

    std::string cmd_str = oss.str();
    const char * cmd    = cmd_str.c_str();
    size_t cmd_len      = std::strlen(cmd);

    char reply[4096];
    size_t reply_len = sizeof(reply);

    std::lock_guard<std::mutex> lock(mCtrlIfaceCmdMutex);

    if (wpa_ctrl_request(cmd_ctrl, cmd, cmd_len, reply, &reply_len, nullptr) != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: error sending NAN-SUBSCRIBE");
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t subscribe_id;
    ReturnErrorOnFailure(StringToUInt32(reply, subscribe_id));

    ChipLogProgress(DeviceLayer, "WiFi-PAF: subscribe_id: [%u], freq: %u", subscribe_id, freq);

    return _Subscribe(subscribe_id, connDiscriminator);
}

CHIP_ERROR WiFiPAFDriverCtrlIface::CancelSubscribe(uint32_t SubscribeId)
{
    ChipLogProgress(DeviceLayer, "WiFi-PAF: cancel subscribe_id: %d ! ", SubscribeId);

    std::ostringstream oss;
    oss << "NAN_CANCEL_SUBSCRIBE subscribe_id=" << SubscribeId;
    std::string cmd_str = oss.str();
    const char * cmd    = cmd_str.c_str();
    size_t cmd_len      = std::strlen(cmd);

    char reply[4096];
    size_t reply_len = sizeof(reply);

    std::lock_guard<std::mutex> lock(mCtrlIfaceCmdMutex);
    if (wpa_ctrl_request(cmd_ctrl, cmd, cmd_len, reply, &reply_len, nullptr) != 0)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to send message");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiPAFDriverCtrlIface::Send(const WiFiPAF::WiFiPAFSession & TxInfo, System::PacketBufferHandle && msgBuf)
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
    char peer_mac[18];
    snprintf(peer_mac, sizeof(peer_mac), "%02x:%02x:%02x:%02x:%02x:%02x", TxInfo.peer_addr[0], TxInfo.peer_addr[1],
             TxInfo.peer_addr[2], TxInfo.peer_addr[3], TxInfo.peer_addr[4], TxInfo.peer_addr[5]);

    std::vector<char> ssi_hex_buffer(msgBuf->DataLength() * 2 + 1);
    BitFlags<Encoding::HexFlags> flags(Encoding::HexFlags::kNone);
    ReturnErrorOnFailure(
        chip::Encoding::BytesToHex(msgBuf->Start(), msgBuf->DataLength(), ssi_hex_buffer.data(), ssi_hex_buffer.size(), flags));
    auto ssi_str = std::string(ssi_hex_buffer.data());

    std::ostringstream oss;
    oss << "NAN_TRANSMIT "
        << "handle=" << TxInfo.id << " req_instance_id=" << TxInfo.peer_id << " address=" << peer_mac << " ssi=" << ssi_str;

    std::string cmd_str = oss.str();
    const char * cmd    = cmd_str.c_str();
    size_t cmd_len      = std::strlen(cmd);

    char reply[4096];
    size_t reply_len = sizeof(reply);

    std::lock_guard<std::mutex> lock(mCtrlIfaceCmdMutex);
    bool result = wpa_ctrl_request(cmd_ctrl, cmd, cmd_len, reply, &reply_len, nullptr) == 0;

    if (!result)
    {
        ChipLogError(DeviceLayer, "WiFi-PAF: Failed to send message");
    }
    ChipLogProgress(Controller, "WiFi-PAF: Outbound message (%lu) done", msgBuf->DataLength());

    CHIP_ERROR ret = _Send(TxInfo, result);

    return ret;
}

CHIP_ERROR WiFiPAFDriverCtrlIface::Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role)
{
    return _Shutdown(id, role);
}

} // namespace DeviceLayer
} // namespace chip
