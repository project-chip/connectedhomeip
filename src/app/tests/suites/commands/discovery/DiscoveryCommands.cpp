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

#include "DiscoveryCommands.h"

#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <random>

CHIP_ERROR DiscoveryCommands::FindCommissionable()
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByShortDiscriminator(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    uint64_t shortDiscriminator = static_cast<uint64_t>((value >> 8) & 0x0F);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kShortDiscriminator, shortDiscriminator);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByLongDiscriminator(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kLongDiscriminator, value);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByCommissioningMode()
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioningMode);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByVendorId(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, value);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByDeviceType(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, value);
    return mDNSResolver.FindCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissioner()
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioner, 1);
    return mDNSResolver.FindCommissioners(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionerByVendorId(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, value);
    return mDNSResolver.FindCommissioners(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionerByDeviceType(uint64_t value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, value);
    return mDNSResolver.FindCommissioners(filter);
}

CHIP_ERROR DiscoveryCommands::SetupDiscoveryCommands()
{
    ReturnErrorOnFailure(TearDownDiscoveryCommands());

    if (mReady == false)
    {
        ReturnErrorOnFailure(mDNSResolver.Init(chip::DeviceLayer::UDPEndPointManager()));
        mReady = true;
    }
    mDNSResolver.SetResolverDelegate(this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryCommands::TearDownDiscoveryCommands()
{
    mDNSResolver.SetResolverDelegate(nullptr);
    return CHIP_NO_ERROR;
}

uint16_t DiscoveryCommands::GetUniqueDiscriminator()
{
    if (mDiscriminatorUseForFiltering == 0)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> distribution(1, 4096);
        mDiscriminatorUseForFiltering = static_cast<uint16_t>(distribution(rng));
    }
    return mDiscriminatorUseForFiltering;
}

void DiscoveryCommands::OnNodeDiscoveryComplete(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    // TODO: If multiple results are found for the same filter, then the test result depends
    //       on which result comes first. At the moment, the code assume that there is only
    //       a single match on the network, but if that's not enough, there may be a need
    //       to implement some sort of list that is built for a given duration before returning
    //
    //       But also, running on CI seems to show cross-talks between CI instances, so multiple
    //       results will comes up. Unexpected advertisements are filtered by validating the
    //       discriminator from the advertisement matches the one coming from the config section
    //       of the test.
    if (nodeData.longDiscriminator != GetUniqueDiscriminator())
    {
        ChipLogError(chipTool, "Non fatal error: Unexpected node advertisment. It will be ignored");
        nodeData.LogDetail();
        return;
    }

    ReturnOnFailure(TearDownDiscoveryCommands());

    nodeData.LogDetail();

    DiscoveryCommandResult data;
    data.hostName           = chip::CharSpan(nodeData.hostName, strlen(nodeData.hostName));
    data.instanceName       = chip::CharSpan(nodeData.instanceName, strlen(nodeData.instanceName));
    data.longDiscriminator  = nodeData.longDiscriminator;
    data.shortDiscriminator = ((nodeData.longDiscriminator >> 8) & 0x0F);
    data.vendorId           = nodeData.vendorId;
    data.productId          = nodeData.productId;
    data.commissioningMode  = nodeData.commissioningMode;
    data.deviceType         = nodeData.deviceType;
    data.deviceName         = chip::CharSpan(nodeData.deviceName, strlen(nodeData.deviceName));
    data.rotatingId         = chip::ByteSpan(nodeData.rotatingId, nodeData.rotatingIdLen);
    data.rotatingIdLen      = nodeData.rotatingIdLen;
    data.pairingHint        = nodeData.pairingHint;
    data.pairingInstruction = chip::CharSpan(nodeData.pairingInstruction, strlen(nodeData.pairingInstruction));
    data.supportsTcp        = nodeData.supportsTcp;
    data.port               = nodeData.port;

    if (!chip::CanCastTo<uint8_t>(nodeData.numIPs))
    {
        ChipLogError(chipTool, "Too many ips.");
        return;
    }
    data.numIPs = static_cast<uint8_t>(nodeData.numIPs);

    if (nodeData.mrpRetryIntervalIdle.HasValue())
    {
        data.mrpRetryIntervalIdle.SetValue(nodeData.mrpRetryIntervalIdle.Value().count());
    }

    if (nodeData.mrpRetryIntervalActive.HasValue())
    {
        data.mrpRetryIntervalActive.SetValue(nodeData.mrpRetryIntervalActive.Value().count());
    }

    OnDiscoveryCommandsResults(data);
}
