/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
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
        ~HostNameRegistrar();

        DNSServiceErrorType Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId);

        CHIP_ERROR Register();
        void Unregister();

    private:
        bool IsLocalOnly() const { return mInterfaceId == kDNSServiceInterfaceIndexLocalOnly; };

        template <typename T> void RegisterInterfaces(std::vector<std::pair<uint32_t, T>> interfaces, uint16_t type)
        {
            for (auto & interface : interfaces) {
                auto interfaceId = interface.first;

                LogErrorOnFailure(RegisterInterface(interfaceId, interface.second, type));
            }
        }

        template <typename T> CHIP_ERROR RegisterInterface(uint32_t interfaceId, const T & interfaceAddress, uint16_t type)
        {
            auto interfaceAddressLen = sizeof(interfaceAddress);

            return RegisterInterface(interfaceId, type, &interfaceAddress, static_cast<uint16_t>(interfaceAddressLen));
        }

        CHIP_ERROR RegisterInterface(uint32_t interfaceId, uint16_t rtype, const void * rdata, uint16_t rdlen);

        CHIP_ERROR StartSharedConnection();
        void StopSharedConnection();
        CHIP_ERROR ResetSharedConnection();

        CHIP_ERROR StartMonitorInterfaces(OnInterfaceChanges interfaceChangesBlock);
        void StopMonitorInterfaces();

        DNSServiceRef mServiceRef = nullptr;
        nw_path_monitor_t mInterfaceMonitor = nullptr;

        std::string mHostname;
        // Default to kDNSServiceInterfaceIndexLocalOnly so we don't mess around
        // with un-registration if we never get Init() called.
        uint32_t mInterfaceId = kDNSServiceInterfaceIndexLocalOnly;
        Inet::IPAddressType mAddressType;

        // We use mLivenessTracker to indicate to blocks that close over us that
        // we've been destroyed.  This is needed because we're not a refcounted
        // object, so the blocks can't keep us alive; they just close over the
        // raw pointer to "this".
        std::shared_ptr<bool> mLivenessTracker;
    };

} // namespace Dnssd
} // namespace chip
