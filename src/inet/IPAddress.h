/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file defines the class <tt>Inet::IPAddress</tt> and
 *      related enumerated constants. The CHIP Inet Layer uses objects
 *      of this class to represent Internet protocol addresses of both
 *      IPv4 and IPv6 address families. (IPv4 addresses are stored
 *      internally as IPv4-Mapped IPv6 addresses.)
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <type_traits>

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/DLLUtil.h>

#include <inet/InetConfig.h>
#include <inet/InetError.h>

#include "inet/IANAConstants.h"

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#include <lwip/init.h>
#include <lwip/ip_addr.h>
#if INET_CONFIG_ENABLE_IPV4
#include <lwip/ip4_addr.h>
#endif // INET_CONFIG_ENABLE_IPV4
#include <lwip/inet.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#include <openthread/icmp6.h>
#include <openthread/ip6.h>
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS
#include "ZephyrSocket.h" // nogncheck
#endif

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT && INET_CONFIG_ENABLE_IPV4
#error Forbidden : native Open Thread implementation with IPV4 enabled
#endif

#include <inet/InetInterface.h>

#define NL_INET_IPV6_ADDR_LEN_IN_BYTES (16)
#define NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES (14)

namespace chip {
namespace Inet {

/**
 * Internet protocol address family
 */
enum class IPAddressType : uint8_t
{
    kUnknown = 0, ///< Not used.
#if INET_CONFIG_ENABLE_IPV4
    kIPv4 = 1, ///< Internet protocol version 4.
#endif         // INET_CONFIG_ENABLE_IPV4
    kIPv6 = 2, ///< Internet protocol version 6.
    kAny  = 3  ///< The unspecified internet address (independent of protocol version).
};

/**
 * Internet protocol v6 multicast flags
 *
 *  Values of the \c IPv6MulticastFlag type are used to call the <tt>IPAddress::MakeIPv6Multicast()</tt> methods.
 *  They indicate the type of IPv6 multicast address to create. These numbers are registered by IETF with IANA.
 */
enum class IPv6MulticastFlag : uint8_t
{
    /** The multicast address is (1) transient (i.e., dynamically-assigned) rather than (0) well-known (i.e, IANA-assigned). */
    kTransient = 0x01,

    /** The multicast address is (1) based on a network prefix. */
    kPrefix = 0x02
};
using IPv6MulticastFlags = BitFlags<IPv6MulticastFlag>;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
/**
 * SockAddr should be used when calling any API that returns (by copying into
 * it) a sockaddr, because that will need enough storage that it can hold data
 * for any socket type.
 *
 * It can also be used when calling an API that accepts a sockaddr, to simplify
 * the type-punning needed.
 */
union SockAddr
{
    sockaddr any;
    sockaddr_in in;
    sockaddr_in6 in6;
    sockaddr_storage storage;
};

/**
 * SockAddrWithoutStorage can be used any time we want to do the sockaddr
 * type-punning but will not store the data ourselves (e.g. we're working with
 * an existing sockaddr pointer, and reintepret it as a
 * pointer-to-SockAddrWithoutStorage).
 */
union SockAddrWithoutStorage
{
    sockaddr any;
    sockaddr_in in;
    sockaddr_in6 in6;
};
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

/**
 * @brief   Internet protocol address
 *
 * @details
 *  The CHIP Inet Layer uses objects of this class to represent Internet
 *  protocol addresses (independent of protocol version).
 *
 */
class DLL_EXPORT IPAddress
{
public:
    /**
     * Maximum length of the string representation of an IP address, including a terminating NUL.
     */
#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    static constexpr uint16_t kMaxStringLength = IP6ADDR_STRLEN_MAX;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    static constexpr uint16_t kMaxStringLength = INET6_ADDRSTRLEN;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN OT_IP6_ADDRESS_STRING_SIZE
#endif
    static constexpr uint16_t kMaxStringLength = OT_IP6_ADDRESS_STRING_SIZE;
#endif

    IPAddress() = default;

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    explicit IPAddress(const ip6_addr_t & ipv6Addr);
#if INET_CONFIG_ENABLE_IPV4 || LWIP_IPV4
    explicit IPAddress(const ip4_addr_t & ipv4Addr);
    explicit IPAddress(const ip_addr_t & addr);
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    explicit IPAddress(const struct in6_addr & ipv6Addr);
#if INET_CONFIG_ENABLE_IPV4
    explicit IPAddress(const struct in_addr & ipv4Addr);
#endif // INET_CONFIG_ENABLE_IPV4
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    explicit IPAddress(const otIp6Address & ipv6Addr);
#endif

    /**
     * @brief   Opaque word array to contain IP addresses (independent of protocol version)
     *
     * @details
     *  IPv6 address use all 128-bits split into four 32-bit network byte
     *  ordered unsigned integers. IPv4 addresses are IPv4-Mapped IPv6 addresses,
     *  i.e. the first two words are zero, the third word contains 0xFFFF in
     *  network byte order, and the fourth word contains the IPv4
     *  address in network byte order.
     */
    uint32_t Addr[4];

    /**
     * @brief   Test whether address is IPv6 compatible.
     *
     * @details
     *  Use this method to check if the address belongs to the IPv6 address
     *  family. Note well: the unspecified address is not an IPv6 address.
     *
     * @retval true   The address is IPv6 and not the unspecified address.
     * @retval false  The address is IPv4 or the unspecified address.
     */
    bool IsIPv6() const;

    /**
     * @brief   Test whether address is IPv6 global unicast address.
     *
     * @details
     *  Use this method to check if the address belongs to the IPv6 address
     *  family and has the global unicast address prefix.
     *
     * @retval true  Address is IPv6 global unicast
     * @retval false Otherwise
     */
    bool IsIPv6GlobalUnicast() const;

    /**
     * @brief   Test whether address is IPv6 unique-local address (ULA).
     *
     * @details
     *  Use this method to check if the address belongs to the IPv6 address
     *  family and has the reserved IPv6 unique-local address prefix.
     *
     * @retval true  Address is IPv6 unique-local
     * @retval false Otherwise
     */
    bool IsIPv6ULA() const;

    /**
     * @brief   Test whether address is IPv6 link-local address (LL).
     *
     * @details
     *  Use this method to check if the address belongs to the IPv6 address
     *  family and has the reserved IPv6 link-local address prefix.
     *
     * @retval true  Address is IPv6 link-local
     * @retval false Otherwise
     */
    bool IsIPv6LinkLocal() const;

    /**
     * @brief   Test whether address is IPv6 multicast.
     *
     * @details
     *  Use this method to check if the address belongs to the IPv6 address
     *  family and has the reserved IPv6 multicast address prefix.
     *
     * @retval true  Address is IPv6 multicast
     * @retval false Otherwise
     */
    bool IsIPv6Multicast() const;

    /**
     * @brief   Test whether address is IPv4 or IPv6 multicast.
     *
     * @details
     *  Use this method to check if the address belongs to the IPv4 or IPv6 address
     *  family and has the reserved IPv4 or IPv6 multicast address prefix.
     *
     * @retval true  Address is IPv4 or IPv6 multicast
     * @retval false Otherwise
     */
    bool IsMulticast() const;

    /**
     * @brief   Extract the IID of an IPv6 ULA address.
     *
     * @details
     *  Use this method with an IPv6 unique-local address (ULA) to extract the
     *  identifier identifier (IID), which is the least significant 64 bits of
     *  the address.
     *
     * @return 64-bit interface identifier, or zero if the IP address is not
     *  an IPv6 unique-local address.
     */
    uint64_t InterfaceId() const;

    /**
     * @brief   Extract the 16-bit subnet identifier of an IPv6 ULA address.
     *
     * @details
     *  Use this method with an IPv6 unique-local address (ULA) to extract the
     *  subnet identifier, which is the least significant 16 bits of the
     *  network prefix. The network prefix is the most significant 64 bits of
     *  of the address. In other words, the subnet identifier is located in
     *  the 7th and 8th bytes of a 16-byte address.
     *
     * @return 16-bit subnet identifier, or zero if the IP address is not
     *  an IPv6 unique-local address.
     */
    uint16_t Subnet() const;

    /**
     * @brief   Extract the 16-bit global network identifier of an IPv6 ULA
     *  address.
     *
     * @details
     *  Use this method with an IPv6 unique-local address (ULA) to extract the
     *  global network identifier, which is the 40 bits immediately following
     *  the distinguished ULA network prefix, i.e. fd00::/8. In other words,
     *  the global network identifier is located in the five bytes from the 2nd
     *  2nd through the 6th bytes in the address.
     *
     * @return 40-bit global network identifier, or zero if the IP address
     *  is not an IPv6 unique-local address.
     */
    uint64_t GlobalId() const;

    /**
     * @brief   Extract the type of the IP address.
     *
     * @details
     *  Use this method to return an value of the enumerated type \c
     *  IPAddressType to indicate the type of the IP address.
     *
     * @retval  IPAddressType::kIPv4 The address is IPv4.
     * @retval  IPAddressType::kIPv6 The address is IPv6.
     * @retval  IPAddressType::kAny  The address is the unspecified address.
     */
    IPAddressType Type() const;

    /**
     * @brief   Compare this IP address with another for equivalence.
     *
     * @param[in]   other   The address to compare.
     *
     * @retval true  If equivalent to \c other
     * @retval false Otherwise
     */
    bool operator==(const IPAddress & other) const;

    /**
     * @brief   Compare this IP address with another for inequivalence.
     *
     * @param[in]   other   The address to compare.
     *
     * @retval true  If equivalent to \c other
     * @retval false Otherwise
     */
    bool operator!=(const IPAddress & other) const;

    /**
     * @brief   Emit the IP address in conventional text presentation format.
     *
     * @param[out]  buf         The address of the emitted text.
     * @param[in]   bufSize     The size of the buffer for the emitted text.
     *
     * @details
     *  Use <tt>ToString(char *buf, uint32_t bufSize) const</tt> to write the
     *  conventional text presentation form of the IP address to the memory
     *  located at \c buf and extending as much as \c bufSize bytes, including
     *  its NUL termination character.
     *
     *  Note Well: not compliant with RFC 5952 on some platforms. Specifically,
     *  zero compression may not be applied according to section 4.2.
     *
     * @return  The argument \c buf if no formatting error, or zero otherwise.
     */
    char * ToString(char * buf, uint32_t bufSize) const;

    /**
     * A version of ToString that writes to a literal and deduces how much space
     * it as to work with.
     */
    template <uint32_t N>
    inline char * ToString(char (&buf)[N]) const
    {
        return ToString(buf, N);
    }

    /**
     * @brief   Scan the IP address from its conventional presentation text.
     *
     * @param[in]   str     The address of the emitted text.
     * @param[out]  output  The object to set to the scanned address.
     *
     * @details
     *  Use <tt>FromString(const char *str, IPAddress& output)</tt> to
     *  overwrite an IP address by scanning the conventional text presentation
     *  located at \c str.
     *
     * @retval true  The presentation format is valid
     * @retval false Otherwise
     */
    static bool FromString(const char * str, IPAddress & output);

    /**
     * @brief   Scan the IP address from its conventional presentation text.
     *
     * @param[in]   str     A pointer to the text to be scanned.
     * @param[in]   strLen  The length of the text to be scanned.
     * @param[out]  output  The object to set to the scanned address.
     *
     * @details
     *  Use <tt>FromString(const char *str, size_t strLen, IPAddress& output)</tt> to
     *  overwrite an IP address by scanning the conventional text presentation
     *  located at \c str.
     *
     * @retval true  The presentation format is valid
     * @retval false Otherwise
     */
    static bool FromString(const char * str, size_t strLen, IPAddress & output);

    /**
     * @brief
     *   Scan the IP address from its conventional presentation text, including
     *   the interface ID if present. (e.g. "fe80::2%wlan0"). If no interface ID
     *   is present, then ifaceOutput will be set to the null interface ID.
     *
     * @param[in]    str          A pointer to the text to be scanned.
     * @param[out]   addrOutput   The object to set to the IP address.
     * @param[out]   ifaceOutput  The object to set to the interface ID.
     *
     * @retval true  The presentation format is valid
     * @retval false Otherwise
     */
    static bool FromString(const char * str, IPAddress & addrOutput, class InterfaceId & ifaceOutput);

    /**
     * @brief   Emit the IP address in standard network representation.
     *
     * @param[in,out]   p   Reference to the cursor to use for writing.
     *
     * @details
     *  Use <tt>WriteAddress(uint8_t *&p)</tt> to encode the IP address in
     *  the binary format defined by RFC 4291 for IPv6 addresses.  IPv4
     *  addresses are encoded according to section 2.5.5.2 "IPv4-Mapped
     *  IPv6 Address".
     */
    void WriteAddress(uint8_t *& p) const;

    /**
     * @brief   Emit the IP address in standard network representation.
     *
     * @param[in,out]   p       Reference to the cursor to use for reading.
     * @param[out]      output  Object to receive decoded IP address.
     *
     * @details
     *  Use <tt>ReadAddress(uint8_t *&p, IPAddress &output)</tt> to decode
     *  the IP address at \c p to the object \c output.
     */
    static void ReadAddress(const uint8_t *& p, IPAddress & output);

    /**
     * @brief   Test whether address is IPv4 compatible.
     *
     * @details
     *  Use this method to check if the address belongs to the IPv4 address
     *  family. Note well: the unspecified address is not an IPv4 address.
     *
     * @retval true   The address is IPv4 and not the unspecified address.
     * @retval false  The address is IPv6 or the unspecified address.
     */
    bool IsIPv4() const;

    /**
     * @brief   Test whether address is IPv4 multicast.
     *
     * @details
     *  Use this method to check if the address is an IPv4 multicast
     *  address.
     *
     * @retval true  Address is the IPv4 multicast
     * @retval false Otherwise
     */
    bool IsIPv4Multicast() const;

    /**
     * @brief   Test whether address is IPv4 broadcast.
     *
     * @details
     *  Use this method to check if the address is the special purpose IPv4
     *  broadcast address.
     *
     * @retval true  Address is the IPv4 broadcast
     * @retval false Otherwise
     */
    bool IsIPv4Broadcast() const;

    /**
     * @fn      ToIPv4() const
     *
     * @brief   Extract the IPv4 address as a platform data structure.
     *
     * @details
     *  Use <tt>ToIPv4() const</tt> to extract the content as an IPv4 address,
     *  if possible. IPv6 addresses and the unspecified address are
     *  extracted as <tt>0.0.0.0</tt>.
     *
     *  The result is either of type <tt>struct in_addr</tt> (on POSIX) or
     *  <tt>ip4_addr_t</tt> (on LwIP).
     *
     * @return  The encapsulated IPv4 address, or \c 0.0.0.0 if the address is
     *      either unspecified or not an IPv4 address.
     */

    /**
     * @fn      ToIPv6() const
     *
     * @brief   Extract the IPv6 address as a platform data structure.
     *
     * @details
     *  Use <tt>ToIPv6() const</tt> to extract the content as an IPv6 address,
     *  if possible. IPv4 addresses and the unspecified address are extracted
     *  as <tt>[::]</tt>.
     *
     *  The result is either of type <tt>struct in6_addr</tt> (on POSIX) or
     *  <tt>ip6_addr_t</tt> (on LwIP).
     *
     * @return  The encapsulated IPv4 address, or \c [::] if the address is
     *      either unspecified or not an IPv4 address.
     */

#if CHIP_SYSTEM_CONFIG_USE_LWIP && !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

    /**
     * @fn      ToLwIPAddr() const
     *
     * @brief   Extract the IP address as a LwIP ip_addr_t structure.
     *
     * @details
     *  Use <tt>ToLwIPAddr() const</tt> to extract the content as an IP address,
     *  if possible.
     *
     * @return  An LwIP ip_addr_t structure corresponding to the IP address.
     */
    ip_addr_t ToLwIPAddr(void) const;

    /**
     * Extract the IP address as a LwIP ip_addr_t structure.
     *
     * If the IP address is Any, the result is IP6_ADDR_ANY unless the requested addressType is kIPv4.
     * If the requested addressType is IPAddressType::kAny, extracts the IP address as an LwIP ip_addr_t structure.
     * Otherwise, returns INET_ERROR_WRONG_ADDRESS_TYPE if the requested addressType does not match the IP address.
     */
    CHIP_ERROR ToLwIPAddr(IPAddressType addressType, ip_addr_t & outAddress) const;

    /**
     * @brief   Convert the INET layer address type to its underlying LwIP type.
     *
     * @details
     *  Use <tt>ToLwIPAddrType(IPAddressType)</tt> to convert the IP address type
     *  to its underlying LwIP address type code.
     */
    static lwip_ip_addr_type ToLwIPAddrType(IPAddressType);

    ip6_addr_t ToIPv6(void) const;

#if INET_CONFIG_ENABLE_IPV4
    ip4_addr_t ToIPv4(void) const;
#endif // INET_CONFIG_ENABLE_IPV4

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    struct in6_addr ToIPv6() const;

#if INET_CONFIG_ENABLE_IPV4
    struct in_addr ToIPv4() const;
#endif // INET_CONFIG_ENABLE_IPV4

    /**
     * Get the IP address from a SockAddr.
     */
    static CHIP_ERROR GetIPAddressFromSockAddr(const SockAddrWithoutStorage & sockaddr, IPAddress & outIPAddress);
    static CHIP_ERROR GetIPAddressFromSockAddr(const sockaddr & sockaddr, IPAddress & outIPAddress)
    {
        return GetIPAddressFromSockAddr(reinterpret_cast<const SockAddrWithoutStorage &>(sockaddr), outIPAddress);
    }
    static IPAddress FromSockAddr(const sockaddr_in6 & sockaddr) { return IPAddress(sockaddr.sin6_addr); }
#if INET_CONFIG_ENABLE_IPV4
    static IPAddress FromSockAddr(const sockaddr_in & sockaddr) { return IPAddress(sockaddr.sin_addr); }
#endif // INET_CONFIG_ENABLE_IPV4

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS || CHIP_SYSTEM_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    otIp6Address ToIPv6() const;
    static IPAddress FromOtAddr(const otIp6Address & address);
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

    /**
     * @brief   Construct an IPv6 unique-local address (ULA) from its parts.
     *
     * @details
     *  Use <tt>MakeULA(uint64_t globalId, uint16_t subnet, uint64_t
     *  interfaceId)</tt> to construct a unique-local address (ULA) with global
     *  network identifier \c globalId, subnet identifier \c subnet and
     *  interface identifier (IID) \c interfaceId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeULA(uint64_t globalId, uint16_t subnet, uint64_t interfaceId);

    /**
     * @brief   Construct an IPv6 link-local address (LL) from its IID.
     *
     * @details
     *  Use <tt>MakeLLA(uint64_t interfaceId)</tt> to construct an IPv6
     *  link-local address (LL) with interface identifier \c interfaceId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeLLA(uint64_t interfaceId);

    /**
     * @brief   Construct an IPv6 multicast address from its parts.
     *
     * @details
     *  Use <tt>MakeIPv6Multicast(uint8_t flags, uint8_t scope,
     *  uint8_t groupId[14])</tt> to construct an IPv6 multicast
     *  address with \c flags for routing scope \c scope and group
     *  identifier octets \c groupId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv6Multicast(IPv6MulticastFlags aFlags, uint8_t aScope,
                                       const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES]);

    /**
     * @brief   Construct an IPv6 multicast address from its parts.
     *
     * @details
     *  Use <tt>MakeIPv6Multicast(uint8_t flags, uint8_t scope,
     *  uint32_t groupId)</tt> to construct an IPv6 multicast
     *  address with \c flags for routing scope \c scope and group
     *  identifier \c groupId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv6Multicast(IPv6MulticastFlags aFlags, uint8_t aScope, uint32_t aGroupId);

    /**
     * @brief   Construct a well-known IPv6 multicast address from its parts.
     *
     * @details
     *  Use <tt>MakeIPv6WellKnownMulticast(uint8_t scope, uint32_t
     *  groupId)</tt> to construct an IPv6 multicast address for
     *  routing scope \c scope and group identifier \c groupId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv6WellKnownMulticast(uint8_t aScope, uint32_t aGroupId);

    /**
     * @brief   Construct a transient IPv6 multicast address from its parts.
     *
     * @details
     *  Use <tt>MakeIPv6TransientMulticast(uint8_t flags, uint8_t scope,
     *  uint8_t groupId[14])</tt> to construct a transient IPv6
     *  multicast address with \c flags for routing scope \c scope and
     *  group identifier octets \c groupId.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv6TransientMulticast(IPv6MulticastFlags aFlags, uint8_t aScope,
                                                const uint8_t aGroupId[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES]);

    /**
     * @brief   Construct a transient, prefix IPv6 multicast address from its parts.
     *
     * @details
     *  Use <tt>MakeIPv6PrefixMulticast(uint8_t scope, uint8_t
     *  prefixlen, const uint64_t prefix, uint32_t groupId)</tt> to
     *  construct a transient, prefix IPv6 multicast address with for
     *  routing scope \c scope and group identifier octets \c groupId,
     *  qualified by the prefix \c prefix of length \c prefixlen bits.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv6PrefixMulticast(uint8_t aScope, uint8_t aPrefixLength, const uint64_t & aPrefix, uint32_t aGroupId);

    /**
     * @brief   Construct an IPv4 broadcast address.
     *
     * @return  The constructed IP address.
     */
    static IPAddress MakeIPv4Broadcast();

    /**
     * @brief   The distinguished unspecified IP address object.
     *
     * @details
     *  This object is used as a constant for equivalence comparisons. It must
     *  not be modified by users of the CHIP Inet Layer.
     */
    static IPAddress Any;

    /**
     * Creates a loopback of the specified type. Type MUST be IPv6/v4.
     *
     * If type is anything else (or IPv4 is not available) an IPv6
     * loopback will be created.
     */
    static IPAddress Loopback(IPAddressType type);
};

static_assert(std::is_trivial<IPAddress>::value, "IPAddress is not trivial");

} // namespace Inet
} // namespace chip
