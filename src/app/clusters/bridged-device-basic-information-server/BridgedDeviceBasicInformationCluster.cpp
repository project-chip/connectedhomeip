/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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
#include <app/clusters/bridged-device-basic-information-server/BridgedDeviceBasicInformationCluster.h>

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/BridgedDeviceBasicInformation/Commands.h>
#include <clusters/BridgedDeviceBasicInformation/Events.h>
#include <clusters/BridgedDeviceBasicInformation/Metadata.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

using namespace chip::app::Clusters::BridgedDeviceBasicInformation;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

namespace {

// TODO: These should come from the metadata
constexpr size_t kNodeLabelMaxLength = 32;

CharSpan ToSpan(const std::string & s)
{
    return CharSpan::fromCharString(s.c_str());
}

CharSpan ToSpan(const std::optional<std::string> & s)
{
    if (!s.has_value())
    {
        // Return empty span if not present
        return {};
    }
    return ToSpan(*s);
}

} // namespace

DataModel::ActionReturnStatus BridgedDeviceBasicInformationCluster::SetNodeLabel(CharSpan nodeLabel)
{
    VerifyOrReturnError(nodeLabel.size() <= kNodeLabelMaxLength, Status::ConstraintError);

    if (nodeLabel.data_equal(CharSpan::fromCharString(mRequiredData.nodeLabel.c_str())))
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    // std::string may not like a nullptr .data() when the charspan is empty.
    if (nodeLabel.empty())
    {
        mRequiredData.nodeLabel.clear();
    }
    else
    {
        mRequiredData.nodeLabel = { nodeLabel.data(), nodeLabel.size() };
    }
    NotifyAttributeChanged(Attributes::NodeLabel::Id);
    mClusterContext.delegate.OnNodeLabelChanged(mRequiredData.nodeLabel);
    return Status::Success;
}

void BridgedDeviceBasicInformationCluster::SetConfigurationVersion(uint32_t configurationVersion)
{
    VerifyOrReturn(mRequiredData.configurationVersion != configurationVersion);

    mRequiredData.configurationVersion = configurationVersion;
    NotifyAttributeChanged(Attributes::ConfigurationVersion::Id);
}

DataModel::ActionReturnStatus BridgedDeviceBasicInformationCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                                  AttributeValueEncoder & encoder)
{
    using namespace chip::app::Clusters::BridgedDeviceBasicInformation::Attributes;

    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(                                                                                           //
            BitFlags<BridgedDeviceBasicInformation::Feature>()                                                           //
                .Set(BridgedDeviceBasicInformation::Feature::kBridgedICDSupport, mClusterContext.icdDelegate != nullptr) //
        );
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case VendorName::Id:
        return encoder.Encode(ToSpan(mFixedData.vendorName));
    case VendorID::Id:
        return encoder.Encode(mFixedData.vendorId.value_or(VendorId::NotSpecified));
    case ProductName::Id:
        return encoder.Encode(ToSpan(mFixedData.productName));
    case ProductID::Id:
        return encoder.Encode(mFixedData.productId.value_or(0));
    case NodeLabel::Id:
        return encoder.Encode(ToSpan(mRequiredData.nodeLabel));
    case HardwareVersion::Id:
        return encoder.Encode(mFixedData.hardwareVersion.value_or(0));
    case HardwareVersionString::Id:
        return encoder.Encode(ToSpan(mFixedData.hardwareVersionString));
    case SoftwareVersion::Id:
        return encoder.Encode(mFixedData.softwareVersion.value_or(0));
    case SoftwareVersionString::Id:
        return encoder.Encode(ToSpan(mFixedData.softwareVersionString));
    case ManufacturingDate::Id:
        return encoder.Encode(ToSpan(mFixedData.manufacturingDate));
    case PartNumber::Id:
        return encoder.Encode(ToSpan(mFixedData.partNumber));
    case ProductURL::Id:
        return encoder.Encode(ToSpan(mFixedData.productUrl));
    case ProductLabel::Id:
        return encoder.Encode(ToSpan(mFixedData.productLabel));
    case SerialNumber::Id:
        return encoder.Encode(ToSpan(mFixedData.serialNumber));
    case Reachable::Id:
        return encoder.Encode(mRequiredData.reachable);
    case UniqueID::Id:
        return encoder.Encode(ToSpan(mRequiredData.uniqueId));
    case ProductAppearance::Id:
        return encoder.Encode(
            mFixedData.productAppearance.value_or(BridgedDeviceBasicInformation::Structs::ProductAppearanceStruct::Type{}));
    case ConfigurationVersion::Id:
        return encoder.Encode(mRequiredData.configurationVersion);
    default:
        return Status::UnsupportedAttribute;
    }
}

DataModel::ActionReturnStatus BridgedDeviceBasicInformationCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                                   AttributeValueDecoder & aDecoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::NodeLabel::Id: {
        CharSpan newNodeLabel;
        ReturnErrorOnFailure(aDecoder.Decode(newNodeLabel));
        return SetNodeLabel(newNodeLabel);
    }
    case Attributes::ConfigurationVersion::Id:
        return Status::UnsupportedWrite; // Not writable via Matter
    default:
        // Other attributes are not writable.
        return Status::UnsupportedWrite;
    }
}

CHIP_ERROR BridgedDeviceBasicInformationCluster::Attributes(const ConcreteClusterPath & path,
                                                            ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace chip::app::Clusters::BridgedDeviceBasicInformation::Attributes;

    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFixedData.vendorName.has_value(), VendorName::kMetadataEntry },
        { mFixedData.vendorId.has_value(), VendorID::kMetadataEntry },
        { mFixedData.productName.has_value(), ProductName::kMetadataEntry },
        { mFixedData.productId.has_value(), ProductID::kMetadataEntry },
        { true, NodeLabel::kMetadataEntry }, // Always present
        { mFixedData.hardwareVersion.has_value(), HardwareVersion::kMetadataEntry },
        { mFixedData.hardwareVersionString.has_value(), HardwareVersionString::kMetadataEntry },
        { mFixedData.softwareVersion.has_value(), SoftwareVersion::kMetadataEntry },
        { mFixedData.softwareVersionString.has_value(), SoftwareVersionString::kMetadataEntry },
        { mFixedData.manufacturingDate.has_value(), ManufacturingDate::kMetadataEntry },
        { mFixedData.partNumber.has_value(), PartNumber::kMetadataEntry },
        { mFixedData.productUrl.has_value(), ProductURL::kMetadataEntry },
        { mFixedData.productLabel.has_value(), ProductLabel::kMetadataEntry },
        { mFixedData.serialNumber.has_value(), SerialNumber::kMetadataEntry },
        { true, UniqueID::kMetadataEntry }, // mandatory for new revisions
        { mFixedData.productAppearance.has_value(), ProductAppearance::kMetadataEntry },
        { true, ConfigurationVersion::kMetadataEntry }, // Always present
    };

    AttributeListBuilder listBuilder(builder);

    // NOTE:
    //   - only Reachable is in kMandatory, even though UniqueID is mandatory for
    //     revision >= 4 (so we hardcode true above)
    return listBuilder.Append(Span(BridgedDeviceBasicInformation::Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR BridgedDeviceBasicInformationCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                                  ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mClusterContext.icdDelegate != nullptr)
    {
        static const DataModel::AcceptedCommandEntry kCommands[]{
            Commands::KeepActive::kMetadataEntry,
        };

        ReturnErrorOnFailure(builder.ReferenceExisting(kCommands));
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus>
BridgedDeviceBasicInformationCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                    chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    using namespace chip::app::Clusters::BridgedDeviceBasicInformation::Commands;
    switch (request.path.mCommandId)
    {
    case KeepActive::Id: {
        KeepActive::DecodableType commandData;
        ReturnErrorOnFailure(DataModel::Decode(input_arguments, commandData));

        VerifyOrReturnError(mClusterContext.icdDelegate != nullptr, Status::UnsupportedCommand);
        return mClusterContext.icdDelegate->OnKeepActive(commandData);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

void BridgedDeviceBasicInformationCluster::GenerateLeaveEvent()
{
    VerifyOrReturn(mContext != nullptr);

    BridgedDeviceBasicInformation::Events::Leave::Type event;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void BridgedDeviceBasicInformationCluster::GenerateActiveChangedEvent(uint32_t promisedActiveMs)
{
    VerifyOrReturn(mContext != nullptr);

    BridgedDeviceBasicInformation::Events::ActiveChanged::Type event;
    event.promisedActiveDuration = promisedActiveMs;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void BridgedDeviceBasicInformationCluster::SetReachable(bool reachable)
{
    VerifyOrReturn(mRequiredData.reachable != reachable);

    mRequiredData.reachable = reachable;
    NotifyAttributeChanged(Attributes::Reachable::Id);

    VerifyOrReturn(mContext != nullptr);
    BridgedDeviceBasicInformation::Events::ReachableChanged::Type event;
    event.reachableNewValue = reachable;
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters
