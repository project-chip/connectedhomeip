/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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

#include "ethernetif.h"
#include "lwip/netifapi.h"

#include <platform/NetworkCommissioning.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
class NxpEthDriver final : public EthernetDriver
{
public:
    class EthernetNetworkIterator final : public NetworkIterator
    {
    public:
        EthernetNetworkIterator(NxpEthDriver * aDriver) : mDriver(aDriver) {}
        size_t Count() { return 1; }
        bool Next(Network & item) override
        {
            if (exhausted)
            {
                return false;
            }
            exhausted = true;
            memcpy(item.networkID, interfaceName, interfaceNameLen);
            item.networkIDLen = interfaceNameLen;
            item.connected    = true;
            return true;
        }
        void Release() override { delete this; }
        ~EthernetNetworkIterator() = default;

        uint8_t interfaceName[kMaxNetworkIDLen];
        uint8_t interfaceNameLen = 0;
        bool exhausted           = false;

    private:
        NxpEthDriver * mDriver;
    };

    netif * GetEthInetIf() { return &netif_app; };

    // BaseDriver
    NetworkIterator * GetNetworks() override { return new EthernetNetworkIterator(this); };
    uint8_t GetMaxNetworks() { return 1; }
    CHIP_ERROR Init(NetworkStatusChangeCallback * networkStatusChangeCallback) override;
    void Shutdown()
    {
        // TODO: This method can be implemented if Ethernet is used along with Wifi/Thread.
    }

    static NxpEthDriver & Instance()
    {
        static NxpEthDriver instance;
        return instance;
    }

private:
    static void print_ip_addresses(struct netif * netif);
    static void eth_netif_ext_status_callback(struct netif * netif, netif_nsc_reason_t reason,
                                              const netif_ext_callback_args_t * args);
    phy_handle_t phyHandle;
    struct netif netif_app;
};

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
