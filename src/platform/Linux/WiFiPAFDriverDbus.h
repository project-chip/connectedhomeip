/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <mutex>
#include <platform/Linux/WpaDbusDefs.h>
#endif

#include <wifipaf/WiFiPAFEndPoint.h>
#include <wifipaf/WiFiPAFLayer.h>

#include <platform/Linux/WiFiPAFDriver.h>

namespace chip {
namespace DeviceLayer {

class WiFiPAFDriverDbus : public WiFiPAFDriver
{
public:
    WiFiPAFDriverDbus(GDBusWpaSupplicant * WpaSupplicant, std::mutex * WpaSupplicantMutex) :
        mWpaSupplicant(WpaSupplicant), mWpaSupplicantMutex(WpaSupplicantMutex)
    {}
    ~WiFiPAFDriverDbus() override {}
    CHIP_ERROR Initialize() override { return CHIP_NO_ERROR; };
    CHIP_ERROR Publish(std::unique_ptr<uint16_t[]> freq_list, uint16_t freq_list_len) override;
    CHIP_ERROR CancelPublish(uint32_t PublishId) override;
    CHIP_ERROR Subscribe(const uint16_t & connDiscriminator, uint16_t mApFreq) override;
    CHIP_ERROR CancelSubscribe(uint32_t SubscribeId) override;
    CHIP_ERROR Send(const WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msgBuf) override;
    CHIP_ERROR Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role) override;

private:
    GDBusWpaSupplicant * mWpaSupplicant;
    std::mutex * mWpaSupplicantMutex;
    void OnDiscoveryResult(GVariant * obj);
    void OnReplied(GVariant * obj);
    void OnNanReceive(GVariant * obj);
    void OnNanPublishTerminated(guint public_id, gchar * reason);
    void OnNanSubscribeTerminated(guint subscribe_id, gchar * reason);
};

} // namespace DeviceLayer
} // namespace chip
