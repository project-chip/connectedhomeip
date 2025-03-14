/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2024 NXP
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

#include <DnssdImplBr.h>
#include <platform/OpenThread/OpenThreadDnssdImpl.h>

using namespace ::chip::DeviceLayer;

namespace chip {
namespace Dnssd {

CHIP_ERROR ChipDnssdInit(DnssdAsyncReturnCallback initCallback, DnssdAsyncReturnCallback errorCallback, void * context)
{
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdInit(initCallback, errorCallback, context));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdInit(initCallback, errorCallback, context));
    }
    else
    {
        initCallback(context, CHIP_ERROR_INCORRECT_STATE);
    }

    return CHIP_NO_ERROR;
}

void ChipDnssdShutdown()
{
    NxpChipDnssdShutdown();
}

CHIP_ERROR ChipDnssdPublishService(const DnssdService * service, DnssdPublishCallback callback, void * context)
{
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdPublishService(service, callback, context));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdPublishService(service, callback, context));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdRemoveServices()
{
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdRemoveServices());
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdRemoveServices());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdFinalizeServiceUpdate()
{
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdFinalizeServiceUpdate());
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
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdBrowse(type, protocol, addressType, interface, callback, context, browseIdentifier));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDnssdStopBrowse(intptr_t browseIdentifier)
{
    return NxpChipDnssdStopBrowse(browseIdentifier);
}

CHIP_ERROR ChipDnssdResolve(DnssdService * service, chip::Inet::InterfaceId interface, DnssdResolveCallback callback,
                            void * context)
{
    if (ConnectivityMgr().IsWiFiStationConnected())
    {
        ReturnErrorOnFailure(NxpChipDnssdResolve(service, interface, callback, context));
    }
    else if (ConnectivityMgr().IsThreadProvisioned())
    {
        ReturnErrorOnFailure(OpenThreadDnssdResolve(service, interface, callback, context));
    }

    return CHIP_NO_ERROR;
}

void ChipDnssdResolveNoLongerNeeded(const char * instanceName)
{
    NxpChipDnssdResolveNoLongerNeeded(instanceName);
}

CHIP_ERROR ChipDnssdReconfirmRecord(const char * hostname, chip::Inet::IPAddress address, chip::Inet::InterfaceId interface)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Dnssd
} // namespace chip
