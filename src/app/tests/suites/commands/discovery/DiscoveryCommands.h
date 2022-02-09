/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <lib/dnssd/ResolverProxy.h>
#include <lib/support/CodeUtils.h>

struct DiscoveryCommandResult
{
    chip::CharSpan hostName;
    chip::CharSpan instanceName;
    uint16_t longDiscriminator;
    uint8_t shortDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
    uint8_t commissioningMode;
    uint16_t deviceType;
    chip::CharSpan deviceName;
    chip::ByteSpan rotatingId;
    uint64_t rotatingIdLen;
    uint16_t pairingHint;
    chip::CharSpan pairingInstruction;
    bool supportsTcp;
    uint8_t numIPs;
    uint16_t port;
    chip::Optional<uint32_t> mrpRetryIntervalIdle;
    chip::Optional<uint32_t> mrpRetryIntervalActive;
};

class DiscoveryCommands : public chip::Dnssd::ResolverDelegate
{
public:
    DiscoveryCommands(){};
    virtual ~DiscoveryCommands(){};

    virtual CHIP_ERROR ContinueOnChipMainThread() = 0;

    CHIP_ERROR FindCommissionable();
    CHIP_ERROR FindCommissionableByShortDiscriminator(uint64_t value);
    CHIP_ERROR FindCommissionableByLongDiscriminator(uint64_t value);
    CHIP_ERROR FindCommissionableByCommissioningMode();
    CHIP_ERROR FindCommissionableByVendorId(uint64_t vendorId);
    CHIP_ERROR FindCommissionableByDeviceType(uint64_t deviceType);

    CHIP_ERROR FindCommissioner();
    CHIP_ERROR FindCommissionerByVendorId(uint64_t vendorId);
    CHIP_ERROR FindCommissionerByDeviceType(uint64_t deviceType);

    CHIP_ERROR SetupDiscoveryCommands();
    CHIP_ERROR TearDownDiscoveryCommands();
    virtual void OnDiscoveryCommandsResults(const DiscoveryCommandResult & nodeData){};

    /////////// ResolverDelegate Interface /////////
    void OnNodeIdResolved(const chip::Dnssd::ResolvedNodeData & nodeData) override{};
    void OnNodeIdResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override{};
    void OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

protected:
    // This function initialize a random discriminator once and returns it all the time afterwards
    uint16_t GetUniqueDiscriminator();

private:
    bool mReady = false;
    chip::Dnssd::ResolverProxy mDNSResolver;
    uint16_t mDiscriminatorUseForFiltering = 0;
};
