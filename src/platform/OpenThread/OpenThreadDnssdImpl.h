/*
 *
 * SPDX-FileCopyrightText: 2021-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
