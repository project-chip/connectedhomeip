/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#pragma once

#include "core/CHIPError.h"
#include "inet/InetInterface.h"
#include "lib/mdns/platform/Mdns.h"
#include "platform/CHIPDeviceConfig.h"

namespace chip {
namespace Mdns {

class ResolveDelegate
{
public:
    virtual void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, const MdnsService & address) = 0;
    virtual ~ResolveDelegate() {}
};

class DiscoveryManager
{
public:
    /**
     * This method initializes the publisher.
     *
     */
    CHIP_ERROR Init();

    /**
     * This method publishes the device on mDNS.
     *
     * This function will fetch device name and other information and publish them
     * via mDNS. If device meta data has changed, you can call this function again
     * to update the information.
     *
     * @param[in] addressType The protocol version of the IP address.
     * @param[in] interface   The interface to send mDNS multicast.
     *
     */
    CHIP_ERROR StartPublishDevice(chip::Inet::IPAddressType addressType = chip::Inet::kIPAddressType_Any,
                                  chip::Inet::InterfaceId interface     = INET_NULL_INTERFACEID);

    /**
     * This function stops publishing the device on mDNS.
     *
     */
    CHIP_ERROR StopPublishDevice();

    /**
     * This function registers the delegate to handle node id resolve results.
     *
     */
    CHIP_ERROR RegisterResolveDelegate(ResolveDelegate * delegate);

    /**
     * This function resolves a node id to its address.
     *
     */
    CHIP_ERROR ResolveNodeId(uint64_t nodeId, uint64_t fabricId, chip::Inet::IPAddressType type = chip::Inet::kIPAddressType_Any);

    static DiscoveryManager & GetInstance() { return sManager; }

private:
    DiscoveryManager() = default;

    DiscoveryManager(const DiscoveryManager &) = delete;
    DiscoveryManager & operator=(const DiscoveryManager &) = delete;

    CHIP_ERROR PublishUnprovisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface);
    CHIP_ERROR PublishProvisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface);
    CHIP_ERROR SetupHostname();

    static void HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error);
    static void HandleMdnsInit(void * context, CHIP_ERROR initError);
    static void HandleMdnsError(void * context, CHIP_ERROR initError);

#if CHIP_ENABLE_MDNS
    uint64_t mUnprovisionedInstanceName;
    bool mMdnsInitialized               = false;
    bool mIsPublishingProvisionedDevice = false;
    bool mIsPublishing                  = false;
#endif // CHIP_ENABLE_MDNS
    ResolveDelegate * mResolveDelegate = nullptr;

    static DiscoveryManager sManager;
};

} // namespace Mdns
} // namespace chip
