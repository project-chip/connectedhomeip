/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Tizen/ConnectivityUtils.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

#include <cerrno>
#include <ifaddrs.h>
#include <limits>
#include <string>
#include <vector>

using namespace chip::app::Clusters::GeneralDiagnostics;
using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

TizenEthernetDriver::EthernetNetworkIterator::EthernetNetworkIterator(TizenEthernetDriver * aDriver) : mDriver(aDriver)
{
    struct ifaddrs * ifaddr = nullptr;
    VerifyOrReturn(getifaddrs(&ifaddr) == 0, ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno)));

    for (const auto * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET)
        {
            mInterfaces.push_back(ifa->ifa_name);
            if (mInterfaces.size() == mDriver->GetMaxNetworks())
            {
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
}

bool TizenEthernetDriver::EthernetNetworkIterator::Next(Network & item)
{
    VerifyOrReturnValue(mInterfacesIdx < mInterfaces.size(), false);

    const auto & iface = mInterfaces[mInterfacesIdx++];
    item.networkIDLen  = std::min(iface.size(), kMaxNetworkIDLen);
    memcpy(item.networkID, iface.c_str(), item.networkIDLen);
    item.connected = true;

    mInterfacesIdx++;
    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
