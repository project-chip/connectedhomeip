/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Support functions for parsing command-line arguments for Inet types
 *
 */
#include <inet/InetArgParser.h>

#include <inet/IPAddress.h>

namespace chip {
namespace ArgParser {

/**
 * Parse an IP address in text form.
 *
 * @param[in]  str    A pointer to a NULL-terminated C string containing
 *                    the address to parse.
 * @param[out] output A reference to an IPAddress object in which the parsed
 *                    value will be stored on success.
 *
 * @return true if the value was successfully parsed; false if not.
 */
bool ParseIPAddress(const char * str, chip::Inet::IPAddress & output)
{
    return chip::Inet::IPAddress::FromString(str, output);
}

} // namespace ArgParser
} // namespace chip
