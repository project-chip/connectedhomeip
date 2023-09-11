/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the network commissioning drivers
 *          for Open IOT SDK platform.
 */

#include <lwip/netif.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/openiotsdk/NetworkCommissioningDriver.h>

using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

NetworkIterator * OpenIoTSDKEthernetDriver::GetNetworks()
{
    auto ret = new EthernetNetworkIterator();

    char buf[NETIF_NAMESIZE];
    char * ifname = netif_index_to_name(0, buf);

    ret->interfaceNameLen = static_cast<uint8_t>(strlen(ifname));
    memcpy(ret->interfaceName, ifname, ret->interfaceNameLen);

    return ret;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
