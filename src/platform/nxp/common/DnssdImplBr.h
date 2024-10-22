/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2024 NXP
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <lib/dnssd/platform/Dnssd.h>

namespace chip {
namespace Dnssd {

CHIP_ERROR NxpChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);

void NxpChipDnssdShutdown();

CHIP_ERROR NxpChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context);

CHIP_ERROR NxpChipDnssdRemoveServices();

CHIP_ERROR NxpChipDnssdFinalizeServiceUpdate();

CHIP_ERROR NxpChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                              chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                              intptr_t * browseIdentifier);

CHIP_ERROR NxpChipDnssdStopBrowse(intptr_t browseIdentifier);

CHIP_ERROR NxpChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                               void * context);

void NxpChipDnssdResolveNoLongerNeeded(const char * instanceName);

} // namespace Dnssd
} // namespace chip
