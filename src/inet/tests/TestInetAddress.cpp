/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018-2019 Google LLC
 *    Copyright (c) 2015-2018 Nest Labs, Inc.
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
 *  @file
 *    This file implements a unit test suite for <tt>chip::Inet::IPAddress</tt>,
 *    a class to store and format IPV4 and IPV6 Internet Protocol addresses.
 *
 */
#include <lib/core/CHIPConfig.h>

#include <inet/IPAddress.h>

#include <string.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/ip_addr.h>

#include <inet/arpa-inet-compatibility.h>

#elif CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
#include <inet/arpa-inet-compatibility.h>
#include <openthread/icmp6.h>
#include <openthread/ip6.h>

#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <nlunit-test.h>

#include <inet/IPPrefix.h>
#include <inet/InetError.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>

using namespace chip;
using namespace chip::Inet;

// Preprocessor Defintions

// clang-format off
#define LLA_PREFIX             0xfe800000
#define ULA_PREFIX             0xfd000000
#define MCAST_PREFIX           0xff000000
#define NUM_MCAST_SCOPES       7
#define NUM_MCAST_GROUPS       2
#define NUM_BYTES_IN_IPV6      16
#define ULA_UP_24_BIT_MASK     0xffffff0000
#define ULA_LO_16_BIT_MASK     0x000000ffff
#define NUM_FIELDS_IN_ADDR     sizeof (IPAddress) / sizeof (uint32_t)
// clang-format on

namespace {

// Type Defintions

// Test input vector format.

enum
{
    // clang-format off

    kTestIsIPv4             = true,
    kTestIsIPv6             = false,

    kTestIsIPv4Multicast    = true,
    kTestIsNotIPv4Multicast = false,

    kTestIsIPv4Broadcast    = true,
    kTestIsNotIPv4Broadcast = false,

    kTestIsMulticast        = true,
    kTestIsNotMulticast     = false,

    kTestIsIPv6Multicast    = true,
    kTestIsNotIPv6Multicast = false,

    kTestIsIPv6ULA          = true,
    kTestIsNotIPv6ULA       = false,

    kTestIsIPv6LLA          = true,
    kTestIsNotIPv6LLA       = false

    // clang-format on
};

struct IPAddressContext
{
    uint32_t mAddrQuartets[4];
    const IPAddressType mAddrType;
    const char * mAddrString;
};

typedef const struct IPAddressContext * IPAddressContextIterator;

struct IPAddressContextRange
{
    IPAddressContextIterator mBegin;
    IPAddressContextIterator mEnd;
};

struct IPAddressExpandedContext
{
    IPAddressContext mAddr;
    bool isIPv4;
    bool isIPv4Multicast;
    bool isIPv4Broadcast;
    bool isMulticast;
    bool isIPv6Multicast;
    bool isIPv6ULA;
    bool isIPv6LLA;

    uint64_t global;
    uint16_t subnet;
    uint64_t interface;
};

typedef const struct IPAddressExpandedContext * IPAddressExpandedContextIterator;

struct IPAddressExpandedContextRange
{
    IPAddressExpandedContextIterator mBegin;
    IPAddressExpandedContextIterator mEnd;
};

struct TestContext
{
    const IPAddressContextRange mIPv6WellKnownMulticastContextRange;
    const IPAddressContextRange mIPv6TransientMulticastContextRange;
    const IPAddressContextRange mIPv6PrefixMulticastContextRange;
    const IPAddressExpandedContextRange mIPAddressExpandedContextRange;
};

// Global Variables

// clang-format off
const IPv6MulticastScope sIPv6MulticastScopes[NUM_MCAST_SCOPES] =
{
    kIPv6MulticastScope_Interface,
    kIPv6MulticastScope_Link,
#if INET_CONFIG_ENABLE_IPV4
    kIPv6MulticastScope_IPv4,
#else
    kIPv6MulticastScope_Realm,
#endif // INET_CONFIG_ENABLE_IPV4
    kIPv6MulticastScope_Admin,
    kIPv6MulticastScope_Site,
    kIPv6MulticastScope_Organization,
    kIPv6MulticastScope_Global
};

const IPV6MulticastGroup sIPv6WellKnownMulticastGroups[NUM_MCAST_GROUPS] = {
    kIPV6MulticastGroup_AllNodes,
    kIPV6MulticastGroup_AllRouters
};

// Test input data.

const struct IPAddressExpandedContext sIPAddressContext[] =
{
    {
         { { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }, IPAddressType::kAny,
        "::" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
        { { 0x26200001, 0x10e70400, 0xe83fb28f, 0x9c3a1941 }, IPAddressType::kIPv6,
                                                                  "2620:1:10e7:400:e83f:b28f:9c3a:1941" } ,
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0xfe800000, 0x00000000, 0x8edcd4ff, 0xfe3aebfb }, IPAddressType::kIPv6,
        "fe80::8edc:d4ff:fe3a:ebfb" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0xff010000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6,
        "ff01::1" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0xfd000000, 0x00010001, 0x00000000, 0x00000001 }, IPAddressType::kIPv6,
        "fd00:0:1:1::1" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsIPv6ULA, kTestIsNotIPv6LLA,
        0x1, 1, 1
    },
    {
         { { 0xfd123456, 0x0001abcd, 0xabcdef00, 0xfedcba09 }, IPAddressType::kIPv6,
        "fd12:3456:1:abcd:abcd:ef00:fedc:ba09" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsIPv6ULA, kTestIsNotIPv6LLA,
        0x1234560001, 0xabcd, 0xabcdef00fedcba09
    },
    {
         { { 0xfdffffff, 0xffffffff, 0xffffffff, 0xffffffff }, IPAddressType::kIPv6,
        "fdff:ffff:ffff:ffff:ffff:ffff:ffff:ffff" },
        kTestIsIPv6, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsIPv6ULA, kTestIsNotIPv6LLA,
        0xffffffffff, 0xffff, 0xffffffffffffffff
    },
#if INET_CONFIG_ENABLE_IPV4
    // IPv4-only
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xffffff00 }, IPAddressType::kIPv4,
        "255.255.255.0" },
        kTestIsIPv4, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0x7f000001 }, IPAddressType::kIPv4,
        "127.0.0.1" },
        kTestIsIPv4, kTestIsNotIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 and IPv4 multicast

    // IPv4 Local subnetwork multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000000 }, IPAddressType::kIPv4,
        "224.0.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000001 }, IPAddressType::kIPv4,
        "224.0.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000080 }, IPAddressType::kIPv4,
        "224.0.0.128" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe00000fe }, IPAddressType::kIPv4,
        "224.0.0.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe00000ff }, IPAddressType::kIPv4,
        "224.0.0.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 Internetwork control multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000100 }, IPAddressType::kIPv4,
        "224.0.1.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000101 }, IPAddressType::kIPv4,
        "224.0.1.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000180 }, IPAddressType::kIPv4,
        "224.0.1.128" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe00001fe }, IPAddressType::kIPv4,
        "224.0.1.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe00001ff }, IPAddressType::kIPv4,
        "224.0.1.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 AD-HOC block 1 multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000200 }, IPAddressType::kIPv4,
        "224.0.2.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0000201 }, IPAddressType::kIPv4,
        "224.0.2.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0008100 }, IPAddressType::kIPv4,
        "224.0.129.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe000fffe }, IPAddressType::kIPv4,
        "224.0.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe000ffff }, IPAddressType::kIPv4,
        "224.0.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 AD-HOC block 2 multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0030000 }, IPAddressType::kIPv4,
        "224.3.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0030001 }, IPAddressType::kIPv4,
        "224.3.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe0040000 }, IPAddressType::kIPv4,
        "224.4.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe004fffe }, IPAddressType::kIPv4,
        "224.4.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe004ffff }, IPAddressType::kIPv4,
        "224.4.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 source-specific multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe8000000 }, IPAddressType::kIPv4,
        "232.0.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe8000001 }, IPAddressType::kIPv4,
        "232.0.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe8800000 }, IPAddressType::kIPv4,
        "232.128.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe8fffffe }, IPAddressType::kIPv4,
        "232.255.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe8ffffff }, IPAddressType::kIPv4,
        "232.255.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 GLOP addressing multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9000000 }, IPAddressType::kIPv4,
        "233.0.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9000001 }, IPAddressType::kIPv4,
        "233.0.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe97e0000 }, IPAddressType::kIPv4,
        "233.126.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fbfffe }, IPAddressType::kIPv4,
        "233.251.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fbffff }, IPAddressType::kIPv4,
        "233.251.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 AD-HOC block 3 multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fc0000 }, IPAddressType::kIPv4,
        "233.252.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fc0001 }, IPAddressType::kIPv4,
        "233.252.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fe0000 }, IPAddressType::kIPv4,
        "233.254.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9fffffe }, IPAddressType::kIPv4,
        "233.255.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xe9ffffff }, IPAddressType::kIPv4,
        "233.255.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 unicast-prefix-based multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xea000000 }, IPAddressType::kIPv4,
        "234.0.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xea000001 }, IPAddressType::kIPv4,
        "234.0.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xea800000 }, IPAddressType::kIPv4,
        "234.128.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xeafffffe }, IPAddressType::kIPv4,
        "234.255.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xeaffffff }, IPAddressType::kIPv4,
        "234.255.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IPv4 administratively scoped multicast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xef000000 }, IPAddressType::kIPv4,
        "239.0.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xef000001 }, IPAddressType::kIPv4,
        "239.0.0.1" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xef800000 }, IPAddressType::kIPv4,
        "239.128.0.0" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xeffffffe }, IPAddressType::kIPv4,
        "239.255.255.254" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xefffffff }, IPAddressType::kIPv4,
        "239.255.255.255" },
        kTestIsIPv4, kTestIsIPv4Multicast, kTestIsNotIPv4Broadcast, kTestIsMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    },
    // IP4 and IPv4 broadcast
    {
         { { 0x00000000, 0x00000000, 0x0000ffff, 0xffffffff }, IPAddressType::kIPv4,
        "255.255.255.255" },
        kTestIsIPv4, kTestIsNotIPv4Multicast, kTestIsIPv4Broadcast, kTestIsNotMulticast, kTestIsNotIPv6Multicast, kTestIsNotIPv6ULA, kTestIsNotIPv6LLA,
        0x0, 0x0, 0x0
    }
#endif // INET_CONFIG_ENABLE_IPV4
};
// clang-format on

const IPAddressContext sIPv6WellKnownMulticastContext[] = {
    // Well-known

    // All-nodes in Various Scopes

    { { 0xff010000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff01::1" },
    { { 0xff020000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff02::1" },
    { { 0xff030000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff03::1" },
    { { 0xff040000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff04::1" },
    { { 0xff050000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff05::1" },
    { { 0xff080000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff08::1" },
    { { 0xff0e0000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff0e::1" },

    // All-routers in Various Scopes

    { { 0xff010000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff01::2" },
    { { 0xff020000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff02::2" },
    { { 0xff030000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff03::2" },
    { { 0xff040000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff04::2" },
    { { 0xff050000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff05::2" },
    { { 0xff080000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff08::2" },
    { { 0xff0e0000, 0x00000000, 0x00000000, 0x00000002 }, IPAddressType::kIPv6, "ff0e::2" }
};

const IPAddressContext sIPv6TransientMulticastContext[] = {
    // Transient

    // Short Transient Group in Various Scopes

    { { 0xff110000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff11::1" },
    { { 0xff120000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff12::1" },
    { { 0xff130000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff13::1" },
    { { 0xff140000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff14::1" },
    { { 0xff150000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff15::1" },
    { { 0xff180000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff18::1" },
    { { 0xff1e0000, 0x00000000, 0x00000000, 0x00000001 }, IPAddressType::kIPv6, "ff1e::1" },

    // Long Transient Group in Various Scopes

    { { 0xff11d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff11:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff12d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff12:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff13d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff13:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff14d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff14:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff15d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff15:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff18d5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff18:d5d6:2ba2:7847:6452:587a:c955:b5a" },
    { { 0xff1ed5d6, 0x2ba27847, 0x6452587a, 0xc9550b5a }, IPAddressType::kIPv6, "ff1e:d5d6:2ba2:7847:6452:587a:c955:b5a" }
};

const IPAddressContext sIPv6PrefixMulticastContext[] = {
    // Prefix

    // 56-bit Prefix with Short Group in Various Scopes

    { { 0xff310038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff31:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff320038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff32:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff330038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff33:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff340038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff34:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff350038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff35:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff380038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff38:38:373a:cba4:d2ad:8d00:1:1" },
    { { 0xff3e0038, 0x373acba4, 0xd2ad8d00, 0x00010001 }, IPAddressType::kIPv6, "ff3e:38:373a:cba4:d2ad:8d00:1:1" },

    // 56-bit Prefix with Long Group in Various Scopes

    { { 0xff310038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff31:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff320038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff32:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff330038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff33:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff340038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff34:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff350038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff35:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff380038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff38:38:373a:cba4:d2ad:8d00:afff:5258" },
    { { 0xff3e0038, 0x373acba4, 0xd2ad8d00, 0xafff5258 }, IPAddressType::kIPv6, "ff3e:38:373a:cba4:d2ad:8d00:afff:5258" },

    // 64-bit Prefix with Short Group in Various Scopes

    { { 0xff310040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff31:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff320040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff32:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff330040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff33:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff340040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff34:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff350040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff35:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff380040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff38:40:6664:3dfb:afa4:385b:1:1" },
    { { 0xff3e0040, 0x66643dfb, 0xafa4385b, 0x00010001 }, IPAddressType::kIPv6, "ff3e:40:6664:3dfb:afa4:385b:1:1" },

    // 64-bit Prefix with Long Group in Various Scopes

    { { 0xff310040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff31:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff320040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff32:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff330040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff33:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff340040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff34:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff350040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff35:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff380040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff38:40:6664:3dfb:afa4:385b:afff:5258" },
    { { 0xff3e0040, 0x66643dfb, 0xafa4385b, 0xafff5258 }, IPAddressType::kIPv6, "ff3e:40:6664:3dfb:afa4:385b:afff:5258" }
};

const size_t kIPv6WellKnownMulticastTestElements = sizeof(sIPv6WellKnownMulticastContext) / sizeof(struct IPAddressContext);
const size_t kIPv6TransientMulticastTestElements = sizeof(sIPv6TransientMulticastContext) / sizeof(struct IPAddressContext);
const size_t kIPv6PrefixMulticastTestElements    = sizeof(sIPv6PrefixMulticastContext) / sizeof(struct IPAddressContext);
const size_t kIPAddressTestElements              = sizeof(sIPAddressContext) / sizeof(struct IPAddressExpandedContext);

// clang-format off
const TestContext sTestContext = {
    {
        &sIPv6WellKnownMulticastContext[0],
        &sIPv6WellKnownMulticastContext[kIPv6WellKnownMulticastTestElements]
    },
    {
        &sIPv6TransientMulticastContext[0],
        &sIPv6TransientMulticastContext[kIPv6TransientMulticastTestElements]
    },
    {
        &sIPv6PrefixMulticastContext[0],
        &sIPv6PrefixMulticastContext[kIPv6PrefixMulticastTestElements]
    },
    {
        &sIPAddressContext[0],
        &sIPAddressContext[kIPAddressTestElements]
    }
};
// clang-format on

// Utility functions.

/**
 *   Load input test directly into IPAddress.
 */
void SetupIPAddress(IPAddress & addr, const struct IPAddressExpandedContext * inContext)
{
    for (size_t i = 0; i < NUM_FIELDS_IN_ADDR; i++)
    {
        addr.Addr[i] = htonl(inContext->mAddr.mAddrQuartets[i]);
    }
}

/**
 *   Zero out IP address.
 */
void ClearIPAddress(IPAddress & addr)
{
    addr = IPAddress::Any;
}

void CheckAddressQuartet(nlTestSuite * inSuite, const uint32_t & inFirstAddressQuartet, const uint32_t & inSecondAddressQuartet,
                         const size_t & inWhich)
{
    const bool lResult = (inFirstAddressQuartet == inSecondAddressQuartet);

    NL_TEST_ASSERT(inSuite, lResult == true);

    if (!lResult)
    {
        fprintf(stdout, "Address quartet %u mismatch: actual 0x%08" PRIX32 ", expected: 0x%08" PRIX32 "\n",
                static_cast<unsigned int>(inWhich), inFirstAddressQuartet, inSecondAddressQuartet);
    }
}

void CheckAddressQuartet(nlTestSuite * inSuite, const IPAddressContext & inContext, const IPAddress & inAddress,
                         const size_t & inWhich)
{
    CheckAddressQuartet(inSuite, inAddress.Addr[inWhich], htonl(inContext.mAddrQuartets[inWhich]), inWhich);
}

void CheckAddressQuartets(nlTestSuite * inSuite, const IPAddress & inFirstAddress, const IPAddress & inSecondAddress)
{
    for (size_t i = 0; i < 4; i++)
    {
        CheckAddressQuartet(inSuite, inFirstAddress.Addr[i], inSecondAddress.Addr[i], i);
    }
}

void CheckAddressQuartets(nlTestSuite * inSuite, const IPAddressContext & inContext, const IPAddress & inAddress)
{
    for (size_t i = 0; i < 4; i++)
    {
        CheckAddressQuartet(inSuite, inContext, inAddress, i);
    }
}

void CheckAddressString(nlTestSuite * inSuite, const char * inActual, const char * inExpected)
{
    const int lResult = strcasecmp(inActual, inExpected);

    NL_TEST_ASSERT(inSuite, lResult == 0);

    if (lResult != 0)
    {
        fprintf(stdout, "Address format mismatch: actual %s, expected %s\n", inActual, inExpected);
    }
}

void CheckAddress(nlTestSuite * inSuite, const IPAddressContext & inContext, const IPAddress & inAddress)
{
    char lAddressBuffer[INET6_ADDRSTRLEN];
    IPAddress lParsedAddress;
    int lResult;

    // Compare the address quartets to their control values.

    CheckAddressQuartets(inSuite, inContext, inAddress);

    // Convert the control string to an address and compare the parsed address to the created address.

    lResult = IPAddress::FromString(inContext.mAddrString, lParsedAddress);
    NL_TEST_ASSERT(inSuite, lResult == true);

    lResult = (inAddress == lParsedAddress);

    if (!static_cast<bool>(lResult))
    {
        fprintf(stdout, "Address parse mismatch for %s\n", inContext.mAddrString);
    }

    // Convert the address to a string and compare it to the control string.

    inAddress.ToString(lAddressBuffer);
#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    // Embedded openthread stack otIp6AddressFromString format the string as a uncompressed IPV6
    // example ff01::1 is formatted has ff01:0:0:0:0:0:0:1
    // But the IPV6 address From string API (otIp6AddressFromString) handle both compressed and uncompressed format.
    char uncompressedAddrStr[INET6_ADDRSTRLEN];
    // Reconvert the previously parsed control string to an uncompressed string format
    lParsedAddress.ToString(uncompressedAddrStr);
    CheckAddressString(inSuite, lAddressBuffer, uncompressedAddrStr);
#else
    CheckAddressString(inSuite, lAddressBuffer, inContext.mAddrString);
#endif
}

// Test functions invoked from the suite.

/**
 *  Test IP address conversion from a string.
 */
void CheckFromString(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        IPAddress::FromString(lCurrent->mAddr.mAddrString, test_addr);

        CheckAddressQuartets(inSuite, lCurrent->mAddr, test_addr);

        char tmpBuf[INET6_ADDRSTRLEN];
        size_t addrStrLen = strlen(lCurrent->mAddr.mAddrString);

        memset(tmpBuf, '1', sizeof(tmpBuf));
        memcpy(tmpBuf, lCurrent->mAddr.mAddrString, addrStrLen);

        IPAddress::FromString(tmpBuf, addrStrLen, test_addr);

        CheckAddressQuartets(inSuite, lCurrent->mAddr, test_addr);

        ++lCurrent;
    }
}

/**
 *  Test IP address conversion to a string.
 */
void CheckToString(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;
    char lAddressBuffer[INET6_ADDRSTRLEN];
    IPAddress lAddress;

    while (lCurrent != lEnd)
    {
        SetupIPAddress(lAddress, lCurrent);

        lAddress.ToString(lAddressBuffer);
#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        // Embedded openthread stack otIp6AddressFromString format the string as a uncompressed IPV6
        // So ff01::1 is formatted has ff01:0:0:0:0:0:0:1
        // But the IPV6 address From string API (otIp6AddressFromString) handle both compressed and uncompressed format.
        // For this test, pass the expected, compressed, string throught the opentread stack address format API
        // so the final check evaluates uncompressed IPV6 strings.
        char uncompressedAddrStr[INET6_ADDRSTRLEN];
        IPAddress tempIpAddr;
        // Set Expected compressed IPV6 String as otIpv6 Address
        IPAddress::FromString(lCurrent->mAddr.mAddrString, strlen(lCurrent->mAddr.mAddrString), tempIpAddr);
        // Reconvert the expected IPV6 String to an uncompressed string format
        tempIpAddr.ToString(uncompressedAddrStr);
        CheckAddressString(inSuite, lAddressBuffer, uncompressedAddrStr);
#else
        CheckAddressString(inSuite, lAddressBuffer, lCurrent->mAddr.mAddrString);
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        ++lCurrent;
    }
}

/**
 *  Test correct identification of IPv6 ULA addresses.
 */
void CheckIsIPv6ULA(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv6ULA() == lCurrent->isIPv6ULA);

        ++lCurrent;
    }
}

/**
 *  Test correct identification of IPv6 Link Local addresses.
 */
void CheckIsIPv6LLA(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv6LinkLocal() == lCurrent->isIPv6LLA);

        ++lCurrent;
    }
}

/**
 *  Test correct identification of IPv6 multicast addresses.
 */
void CheckIsIPv6Multicast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv6Multicast() == lCurrent->isIPv6Multicast);

        ++lCurrent;
    }
}

/**
 *  Test correct identification of multicast addresses.
 */
void CheckIsMulticast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsMulticast() == lCurrent->isMulticast);

        ++lCurrent;
    }
}

/**
 *  Test IPAddress equal operator.
 */
void CheckOperatorEqual(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext            = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lFirstCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lFirstEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lFirstCurrent != lFirstEnd)
    {
        IPAddressExpandedContextIterator lSecondCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
        IPAddressExpandedContextIterator lSecondEnd     = lContext->mIPAddressExpandedContextRange.mEnd;
        IPAddress test_addr_1;

        SetupIPAddress(test_addr_1, lFirstCurrent);

        while (lSecondCurrent != lSecondEnd)
        {
            IPAddress test_addr_2;

            SetupIPAddress(test_addr_2, lSecondCurrent);

            if (lFirstCurrent == lSecondCurrent)
            {
                NL_TEST_ASSERT(inSuite, test_addr_1 == test_addr_2);
            }
            else
            {
                NL_TEST_ASSERT(inSuite, !(test_addr_1 == test_addr_2));
            }

            ++lSecondCurrent;
        }

        ++lFirstCurrent;
    }
}

/**
 *  Test IPAddress not-equal operator.
 */
void CheckOperatorNotEqual(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext            = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lFirstCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lFirstEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lFirstCurrent != lFirstEnd)
    {
        IPAddressExpandedContextIterator lSecondCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
        IPAddressExpandedContextIterator lSecondEnd     = lContext->mIPAddressExpandedContextRange.mEnd;
        IPAddress test_addr_1;

        SetupIPAddress(test_addr_1, lFirstCurrent);

        while (lSecondCurrent != lSecondEnd)
        {
            IPAddress test_addr_2;

            SetupIPAddress(test_addr_2, lSecondCurrent);

            if (lFirstCurrent == lSecondCurrent)
            {
                NL_TEST_ASSERT(inSuite, !(test_addr_1 != test_addr_2));
            }
            else
            {
                NL_TEST_ASSERT(inSuite, test_addr_1 != test_addr_2);
            }

            ++lSecondCurrent;
        }

        ++lFirstCurrent;
    }
}

/**
 *  Test IPAddress assign operator.
 */
void CheckOperatorAssign(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext            = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lFirstCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lFirstEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lFirstCurrent != lFirstEnd)
    {
        IPAddressExpandedContextIterator lSecondCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
        IPAddressExpandedContextIterator lSecondEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

        while (lSecondCurrent != lSecondEnd)
        {
            IPAddress test_addr_1, test_addr_2;

            ClearIPAddress(test_addr_1);
            SetupIPAddress(test_addr_2, lSecondCurrent);

            // Use operator to assign IPAddress from test_addr_2 to test_addr_1
            test_addr_1 = test_addr_2;

            CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

            ++lSecondCurrent;
        }

        ++lFirstCurrent;
    }
}

/**
 *   Test IPAddress v6 conversion to native representation.
 */
void CheckToIPv6(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;
        uint32_t addr[4];

        addr[0] = htonl(lCurrent->mAddr.mAddrQuartets[0]);
        addr[1] = htonl(lCurrent->mAddr.mAddrQuartets[1]);
        addr[2] = htonl(lCurrent->mAddr.mAddrQuartets[2]);
        addr[3] = htonl(lCurrent->mAddr.mAddrQuartets[3]);

        SetupIPAddress(test_addr, lCurrent);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        ip6_addr_t ip_addr_1 = { 0 }, ip_addr_2 = { 0 };
        memcpy(ip_addr_1.addr, addr, sizeof(addr));
#if LWIP_IPV6_SCOPES
        ip_addr_1.zone = 0;
#endif
#elif CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        otIp6Address ip_addr_1 = { 0 }, ip_addr_2 = { 0 };
        memcpy(ip_addr_1.mFields.m32, addr, sizeof(addr));
#else
        struct in6_addr ip_addr_1, ip_addr_2;
        ip_addr_1 = *reinterpret_cast<struct in6_addr *>(addr);
#endif
        ip_addr_2 = test_addr.ToIPv6();

        NL_TEST_ASSERT(inSuite, !memcmp(&ip_addr_1, &ip_addr_2, sizeof(ip_addr_1)));

        ++lCurrent;
    }
}

/**
 *   Test native IPv6 conversion into IPAddress.
 */
void CheckFromIPv6(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr_1, test_addr_2;
        uint32_t addr[4];

        addr[0] = htonl(lCurrent->mAddr.mAddrQuartets[0]);
        addr[1] = htonl(lCurrent->mAddr.mAddrQuartets[1]);
        addr[2] = htonl(lCurrent->mAddr.mAddrQuartets[2]);
        addr[3] = htonl(lCurrent->mAddr.mAddrQuartets[3]);

        SetupIPAddress(test_addr_1, lCurrent);
        ClearIPAddress(test_addr_2);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        ip6_addr_t ip_addr;
        memcpy(ip_addr.addr, addr, sizeof(addr));
#elif CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
        otIp6Address ip_addr;
        memcpy(ip_addr.mFields.m32, addr, sizeof(addr));
#else
        struct in6_addr ip_addr;
        ip_addr = *reinterpret_cast<struct in6_addr *>(addr);
#endif
        test_addr_2 = IPAddress(ip_addr);

        CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

        ++lCurrent;
    }
}

#if INET_CONFIG_ENABLE_IPV4
/**
 *  Test correct identification of IPv4 addresses.
 */
void CheckIsIPv4(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv4() == lCurrent->isIPv4);

        ++lCurrent;
    }
}

/**
 *  Test correct identification of IPv4 multicast addresses.
 */
void CheckIsIPv4Multicast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv4Multicast() == lCurrent->isIPv4Multicast);

        ++lCurrent;
    }
}

/**
 *  Test correct identification of IPv4 broadcast addresses.
 */
void CheckIsIPv4Broadcast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.IsIPv4Broadcast() == lCurrent->isIPv4Broadcast);

        ++lCurrent;
    }
}

/**
 *   Test IPAddress v4 conversion to native representation.
 */
void CheckToIPv4(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        ip4_addr_t ip_addr_1, ip_addr_2;

        ip_addr_1.addr = htonl(lCurrent->mAddr.mAddrQuartets[3]);
#else
        struct in_addr ip_addr_1, ip_addr_2;

        ip_addr_1.s_addr = htonl(lCurrent->mAddr.mAddrQuartets[3]);
#endif
        ip_addr_2 = test_addr.ToIPv4();

        NL_TEST_ASSERT(inSuite, !memcmp(&ip_addr_1, &ip_addr_2, sizeof(ip_addr_1)));

        ++lCurrent;
    }
}

/**
 *   Test native IPv4 conversion into IPAddress.
 */
void CheckFromIPv4(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr_1, test_addr_2;

        SetupIPAddress(test_addr_1, lCurrent);
        ClearIPAddress(test_addr_2);

        // Convert to IPv4 (test_addr_1);
        test_addr_1.Addr[0] = 0;
        test_addr_1.Addr[1] = 0;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        ip4_addr_t ip_addr;
        ip_addr.addr        = htonl(lCurrent->mAddr.mAddrQuartets[3]);
        test_addr_1.Addr[2] = lwip_htonl(0xffff);
#else
        struct in_addr ip_addr;
        ip_addr.s_addr      = htonl(lCurrent->mAddr.mAddrQuartets[3]);
        test_addr_1.Addr[2] = htonl(0xffff);
#endif
        test_addr_2 = IPAddress(ip_addr);

        CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

        ++lCurrent;
    }
}
#endif // INET_CONFIG_ENABLE_IPV4

/**
 *   Test IPAddress address conversion from socket.
 */
void CheckFromSocket(nlTestSuite * inSuite, void * inContext)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    (void) inSuite;
    // This test is not supported LWIP or OPEN_THREAD_ENDPOINT stacks.
#else // CHIP_SYSTEM_CONFIG_USE_LWIP
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr_1, test_addr_2;
        uint32_t addr[4];
        struct sockaddr_in6 sock_v6;
#if INET_CONFIG_ENABLE_IPV4
        struct sockaddr_in sock_v4;
#endif // INET_CONFIG_ENABLE_IPV4

        addr[0] = htonl(lCurrent->mAddr.mAddrQuartets[0]);
        addr[1] = htonl(lCurrent->mAddr.mAddrQuartets[1]);
        addr[2] = htonl(lCurrent->mAddr.mAddrQuartets[2]);
        addr[3] = htonl(lCurrent->mAddr.mAddrQuartets[3]);

        SetupIPAddress(test_addr_1, lCurrent);
        ClearIPAddress(test_addr_2);

        switch (lCurrent->mAddr.mAddrType)
        {
#if INET_CONFIG_ENABLE_IPV4
        case IPAddressType::kIPv4:
            memset(&sock_v4, 0, sizeof(struct sockaddr_in));
            sock_v4.sin_family = AF_INET;
            memcpy(&sock_v4.sin_addr.s_addr, &addr[3], sizeof(struct in_addr));
            test_addr_2 = IPAddress::FromSockAddr(sock_v4);
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        case IPAddressType::kIPv6:
            memset(&sock_v6, 0, sizeof(struct sockaddr_in6));
            sock_v6.sin6_family = AF_INET6;
            memcpy(&sock_v6.sin6_addr.s6_addr, addr, sizeof(struct in6_addr));
            test_addr_2 = IPAddress::FromSockAddr(sock_v6);
            break;

        case IPAddressType::kAny:
            memset(&sock_v6, 0, sizeof(struct sockaddr_in6));
            sock_v6.sin6_family = 0;
            memcpy(&sock_v6.sin6_addr.s6_addr, addr, sizeof(struct in6_addr));
            test_addr_2 = IPAddress::FromSockAddr(sock_v6);
            break;

        default:
            continue;
        }

        CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

        ++lCurrent;
    }
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
}

/**
 *  Test IP address type.
 */
void CheckType(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.Type() == lCurrent->mAddr.mAddrType);

        ++lCurrent;
    }
}

/**
 *  Test the Any address global.
 */
void CheckAnyAddress(nlTestSuite * inSuite, void * inContext)
{
    const IPAddress test_addr = IPAddress::Any;
    IPAddressType test_type   = test_addr.Type();

    NL_TEST_ASSERT(inSuite, test_type == IPAddressType::kAny);
}

/**
 *  Test IP address interface ID.
 */
void CheckInterface(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.InterfaceId() == lCurrent->interface);

        ++lCurrent;
    }
}

/**
 *  Test IP address subnet.
 */
void CheckSubnet(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.Subnet() == lCurrent->subnet);

        ++lCurrent;
    }
}

/**
 *  Test IP address global ID.
 */
void CheckGlobal(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

        NL_TEST_ASSERT(inSuite, test_addr.GlobalId() == lCurrent->global);

        ++lCurrent;
    }
}

/**
 *  Test address encoding with chip::Encoding.
 */
void CheckEncoding(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;
        uint8_t * p;
        uint8_t buffer[NUM_BYTES_IN_IPV6];

        SetupIPAddress(test_addr, lCurrent);
        memset(&buffer, 0, NUM_BYTES_IN_IPV6);
        p = buffer;

        // Call EncodeAddress function that we test.
        test_addr.WriteAddress(p);

        // buffer has address in network byte order
        NL_TEST_ASSERT(inSuite, buffer[3] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[0]));
        NL_TEST_ASSERT(inSuite, buffer[2] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[0] >> 8));
        NL_TEST_ASSERT(inSuite, buffer[1] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[0] >> 16));
        NL_TEST_ASSERT(inSuite, buffer[0] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[0] >> 24));

        NL_TEST_ASSERT(inSuite, buffer[7] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[1]));
        NL_TEST_ASSERT(inSuite, buffer[6] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[1] >> 8));
        NL_TEST_ASSERT(inSuite, buffer[5] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[1] >> 16));
        NL_TEST_ASSERT(inSuite, buffer[4] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[1] >> 24));

        NL_TEST_ASSERT(inSuite, buffer[11] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[2]));
        NL_TEST_ASSERT(inSuite, buffer[10] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[2] >> 8));
        NL_TEST_ASSERT(inSuite, buffer[9] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[2] >> 16));
        NL_TEST_ASSERT(inSuite, buffer[8] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[2] >> 24));

        NL_TEST_ASSERT(inSuite, buffer[15] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[3]));
        NL_TEST_ASSERT(inSuite, buffer[14] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[3] >> 8));
        NL_TEST_ASSERT(inSuite, buffer[13] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[3] >> 16));
        NL_TEST_ASSERT(inSuite, buffer[12] == (uint8_t) (lCurrent->mAddr.mAddrQuartets[3] >> 24));

        ++lCurrent;
    }
}

/**
 *  Test address decoding with chip::Decoding.
 */
void CheckDecoding(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr_1, test_addr_2;
        uint8_t buffer[NUM_BYTES_IN_IPV6];
        const uint8_t * p;
        uint8_t b;

        SetupIPAddress(test_addr_1, lCurrent);
        ClearIPAddress(test_addr_2);
        memset(&buffer, 0, NUM_BYTES_IN_IPV6);
        p = buffer;

        for (b = 0; b < NUM_BYTES_IN_IPV6; b++)
        {
            buffer[b] = static_cast<uint8_t>(lCurrent->mAddr.mAddrQuartets[b / 4] >> ((3 - b % 4) * 8));
        }

        // Call ReadAddress function that we test.
        IPAddress::ReadAddress(const_cast<const uint8_t *&>(p), test_addr_2);

        CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

        ++lCurrent;
    }
}

/**
 *  Test address symmetricity of encoding and decoding with chip::(De/En)code.
 */
void CheckEcodeDecodeSymmetricity(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr_1, test_addr_2;
        uint8_t buffer[NUM_BYTES_IN_IPV6];
        uint8_t * p;

        SetupIPAddress(test_addr_1, lCurrent);
        ClearIPAddress(test_addr_2);
        memset(&buffer, 0, NUM_BYTES_IN_IPV6);

        p = buffer;

        // Call EncodeAddress function that we test.
        test_addr_1.WriteAddress(p);

        // Move pointer back to the beginning of the buffer.
        p = buffer;

        // Call ReadAddress function that we test.
        IPAddress::ReadAddress(const_cast<const uint8_t *&>(p), test_addr_2);

        CheckAddressQuartets(inSuite, test_addr_1, test_addr_2);

        ++lCurrent;
    }
}

/**
 *  Test assembling ULA address.
 */
void CheckMakeULA(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        // Call MakeULA function that we test.
        test_addr = IPAddress::MakeULA(lCurrent->global, lCurrent->subnet, lCurrent->interface);

        NL_TEST_ASSERT(
            inSuite, test_addr.Addr[0] == htonl(static_cast<uint32_t>(ULA_PREFIX | (lCurrent->global & ULA_UP_24_BIT_MASK) >> 16)));
        NL_TEST_ASSERT(inSuite,
                       test_addr.Addr[1] ==
                           htonl(static_cast<uint32_t>((lCurrent->global & ULA_LO_16_BIT_MASK) << 16 | lCurrent->subnet)));
        NL_TEST_ASSERT(inSuite, test_addr.Addr[2] == htonl(static_cast<uint32_t>(lCurrent->interface >> 32)));
        NL_TEST_ASSERT(inSuite, test_addr.Addr[3] == htonl(static_cast<uint32_t>(lCurrent->interface)));

        ++lCurrent;
    }
}

/**
 *  Test assembling LLA address.
 */
void CheckMakeLLA(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        // Call MakeLLA function that we test.
        test_addr = IPAddress::MakeLLA(lCurrent->interface);

        NL_TEST_ASSERT(inSuite, test_addr.Addr[0] == htonl(LLA_PREFIX));
        NL_TEST_ASSERT(inSuite, test_addr.Addr[1] == 0);
        NL_TEST_ASSERT(inSuite, test_addr.Addr[2] == htonl(static_cast<uint32_t>(lCurrent->interface >> 32)));
        NL_TEST_ASSERT(inSuite, test_addr.Addr[3] == htonl(static_cast<uint32_t>(lCurrent->interface)));

        ++lCurrent;
    }
}

void CheckMakeIPv6WellKnownMulticast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext = static_cast<const struct TestContext *>(inContext);
    IPAddressContextIterator lCurrent   = lContext->mIPv6WellKnownMulticastContextRange.mBegin;
    IPAddressContextIterator lEnd       = lContext->mIPv6WellKnownMulticastContextRange.mEnd;
    size_t lGroupIndex;
    size_t lScopeIndex;
    IPAddress lAddress;

    if (lCurrent != lEnd)
    {
        for (lGroupIndex = 0; lGroupIndex < NUM_MCAST_GROUPS && (lCurrent != lEnd); lGroupIndex++)
        {
            for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
            {
                const IPv6MulticastScope lScope = sIPv6MulticastScopes[lScopeIndex];
                const IPV6MulticastGroup lGroup = sIPv6WellKnownMulticastGroups[lGroupIndex];

                lAddress = IPAddress::MakeIPv6WellKnownMulticast(lScope, lGroup);

                CheckAddress(inSuite, *lCurrent, lAddress);

                ++lCurrent;
            }
        }
    }
}

void CheckMakeIPv6TransientMulticast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext = static_cast<const struct TestContext *>(inContext);
    IPAddressContextIterator lCurrent   = lContext->mIPv6TransientMulticastContextRange.mBegin;
    IPAddressContextIterator lEnd       = lContext->mIPv6TransientMulticastContextRange.mEnd;
    constexpr IPv6MulticastFlags lFlags;
    size_t lScopeIndex;
    IPAddress lAddress;

    if (lCurrent != lEnd)
    {
        // Short Transient Group in Various Scopes

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope                             = sIPv6MulticastScopes[lScopeIndex];
            const uint8_t lGroup[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

            lAddress = IPAddress::MakeIPv6TransientMulticast(lFlags, lScope, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }

        // Long Transient Group in Various Scopes

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope                             = sIPv6MulticastScopes[lScopeIndex];
            const uint8_t lGroup[NL_INET_IPV6_MCAST_GROUP_LEN_IN_BYTES] = { 0xd5, 0xd6, 0x2b, 0xa2, 0x78, 0x47, 0x64,
                                                                            0x52, 0x58, 0x7a, 0xc9, 0x55, 0x0b, 0x5a };

            lAddress = IPAddress::MakeIPv6TransientMulticast(lFlags, lScope, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }
    }
}

void CheckMakeIPv6PrefixMulticast(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext = static_cast<const struct TestContext *>(inContext);
    IPAddressContextIterator lCurrent   = lContext->mIPv6PrefixMulticastContextRange.mBegin;
    IPAddressContextIterator lEnd       = lContext->mIPv6PrefixMulticastContextRange.mEnd;
    uint8_t lPrefixLength;
    uint64_t lPrefix;
    uint32_t lGroup;
    size_t lScopeIndex;
    IPAddress lAddress;

    if (lCurrent != lEnd)
    {
        // 56-bit Prefix with Short Group in Various Scopes

        lPrefixLength = 56;
        lPrefix       = 0x373acba4d2ad8d00;
        lGroup        = 0x00010001;

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope = sIPv6MulticastScopes[lScopeIndex];

            lAddress = IPAddress::MakeIPv6PrefixMulticast(lScope, lPrefixLength, lPrefix, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }

        // 56-bit Prefix with Long Group in Various Scopes

        lPrefixLength = 56;
        lPrefix       = 0x373acba4d2ad8d00;
        lGroup        = 0xafff5258;

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope = sIPv6MulticastScopes[lScopeIndex];

            lAddress = IPAddress::MakeIPv6PrefixMulticast(lScope, lPrefixLength, lPrefix, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }

        // 64-bit Prefix with Short Group in Various Scopes

        lPrefixLength = 64;
        lPrefix       = 0x66643dfbafa4385b;
        lGroup        = 0x00010001;

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope = sIPv6MulticastScopes[lScopeIndex];

            lAddress = IPAddress::MakeIPv6PrefixMulticast(lScope, lPrefixLength, lPrefix, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }

        // 64-bit Prefix with Long Group in Various Scopes

        lPrefixLength = 64;
        lPrefix       = 0x66643dfbafa4385b;
        lGroup        = 0xafff5258;

        for (lScopeIndex = 0; lScopeIndex < NUM_MCAST_SCOPES && (lCurrent != lEnd); lScopeIndex++)
        {
            const IPv6MulticastScope lScope = sIPv6MulticastScopes[lScopeIndex];

            lAddress = IPAddress::MakeIPv6PrefixMulticast(lScope, lPrefixLength, lPrefix, lGroup);

            CheckAddress(inSuite, *lCurrent, lAddress);

            ++lCurrent;
        }
    }
}

/**
 *  Test IPPrefix.
 */
void CheckIPPrefix(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;
    size_t i                                  = 0;

    while (lCurrent != lEnd)
    {
        IPPrefix ipprefix_1, ipprefix_2;
        IPAddress test_addr_1;

        SetupIPAddress(test_addr_1, lCurrent);

        ipprefix_1.IPAddr = test_addr_1;
        ipprefix_1.Length = static_cast<uint8_t>(128 - (i++ % 128));
        ipprefix_2        = ipprefix_1;

        NL_TEST_ASSERT(inSuite, !ipprefix_1.IsZero());
        NL_TEST_ASSERT(inSuite, !ipprefix_2.IsZero());
        NL_TEST_ASSERT(inSuite, ipprefix_1 == ipprefix_2);
        NL_TEST_ASSERT(inSuite, !(ipprefix_1 != ipprefix_2));
#if !CHIP_SYSTEM_CONFIG_USE_LWIP
        NL_TEST_ASSERT(inSuite, ipprefix_1.MatchAddress(test_addr_1));
#endif
        ++lCurrent;
    }
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP

bool sameLwIPAddress(const ip6_addr_t & a, const ip6_addr_t & b)
{
    return (a.addr[0] == b.addr[0]) && (a.addr[1] == b.addr[1]) && (a.addr[2] == b.addr[2]) && (a.addr[3] == b.addr[3]);
}

#if LWIP_IPV4 && LWIP_IPV6
bool sameLwIPAddress(const ip_addr_t & a, const ip_addr_t & b)
{
    switch (a.type)
    {
    case IPADDR_TYPE_V4:
        return (b.type == IPADDR_TYPE_V4) && (a.u_addr.ip4.addr == b.u_addr.ip4.addr);
    case IPADDR_TYPE_V6:
        return (b.type == IPADDR_TYPE_V6) && sameLwIPAddress(a.u_addr.ip6, b.u_addr.ip6);
    default:
        return false;
    }
}
#endif // LWIP_IPV4 && LWIP_IPV6

/**
 * Test ToLwIPAddress()
 */
void CheckToLwIPAddr(nlTestSuite * inSuite, void * inContext)
{
    const struct TestContext * lContext       = static_cast<const struct TestContext *>(inContext);
    IPAddressExpandedContextIterator lCurrent = lContext->mIPAddressExpandedContextRange.mBegin;
    IPAddressExpandedContextIterator lEnd     = lContext->mIPAddressExpandedContextRange.mEnd;
    CHIP_ERROR err;
    ip_addr_t lwip_expected_addr, lwip_check_addr;

    while (lCurrent != lEnd)
    {
        IPAddress test_addr;

        SetupIPAddress(test_addr, lCurrent);

#if INET_CONFIG_ENABLE_IPV4
        if (lCurrent->mAddr.mAddrType == IPAddressType::kIPv4)
        {
            ip_addr_copy_from_ip4(lwip_expected_addr, test_addr.ToIPv4());
            lwip_check_addr = test_addr.ToLwIPAddr();
            NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

            err = test_addr.ToLwIPAddr(IPAddressType::kAny, lwip_check_addr);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

            err = test_addr.ToLwIPAddr(IPAddressType::kIPv4, lwip_check_addr);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
            NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

            err = test_addr.ToLwIPAddr(IPAddressType::kIPv6, lwip_check_addr);
            NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
        }
        else
#endif // INET_CONFIG_ENABLE_IPV4
            if (lCurrent->mAddr.mAddrType == IPAddressType::kIPv6)
            {
                ip_addr_copy_from_ip6(lwip_expected_addr, test_addr.ToIPv6());
                lwip_check_addr = test_addr.ToLwIPAddr();
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

                err = test_addr.ToLwIPAddr(IPAddressType::kAny, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

                err = test_addr.ToLwIPAddr(IPAddressType::kIPv6, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(lwip_expected_addr, lwip_check_addr));

#if INET_CONFIG_ENABLE_IPV4
                err = test_addr.ToLwIPAddr(IPAddressType::kIPv4, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == INET_ERROR_WRONG_ADDRESS_TYPE);
#endif // INET_CONFIG_ENABLE_IPV4
            }
            else if (lCurrent->mAddr.mAddrType == IPAddressType::kAny)
            {
                lwip_check_addr = test_addr.ToLwIPAddr();
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(*IP6_ADDR_ANY, lwip_check_addr));

                err = test_addr.ToLwIPAddr(IPAddressType::kAny, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(*IP6_ADDR_ANY, lwip_check_addr));

                err = test_addr.ToLwIPAddr(IPAddressType::kIPv6, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(*IP6_ADDR_ANY, lwip_check_addr));

#if INET_CONFIG_ENABLE_IPV4
                err = test_addr.ToLwIPAddr(IPAddressType::kIPv4, lwip_check_addr);
                NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
                NL_TEST_ASSERT(inSuite, sameLwIPAddress(*IP4_ADDR_ANY, lwip_check_addr));
#endif // INET_CONFIG_ENABLE_IPV4
            }

        ++lCurrent;
    }
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Address Type",                                CheckType),
    NL_TEST_DEF("Any Address Global",                          CheckAnyAddress),
    NL_TEST_DEF("Address Encoding (Writing)",                  CheckEncoding),
    NL_TEST_DEF("Address Decoding (Reading)",                  CheckDecoding),
    NL_TEST_DEF("Address Encode / Decode Symmetricity",        CheckEcodeDecodeSymmetricity),
    NL_TEST_DEF("From String Conversion",                      CheckFromString),
    NL_TEST_DEF("To String Conversion",                        CheckToString),
#if INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("IPv4 Detection",                              CheckIsIPv4),
    NL_TEST_DEF("IPv4 Multicast Detection",                    CheckIsIPv4Multicast),
    NL_TEST_DEF("IPv4 Broadcast Detection",                    CheckIsIPv4Broadcast),
    NL_TEST_DEF("Convert IPv4 to IPAddress",                   CheckFromIPv4),
    NL_TEST_DEF("Convert IPAddress to IPv4",                   CheckToIPv4),
#endif // INET_CONFIG_ENABLE_IPV4
    NL_TEST_DEF("IPv6 ULA Detection",                          CheckIsIPv6ULA),
    NL_TEST_DEF("IPv6 Link Local Detection",                   CheckIsIPv6LLA),
    NL_TEST_DEF("IPv6 Multicast Detection",                    CheckIsIPv6Multicast),
    NL_TEST_DEF("Multicast Detection",                         CheckIsMulticast),
    NL_TEST_DEF("Equivalence Operator",                        CheckOperatorEqual),
    NL_TEST_DEF("Non-Equivalence Operator",                    CheckOperatorNotEqual),
    NL_TEST_DEF("Assign Operator",                             CheckOperatorAssign),
    NL_TEST_DEF("Convert IPv6 to IPAddress",                   CheckFromIPv6),
    NL_TEST_DEF("Convert IPAddress to IPv6",                   CheckToIPv6),
    NL_TEST_DEF("Assign address from socket",                  CheckFromSocket),
    NL_TEST_DEF("Address Interface ID",                        CheckInterface),
    NL_TEST_DEF("Address Subnet",                              CheckSubnet),
    NL_TEST_DEF("Address Global ID",                           CheckGlobal),
    NL_TEST_DEF("Assemble IPv6 ULA address",                   CheckMakeULA),
    NL_TEST_DEF("Assemble IPv6 LLA address",                   CheckMakeLLA),
    NL_TEST_DEF("Assemble IPv6 Well-known Multicast address",  CheckMakeIPv6WellKnownMulticast),
    NL_TEST_DEF("Assemble IPv6 Transient Multicast address",   CheckMakeIPv6TransientMulticast),
    NL_TEST_DEF("Assemble IPv6 Prefix Multicast address",      CheckMakeIPv6PrefixMulticast),
    NL_TEST_DEF("IPPrefix test",                               CheckIPPrefix),
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    NL_TEST_DEF("Convert IPAddress to LwIP address",           CheckToLwIPAddr),
#endif
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
int TestSetup(void * inContext)
{
    return (SUCCESS);
}

/**
 *  Tear down the test suite.
 */
int TestTeardown(void * inContext)
{
    return (SUCCESS);
}

} // namespace

int TestInetAddress()
{
    // clang-format off
    nlTestSuite theSuite = {
        "inet-address",
        &sTests[0],
        TestSetup,
        TestTeardown
    };
    // clang-format on

    // Run test suite against one context.
    nlTestRunner(&theSuite, const_cast<TestContext *>(&sTestContext));

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInetAddress)
