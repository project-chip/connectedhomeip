/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "lib/dnssd/platform/Dnssd.h"

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OpenThread/OpenThreadUtils.h>

namespace chip {
namespace Dnssd {

CHIP_ERROR OpenThreadDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context);

CHIP_ERROR OpenThreadDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context);

CHIP_ERROR OpenThreadDnssdRemoveServices();

CHIP_ERROR OpenThreadDnssdFinalizeServiceUpdate();

CHIP_ERROR OpenThreadDnssdBrowse(const char * type, DnssdServiceProtocol protocol, Inet::IPAddressType addressType,
                                 Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                                 intptr_t * browseIdentifier);

CHIP_ERROR OpenThreadDnssdResolve(DnssdService * browseResult, Inet::InterfaceId interface, DnssdResolveCallback callback,
                                  void * context);

} // namespace Dnssd
} // namespace chip
