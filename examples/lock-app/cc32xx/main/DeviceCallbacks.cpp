/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "DeviceCallbacks.h"
#include <app/server/Dnssd.h>

using namespace chip;
using namespace chip::DeviceLayer;
using namespace chip::System;

DeviceCallbacksDelegate * appDelegate = nullptr;
extern "C" void cc32xxLog(const char * aFormat, ...);

void DeviceCallbacks::DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            cc32xxLog("DeviceEventCallback:Start DNS Server");
            chip::app::DnssdServer::Instance().StartServer();
        }
    }
}
