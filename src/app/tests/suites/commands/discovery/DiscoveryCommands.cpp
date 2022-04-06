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

CHIP_ERROR DiscoveryCommandResult::Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
{
    chip::TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, chip::TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(0), hostName));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(1), instanceName));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(2), longDiscriminator));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(3), shortDiscriminator));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(4), vendorId));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(5), productId));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(6), commissioningMode));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(7), deviceType));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(8), deviceName));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(9), rotatingId));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(10), rotatingIdLen));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(11), pairingHint));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(12), pairingInstruction));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(13), supportsTcp));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(14), numIPs));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(15), port));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(16), mrpRetryIntervalIdle));
    ReturnErrorOnFailure(chip::app::DataModel::Encode(writer, chip::TLV::ContextTag(17), mrpRetryIntervalActive));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiscoveryCommandResult::Decode(chip::TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVType outer;
    VerifyOrReturnError(chip::TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(chip::TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (chip::TLV::TagNumFromTag(reader.GetTag()))
        {
        case 0:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, hostName));
            break;
        case 1:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, instanceName));
            break;
        case 2:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, longDiscriminator));
            break;
        case 3:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, shortDiscriminator));
            break;
        case 4:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, vendorId));
            break;
        case 5:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, productId));
            break;
        case 6:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, commissioningMode));
            break;
        case 7:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, deviceType));
            break;
        case 8:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, deviceName));
            break;
        case 9:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, rotatingId));
            break;
        case 10:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, rotatingIdLen));
            break;
        case 11:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, pairingHint));
            break;
        case 12:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, pairingInstruction));
            break;
        case 13:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, supportsTcp));
            break;
        case 14:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, numIPs));
            break;
        case 15:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, port));
            break;
        case 16:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, mrpRetryIntervalIdle));
            break;
        case 17:
            ReturnErrorOnFailure(chip::app::DataModel::Decode(reader, mrpRetryIntervalActive));
            break;
        default:
            break;
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));

    return CHIP_NO_ERROR;
};
