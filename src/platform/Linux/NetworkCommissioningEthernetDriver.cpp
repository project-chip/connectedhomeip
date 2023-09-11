/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/SafePointerCast.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/ConnectivityUtils.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

NetworkIterator * LinuxEthernetDriver::GetNetworks()
{
    auto ret = new EthernetNetworkIterator();
    ConnectivityUtils::GetEthInterfaceName(SafePointerCast<char *>(ret->interfaceName), sizeof(ret->interfaceName));
    ret->interfaceNameLen = static_cast<uint8_t>(strnlen(SafePointerCast<char *>(ret->interfaceName), sizeof(ret->interfaceName)));
    return ret;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
