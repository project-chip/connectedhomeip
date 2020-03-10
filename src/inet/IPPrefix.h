/*
 *
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file defines the concrete class
 *      <tt>nl::Inet::IPPrefix</tt>, which comprise two member fields:
 *      a) a IP address and b) a length field. The Nest Inet Layer
 *      uses objects of this class to represent Internet protocol
 *      address prefixes of both IPv4 and IPv6 address families.
 */

#ifndef IPPREFIX_H
#define IPPREFIX_H

#include <InetLayer/IPAddress.h>

#define NL_INET_IPV6_DEFAULT_PREFIX_LEN               (64)
#define NL_INET_IPV6_MAX_PREFIX_LEN                   (128)

namespace nl {
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
    /** An IPv6 or IPv4 address. */
    IPAddress    IPAddr;

    /**
     * @brief  Length of the prefix.
     *
     * @details
     *  Note well: this field is public, and it is an invariant of this class
     *  that <tt>Length <= 32</tt> where the type of \c IPAddr is
     *  \c kIPAddressType_IPv4 and <tt>Length <= 128</tt> where the type of
     *  \c IPAddr is \c kIPAddressType_IPv6.
     */
    uint8_t      Length;

    /**
     * A distinguished object where the type of \c IPAddr is
     * \c kIPAddressType_Any and <tt>Length == 0</tt>.
     */
    static IPPrefix Zero;

    /**
     * @brief   Compares the prefix with the distinguished \c Zero value.
     *
     * @details
     *  Note well: a prefix is not equivalent to \c Zero if the type of
     *  \c IPAddr is not \c kIPAddressType_Any.
     *
     * @return  \c true if equivalent to \c Zero, else \c false.
     */
    bool IsZero(void) const;

    /**
     * @brief   Compares the prefix with another for equivalence.
     *
     * @details
     *  Note well: two prefixes are not equivalent unless the \c IPAddr fields
     *  are completely equivalent, i.e. all 128 bits must be identical.
     *
     * @return  \c true if equivalent, else \c false.
     */
    bool operator ==(const IPPrefix& other) const;

    /**
     * @brief   Compares the prefix with another for inequivalence.
     *
     * @details
     *  Note well: two prefixes are not equivalent unless the \c IPAddr fields
     *  are completely equivalent, i.e. all 128 bits must be identical.
     *
     * @return  \c false if equivalent, else \c false.
     */
    bool operator !=(const IPPrefix& other) const;

    /**
     * @brief   Conventional assignment operator.
     *
     * @param[in]   other   the prefix to copy.
     *
     * @return  a reference to this object.
     */
    IPPrefix& operator =(const IPPrefix& other);

    /**
     * @brief   Test if an address matches the prefix.
     *
     * @param[in]   addr   the address to test.
     *
     * @return  \c true if \c addr has the prefix, else \c false.
     */
    bool MatchAddress(const IPAddress& addr) const;
};

} // namespace Inet
} // namespace nl

#endif // !defined(IPPREFIX_H)
