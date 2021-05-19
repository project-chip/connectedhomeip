/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPCore.h>
#include <stack/Stack.h>
#include <transport/raw/TCP.h>

constexpr size_t kMaxPayloadSize            = 1280;
constexpr size_t kResponseTimeOut           = 1000;
constexpr chip::Transport::AdminId gAdminId = 0;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT

class TransportConfigurationWithTcp : chip::TransportConfiguration
{
public:
    static constexpr size_t kMaxTcpActiveConnectionCount = 4;
    static constexpr size_t kMaxTcpPendingPackets        = 4;

    using transport = chip::TransportMgr<
#if INET_CONFIG_ENABLE_IPV4
        chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>, chip::Transport::UDP,
#endif
        chip::Transport::TCP<kMaxTcpActiveConnectionCount, kMaxTcpPendingPackets>, chip::Transport::UDP>;

    CHIP_ERROR Init(const chip::StackParameters & parameters, chip::Inet::InetLayer & inetLayer, chip::Ble::BleLayer * bleLayer)
    {
        return mTransportManager.Init(
#if INET_CONFIG_ENABLE_IPV4
            chip::Transport::TcpListenParameters(&inetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4).SetListenPort(parameters.GetListenPort()),
            chip::Transport::UdpListenParameters(&inetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv4).SetListenPort(parameters.GetListenPort()),
#endif
            chip::Transport::TcpListenParameters(&inetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6).SetListenPort(parameters.GetListenPort()),
            chip::Transport::UdpListenParameters(&inetLayer).SetAddressType(chip::Inet::kIPAddressType_IPv6).SetListenPort(parameters.GetListenPort()));
    }

    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    chip::TransportMgrBase & Get() { return mTransportManager; }

private:
    transport mTransportManager;
};

extern chip::Stack<TransportConfigurationWithTcp> gStack;
#else
extern chip::Stack<> gStack;
#endif
