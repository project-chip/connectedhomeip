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

#include "DnssdHostNameRegistrar.h"
#include "DnssdImpl.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <netdb.h>

constexpr DNSServiceFlags kRegisterRecordFlags = kDNSServiceFlagsShared;

namespace chip {
namespace Dnssd {

namespace {

static void OnRegisterRecord(DNSServiceRef sdRef, DNSRecordRef recordRef, DNSServiceFlags flags, DNSServiceErrorType err,
                             void * context)
{
    ChipLogDetail(Discovery, "Mdns: %s flags: %d", __func__, flags);

    auto sdCtx = reinterpret_cast<RegisterRecordContext *>(context);

    auto & registrar = sdCtx->mRegisterContext->mHostNameRegistrar;
    if (!registrar.IncrementRegistrationCount(err))
    {
        sdCtx->Finalize(registrar.HasRegisteredRegistrant() ? kDNSServiceErr_NoError : kDNSServiceErr_Unknown);
    }
}

} // namespace

bool InterfaceRegistrant::Init(const struct ifaddrs * ifa, Inet::IPAddressType addressType, uint32_t interfaceId)
{
    VerifyOrReturnValue(ifa != nullptr, false);
    VerifyOrReturnValue(ifa->ifa_addr != nullptr, false);
    VerifyOrReturnValue(HasValidFlags(ifa->ifa_flags), false);
    VerifyOrReturnValue(IsValidInterfaceId(interfaceId, if_nametoindex(ifa->ifa_name)), false);
    VerifyOrReturnValue(HasValidType(addressType, ifa->ifa_addr->sa_family), false);

    // The incoming interface id can be kDNSServiceInterfaceIndexAny, but here what needs to be done is to
    // associate a given ip with a specific interface id, so the incoming interface id is used as a hint
    // to validate if the current interface is of interest or not, but it is not what is stored internally.
    mInterfaceId          = if_nametoindex(ifa->ifa_name);
    mInterfaceAddressType = ifa->ifa_addr->sa_family;

    if (IsIPv4())
    {
        mInterfaceAddress.ipv4 = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr)->sin_addr;
    }
    else
    {
        mInterfaceAddress.ipv6 = reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr)->sin6_addr;
    }

    return true;
}

DNSServiceErrorType InterfaceRegistrant::Register(DNSServiceRef sdRef, RegisterRecordContext * sdCtx, const char * hostname) const
{
    LogDetail();

    uint16_t rrtype    = IsIPv4() ? kDNSServiceType_A : kDNSServiceType_AAAA;
    uint16_t rdlen     = IsIPv4() ? sizeof(in_addr) : sizeof(in6_addr);
    const void * rdata = IsIPv4() ? static_cast<const void *>(GetIPv4()) : static_cast<const void *>(GetIPv6());

    DNSRecordRef dnsRecordRef;
    return DNSServiceRegisterRecord(sdRef, &dnsRecordRef, kRegisterRecordFlags, mInterfaceId, hostname, rrtype, kDNSServiceClass_IN,
                                    rdlen, rdata, 0, OnRegisterRecord, sdCtx);
}

bool InterfaceRegistrant::HasValidFlags(unsigned int flags)
{
    return !(flags & IFF_POINTOPOINT) && (flags & IFF_RUNNING) && (flags & IFF_MULTICAST);
}

bool InterfaceRegistrant::HasValidType(Inet::IPAddressType addressType, const sa_family_t addressFamily)
{
    bool useIPv4 = addressType == Inet::IPAddressType::kIPv4 || addressType == Inet::IPAddressType::kAny;
    bool useIPv6 = addressType == Inet::IPAddressType::kIPv6 || addressType == Inet::IPAddressType::kAny;

    return (useIPv6 && addressFamily == AF_INET6) || (useIPv4 && addressFamily == AF_INET);
}

bool InterfaceRegistrant::IsValidInterfaceId(uint32_t targetInterfaceId, unsigned int currentInterfaceId)
{
    return targetInterfaceId == kDNSServiceInterfaceIndexAny || targetInterfaceId == currentInterfaceId;
}

void InterfaceRegistrant::LogDetail() const
{
    char interfaceName[IFNAMSIZ] = {};
    VerifyOrReturn(if_indextoname(mInterfaceId, interfaceName) != nullptr);

    if (IsIPv4())
    {
        char addr[INET_ADDRSTRLEN] = {};
        inet_ntop(AF_INET, GetIPv4(), addr, sizeof(addr));
        ChipLogDetail(Discovery, "\tInterface: %s (%u) ipv4: %s", interfaceName, mInterfaceId, addr);
    }
    else
    {
        char addr[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, GetIPv6(), addr, sizeof(addr));
        ChipLogDetail(Discovery, "\tInterface: %s (%u) ipv6: %s", interfaceName, mInterfaceId, addr);
    }
}

bool HostNameRegistrar::Init(const char * hostname, Inet::IPAddressType addressType, uint32_t interfaceId)
{
    mHostName = hostname;

    // When the target interface is kDNSServiceInterfaceIndexLocalOnly, there is no need to map the hostname to
    // an IP address.
    VerifyOrReturnValue(interfaceId != kDNSServiceInterfaceIndexLocalOnly, true);

    struct ifaddrs * ifap;
    VerifyOrReturnValue(getifaddrs(&ifap) >= 0, false);

    for (struct ifaddrs * ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        InterfaceRegistrant registrant;
        if (registrant.Init(ifa, addressType, interfaceId))
        {
            mRegistry.push_back(registrant);
        }
    }

    freeifaddrs(ifap);

    return mRegistry.size() != 0;
}

CHIP_ERROR HostNameRegistrar::Register(RegisterContext * registerCtx)
{
    // If the target interface is kDNSServiceInterfaceIndexLocalOnly, there are no interfaces to register against
    // the dns daemon.
    if (mRegistry.size() == 0)
    {
        return registerCtx->Finalize();
    }

    DNSServiceRef sdRef;
    auto err = DNSServiceCreateConnection(&sdRef);
    VerifyOrReturnError(kDNSServiceErr_NoError == err, CHIP_ERROR_INTERNAL);

    RegisterRecordContext * sdCtx = Platform::New<RegisterRecordContext>(registerCtx);
    VerifyOrReturnError(nullptr != sdCtx, CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(Discovery, "Mdns: Mapping %s to:", mHostName.c_str());

    for (auto & registrant : mRegistry)
    {
        err = registrant.Register(sdRef, sdCtx, mHostName.c_str());

        // An error to register a particular registrant is not fatal unless all registrants have failed.
        // Otherwise, if there is an errror, it indicates that this registrant will not trigger a callback
        // with its registration status. So we take the registration failure into account here.
        if (kDNSServiceErr_NoError != err)
        {
            VerifyOrReturnError(IncrementRegistrationCount(err), sdCtx->Finalize(err));
        }
    }

    return MdnsContexts::GetInstance().Add(sdCtx, sdRef);
}

bool HostNameRegistrar::IncrementRegistrationCount(DNSServiceErrorType err)
{
    mRegistrationCount++;
    VerifyOrDie(mRegistrationCount <= mRegistry.size());

    // A single registration success is enough to consider the whole process a success.
    // This is very permissive in the sense that the interface that has succeeded may not be
    // enough to do anything useful, but on the other hand, the failure of a single interface
    // to successfuly registered does not makes it obvious that it won't work.
    if (kDNSServiceErr_NoError == err)
    {
        mRegistrationSuccess = true;
    }

    return mRegistrationCount != mRegistry.size();
}

} // namespace Dnssd
} // namespace chip
