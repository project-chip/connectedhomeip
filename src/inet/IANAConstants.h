/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file contains constants corresponding to numbers assigned
 *      by the Internet Assigned Number Authority (IANA) that are used
 *      in the abstract programming interface of the Inet layer, and
 *      which are defined independently here from their corresponding
 *      definitions in any underlying IP stacks.
 */

#pragma once

namespace chip {
namespace Inet {

/**
 *  @enum IPVersion
 *
 *  The version of the IP protocol being used.
 *
 */
typedef enum
{
    kIPVersion_6 = 6, /**< IPv6 */
#if INET_CONFIG_ENABLE_IPV4
    kIPVersion_4 = 4, /**< IPv4 */
#endif                // INET_CONFIG_ENABLE_IPV4
} IPVersion;

/**
 * @brief   Internet protocol multicast address scope
 *
 * @details
 *  Values of the \c IPv6MulticastScope type are used to call the
 *  <tt>IPAddress::MakeIPv6Multicast()</tt> method. They indicate the routable
 *  scope of the multicast group designated by the result. These numbers are
 *  registered by IETF with IANA.
 */
typedef enum
{
    /** Interface-local scope. */
    kIPv6MulticastScope_Interface = 1,

    /** Link-local scope. */
    kIPv6MulticastScope_Link = 2,

#if INET_CONFIG_ENABLE_IPV4
    /** Realm-local ("IPv4") scope. */
    kIPv6MulticastScope_IPv4 = 3,
#else  // !INET_CONFIG_ENABLE_IPV4
    /** Realm-local scope. */
    kIPv6MulticastScope_Realm = 3,
#endif // !INET_CONFIG_ENABLE_IPV4

    /** Realm-local scope. */
    kIPv6MulticastScope_Admin = 4,

    /** Realm-local scope. */
    kIPv6MulticastScope_Site = 5,

    /** Organization-local scope. */
    kIPv6MulticastScope_Organization = 8,

    /** Global scope. */
    kIPv6MulticastScope_Global = 14
} IPv6MulticastScope;

/**
 * @brief   Internet protocol multicast group identifiers
 *
 * @details
 *  Values of the \c IPV6MulticastGroup type are used to call the
 *  <tt>IPAddress::MakeIPv6Multicast()</tt> method. They indicate
 *  distinguished group identifiers registered by IETF with IANA.
 */
enum IPV6MulticastGroup
{
    kIPV6MulticastGroup_AllNodes   = 1, /** RFC 4291 */
    kIPV6MulticastGroup_AllRouters = 2  /** RFC 4291 */
};

} // namespace Inet
} // namespace chip
