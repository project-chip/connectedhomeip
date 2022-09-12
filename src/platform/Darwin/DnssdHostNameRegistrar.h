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
#include <vector>

namespace chip {
namespace Dnssd {

struct RegisterContext;
struct RegisterRecordContext;

class InterfaceRegistrant
{
public:
    /**
     * Initialize an interface if it match filtering arguments.
     *
     * @param[in] ifa         The interface to initialize from.
     * @param[in] addressType An allowed address type.
     *                        Passing in Inet::IPAddressType::kAny effectively allow all address types.
     * @param[in] interfaceId An allowed interface id.
     *                        Passing in kDNSServiceInterfaceIndexAny effectively allow all interface ids.
     *
     * @return A boolean indicating if the initialization from the data of the ifaddrs was allowed by the filtering parameters.
     */
    bool Init(const struct ifaddrs * ifa, Inet::IPAddressType addressType, uint32_t interfaceId);

    DNSServiceErrorType Register(DNSServiceRef sdRef, RegisterRecordContext * sdCtx, const char * hostname) const;

private:
    static bool HasValidFlags(unsigned int flags);
    static bool HasValidType(Inet::IPAddressType addressType, const sa_family_t addressFamily);
    static bool IsValidInterfaceId(uint32_t targetInterfaceId, unsigned int currentInterfaceId);

    const struct in_addr * GetIPv4() const { return &mInterfaceAddress.ipv4; }
    const struct in6_addr * GetIPv6() const { return &mInterfaceAddress.ipv6; }

    bool IsIPv4() const { return mInterfaceAddressType == AF_INET; };
    bool IsIPv6() const { return mInterfaceAddressType == AF_INET6; };

    void LogDetail() const;

    union InterfaceAddress
    {
        struct in_addr ipv4;
        struct in6_addr ipv6;
    };

    uint32_t mInterfaceId;
    InterfaceAddress mInterfaceAddress;
    sa_family_t mInterfaceAddressType;
};

class HostNameRegistrar
{
public:
    bool Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId);

    CHIP_ERROR Register(RegisterContext * registerCtx);

    /**
     * IncrementRegistrationCount is used to indicate a registrant status.
     * When all registrants status has been recovered, it returns false to indicate
     * that the registration process is finished.
     *
     * @param[in] err   The registration status
     *
     * @return          A boolean indicating if the registration process needs to continue.
     */
    bool IncrementRegistrationCount(DNSServiceErrorType err);

    bool HasRegisteredRegistrant() { return mRegistrationSuccess; }

private:
    std::string mHostName;
    std::vector<InterfaceRegistrant> mRegistry;
    uint8_t mRegistrationCount = 0;
    bool mRegistrationSuccess  = false;
};

} // namespace Dnssd
} // namespace chip
