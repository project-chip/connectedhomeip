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

#include <wifipaf/WiFiPAFEndPoint.h>
#include <wifipaf/WiFiPAFLayer.h>

namespace chip {
namespace DeviceLayer {

const char srv_name[] = "_matterc._udp";
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

class WiFiPAFDriver
{
public:
    virtual ~WiFiPAFDriver() {}
    virtual CHIP_ERROR Initialize()                                                                             = 0;
    virtual CHIP_ERROR Publish(std::unique_ptr<uint16_t[]> freq_list, uint16_t freq_list_len)                   = 0;
    virtual CHIP_ERROR CancelPublish(uint32_t PublishId)                                                        = 0;
    virtual CHIP_ERROR Subscribe(const uint16_t & connDiscriminator, uint16_t mApFreq)                          = 0;
    virtual CHIP_ERROR CancelSubscribe(uint32_t SubscribeId)                                                    = 0;
    virtual CHIP_ERROR Send(const WiFiPAF::WiFiPAFSession & TxInfo, chip::System::PacketBufferHandle && msgBuf) = 0;
    virtual CHIP_ERROR Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role)                                         = 0;

protected:
    static PAFPublishSSI BuildSSI();
    CHIP_ERROR _Publish(uint32_t publish_id);
    CHIP_ERROR _Subscribe(uint32_t subscribe_id, uint16_t discriminator);
    void _OnDiscoveryResult(uint32_t subscribe_id, uint32_t peer_publish_id, uint32_t srv_proto_type,
                            std::array<uint8_t, 6> peer_addr, const PAFPublishSSI * pPublishSSI);
    void _OnReplied(uint32_t peer_subscribe_id, uint32_t publish_id, uint32_t srv_proto_type, std::array<uint8_t, 6> peer_addr,
                    const PAFPublishSSI * pPublishSSI);
    void _OnNanReceive(const WiFiPAF::WiFiPAFSession & rxInfo, System::PacketBufferHandle rxData);
    void _OnNanPublishTerminated(uint32_t publish_id, const char * reason);
    void _OnNanSubscribeTerminated(uint32_t subscribe_id, const char * reason);
    CHIP_ERROR _Shutdown(uint32_t id, WiFiPAF::WiFiPafRole role);
    CHIP_ERROR _Send(const WiFiPAF::WiFiPAFSession & TxInfo, bool result);
};
} // namespace DeviceLayer
} // namespace chip
