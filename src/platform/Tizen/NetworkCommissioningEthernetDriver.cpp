/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <ifaddrs.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <app-common/zap-generated/cluster-enums.h>
#include <lib/support/CodeUtils.h>
#include <platform/NetworkCommissioning.h>
#include <platform/Tizen/ConnectivityUtils.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

TizenEthernetDriver::EthernetNetworkIterator::EthernetNetworkIterator(TizenEthernetDriver * aDriver) : mDriver(aDriver)
{
    struct ifaddrs * ifaddr = nullptr;
    VerifyOrReturn(getifaddrs(&ifaddr) == 0, ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno)));

    for (const auto * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (DeviceLayer::Internal::ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) ==
            app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet)
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
    static_assert(kMaxNetworkIDLen <= UINT8_MAX, "Our length won't fit in networkIDLen");
    item.networkIDLen = static_cast<uint8_t>(std::min(iface.size(), kMaxNetworkIDLen));
    memcpy(item.networkID, iface.c_str(), item.networkIDLen);
    item.connected = true;

    mInterfacesIdx++;
    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
