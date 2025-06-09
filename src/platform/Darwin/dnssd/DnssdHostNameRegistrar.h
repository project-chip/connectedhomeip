/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <dns_sd.h>
#include <lib/dnssd/platform/Dnssd.h>

#include <string>

#include <platform/Darwin/inet/NetworkMonitor.h>

namespace chip {
namespace Dnssd {

    typedef void (^OnRegisterRecordCallback)(DNSServiceErrorType error);

    class HostNameRegistrar : public Inet::Darwin::NetworkMonitor {
    public:
        ~HostNameRegistrar();

        DNSServiceErrorType Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId);

        CHIP_ERROR Register(OnRegisterRecordCallback callback);
        void Unregister();

    private:
        template <typename T>
        void RegisterInterfaces(std::vector<std::pair<nw_interface_t, T>> interfaces, uint16_t type)
        {
            for (auto & interface : interfaces) {
                auto interfaceId = nw_interface_get_index(interface.first);

                LogErrorOnFailure(RegisterInterface(interfaceId, interface.second, type));
            }
        }

        template <typename T>
        CHIP_ERROR RegisterInterface(uint32_t interfaceId, const T & interfaceAddress, uint16_t type)
        {
            auto interfaceAddressLen = sizeof(interfaceAddress);

            return RegisterInterface(interfaceId, type, &interfaceAddress, static_cast<uint16_t>(interfaceAddressLen));
        }

        CHIP_ERROR RegisterInterface(uint32_t interfaceId, uint16_t rtype, const void * rdata, uint16_t rdlen);

        CHIP_ERROR StartSharedConnection();
        void StopSharedConnection();
        CHIP_ERROR ResetSharedConnection();

        DNSServiceRef mServiceRef = nullptr;
        std::string mHostname;

        static void OnRegisterRecord(DNSServiceRef sdRef, DNSRecordRef recordRef, DNSServiceFlags flags, DNSServiceErrorType err, void * context);
        OnRegisterRecordCallback mOnRegisterRecordCallback = nullptr;
    };

} // namespace Dnssd
} // namespace chip
