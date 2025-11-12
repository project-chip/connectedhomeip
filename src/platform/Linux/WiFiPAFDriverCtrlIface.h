/*
 *
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

#pragma once

#include <mutex>
#include <unordered_map>

#include <glib.h>

#include <wifipaf/WiFiPAFEndPoint.h>
#include <wifipaf/WiFiPAFLayer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF_HOSTAPD
#define WIFIPAF_CTRL_IFACE_PATH_PREFIX "/var/run/hostapd/"
#else
#define WIFIPAF_CTRL_IFACE_PATH_PREFIX "/var/run/wpa_supplicant/"
#endif

#define WIFIPAF_CTRL_IFACE_PATH WIFIPAF_CTRL_IFACE_PATH_PREFIX CHIP_DEVICE_CONFIG_WIFI_STATION_IF_NAME

struct wpa_ctrl;

namespace chip {
namespace DeviceLayer {

class WiFiPAFDriverCtrlIface : public WiFiPAFDriver
{
public:
    ~WiFiPAFDriverCtrlIface() override;
    CHIP_ERROR Initialize() override;
    CHIP_ERROR Publish(std::unique_ptr<uint16_t[]> freq_list, uint16_t freq_list_len) override;
    CHIP_ERROR CancelPublish(uint32_t PublishId) override;
    CHIP_ERROR Subscribe(const uint16_t & connDiscriminator, uint16_t mApFreq) override;
    CHIP_ERROR CancelSubscribe(uint32_t SubscribeId) override;
    CHIP_ERROR Send(const WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msgBuf) override;
    CHIP_ERROR Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role) override;

private:
    const std::string kCtrlIfacePath = std::string(WIFIPAF_CTRL_IFACE_PATH);
    struct wpa_ctrl * ctrl;
    struct wpa_ctrl * cmd_ctrl;
    std::mutex mCtrlIfaceCmdMutex;
    GIOChannel * gio_channel;
    GSource * gsource;
    static gboolean OnWiFiManagerFdActivity(GIOChannel * source, GIOCondition condition, gpointer data);
    void OnDiscoveryResult(std::unordered_map<std::string, std::string> discov_info);
    void OnReplied(std::unordered_map<std::string, std::string> reply_info);
    void OnNanReceive(std::unordered_map<std::string, std::string> received_data);
    void OnNanPublishTerminated(std::unordered_map<std::string, std::string> publish_terminated_data);
    void OnNanSubscribeTerminated(std::unordered_map<std::string, std::string> subscribe_terminated_data);
};

} // namespace DeviceLayer
} // namespace chip
