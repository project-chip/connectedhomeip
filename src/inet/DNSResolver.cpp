/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements DNSResolver, the object that abstracts
 *      Domain Name System (DNS) resolution in InetLayer.
 *
 */

#include <inet/DNSResolver.h>

#include <inet/InetLayer.h>
#include <inet/InetLayerEvents.h>

#include <support/CodeUtils.h>

#include <string.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/dns.h>
#include <lwip/init.h>
#include <lwip/tcpip.h>

#if LWIP_VERSION_MAJOR <= 2
#define LWIP_DNS_FOUND_CALLBACK_TYPE dns_found_callback
#endif // LWIP_VERSION_MAJOR < 2
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace chip {
namespace Inet {

chip::System::ObjectPool<DNSResolver, INET_CONFIG_NUM_DNS_RESOLVERS> DNSResolver::sPool;

/**
 *  This method revolves a host name into a list of IP addresses.
 *
 *  @note
 *     Even if the operation completes successfully,
 *     the result might be a zero-length list of IP addresses.
 *     Most of the error generated are returned via the
 *     application callback.
 *
 *  @param[in]  hostName    A pointer to a C string representing the host name
 *                          to be queried.
 *  @param[in]  hostNameLen The string length of host name.
 *  @param[in]  maxAddrs    The maximum number of addresses to store in the DNS
 *                          table.
 *  @param[in]  options     An integer value controlling how host name address
 *                          resolution is performed.  Values are from the #DNSOptions
 *                          enumeration.
 *  @param[in]  addrArray   A pointer to the DNS table.
 *  @param[in]  onComplete  A pointer to the callback function when a DNS
 *                          request is complete.
 *  @param[in]  appState    A pointer to the application state to be passed to
 *                          onComplete when a DNS request is complete.
 *
 *  @retval INET_NO_ERROR                   if a DNS request is handled
 *                                          successfully.
 *
 *  @retval INET_ERROR_NOT_IMPLEMENTED      if DNS resolution is not enabled on
 *                                          the underlying platform.
 *
 *  @retval _other_                         if other POSIX network or OS error
 *                                          was returned by the underlying DNS
 *                                          resolver implementation.
 *
 */
INET_ERROR DNSResolver::Resolve(const char * hostName, uint16_t hostNameLen, uint8_t options, uint8_t maxAddrs,
                                IPAddress * addrArray, DNSResolver::OnResolveCompleteFunct onComplete, void * appState)
{
    INET_ERROR res = INET_NO_ERROR;

#if !CHIP_SYSTEM_CONFIG_USE_SOCKETS && !LWIP_DNS
    Release();
    return INET_ERROR_NOT_IMPLEMENTED;
#endif // !CHIP_SYSTEM_CONFIG_USE_SOCKETS && !LWIP_DNS

    uint8_t addrFamilyOption = (options & kDNSOption_AddrFamily_Mask);
    uint8_t optionFlags      = (options & kDNSOption_Flags_Mask);

    // Check that the supplied options are valid.
    if ((addrFamilyOption != kDNSOption_AddrFamily_Any && addrFamilyOption != kDNSOption_AddrFamily_IPv4Only &&
         addrFamilyOption != kDNSOption_AddrFamily_IPv4Preferred && addrFamilyOption != kDNSOption_AddrFamily_IPv6Only &&
         addrFamilyOption != kDNSOption_AddrFamily_IPv6Preferred) ||
        (optionFlags & ~kDNSOption_ValidFlags) != 0)
    {
        Release();
        return INET_ERROR_BAD_ARGS;
    }

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || (CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_DNS)

    // TODO: Eliminate the need for a local buffer when running on LwIP by changing
    // the LwIP DNS interface to support non-nul terminated strings.

    char hostNameBuf[NL_DNS_HOSTNAME_MAX_LEN + 1]; // DNS limits hostnames to 253 max characters.

    memcpy(hostNameBuf, hostName, hostNameLen);
    hostNameBuf[hostNameLen] = 0;

    AppState   = appState;
    AddrArray  = addrArray;
    MaxAddrs   = maxAddrs;
    NumAddrs   = 0;
    DNSOptions = options;
    OnComplete = onComplete;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5

    u8_t lwipAddrType;

#if INET_CONFIG_ENABLE_IPV4
    switch (addrFamilyOption)
    {
    case kDNSOption_AddrFamily_IPv4Only:
        lwipAddrType = LWIP_DNS_ADDRTYPE_IPV4;
        break;
    case kDNSOption_AddrFamily_Any:
    case kDNSOption_AddrFamily_IPv4Preferred:
        lwipAddrType = LWIP_DNS_ADDRTYPE_IPV4_IPV6;
        break;
    case kDNSOption_AddrFamily_IPv6Only:
        lwipAddrType = LWIP_DNS_ADDRTYPE_IPV6;
        break;
    case kDNSOption_AddrFamily_IPv6Preferred:
        lwipAddrType = LWIP_DNS_ADDRTYPE_IPV6_IPV4;
        break;
    default:
        chipDie();
    }
#else  // INET_CONFIG_ENABLE_IPV4
    lwipAddrType = LWIP_DNS_ADDRTYPE_IPV6;
#endif // INET_CONFIG_ENABLE_IPV4

#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5

#if INET_CONFIG_ENABLE_IPV4
    if (addrFamilyOption == kDNSOption_AddrFamily_IPv6Only)
#endif
    {
        Release();
        return INET_ERROR_NOT_IMPLEMENTED;
    }

#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    ip_addr_t lwipAddr;
    LWIP_DNS_FOUND_CALLBACK_TYPE lwipCallback = reinterpret_cast<LWIP_DNS_FOUND_CALLBACK_TYPE>(LwIPHandleResolveComplete);

    err_t lwipErr =
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        dns_gethostbyname_addrtype(hostNameBuf, &lwipAddr, lwipCallback, this, lwipAddrType);
#else
        dns_gethostbyname(hostNameBuf, &lwipAddr, lwipCallback, this);
#endif

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    if (lwipErr == ERR_OK)
    {
        chip::System::Layer & lSystemLayer = SystemLayer();

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        AddrArray[0] = IPAddress::FromLwIPAddr(lwipAddr);
#else
        AddrArray[0] = IPAddress::FromIPv4(lwipAddr);
#endif
        NumAddrs = 1;

        lSystemLayer.PostEvent(*this, kInetEvent_DNSResolveComplete, 0);
    }
    else if (lwipErr != ERR_INPROGRESS)
    {
        res = chip::System::MapErrorLwIP(lwipErr);
        Release();
    }

    return res;

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    struct addrinfo gaiHints;
    struct addrinfo * gaiResults = nullptr;
    int gaiReturnCode;

    // Configure the hints argument for getaddrinfo()
    InitAddrInfoHints(gaiHints);

    // Call getaddrinfo() to perform the name resolution.
    gaiReturnCode = getaddrinfo(hostNameBuf, nullptr, &gaiHints, &gaiResults);

    // Process the return code and results list returned by getaddrinfo(). If the call
    // was successful this will copy the resultant addresses into the caller's array.
    res = ProcessGetAddrInfoResult(gaiReturnCode, gaiResults);

    // Invoke the caller's completion function.
    onComplete(appState, res, NumAddrs, addrArray);

    // Release DNSResolver object.
    Release();

    return INET_NO_ERROR;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || (CHIP_SYSTEM_CONFIG_USE_LWIP && LWIP_DNS)
}

/**
 *  This method cancels DNS requests that are in progress.
 *
 *  @retval INET_NO_ERROR.
 *
 */
INET_ERROR DNSResolver::Cancel()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // NOTE: LwIP does not support canceling DNS requests that are in progress.  As a consequence,
    // we can't release the DNSResolver object until LwIP calls us back (because LwIP retains a
    // pointer to the DNSResolver object while the request is active).  However, now that the
    // application has called Cancel() we have to make sure to NOT call their OnComplete function
    // when the request completes.
    //
    // To ensure the right thing happens, we NULL the OnComplete pointer here, which signals the
    // code in HandleResolveComplete() and LwIPHandleResolveComplete() to not interact with the
    // application's state data (AddrArray) and to not call the application's callback. This has
    // to happen with the LwIP lock held, since LwIPHandleResolveComplete() runs on LwIP's thread.

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Signal that the request has been canceled by clearing the state of the resolver object.
    OnComplete = NULL;
    AddrArray  = NULL;
    MaxAddrs   = 0;
    NumAddrs   = 0;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    // NOTE: DNS lookups can be canceled only when using the asynchronous mode.

    InetLayer & inet = Layer();

    OnComplete = nullptr;
    AppState   = nullptr;
    inet.mAsyncDNSResolver.Cancel(*this);

#endif // INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return INET_NO_ERROR;
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  This method is called by InetLayer on success, failure, or timeout of a
 *  DNS request.
 *
 */
void DNSResolver::HandleResolveComplete()
{
    // Call the application's completion handler if the request hasn't been canceled.
    if (OnComplete != NULL)
        OnComplete(AppState, (NumAddrs > 0) ? INET_NO_ERROR : INET_ERROR_HOST_NOT_FOUND, NumAddrs, AddrArray);

    // Release the resolver object.
    Release();
}

/**
 *  This method is called by LwIP network stack on success, failure, or timeout
 *  of a DNS request.
 *
 *  @param[in]  name            A pointer to a NULL-terminated C string
 *                              representing the host name that is queried.
 *  @param[in]  ipaddr          A pointer to a list of resolved IP addresses.
 *  @param[in]  callback_arg    A pointer to the arguments that are passed to
 *                              the callback function.
 *
 */
#if LWIP_VERSION_MAJOR > 1
void DNSResolver::LwIPHandleResolveComplete(const char * name, const ip_addr_t * ipaddr, void * callback_arg)
#else  // LWIP_VERSION_MAJOR <= 1
void DNSResolver::LwIPHandleResolveComplete(const char * name, ip_addr_t * ipaddr, void * callback_arg)
#endif // LWIP_VERSION_MAJOR <= 1
{
    DNSResolver * resolver = (DNSResolver *) callback_arg;

    if (resolver != NULL)
    {
        chip::System::Layer & lSystemLayer = resolver->SystemLayer();

        // Copy the resolved address to the application supplied buffer, but only if the request hasn't been canceled.
        if (resolver->OnComplete != NULL && ipaddr != NULL)
        {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
            resolver->AddrArray[0] = IPAddress::FromLwIPAddr(*ipaddr);
#else
            resolver->AddrArray[0] = IPAddress::FromIPv4(*ipaddr);
#endif
            resolver->NumAddrs = 1;
        }

        lSystemLayer.PostEvent(*resolver, kInetEvent_DNSResolveComplete, 0);
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

void DNSResolver::InitAddrInfoHints(struct addrinfo & hints)
{
    uint8_t addrFamilyOption = (DNSOptions & kDNSOption_AddrFamily_Mask);

    memset(&hints, 0, sizeof(hints));
#if INET_CONFIG_ENABLE_IPV4
    if (addrFamilyOption == kDNSOption_AddrFamily_IPv4Only)
    {
        hints.ai_family = AF_INET;
    }
    else if (addrFamilyOption == kDNSOption_AddrFamily_IPv6Only)
    {
        hints.ai_family = AF_INET6;
    }
    else
    {
        hints.ai_family = AF_UNSPEC;
    }
#else  // INET_CONFIG_ENABLE_IPV4
    hints.ai_family = AF_INET6;
#endif // INET_CONFIG_ENABLE_IPV4
    hints.ai_flags = AI_ADDRCONFIG;
}

INET_ERROR DNSResolver::ProcessGetAddrInfoResult(int returnCode, struct addrinfo * results)
{
    INET_ERROR err = INET_NO_ERROR;

    // If getaddrinfo() succeeded, copy addresses in the returned addrinfo structures into the
    // application's output array...
    if (returnCode == 0)
    {
        NumAddrs = 0;

#if INET_CONFIG_ENABLE_IPV4

        // Based on the address family option specified by the application, determine which
        // types of addresses should be returned and the order in which they should appear.
        uint8_t addrFamilyOption = (DNSOptions & kDNSOption_AddrFamily_Mask);
        int primaryFamily, secondaryFamily;
        switch (addrFamilyOption)
        {
        case kDNSOption_AddrFamily_Any:
            primaryFamily   = AF_UNSPEC;
            secondaryFamily = AF_UNSPEC;
            break;
        case kDNSOption_AddrFamily_IPv4Only:
            primaryFamily   = AF_INET;
            secondaryFamily = AF_UNSPEC;
            break;
        case kDNSOption_AddrFamily_IPv4Preferred:
            primaryFamily   = AF_INET;
            secondaryFamily = AF_INET6;
            break;
        case kDNSOption_AddrFamily_IPv6Only:
            primaryFamily   = AF_INET6;
            secondaryFamily = AF_UNSPEC;
            break;
        case kDNSOption_AddrFamily_IPv6Preferred:
            primaryFamily   = AF_INET6;
            secondaryFamily = AF_INET;
            break;
        default:
            chipDie();
        }

        // Determine the number of addresses of each family present in the results.
        // In the case of the secondary address family, only count these if they are
        // to be returned in the results.
        uint8_t numPrimaryAddrs   = CountAddresses(primaryFamily, results);
        uint8_t numSecondaryAddrs = (secondaryFamily != AF_UNSPEC) ? CountAddresses(secondaryFamily, results) : 0;
        // Make sure numAddrs can actually fit the sum.
        uint16_t numAddrs = static_cast<uint16_t>(numPrimaryAddrs + numSecondaryAddrs);

        // If the total number of addresses to be returned exceeds the application
        // specified max, ensure that at least 1 address from the secondary family
        // appears in the result (unless of course there are no such addresses or
        // the max is set to 1).
        // This ensures the application will try at least one secondary address
        // when attempting to communicate with the host.
        if (numAddrs > MaxAddrs && MaxAddrs > 1 && numPrimaryAddrs > 0 && numSecondaryAddrs > 0)
        {
            numPrimaryAddrs = ::chip::min(numPrimaryAddrs, static_cast<uint8_t>(MaxAddrs - 1));
        }

        // Copy the primary addresses into the beginning of the application's output array,
        // up to the limit determined above.
        CopyAddresses(primaryFamily, numPrimaryAddrs, results);

        // If secondary addresses are being returned, copy them into the output array after
        // the primary addresses.
        if (numSecondaryAddrs != 0)
        {
            CopyAddresses(secondaryFamily, numSecondaryAddrs, results);
        }

#else // INET_CONFIG_ENABLE_IPV4

        // Copy IPv6 addresses into the application's output array.
        CopyAddresses(AF_INET6, UINT8_MAX, results);

#endif // INET_CONFIG_ENABLE_IPV4

        // If in the end no addresses were returned, treat this as a "host not found" error.
        if (NumAddrs == 0)
        {
            err = INET_ERROR_HOST_NOT_FOUND;
        }
    }

    // Otherwise, getaddrinfo() failed, so translate the return code to an appropriate
    // Inet error...
    else
    {
        switch (returnCode)
        {
        case EAI_NONAME:
        case EAI_NODATA:
        case EAI_ADDRFAMILY:
            // Each of these errors is translated to "host not found" for simplicity at the
            // application layer. On most systems, the errors have the following meanings:
            //    EAI_NONAME is returned when there are no DNS records for the requested host name.
            //    EAI_NODATA is returned when there are no host records (A or AAAA) for the requested
            //      name, but other records do exist (e.g. MX or TXT).
            //    EAI_ADDRFAMILY is returned when a text-form address is given as the name, but its
            //      address family (IPv4 or IPv6) does not match the value specified in hints.ai_family.
            err = INET_ERROR_HOST_NOT_FOUND;
            break;
        case EAI_AGAIN:
            err = INET_ERROR_DNS_TRY_AGAIN;
            break;
        case EAI_SYSTEM:
            err = chip::System::MapErrorPOSIX(errno);
            break;
        default:
            err = INET_ERROR_DNS_NO_RECOVERY;
            break;
        }
    }

    // Free the results structure.
    if (results != nullptr)
        freeaddrinfo(results);

    return err;
}

void DNSResolver::CopyAddresses(int family, uint8_t count, const struct addrinfo * addrs)
{
    for (const struct addrinfo * addr = addrs; addr != nullptr && NumAddrs < MaxAddrs && count > 0; addr = addr->ai_next)
    {
        if (family == AF_UNSPEC || addr->ai_addr->sa_family == family)
        {
            AddrArray[NumAddrs++] = IPAddress::FromSockAddr(*addr->ai_addr);
            count--;
        }
    }
}

uint8_t DNSResolver::CountAddresses(int family, const struct addrinfo * addrs)
{
    uint8_t count = 0;

    for (const struct addrinfo * addr = addrs; addr != nullptr && count < UINT8_MAX; addr = addr->ai_next)
    {
        if (family == AF_UNSPEC || addr->ai_addr->sa_family == family)
        {
            count++;
        }
    }

    return count;
}

#if INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

void DNSResolver::HandleAsyncResolveComplete()
{
    // Copy the resolved address to the application supplied buffer, but only if the request hasn't been canceled.
    if (OnComplete && mState != kState_Canceled)
    {
        OnComplete(AppState, asyncDNSResolveResult, NumAddrs, AddrArray);
    }

    Release();
}
#endif // INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
