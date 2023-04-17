/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include "DnssdImpl.h"
#include "MdnsError.h"

#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::Dnssd;

namespace {

constexpr uint8_t kDnssdKeyMaxSize          = 32;
constexpr uint8_t kDnssdTxtRecordMaxEntries = 20;

std::string GetHostNameWithoutDomain(const char * hostnameWithDomain)
{
    std::string hostname(hostnameWithDomain);
    size_t position = hostname.find(".");
    if (position != std::string::npos)
    {
        hostname.erase(position);
    }

    return hostname;
}

std::string GetFullTypeWithoutSubTypes(std::string fullType)
{
    size_t position = fullType.find(",");
    if (position != std::string::npos)
    {
        fullType.erase(position);
    }

    return fullType;
}

void GetTextEntries(DnssdService & service, const unsigned char * data, uint16_t len)
{
    uint16_t recordCount   = TXTRecordGetCount(len, data);
    service.mTextEntrySize = recordCount;
    service.mTextEntries   = static_cast<TextEntry *>(chip::Platform::MemoryCalloc(kDnssdTxtRecordMaxEntries, sizeof(TextEntry)));

    for (uint16_t i = 0; i < recordCount; i++)
    {
        char key[kDnssdKeyMaxSize];
        uint8_t valueLen;
        const void * valuePtr;

        auto err = TXTRecordGetItemAtIndex(len, data, i, kDnssdKeyMaxSize, key, &valueLen, &valuePtr);
        if (kDNSServiceErr_NoError != err)
        {
            // If there is an error with a txt record stop the parsing here.
            service.mTextEntrySize = i;
            break;
        }

        if (valueLen >= chip::Dnssd::kDnssdTextMaxSize)
        {
            // Truncation, but nothing better we can do
            valueLen = chip::Dnssd::kDnssdTextMaxSize - 1;
        }

        char value[chip::Dnssd::kDnssdTextMaxSize];
        memcpy(value, valuePtr, valueLen);
        value[valueLen] = 0;

        auto & textEntry    = service.mTextEntries[i];
        textEntry.mKey      = strdup(key);
        textEntry.mData     = reinterpret_cast<const uint8_t *>(strdup(value));
        textEntry.mDataSize = valueLen;
    }
}

DNSServiceProtocol GetProtocol(const chip::Inet::IPAddressType & addressType)
{
#if INET_CONFIG_ENABLE_IPV4
    if (addressType == chip::Inet::IPAddressType::kIPv4)
    {
        return kDNSServiceProtocol_IPv4;
    }

    if (addressType == chip::Inet::IPAddressType::kIPv6)
    {
        return kDNSServiceProtocol_IPv6;
    }

    return kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6;
#else
    // without IPv4, IPv6 is the only option
    return kDNSServiceProtocol_IPv6;
#endif
}

} // namespace

namespace chip {
namespace Dnssd {

CHIP_ERROR GenericContext::FinalizeInternal(const char * errorStr, CHIP_ERROR err)
{
    if (MdnsContexts::GetInstance().Has(this) == CHIP_NO_ERROR)
    {
        if (CHIP_NO_ERROR == err)
        {
            DispatchSuccess();
        }
        else
        {
            DispatchFailure(errorStr, err);
        }
    }
    else
    {
        chip::Platform::Delete(this);
    }

    return err;
}

CHIP_ERROR GenericContext::Finalize(CHIP_ERROR err)
{
    return FinalizeInternal(err.AsString(), err);
}

CHIP_ERROR GenericContext::Finalize(DNSServiceErrorType err)
{
    return FinalizeInternal(Error::ToString(err), Error::ToChipError(err));
}

MdnsContexts::~MdnsContexts()
{
    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        Delete(*iter);
        mContexts.erase(iter);
    }
}

CHIP_ERROR MdnsContexts::Add(GenericContext * context, DNSServiceRef sdRef)
{
    VerifyOrReturnError(context != nullptr || sdRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (context == nullptr)
    {
        DNSServiceRefDeallocate(sdRef);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (sdRef == nullptr)
    {
        chip::Platform::Delete(context);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    auto err = DNSServiceSetDispatchQueue(sdRef, chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue());
    if (kDNSServiceErr_NoError != err)
    {
        // We can't just use our Delete to deallocate the service ref here,
        // because our context may not have its serviceRef set yet.
        DNSServiceRefDeallocate(sdRef);
        chip::Platform::Delete(context);
        return Error::ToChipError(err);
    }

    context->serviceRef = sdRef;
    mContexts.push_back(context);

    return CHIP_NO_ERROR;
}

bool MdnsContexts::RemoveWithoutDeleting(GenericContext * context)
{
    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if (*iter != context)
        {
            iter++;
            continue;
        }

        mContexts.erase(iter);
        return true;
    }

    return false;
}

CHIP_ERROR MdnsContexts::Remove(GenericContext * context)
{
    bool found = RemoveWithoutDeleting(context);
    if (found)
    {
        Delete(context);
    }
    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::RemoveAllOfType(ContextType type)
{
    bool found = false;

    std::vector<GenericContext *>::const_iterator iter = mContexts.cbegin();
    while (iter != mContexts.cend())
    {
        if ((*iter)->type != type)
        {
            iter++;
            continue;
        }

        Delete(*iter);
        mContexts.erase(iter);
        found = true;
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

void MdnsContexts::Delete(GenericContext * context)
{
    if (context->serviceRef != nullptr)
    {
        DNSServiceRefDeallocate(context->serviceRef);
    }
    chip::Platform::Delete(context);
}

CHIP_ERROR MdnsContexts::Has(GenericContext * context)
{
    std::vector<GenericContext *>::iterator iter;

    for (iter = mContexts.begin(); iter != mContexts.end(); iter++)
    {
        if ((*iter) == context)
        {
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR MdnsContexts::GetRegisterContextOfType(const char * type, RegisterContext ** context)
{
    bool found = false;
    std::vector<GenericContext *>::iterator iter;

    for (iter = mContexts.begin(); iter != mContexts.end(); iter++)
    {
        if ((*iter)->type == ContextType::Register && (static_cast<RegisterContext *>(*iter))->matches(type))
        {
            *context = static_cast<RegisterContext *>(*iter);
            found    = true;
            break;
        }
    }

    return found ? CHIP_NO_ERROR : CHIP_ERROR_KEY_NOT_FOUND;
}

ResolveContext * MdnsContexts::GetExistingResolveForInstanceName(const char * instanceName)
{
    for (auto & ctx : mContexts)
    {
        if (ctx->type == ContextType::Resolve && (static_cast<ResolveContext *>(ctx))->Matches(instanceName))
        {
            return static_cast<ResolveContext *>(ctx);
        }
    }

    return nullptr;
}

RegisterContext::RegisterContext(const char * sType, const char * instanceName, DnssdPublishCallback cb, void * cbContext)
{
    type     = ContextType::Register;
    context  = cbContext;
    callback = cb;

    mType         = sType;
    mInstanceName = instanceName;
}

void RegisterContext::DispatchFailure(const char * errorStr, CHIP_ERROR err)
{
    ChipLogError(Discovery, "Mdns: Register failure (%s)", errorStr);
    callback(context, nullptr, nullptr, err);
    MdnsContexts::GetInstance().Remove(this);
}

void RegisterContext::DispatchSuccess()
{
    std::string typeWithoutSubTypes = GetFullTypeWithoutSubTypes(mType);
    callback(context, typeWithoutSubTypes.c_str(), mInstanceName.c_str(), CHIP_NO_ERROR);

    // Once a service has been properly published it is normally unreachable because the hostname has not yet been
    // registered against the dns daemon. Register the records mapping the hostname to our IP.
    mHostNameRegistrar.Register();
}

BrowseContext * BrowseContext::sContextDispatchingSuccess = nullptr;

BrowseContext::BrowseContext(void * cbContext, DnssdBrowseCallback cb, DnssdServiceProtocol cbContextProtocol)
{
    type     = ContextType::Browse;
    context  = cbContext;
    callback = cb;
    protocol = cbContextProtocol;
}

void BrowseContext::DispatchFailure(const char * errorStr, CHIP_ERROR err)
{
    ChipLogError(Discovery, "Mdns: Browse failure (%s)", errorStr);
    callback(context, nullptr, 0, true, err);
    MdnsContexts::GetInstance().Remove(this);
}

void BrowseContext::DispatchSuccess()
{
    // This should never be called: We either DispatchPartialSuccess or
    // DispatchFailure.
    VerifyOrDie(false);
}

void BrowseContext::DispatchPartialSuccess()
{
    sContextDispatchingSuccess = this;
    callback(context, services.data(), services.size(), false, CHIP_NO_ERROR);
    sContextDispatchingSuccess = nullptr;
    services.clear();
}

ResolveContext::ResolveContext(void * cbContext, DnssdResolveCallback cb, chip::Inet::IPAddressType cbAddressType,
                               const char * instanceNameToResolve, BrowseContext * browseCausingResolve,
                               std::shared_ptr<uint32_t> && consumerCounterToUse) :
    browseThatCausedResolve(browseCausingResolve)
{
    type            = ContextType::Resolve;
    context         = cbContext;
    callback        = cb;
    protocol        = GetProtocol(cbAddressType);
    instanceName    = instanceNameToResolve;
    consumerCounter = std::move(consumerCounterToUse);
}

ResolveContext::~ResolveContext() {}

void ResolveContext::DispatchFailure(const char * errorStr, CHIP_ERROR err)
{
    ChipLogError(Discovery, "Mdns: Resolve failure (%s)", errorStr);
    // Remove before dispatching, so calls back into
    // ChipDnssdResolveNoLongerNeeded don't find us and try to also remove us.
    bool needDelete = MdnsContexts::GetInstance().RemoveWithoutDeleting(this);

    callback(context, nullptr, Span<Inet::IPAddress>(), err);

    if (needDelete)
    {
        MdnsContexts::GetInstance().Delete(this);
    }
}

void ResolveContext::DispatchSuccess()
{
    // Remove before dispatching, so calls back into
    // ChipDnssdResolveNoLongerNeeded don't find us and try to also remove us.
    bool needDelete = MdnsContexts::GetInstance().RemoveWithoutDeleting(this);

    for (auto & interface : interfaces)
    {
        auto & ips = interface.second.addresses;

        // Some interface may not have any ips, just ignore them.
        if (ips.size() == 0)
        {
            continue;
        }

        ChipLogProgress(Discovery, "Mdns: Resolve success on interface %" PRIu32, interface.first);
        callback(context, &interface.second.service, Span<Inet::IPAddress>(ips.data(), ips.size()), CHIP_NO_ERROR);
        break;
    }

    if (needDelete)
    {
        MdnsContexts::GetInstance().Delete(this);
    }
}

CHIP_ERROR ResolveContext::OnNewAddress(uint32_t interfaceId, const struct sockaddr * address)
{
    // If we don't have any information about this interfaceId, just ignore the
    // address, since it won't be usable anyway without things like the port.
    // This can happen if "local" is set up as a search domain in the DNS setup
    // on the system, because the hostnames we are looking up all end in
    // ".local".  In other words, we can get regular DNS results in here, not
    // just DNS-SD ones.
    if (interfaces.find(interfaceId) == interfaces.end())
    {
        return CHIP_NO_ERROR;
    }

    chip::Inet::IPAddress ip;
    ReturnErrorOnFailure(chip::Inet::IPAddress::GetIPAddressFromSockAddr(*address, ip));
    interfaces[interfaceId].addresses.push_back(ip);

#ifdef CHIP_PROGRESS_LOGGING
    char addrStr[INET6_ADDRSTRLEN];
    ip.ToString(addrStr, sizeof(addrStr));
    ChipLogProgress(Discovery, "Mdns: %s interface: %" PRIu32 " ip:%s", __func__, interfaceId, addrStr);
#endif // CHIP_PROGRESS_LOGGING

    return CHIP_NO_ERROR;
}

CHIP_ERROR ResolveContext::OnNewLocalOnlyAddress()
{
    sockaddr_in6 sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin6_len    = sizeof(sockaddr);
    sockaddr.sin6_family = AF_INET6;
    sockaddr.sin6_addr   = in6addr_loopback;
    sockaddr.sin6_port   = htons((unsigned short) interfaces[kDNSServiceInterfaceIndexLocalOnly].service.mPort);

    return OnNewAddress(kDNSServiceInterfaceIndexLocalOnly, reinterpret_cast<struct sockaddr *>(&sockaddr));
}

bool ResolveContext::HasAddress()
{
    for (auto & interface : interfaces)
    {
        if (interface.second.addresses.size())
        {
            return true;
        }
    }

    return false;
}

void ResolveContext::OnNewInterface(uint32_t interfaceId, const char * fullname, const char * hostnameWithDomain, uint16_t port,
                                    uint16_t txtLen, const unsigned char * txtRecord)
{
#if CHIP_PROGRESS_LOGGING
    std::string txtString;
    auto txtRecordIter  = txtRecord;
    size_t remainingLen = txtLen;
    while (remainingLen > 0)
    {
        size_t len = *txtRecordIter;
        ++txtRecordIter;
        --remainingLen;
        len = min(len, remainingLen);
        chip::Span<const unsigned char> bytes(txtRecordIter, len);
        if (txtString.size() > 0)
        {
            txtString.push_back(',');
        }
        for (auto & byte : bytes)
        {
            if ((std::isalnum(byte) || std::ispunct(byte)) && byte != '\\' && byte != ',')
            {
                txtString.push_back(static_cast<char>(byte));
            }
            else
            {
                char hex[5];
                snprintf(hex, sizeof(hex), "\\x%02x", byte);
                txtString.append(hex);
            }
        }
        txtRecordIter += len;
        remainingLen -= len;
    }
#endif // CHIP_PROGRESS_LOGGING
    ChipLogProgress(Discovery, "Mdns : %s hostname:%s fullname:%s interface: %" PRIu32 " port: %u TXT:\"%s\"", __func__,
                    hostnameWithDomain, fullname, interfaceId, ntohs(port), txtString.c_str());

    InterfaceInfo interface;
    interface.service.mPort = ntohs(port);

    if (kDNSServiceInterfaceIndexLocalOnly == interfaceId)
    {
        // Set interface to ANY (0) - network stack can decide how to route this.
        interface.service.mInterface = Inet::InterfaceId(0);
    }
    else
    {
        interface.service.mInterface = Inet::InterfaceId(interfaceId);
    }

    // The hostname parameter contains the hostname followed by the domain. But the mHostName field is sized
    // to contain either a 12 bytes mac address or an extended address of at most 16 bytes, not the domain name.
    auto hostname = GetHostNameWithoutDomain(hostnameWithDomain);
    Platform::CopyString(interface.service.mHostName, hostname.c_str());
    Platform::CopyString(interface.service.mName, fullname);

    GetTextEntries(interface.service, txtRecord, txtLen);

    // If for some reason the hostname can not fit into the hostname field (e.g it is not a mac address) then
    // DNSServiceGetAddrInfo will never return anything. So instead, copy the name as the FQDN and use it for
    // resolving.
    interface.fullyQualifiedDomainName = hostnameWithDomain;

    interfaces.insert(std::make_pair(interfaceId, std::move(interface)));
}

bool ResolveContext::HasInterface()
{
    return interfaces.size();
}

InterfaceInfo::InterfaceInfo()
{
    service.mTextEntrySize = 0;
    service.mTextEntries   = nullptr;
}

InterfaceInfo::InterfaceInfo(InterfaceInfo && other) :
    service(std::move(other.service)), addresses(std::move(other.addresses)),
    fullyQualifiedDomainName(std::move(other.fullyQualifiedDomainName))
{
    // Make sure we're not trying to free any state from the other DnssdService,
    // since we took over ownership of its allocated bits.
    other.service.mTextEntrySize = 0;
    other.service.mTextEntries   = nullptr;
}

InterfaceInfo::~InterfaceInfo()
{
    if (service.mTextEntries == nullptr)
    {
        return;
    }

    const size_t count = service.mTextEntrySize;
    for (size_t i = 0; i < count; i++)
    {
        const auto & textEntry = service.mTextEntries[i];
        free(const_cast<char *>(textEntry.mKey));
        free(const_cast<uint8_t *>(textEntry.mData));
    }
    Platform::MemoryFree(const_cast<TextEntry *>(service.mTextEntries));
}

} // namespace Dnssd
} // namespace chip
