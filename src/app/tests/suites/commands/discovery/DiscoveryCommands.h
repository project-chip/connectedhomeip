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

#include <app/MessageDef/StatusIB.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/support/CodeUtils.h>

#include <app-common/zap-generated/tests/simulated-cluster-objects.h>

class DiscoveryCommands : public chip::Dnssd::CommissioningResolveDelegate, public chip::Dnssd::OperationalResolveDelegate
{
public:
    DiscoveryCommands(){};
    ~DiscoveryCommands() override{};

    virtual void OnResponse(const chip::app::StatusIB & status, chip::TLV::TLVReader * data) = 0;
    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err)                              = 0;

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

    /////////// CommissioningDelegate Interface /////////
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    /////////// OperationalDelegate Interface /////////
    void OnOperationalNodeResolved(const chip::Dnssd::ResolvedNodeData & nodeData) override{};
    void OnOperationalNodeResolutionFailed(const chip::PeerId & peerId, CHIP_ERROR error) override{};

private:
    bool mReady = false;
    chip::Dnssd::ResolverProxy mDNSResolver;
};
