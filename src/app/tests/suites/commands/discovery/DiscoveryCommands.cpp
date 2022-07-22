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

CHIP_ERROR
DiscoveryCommands::FindCommissionable(const char * identity,
                                      const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionable::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByShortDiscriminator(
    const char * identity,
    const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByShortDiscriminator::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    uint64_t shortDiscriminator = static_cast<uint64_t>((value.value >> 8) & 0x0F);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kShortDiscriminator, shortDiscriminator);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByLongDiscriminator(
    const char * identity,
    const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByLongDiscriminator::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kLongDiscriminator, value.value);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByCommissioningMode(
    const char * identity,
    const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByCommissioningMode::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioningMode);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByVendorId(
    const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByVendorId::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, value.value);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionableByDeviceType(
    const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionableByDeviceType::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, value.value);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR
DiscoveryCommands::FindCommissioner(const char * identity,
                                    const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissioner::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kCommissioner, 1);
    return mDNSResolver.DiscoverCommissioners(filter);
}

CHIP_ERROR
DiscoveryCommands::FindCommissionerByVendorId(
    const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionerByVendorId::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kVendorId, value.value);
    return mDNSResolver.DiscoverCommissioners(filter);
}

CHIP_ERROR DiscoveryCommands::FindCommissionerByDeviceType(
    const char * identity, const chip::app::Clusters::DiscoveryCommands::Commands::FindCommissionerByDeviceType::Type & value)
{
    ReturnErrorOnFailure(SetupDiscoveryCommands());

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kDeviceType, value.value);
    return mDNSResolver.DiscoverCommissioners(filter);
}

CHIP_ERROR DiscoveryCommands::SetupDiscoveryCommands()
{
    ReturnErrorOnFailure(TearDownDiscoveryCommands());

    if (!mReady)
    {
        ReturnErrorOnFailure(mDNSResolver.Init(chip::DeviceLayer::UDPEndPointManager()));
        mReady = true;
    }
    mDNSResolver.SetOperationalDelegate(this);
    mDNSResolver.SetCommissioningDelegate(this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryCommands::TearDownDiscoveryCommands()
{
    mDNSResolver.SetOperationalDelegate(nullptr);
    mDNSResolver.SetCommissioningDelegate(nullptr);
    return CHIP_NO_ERROR;
}

void DiscoveryCommands::OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData)
{
    // TODO: If multiple results are found for the same filter, then the test result depends
    //       on which result comes first. At the moment, the code assume that there is only
    //       a single match on the network, but if that's not enough, there may be a need
    //       to implement some sort of list that is built for a given duration before returning
    ReturnOnFailure(TearDownDiscoveryCommands());

    nodeData.LogDetail();

    chip::DiscoveryCommandResponse data;
    data.hostName           = chip::CharSpan(nodeData.resolutionData.hostName, strlen(nodeData.resolutionData.hostName));
    data.instanceName       = chip::CharSpan(nodeData.commissionData.instanceName, strlen(nodeData.commissionData.instanceName));
    data.longDiscriminator  = nodeData.commissionData.longDiscriminator;
    data.shortDiscriminator = ((nodeData.commissionData.longDiscriminator >> 8) & 0x0F);
    data.vendorId           = nodeData.commissionData.vendorId;
    data.productId          = nodeData.commissionData.productId;
    data.commissioningMode  = nodeData.commissionData.commissioningMode;
    data.deviceType         = nodeData.commissionData.deviceType;
    data.deviceName         = chip::CharSpan(nodeData.commissionData.deviceName, strlen(nodeData.commissionData.deviceName));
    data.rotatingId         = chip::ByteSpan(nodeData.commissionData.rotatingId, nodeData.commissionData.rotatingIdLen);
    data.rotatingIdLen      = nodeData.commissionData.rotatingIdLen;
    data.pairingHint        = nodeData.commissionData.pairingHint;
    data.pairingInstruction =
        chip::CharSpan(nodeData.commissionData.pairingInstruction, strlen(nodeData.commissionData.pairingInstruction));
    data.supportsTcp = nodeData.resolutionData.supportsTcp;
    data.port        = nodeData.resolutionData.port;

    if (!chip::CanCastTo<uint8_t>(nodeData.resolutionData.numIPs))
    {
        ChipLogError(chipTool, "Too many ips.");
        return;
    }
    data.numIPs = static_cast<uint8_t>(nodeData.resolutionData.numIPs);

    if (nodeData.resolutionData.mrpRetryIntervalIdle.HasValue())
    {
        data.mrpRetryIntervalIdle.SetValue(nodeData.resolutionData.mrpRetryIntervalIdle.Value().count());
    }

    if (nodeData.resolutionData.mrpRetryIntervalActive.HasValue())
    {
        data.mrpRetryIntervalActive.SetValue(nodeData.resolutionData.mrpRetryIntervalActive.Value().count());
    }

    chip::app::StatusIB status;
    status.mStatus = chip::Protocols::InteractionModel::Status::Success;

    constexpr uint32_t kMaxDataLen = 4096;
    uint8_t * buffer               = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), kMaxDataLen));
    if (buffer == nullptr)
    {
        ChipLogError(chipTool, "Can not dispatch mdns data: %s", chip::ErrorStr(CHIP_ERROR_NO_MEMORY));
        return;
    }

    chip::TLV::TLVWriter writer;
    writer.Init(buffer, kMaxDataLen);
    CHIP_ERROR err = data.Encode(writer, chip::TLV::AnonymousTag());
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(chipTool, "Can not encode mdns data: %s", chip::ErrorStr(err));
        return;
    }

    uint32_t dataLen = writer.GetLengthWritten();
    writer.Finalize();

    chip::TLV::TLVReader reader;
    reader.Init(buffer, dataLen);
    reader.Next();

    OnResponse(status, &reader);

    chip::Platform::MemoryFree(buffer);
}
