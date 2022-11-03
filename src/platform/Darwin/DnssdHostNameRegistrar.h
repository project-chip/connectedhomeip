/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <dns_sd.h>
#include <lib/dnssd/platform/Dnssd.h>

#include <string>
#include <vector>

#include <Network/Network.h>

namespace chip {
namespace Dnssd {

    typedef std::pair<uint32_t, in_addr> InetInterface;
    typedef std::pair<uint32_t, in6_addr> Inet6Interface;
    typedef std::vector<InetInterface> InetInterfacesVector;
    typedef std::vector<std::pair<uint32_t, in6_addr>> Inet6InterfacesVector;
    typedef void (^OnInterfaceChanges)(InetInterfacesVector inetInterfaces, Inet6InterfacesVector inet6Interfaces);

    class HostNameRegistrar {
    public:
        void Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId);

        CHIP_ERROR Register();
        void Unregister();

    private:
        bool IsLocalOnly() const { return mInterfaceId == kDNSServiceInterfaceIndexLocalOnly; };

        template <typename T> void RegisterInterfaces(std::vector<std::pair<uint32_t, T>> interfaces, uint16_t type)
        {
            for (auto & interface : interfaces) {
                auto interfaceId = interface.first;
                auto interfaceAddress = static_cast<const void *>(&interface.second);
                auto interfaceAddressLen = sizeof(interface.second);

                LogErrorOnFailure(RegisterInterface(interfaceId, type, interfaceAddress, interfaceAddressLen));
            }
        }

        CHIP_ERROR RegisterInterface(uint32_t interfaceId, uint16_t rtype, const void * rdata, uint16_t rdlen);

        CHIP_ERROR StartSharedConnection();
        void StopSharedConnection();
        CHIP_ERROR ResetSharedConnection();
        DNSServiceRef mServiceRef;

        CHIP_ERROR StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock);
        void StopMonitorInterfaces();
        nw_path_monitor_t mInterfaceMonitor;

        std::string mHostname;
        uint32_t mInterfaceId;
        Inet::IPAddressType mAddressType;
    };

} // namespace Dnssd
} // namespace chip
