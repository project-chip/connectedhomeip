/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    CHIP_ERROR FindCommissionable(const char * identity,
                                  const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionable::Type & value);
    CHIP_ERROR FindCommissionableByShortDiscriminator(
        const char * identity,
        const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByShortDiscriminator::Type & value);
    CHIP_ERROR FindCommissionableByLongDiscriminator(
        const char * identity,
        const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByLongDiscriminator::Type & value);
    CHIP_ERROR FindCommissionableByCommissioningMode(
        const char * identity,
        const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByCommissioningMode::Type & value);
    CHIP_ERROR FindCommissionableByVendorId(
        const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByVendorId::Type & value);
    CHIP_ERROR FindCommissionableByDeviceType(
        const char * identity,
        const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByDeviceType::Type & value);

    CHIP_ERROR FindCommissioner(const char * identity,
                                const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissioner::Type & value);
    CHIP_ERROR
    FindCommissionerByVendorId(const char * identity,
                               const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionerByVendorId::Type & value);
    CHIP_ERROR FindCommissionerByDeviceType(
        const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionerByDeviceType::Type & value);

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
