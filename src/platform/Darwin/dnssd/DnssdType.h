/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#pragma once

#include <lib/dnssd/platform/Dnssd.h>
#include <string.h>
#include <string>

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
