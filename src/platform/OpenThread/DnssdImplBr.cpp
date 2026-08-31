/*
 *
 *    Copyright (c) 2024, 2026 Project CHIP Authors
 *    Copyright 2024, 2026  NXP
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
#include "platform/CHIPDeviceLayer.h"

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/OpenThread/OpenThreadBrDnssdImpl.h>
#include <platform/OpenThread/OpenThreadDnssdImpl.h>

using namespace ::chip::DeviceLayer;

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    CHIP_ERROR error = CHIP_ERROR_INCORRECT_STATE;

    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        error = OpenThreadBrDnssdInit(initCallback, errorCallback, context);
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        error = OpenThreadDnssdInit(initCallback, errorCallback, context);
    }

    return error;
}

void ChipDnssdShutdown()
{
    OpenThreadBrDnssdShutdown();
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadBrDnssdPublishService(service, callback, context));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdPublishService(service, callback, context));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadBrDnssdRemoveServices());
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdRemoveServices());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadBrDnssdFinalizeServiceUpdate());
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdFinalizeServiceUpdate());
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdBrowse(const char * type, DnssdServiceProtocol protocol, chip::Inet::IPAddressType addressType,
                           chip::Inet::InterfaceId interface, DnssdBrowseCallback callback, void * context,
                           intptr_t * browseIdentifier)
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadBrDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return OpenThreadBrDnssdStopBrowse(browseIdentifier);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    if (ConnectivityMgr().IsWiFiStationProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadBrDnssdResolve(service, interface, callback, context));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdResolve(service, interface, callback, context));
    }

    return CHIP_NO_ERROR;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName)
{
    OpenThreadBrDnssdResolveNoLongerNeeded(instanceName);
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
