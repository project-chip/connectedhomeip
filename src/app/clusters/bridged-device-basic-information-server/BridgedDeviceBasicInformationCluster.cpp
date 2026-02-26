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

#include <app/persistence/AttributePersistence.h>
#include <app/persistence/String.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/BridgedDeviceBasicInformation/Attributes.h>
#include <clusters/BridgedDeviceBasicInformation/Commands.h>
#include <clusters/BridgedDeviceBasicInformation/Events.h>
#include <clusters/BridgedDeviceBasicInformation/Metadata.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>

#include <algorithm>

using namespace chip::app::Clusters::BridgedDeviceBasicInformation;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

namespace {

static constexpr uint32_t kMinKeepActiveTimeoutMs = 30 * 1000;
static constexpr uint32_t kMaxKeepActiveTimeoutMs = 3600 * 1000;

CharSpan ToSpan(const std::string & s)
{
    return { s.data(), s.size() };
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
    return SetNodeLabelInternal(nodeLabel, PersistenceMode::kPersist);
}

DataModel::ActionReturnStatus BridgedDeviceBasicInformationCluster::SetNodeLabelInternal(CharSpan nodeLabel, PersistenceMode mode)
{
    VerifyOrReturnError(nodeLabel.size() <= Attributes::NodeLabel::TypeInfo::MaxLength(), Status::ConstraintError);

    // std::string may not like a nullptr .data() when the charspan is empty.
    const std::string newValue = nodeLabel.empty() ? std::string() : std::string{ nodeLabel.data(), nodeLabel.size() };

    if (mRequiredData.nodeLabel == newValue)
    {
        return DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp;
    }

    auto status = mClusterContext.delegate.OnNodeLabelChanged(newValue);
    if (status != Status::Success)
    {
        return status;
    }

    if (mode == PersistenceMode::kPersist && mContext != nullptr)
    {
        AttributePersistence persistence(mContext->attributeStorage);
        Storage::String<Attributes::NodeLabel::TypeInfo::MaxLength()> storageString;
        storageString.SetContent(nodeLabel);
        CHIP_ERROR err = persistence.StoreString(
            { mPath.mEndpointId, BridgedDeviceBasicInformation::Id, Attributes::NodeLabel::Id }, storageString);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to persist NodeLabel: %" CHIP_ERROR_FORMAT, err.Format());
            return Status::Failure;
        }
    }

    mRequiredData.nodeLabel = newValue;
    NotifyAttributeChanged(Attributes::NodeLabel::Id);

    return Status::Success;
}

CHIP_ERROR BridgedDeviceBasicInformationCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence persistence(context.attributeStorage);
    Storage::String<Attributes::NodeLabel::TypeInfo::MaxLength()> storedLabel;

    if (persistence.LoadString({ mPath.mEndpointId, BridgedDeviceBasicInformation::Id, Attributes::NodeLabel::Id }, storedLabel))
    {
        // LoadString already handles logging in case of load errors.
        // We do not want to re-persist what we just loaded from NVM, hence kDoNotPersist.
        // Failure here is unlikely and most applications cannot recover from it, so we ignore the status.
        RETURN_SAFELY_IGNORED SetNodeLabelInternal(storedLabel.Content(), PersistenceMode::kDoNotPersist);
    }
    else
    {
        // missing label, we keep whatever is already in the cluster (e.g. set at startup)
        Storage::String<Attributes::NodeLabel::TypeInfo::MaxLength()> initialLabel;
        initialLabel.SetContent(ToSpan(mRequiredData.nodeLabel));

        // Ignore errors on purpose: not stored, but already an initial value from the app, so
        // same value is likely to be provided again. Failure to store here should not cause the cluster
        // to stop initializing.
        RETURN_SAFELY_IGNORED persistence.StoreString(
            { mPath.mEndpointId, BridgedDeviceBasicInformation::Id, Attributes::NodeLabel::Id }, initialLabel);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BridgedDeviceBasicInformationCluster::IncreaseConfigurationVersion()
{
    ReturnErrorOnFailure(mClusterContext.parentVersionConfiguration.IncreaseConfigurationVersion());

    mRequiredData.configurationVersion++;
    NotifyAttributeChanged(Attributes::ConfigurationVersion::Id);

    return CHIP_NO_ERROR;
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

void BridgedDeviceBasicInformationCluster::Shutdown(ClusterShutdownType s)
{
    DefaultServerCluster::Shutdown(s);

    // if we are shutting down, stop processing active timers
    CancelPendingActiveTimer();

    // we also do not expect to stay active to be propagated. Cluster is inactive.
    mStayActiveDurationMs.reset();
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

        VerifyOrReturnError(commandData.timeoutMs >= kMinKeepActiveTimeoutMs, Status::ConstraintError);
        VerifyOrReturnError(commandData.timeoutMs <= kMaxKeepActiveTimeoutMs, Status::ConstraintError);

        if (!mStayActiveDurationMs.has_value())
        {
            // brand new "pending active" state
            mStayActiveDurationMs = commandData.stayActiveDuration;
            StartPendingActiveTimer(System::Clock::Milliseconds32(commandData.timeoutMs));
            return mClusterContext.icdDelegate->OnEnterPendingActive();
        }

        // already in pending active state, may need to sleep longer. The logic is:
        //   - we need to keep the "max" stay active duration
        //   - we need to potentially extend our timeout
        mStayActiveDurationMs = std::max(mStayActiveDurationMs.value_or(0), commandData.stayActiveDuration);
        StartPendingActiveTimer(System::Clock::Milliseconds32(commandData.timeoutMs));

        return Status::Success;
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

void BridgedDeviceBasicInformationCluster::TimerFired()
{
    VerifyOrReturn(mStayActiveDurationMs.has_value());

    mStayActiveDurationMs.reset();

    VerifyOrReturn(mClusterContext.icdDelegate != nullptr);
    mClusterContext.icdDelegate->OnPendingActiveExpired();
}

void BridgedDeviceBasicInformationCluster::StartPendingActiveTimer(System::Clock::Milliseconds32 timeout)
{
    System::Clock::Timestamp nextTimeout = mClusterContext.timerDelegate.GetCurrentMonotonicTimestamp() + timeout;

    // extending the timeout (or no timeout at all...)
    VerifyOrReturn(nextTimeout > mPendingActiveExpiryTime);

    // start a new timer
    CancelPendingActiveTimer();
    LogErrorOnFailure(mClusterContext.timerDelegate.StartTimer(this, timeout));
    mPendingActiveExpiryTime = nextTimeout;
}

void BridgedDeviceBasicInformationCluster::CancelPendingActiveTimer()
{
    mClusterContext.timerDelegate.CancelTimer(this);

    // mark as "expired now" to not assume there is a pending timer in `Start`
    mPendingActiveExpiryTime = mClusterContext.timerDelegate.GetCurrentMonotonicTimestamp();
}

void BridgedDeviceBasicInformationCluster::NotifyDeviceActive()
{
    VerifyOrReturn(mStayActiveDurationMs.has_value());

    if (mContext != nullptr)
    {
        BridgedDeviceBasicInformation::Events::ActiveChanged::Type event;
        event.promisedActiveDuration = *mStayActiveDurationMs;
        mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
    }

    CancelPendingActiveTimer();
    mStayActiveDurationMs.reset();
}

} // namespace chip::app::Clusters
