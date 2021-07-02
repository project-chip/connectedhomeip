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
 *      This file implements methods for parsing host names or IP
 *      addresses and an optional port number and/or an optional
 *      interface name from a human-readable string.
 *
 */

#include <string.h>
#include <type_traits>

#include <support/DLLUtil.h>

#include <inet/InetLayer.h>

namespace chip {
namespace Inet {

/**
 *  Parse a human-readable string containing a host or IP address and
 *  an optional port number (separated by a ':'), supporting the
 *  following formats:
 *
 *    * <host-name>
 *    * <host-name>:\<port\>
 *    * <ip-4-addr>
 *    * <ip-4-addr>:\<port\>
 *    * <ip-6-addr>
 *    * [<ip-6-addr>]:\<port\>
 *
 *  @param[in]  aString     The human-reable string to parse.
 *
 *  @param[in]  aStringLen  The length, in characters, of aString.
 *
 *  @param[out] aHost       A pointer to the host name portion of the parsed
 *                          string.
 *
 *  @param[out] aHostLen    The length, in characters, of aHost.
 *
 *  @param[out] aPort       The port number, if present and successfully
 *                          parsed; otherwise, 0.
 *
 *  @return  #INET_ERROR_INVALID_HOST_NAME   If the input to be parsed is of
 *                                           zero-length or otherwise
 *                                           malformed.
 *  @return  #INET_ERROR_HOST_NAME_TOO_LONG  If the host name exceeds 253
 *                                           characters.
 *  @return  #CHIP_NO_ERROR                  On success.
 *
 */
DLL_EXPORT CHIP_ERROR ParseHostAndPort(const char * aString, uint16_t aStringLen, const char *& aHost, uint16_t & aHostLen,
                                       uint16_t & aPort)
{
    const char * end = aString + aStringLen;
    const char * p;

    if (aStringLen == 0)
        return INET_ERROR_INVALID_HOST_NAME;

    // If the string starts with a [ then it is a backeted
    // host/address, possibly with a port number following it.

    if (*aString == '[')
    {
        // Search for the end bracket.
        p = static_cast<const char *>(memchr(aString, ']', aStringLen));
        if (p == nullptr)
            return INET_ERROR_INVALID_HOST_NAME;

        // Return the IPv6 address.
        aHost = aString + 1;
        // Cast is safe because we know p != aString, so p >= aHost, and at the
        // same time p - aString < aStringLen, which is uint16_t.
        static_assert(std::is_same<decltype(aStringLen), uint16_t>::value, "String length might be too big");
        aHostLen = static_cast<uint16_t>(p - aHost);

        // Skip the end bracket.
        p++;
    }

    // Otherwise, not a bracketed IPv6 address...
    else
    {
        // Search for a colon.
        p = static_cast<const char *>(memchr(aString, ':', aStringLen));

        // If the string contains no colons, then it is a host name or
        // IPv4 address without a port.
        //
        // If the string contains MULTIPLE colons, then it is an IPv6
        // address without a port.
        //
        // Note: The cast is safe because p points into the string of p is not
        // null, so end - p - 1 can't be negative.
        if (p == nullptr || memchr(p + 1, ':', static_cast<size_t>(end - p - 1)) != nullptr)
            p = end;

        // Return the host/address portion.
        aHost = aString;
        // Cast is safe because we know p - aString < aStringLen, which is
        // uint16_t.
        static_assert(std::is_same<decltype(aStringLen), uint16_t>::value, "String length might be too big");
        aHostLen = static_cast<uint16_t>(p - aString);
    }

    // Enforce the DNS limit on the maximum length of a host name.
    if (aHostLen > 253)
        return INET_ERROR_HOST_NAME_TOO_LONG;

    // If there are more characters after the host name...
    if (p < end)
    {
        // Verify the presence of a colon.
        if (*p++ != ':')
            return INET_ERROR_INVALID_HOST_NAME;

        // Verify that the port number portion is not too long.
        if ((end - p) > 5)
            return INET_ERROR_INVALID_HOST_NAME;

        // Parse the port number.
        aPort = 0;
        for (; p < end; p++)
            if (*p >= '0' && *p <= '9')
                aPort = static_cast<uint16_t>((aPort * 10) + (*p - '0'));
            else
                return INET_ERROR_INVALID_HOST_NAME;
    }

    // Otherwise, tell the caller there was no port number.
    else
        aPort = 0;

    return CHIP_NO_ERROR;
}

/**
 *  Parse a human-readable string containing a host or IP address, an
 *  optional port number (separated by a ':'), and an optional
 *  interface name (separated by a '%'), supporting the following
 *  formats:
 *
 *    * <host-name>
 *    * <host-name>%\<interface\>
 *    * <host-name>:\<port\>
 *    * <host-name>:\<port\>%\<interface\>
 *    * <ip-4-addr>
 *    * <ip-4-addr>%\<interface\>
 *    * <ip-4-addr>:\<port\>
 *    * <ip-4-addr>:\<port\>%\<interface\>
 *    * <ip-6-addr>
 *    * <ip-6-addr>%\<interface\>
 *    * [<ip-6-addr>]:\<port\>
 *    * [<ip-6-addr>]:\<port\>%\<interface\>
 *
 *  @param[in]  aString        The human-reable string to parse.
 *
 *  @param[in]  aStringLen     The length, in characters, of aString.
 *
 *  @param[out] aHost          A pointer to the host name portion of the parsed
 *                             string.
 *
 *  @param[out] aHostLen       The length, in characters, of aHost.
 *
 *  @param[out] aPort          The port number, if present and successfully
 *                             parsed; otherwise, 0.
 *
 *  @param[out] aInterface     A pointer to the interface portion of the parsed
 *                             string.
 *
 *  @param[out] aInterfaceLen  The length, in characters, of aInterface.
 *
 *  @return  #INET_ERROR_INVALID_HOST_NAME   If the input to be parsed is of
 *                                           zero-length or otherwise
 *                                           malformed.
 *  @return  #INET_ERROR_HOST_NAME_TOO_LONG  If the host name exceeds 253
 *                                           characters.
 *  @return  #CHIP_NO_ERROR                  On success.
 *
 */
DLL_EXPORT CHIP_ERROR ParseHostPortAndInterface(const char * aString, uint16_t aStringLen, const char *& aHost, uint16_t & aHostLen,
                                                uint16_t & aPort, const char *& aInterface, uint16_t & aInterfaceLen)
{
    const char * end = aString + aStringLen;

    aInterface    = nullptr;
    aInterfaceLen = 0;

    for (uint16_t i = 1; i < aStringLen; i++)
    {
        char ch = *(end - i);
        if (ch == '%')
        {
            aInterface    = end - i + 1;
            aInterfaceLen = static_cast<uint16_t>(i - 1);
            aStringLen    = static_cast<uint16_t>(aStringLen - i);
            break;
        }
        if (ch == ':' || ch == ']')
            break;
    }

    return ParseHostAndPort(aString, aStringLen, aHost, aHostLen, aPort);
}

} // namespace Inet
} // namespace chip
