/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines the concrete class
 *      <tt>Inet::IPPrefix</tt>, which comprise two member fields:
 *      a) a IP address and b) a length field. The CHIP Inet Layer
 *      uses objects of this class to represent Internet protocol
 *      address prefixes of both IPv4 and IPv6 address families.
 */

#pragma once

#include <inet/IPAddress.h>

#define CHIP_INET_IPV6_DEFAULT_PREFIX_LEN (64)
#define CHIP_INET_IPV6_MAX_PREFIX_LEN (128)

namespace chip {
namespace Inet {

/**
 * @brief   Internet protocol address prefix
 *
 * @details
 *  Use objects of the \c IPPrefix class to represent Internet protocol address
 *  prefixes of both IPv4 and IPv6 address families.
 */
class IPPrefix
{
public:
    IPPrefix() = default;
    IPPrefix(const IPAddress & ipAddress, uint8_t length) : IPAddr(ipAddress), Length(length) {}

    /** An IPv6 or IPv4 address. */
    IPAddress IPAddr;

    /**
     * @brief  Length of the prefix.
     *
     * @details
     *  Note well: this field is public, and it is an invariant of this class
     *  that <tt>Length <= 32</tt> where the type of \c IPAddr is
     *  \c IPAddressType::kIPv4 and <tt>Length <= 128</tt> where the type of
     *  \c IPAddr is \c IPAddressType::kIPv6.
     */
    uint8_t Length;

    /**
     * A distinguished object where the type of \c IPAddr is
     * \c IPAddressType::kAny and <tt>Length == 0</tt>.
     */
    static IPPrefix Zero;

    /**
     * @brief   Compares the prefix with the distinguished \c Zero value.
     *
     * @details
     *  Note well: a prefix is not equivalent to \c Zero if the type of
     *  \c IPAddr is not \c IPAddressType::kAny.
     *
     * @return  \c true if equivalent to \c Zero, else \c false.
     */
    bool IsZero() const;

    /**
     * @brief   Compares the prefix with another for equivalence.
     *
     * @details
     *  Note well: two prefixes are not equivalent unless the \c IPAddr fields
     *  are completely equivalent, i.e. all 128 bits must be identical.
     *
     * @return  \c true if equivalent, else \c false.
     */
    bool operator==(const IPPrefix & other) const;

    /**
     * @brief   Compares the prefix with another for inequivalence.
     *
     * @details
     *  Note well: two prefixes are not equivalent unless the \c IPAddr fields
     *  are completely equivalent, i.e. all 128 bits must be identical.
     *
     * @return  \c false if equivalent, else \c false.
     */
    bool operator!=(const IPPrefix & other) const;

    /**
     * @brief   Test if an address matches the prefix.
     *
     * @param[in]   addr   the address to test.
     *
     * @return  \c true if \c addr has the prefix, else \c false.
     */
    bool MatchAddress(const IPAddress & addr) const;
};

} // namespace Inet
} // namespace chip
