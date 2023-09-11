/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>

#import <CoreWLAN/CoreWLAN.h>
#import <SystemConfiguration/SCNetworkConfiguration.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;

namespace {
CWInterface * GetDefaultWiFiInterface()
{
    auto WiFiClient = [CWWiFiClient sharedWiFiClient];
    VerifyOrReturnError(WiFiClient != nil, nil);

    CWInterface * interface = [WiFiClient interface];
    VerifyOrReturnError(interface != nil, nil);

    return interface;
}
}

namespace chip {
namespace DeviceLayer {
    CHIP_ERROR ConnectivityManagerImpl::ConnectWiFiNetwork(WiFiNetworkInfos & infos)
    {
        auto interface = GetDefaultWiFiInterface();
        VerifyOrReturnError(interface != nil, CHIP_ERROR_INCORRECT_STATE);

        auto ssidData = [NSData dataWithBytes:infos.ssid length:infos.ssidLen];
        NSError * error = nil;
        auto networks = [[interface scanForNetworksWithSSID:ssidData error:&error] allObjects];
        VerifyOrReturnError(error == nil, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError([networks count] != 0, CHIP_ERROR_KEY_NOT_FOUND);

        auto credentialsData = [NSData dataWithBytes:infos.credentials length:infos.credentialsLen];
        auto credentials = [[NSString alloc] initWithData:credentialsData encoding:NSUTF8StringEncoding];
        for (CWNetwork * network in networks) {
            if ([interface associateToNetwork:network password:credentials error:&error]) {
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_NOT_CONNECTED;
    }

    ConnectivityManagerImpl::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode()
    {
        auto interface = GetDefaultWiFiInterface();
        VerifyOrReturnError(interface != nil, kWiFiStationMode_Disabled);
        return kWiFiStationMode_Enabled;
    }

    CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val) { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

    bool ConnectivityManagerImpl::_IsWiFiStationConnected()
    {
        bool status = false;

        if (_GetWiFiStationMode() == kWiFiStationMode_Enabled) {
            char interfaceName[IFNAMSIZ];
            VerifyOrReturnError(CHIP_NO_ERROR == GetWiFiInterfaceName(interfaceName, sizeof(interfaceName)), false);
            VerifyOrReturnError(CHIP_NO_ERROR == GetInterfaceStatus(interfaceName, &status), false);
        }

        return status;
    }

    bool ConnectivityManagerImpl::_IsWiFiStationProvisioned()
    {
        return ConfigurationManagerImpl::GetDefaultInstance().HasWiFiNetworkInformations();
    }

    void ConnectivityManagerImpl::_ClearWiFiStationProvision()
    {
        ConfigurationManagerImpl::GetDefaultInstance().ClearWiFiNetworkInformations();
    }

    bool ConnectivityManagerImpl::_CanStartWiFiScan() { return _IsWiFiStationConnected(); }

    CHIP_ERROR ConnectivityManagerImpl::GetWiFiInterfaceName(char * outName, size_t maxLen)
    {
        auto interface = GetDefaultWiFiInterface();
        VerifyOrReturnError(interface != nil, CHIP_ERROR_INCORRECT_STATE);

        memcpy(outName, [interface.interfaceName UTF8String], maxLen);
        return CHIP_NO_ERROR;
    }

}
}
