/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/dnssd/platform/Dnssd.h>
#include <string.h>

namespace chip {
namespace Dnssd {
namespace Internal {

std::string GetFullType(const char * type, DnssdServiceProtocol protocol);
std::string GetFullType(const DnssdService * service);

std::string GetFullTypeWithSubTypes(const DnssdService * service);
std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol);
std::string GetFullTypeWithSubTypes(const char * type, DnssdServiceProtocol protocol, const char * subTypes[], size_t subTypeSize);

std::string GetFullTypeWithoutSubTypes(std::string fullType);

std::string GetBaseType(const char * fulltype);

} // namespace Internal
} // namespace Dnssd
} // namespace chip
