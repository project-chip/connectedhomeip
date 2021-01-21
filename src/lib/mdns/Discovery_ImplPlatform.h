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
#include "lib/mdns/Advertiser.h"
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

class DiscoveryImplPlatform : public ServiceAdvertiser
{
public:
    /**
     * This method initializes the publisher.
     *
     */
    CHIP_ERROR Init();

    CHIP_ERROR Start(Inet::InetLayer * inetLayer, uint16_t port) override;

    /// Advertises the CHIP node as an operational node
    CHIP_ERROR Advertise(const OperationalAdvertisingParameters & params) override;

    /// Advertises the CHIP node as a commisioning/commissionable node
    CHIP_ERROR Advertise(const CommissionAdvertisingParameters & params) override;

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

    static DiscoveryImplPlatform & GetInstance() { return sManager; }

private:
    DiscoveryImplPlatform();

    DiscoveryImplPlatform(const DiscoveryImplPlatform &) = delete;
    DiscoveryImplPlatform & operator=(const DiscoveryImplPlatform &) = delete;

    CHIP_ERROR PublishUnprovisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface);
    CHIP_ERROR PublishProvisionedDevice(chip::Inet::IPAddressType addressType, chip::Inet::InterfaceId interface);
    CHIP_ERROR SetupHostname();

    static void HandleNodeIdResolve(void * context, MdnsService * result, CHIP_ERROR error);
    static void HandleMdnsInit(void * context, CHIP_ERROR initError);
    static void HandleMdnsError(void * context, CHIP_ERROR initError);

    OperationalAdvertisingParameters mOperationalAdvertisingParams;
    bool mIsOperationalPublishing = false;
    uint64_t mCommissionInstanceName;
    CommissionAdvertisingParameters mCommissioningdvertisingParams;
    bool mIsCommissionalPublishing = false;

    bool mMdnsInitialized              = false;
    ResolveDelegate * mResolveDelegate = nullptr;

    static DiscoveryImplPlatform sManager;
};

} // namespace Mdns
} // namespace chip
