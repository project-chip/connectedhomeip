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
 *      This file defines DNSResolver, the object that abstracts
 *      Domain Name System (DNS) resolution in InetLayer.
 *
 */

#pragma once

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayerBasis.h>

#define NL_DNS_HOSTNAME_MAX_LEN (253)

struct addrinfo;

namespace chip {
namespace Inet {

class InetLayer;

/**
 * Options controlling how IP address resolution is performed.
 */
enum DNSOptions
{
    kDNSOption_AddrFamily_Mask = 0x07, ///< Bits within a DNSOptions integer value representing the desired address family.
    kDNSOption_Flags_Mask      = 0xF8, ///< Bits within a DNSOptions integer value reserved for flags.

    // Address Family Choices
    kDNSOption_AddrFamily_Any = 0x00, ///< Return IPv4 and/or IPv6 addresses in the order returned by the nameserver.
#if INET_CONFIG_ENABLE_IPV4
    kDNSOption_AddrFamily_IPv4Only      = 0x01, ///< Return only IPv4 addresses.
    kDNSOption_AddrFamily_IPv4Preferred = 0x02, ///< Return IPv4 and/or IPv6 addresses, with IPv4 addresses listed first.
#endif
    kDNSOption_AddrFamily_IPv6Only      = 0x03, ///< Return only IPv6 addresses.
    kDNSOption_AddrFamily_IPv6Preferred = 0x04, ///< Return IPv4 and/or IPv6 addresses, with IPv6 addresses listed first.

    // NOTE: At present there are no DNSOption flags define.
    kDNSOption_ValidFlags = 0, ///< Set of all valid DNSOption flags.

    kDNSOption_Default = kDNSOption_AddrFamily_Any
};

/**
 *  @class DNSResolver
 *
 *  @brief
 *    This is an internal class to InetLayer that provides the abstraction of
 *    Domain Name System (DNS) resolution in InetLayer. There is no public
 *    interface available for the application layer.
 *
 */
class DNSResolver : public InetLayerBasis
{
private:
    friend class InetLayer;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS
    friend class AsyncDNSResolverSockets;

    /// States of the DNSResolver object with respect to hostname resolution.
    typedef enum DNSResolverState{
        kState_Unused   = 0, ///< Used to indicate that the DNSResolver object is not used.
        kState_Active   = 2, ///< Used to indicate that a DNS resolution is being performed on the DNSResolver object.
        kState_Complete = 3, ///< Used to indicate that the DNS resolution on the DNSResolver object is complete.
        kState_Canceled = 4, ///< Used to indicate that the DNS resolution on the DNSResolver has been canceled.
    } DNSResolverState;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#endif // INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

    /**
     * @brief   Type of event handling function called when a DNS request completes.
     *
     * @param[in]   appState    Application state pointer.
     * @param[in]   err         Error code.
     * @param[in]   addrCount   Number of IP addresses in the \c addrArray list.
     * @param[in]   addrArray   List of addresses in the answer.
     *
     * @details
     *  Provide a function of this type to the \c Resolve method to process
     *  completion events.
     */
    typedef void (*OnResolveCompleteFunct)(void * appState, CHIP_ERROR err, uint8_t addrCount, IPAddress * addrArray);

    static chip::System::ObjectPool<DNSResolver, INET_CONFIG_NUM_DNS_RESOLVERS> sPool;

    /**
     *  A pointer to the callback function when a DNS request is complete.
     */
    OnResolveCompleteFunct OnComplete;

    /**
     *  A pointer to the DNS table that stores a list of resolved addresses.
     */
    IPAddress * AddrArray;

    /**
     *  The maximum number of addresses that could be stored in the DNS table.
     */
    uint8_t MaxAddrs;

    /**
     *  The actual number of addresses that are stored in the DNS table.
     */
    uint8_t NumAddrs;

    /**
     * DNS options for the current request.
     */
    uint8_t DNSOptions;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    void InitAddrInfoHints(struct addrinfo & hints);
    CHIP_ERROR ProcessGetAddrInfoResult(int returnCode, struct addrinfo * results);
    void CopyAddresses(int family, uint8_t maxAddrs, const struct addrinfo * addrs);
    uint8_t CountAddresses(int family, const struct addrinfo * addrs);

#if INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

    /* Hostname that requires resolution */
    char asyncHostNameBuf[NL_DNS_HOSTNAME_MAX_LEN + 1]; // DNS limits hostnames to 253 max characters.

    CHIP_ERROR asyncDNSResolveResult;
    /* The next DNSResolver object in the asynchronous DNS resolution queue. */
    DNSResolver * pNextAsyncDNSResolver;

    DNSResolverState mState;

    void HandleAsyncResolveComplete();

#endif // INET_CONFIG_ENABLE_ASYNC_DNS_SOCKETS

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    CHIP_ERROR Resolve(const char * hostName, uint16_t hostNameLen, uint8_t options, uint8_t maxAddrs, IPAddress * addrArray,
                       OnResolveCompleteFunct onComplete, void * appState);
    CHIP_ERROR Cancel();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    void HandleResolveComplete(void);
#if LWIP_VERSION_MAJOR > 1
    static void LwIPHandleResolveComplete(const char * name, const ip_addr_t * ipaddr, void * callback_arg);
#else  // LWIP_VERSION_MAJOR <= 1
    static void LwIPHandleResolveComplete(const char * name, ip_addr_t * ipaddr, void * callback_arg);
#endif // LWIP_VERSION_MAJOR <= 1
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
};

} // namespace Inet
} // namespace chip
