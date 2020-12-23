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
#include "lib/mdns/ServicePool.h"
#include "lib/mdns/platform/Mdns.h"
#include "platform/CHIPDeviceConfig.h"
#include "system/SystemLayer.h"

namespace chip {
namespace Mdns {

constexpr uint64_t kUndefinedNodeId   = 0;
constexpr uint64_t kUndefinedFabricId = 0;

class ResolveDelegate
{
public:
    virtual void HandleNodeIdResolve(CHIP_ERROR error, uint64_t nodeId, uint64_t fabricId, const Inet::IPAddress & address,
                                     uint16_t port) = 0;
    virtual ~ResolveDelegate() {}
};

class DiscoveryManager : public MdnsNotificationDelegate
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
    CHIP_ERROR StartPublishDevice(Inet::IPAddressType addressType = Inet::kIPAddressType_Any,
                                  Inet::InterfaceId interface     = INET_NULL_INTERFACEID);

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
    CHIP_ERROR ResolveNodeId(uint64_t nodeId, uint64_t fabricId, Inet::IPAddressType type = Inet::kIPAddressType_Any);

    void AddMdnsService(const MdnsService & service) override;

    void UpdateMdnsService(const MdnsService & service) override;

    void RemoveMdnsService(const MdnsService & service) override;

    static DiscoveryManager & GetInstance() { return sManager; }

private:
    DiscoveryManager() = default;

    DiscoveryManager(const DiscoveryManager &) = delete;
    DiscoveryManager & operator=(const DiscoveryManager &) = delete;

    CHIP_ERROR PublishUnprovisionedDevice(Inet::IPAddressType addressType, Inet::InterfaceId interface);
    CHIP_ERROR PublishProvisionedDevice(Inet::IPAddressType addressType, Inet::InterfaceId interface);
    CHIP_ERROR SetupHostname();

    static void HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error);

    static void HandleMdnsInit(void * context, CHIP_ERROR initError);
    static void HandleMdnsError(void * context, CHIP_ERROR initError);
    static void RehashServicePool(System::Layer * systemLayer, void * appState, System::Error error);

#if CHIP_ENABLE_MDNS
    uint64_t mUnprovisionedInstanceName;
    bool mMdnsInitialized               = false;
    bool mIsPublishingProvisionedDevice = false;
    bool mIsPublishing                  = false;
    bool mIsRehashPending               = false;
    ServicePool mServicePool;
#endif // CHIP_ENABLE_MDNS
    ResolveDelegate * mResolveDelegate = nullptr;

    static DiscoveryManager sManager;
};

} // namespace Mdns
} // namespace chip
