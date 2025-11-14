/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/SafeAttributePersistenceProvider.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Metadata.h>
#include <clusters/HepaFilterMonitoring/Metadata.h>
#include <platform/DeviceInfoProvider.h>
#include <tracing/macros.h>

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using namespace chip::app::Clusters::ResourceMonitoring::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

ResourceMonitoringCluster::ResourceMonitoringCluster(
    EndpointId aEndpointId, ClusterId aClusterId, const BitFlags<ResourceMonitoring::Feature> enabledFeatures,
    chip::Optional<OptionalAttributeSet> optionalAttributeSet,
    ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
    bool aResetConditionCommandSupported) :
    DefaultServerCluster(ConcreteClusterPath(aEndpointId, aClusterId)),
    mDegradationDirection(aDegradationDirection), mResetConditionCommandSupported(aResetConditionCommandSupported),
    mEnabledFeatures(enabledFeatures), mOptionalAttributeSet(optionalAttributeSet)
{}

CHIP_ERROR ResourceMonitoringCluster::SetDelegate(Delegate * aDelegate)
{
    VerifyOrReturnError(aDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mDelegate = aDelegate;
    mDelegate->SetInstance(this);
    return mDelegate->Init();
}

DataModel::ActionReturnStatus ResourceMonitoringCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                        AttributeValueDecoder & decoder)
{
    return NotifyAttributeChangedIfSuccess(request.path.mAttributeId, WriteImpl(request, decoder));
}

DataModel::ActionReturnStatus ResourceMonitoringCluster::WriteImpl(const DataModel::WriteAttributeRequest & request,
                                                                   AttributeValueDecoder & decoder)
{
    AttributePersistence persistence{ mContext->attributeStorage };

    switch (request.path.mAttributeId)
    {
    case ResourceMonitoring::Attributes::LastChangedTime::Id: {

        uint32_t lastChangedTime = 0; // Initialize to a known value

        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(request.path, decoder, lastChangedTime);
        if (status == CHIP_NO_ERROR)
        {
            mLastChangedTime = DataModel::MakeNullable(lastChangedTime);
        }
        return status;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedWrite;
    }
}

// Implements the read functionality for non-standard attributes.
DataModel::ActionReturnStatus ResourceMonitoringCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                       AttributeValueEncoder & encoder)
{

    ChipLogDetail(Zcl, "ResourceMonitoringCluster::ReadAttribute: clusterId=0x%04lX, attributeId=0x%04lX", request.path.mClusterId,
                  request.path.mAttributeId);

    switch (request.path.mAttributeId)
    {
    case ResourceMonitoring::Attributes::Condition::Id:
        return encoder.Encode(mCondition);

    case ResourceMonitoring::Attributes::FeatureMap::Id:
        return encoder.Encode(mEnabledFeatures);

    case ResourceMonitoring::Attributes::DegradationDirection::Id:
        return encoder.Encode(mDegradationDirection);

    case ResourceMonitoring::Attributes::ChangeIndication::Id:
        return encoder.Encode(mChangeIndication);

    case ResourceMonitoring::Attributes::InPlaceIndicator::Id:
        return encoder.Encode(mInPlaceIndicator);

    case ResourceMonitoring::Attributes::LastChangedTime::Id:
        return encoder.Encode(mLastChangedTime);

    case ResourceMonitoring::Attributes::ReplacementProductList::Id: {
        CHIP_ERROR err = ReadReplaceableProductList(encoder);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Error reading ReplacementProductList attribute: %" CHIP_ERROR_FORMAT, err.Format());
        }
        return DataModel::ActionReturnStatus{ err };
    }
    case ResourceMonitoring::Attributes::ClusterRevision::Id:
        return encoder.Encode(HepaFilterMonitoring::kRevision);

    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ResourceMonitoringCluster::Attributes(const ConcreteClusterPath & path,
                                                 ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const bool haveCondition = mEnabledFeatures.Has(Feature::kCondition);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributesEntries[] = {
        { haveCondition, Condition::kMetadataEntry },
        { haveCondition, DegradationDirection::kMetadataEntry },
        { mOptionalAttributeSet.HasValue() && mOptionalAttributeSet.Value().IsSet(InPlaceIndicator::Id),
          InPlaceIndicator::kMetadataEntry },
        { mOptionalAttributeSet.HasValue() && mOptionalAttributeSet.Value().IsSet(LastChangedTime::Id),
          LastChangedTime::kMetadataEntry },
    };

    return listBuilder.Append(Span(HepaFilterMonitoring::Attributes::kMandatoryMetadata), Span(optionalAttributesEntries));
}

CHIP_ERROR ResourceMonitoringCluster::ReadReplaceableProductList(AttributeValueEncoder & aEncoder)
{
    ChipLogDetail(Zcl, "ResourceMonitoringCluster::ReadReplaceableProductList: clusterId=0x%04lX, attributeId=0x%04lX",
                  mPath.mClusterId, ResourceMonitoring::Attributes::ReplacementProductList::Id);

    VerifyOrReturnError(mEnabledFeatures.Has(ResourceMonitoring::Feature::kReplacementProductList), CHIP_NO_ERROR);

    ReplacementProductListManager * productListManagerInstance = GetReplacementProductListManagerInstance();
    if (nullptr == productListManagerInstance)
    {
        aEncoder.EncodeEmptyList();
        return CHIP_NO_ERROR;
    }

    productListManagerInstance->Reset();

    return aEncoder.EncodeList([productListManagerInstance](const auto & encoder) -> CHIP_ERROR {
        ReplacementProductStruct replacementProductStruct;
        CHIP_ERROR iteratorError = productListManagerInstance->Next(replacementProductStruct);

        while (CHIP_NO_ERROR == iteratorError)
        {
            ReturnErrorOnFailure(encoder.Encode(replacementProductStruct));
            iteratorError = productListManagerInstance->Next(replacementProductStruct);
        }

        return (CHIP_ERROR_PROVIDER_LIST_EXHAUSTED == iteratorError) ? CHIP_NO_ERROR : iteratorError;
    });
}

ResourceMonitoring::ReplacementProductListManager * ResourceMonitoringCluster::GetReplacementProductListManagerInstance()
{
    return mReplacementProductListManager;
}

void ResourceMonitoringCluster::SetReplacementProductListManagerInstance(
    ResourceMonitoring::ReplacementProductListManager * replacementProductListManager)
{
    mReplacementProductListManager = replacementProductListManager;
}

chip::Protocols::InteractionModel::Status ResourceMonitoringCluster::UpdateCondition(uint8_t newCondition)
{
    auto oldConditionattr = mCondition;
    mCondition            = newCondition;
    if (mCondition != oldConditionattr)
    {
        if (mPath.mClusterId == HepaFilterMonitoring::Id)
        {
            NotifyAttributeChanged(HepaFilterMonitoring::Attributes::Condition::Id);
        }

        if (mPath.mClusterId == ActivatedCarbonFilterMonitoring::Id)
        {
            NotifyAttributeChanged(ActivatedCarbonFilterMonitoring::Attributes::Condition::Id);
        }
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status ResourceMonitoringCluster::UpdateChangeIndication(
    chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum aNewChangeIndication)
{
    if (aNewChangeIndication == chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum::kWarning)
    {
        if (!mEnabledFeatures.Has(ResourceMonitoring::Feature::kWarning))
        {
            return Protocols::InteractionModel::Status::InvalidValue;
        }
    }
    auto oldChangeIndication = mChangeIndication;
    mChangeIndication        = aNewChangeIndication;
    if (mChangeIndication != oldChangeIndication)
    {
        NotifyAttributeChanged(ResourceMonitoring::Attributes::ChangeIndication::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status ResourceMonitoringCluster::UpdateInPlaceIndicator(bool newInPlaceIndicator)
{
    auto oldInPlaceIndicator = mInPlaceIndicator;
    mInPlaceIndicator        = newInPlaceIndicator;
    if (mInPlaceIndicator != oldInPlaceIndicator)
    {
        if (mPath.mClusterId == HepaFilterMonitoring::Id)
        {
            NotifyAttributeChanged(HepaFilterMonitoring::Attributes::InPlaceIndicator::Id);
        }

        if (mPath.mClusterId == ActivatedCarbonFilterMonitoring::Id)
        {
            NotifyAttributeChanged(ActivatedCarbonFilterMonitoring::Attributes::InPlaceIndicator::Id);
        }
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status
ResourceMonitoringCluster::UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime)
{
    auto oldLastchangedTime = mLastChangedTime;
    mLastChangedTime        = aNewLastChangedTime;
    if (mLastChangedTime != oldLastchangedTime)
    {
        if (mPath.mClusterId == HepaFilterMonitoring::Id)
        {
            chip::app::GetSafeAttributePersistenceProvider()->WriteScalarValue(
                ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, HepaFilterMonitoring::Attributes::LastChangedTime::Id),
                mLastChangedTime);
            NotifyAttributeChanged(HepaFilterMonitoring::Attributes::LastChangedTime::Id);
        }

        if (mPath.mClusterId == ActivatedCarbonFilterMonitoring::Id)
        {
            chip::app::GetSafeAttributePersistenceProvider()->WriteScalarValue(
                ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId,
                                      ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::Id),
                mLastChangedTime);
            NotifyAttributeChanged(ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::Id);
        }
    }
    return Protocols::InteractionModel::Status::Success;
}

void ResourceMonitoringCluster::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mPath.mClusterId == HepaFilterMonitoring::Id)
    {
        err = chip::app::GetSafeAttributePersistenceProvider()->ReadScalarValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId, HepaFilterMonitoring::Attributes::LastChangedTime::Id),
            mLastChangedTime);
    }
    else if (mPath.mClusterId == ActivatedCarbonFilterMonitoring::Id)
    {
        err = chip::app::GetSafeAttributePersistenceProvider()->ReadScalarValue(
            ConcreteAttributePath(mPath.mEndpointId, mPath.mClusterId,
                                  ActivatedCarbonFilterMonitoring::Attributes::LastChangedTime::Id),
            mLastChangedTime);
    }

    if (err == CHIP_NO_ERROR)
    {
        if (mLastChangedTime.IsNull())
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as null");
        }
        else
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as %lu",
                          (long unsigned int) mLastChangedTime.Value()); // on some platforms uint32_t is a long, cast it to
                                                                         // unsigned long on all platforms to prevent CI errors
        }
    }
    else
    {
        // If we cannot find the previous LastChangedTime, we will assume it to be null.
        ChipLogDetail(Zcl, "ResourceMonitoring: Unable to load the LastChangedTime from the KVS. Assuming null");
    }
}

CHIP_ERROR ResourceMonitoringCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));
    LoadPersistentAttributes();

    return CHIP_NO_ERROR;
}

// This method is called by the interaction model engine when a command destined for this instance is received.
std::optional<DataModel::ActionReturnStatus> ResourceMonitoringCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                      chip::TLV::TLVReader & input_arguments,
                                                                                      CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case ResourceMonitoring::Commands::ResetCondition::Id:
        ChipLogDetail(Zcl, "ResourceMonitoring::Commands::ResetCondition");

        ResourceMonitoring::Commands::ResetCondition::DecodableType data;
        ReturnErrorOnFailure(data.Decode(input_arguments));

        return ResetCondition(request.path, data, handler);
    }

    return Status::UnsupportedCommand;
}

std::optional<DataModel::ActionReturnStatus>
ResourceMonitoringCluster::ResetCondition(const ConcreteCommandPath & commandPath,
                                          const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData,
                                          CommandHandler * handler)
{
    Status resetConditionStatus = mDelegate->OnResetCondition();

    handler->AddStatus(commandPath, resetConditionStatus);

    return std::nullopt;
}

CHIP_ERROR ResourceMonitoringCluster::AcceptedCommands(const ConcreteClusterPath & cluster,
                                                       ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(1));
    if (mResetConditionCommandSupported)
    {
        ReturnErrorOnFailure(builder.Append(ResourceMonitoring::Commands::ResetCondition::kMetadataEntry));
    }

    return CHIP_NO_ERROR;
}

uint8_t ResourceMonitoringCluster::GetCondition() const
{
    return mCondition;
}
chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum ResourceMonitoringCluster::GetChangeIndication() const
{
    return mChangeIndication;
}

chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum ResourceMonitoringCluster::GetDegradationDirection() const
{
    return mDegradationDirection;
}

bool ResourceMonitoringCluster::GetInPlaceIndicator() const
{
    return mInPlaceIndicator;
}

DataModel::Nullable<uint32_t> ResourceMonitoringCluster::GetLastChangedTime() const
{
    return mLastChangedTime;
}

bool ResourceMonitoringCluster::HasFeature(ResourceMonitoring::Feature aFeature) const
{
    return mEnabledFeatures.Has(aFeature);
}

bool ResourceMonitoringCluster::HasOptionalAttribute(AttributeId aAttribute) const
{
    return mOptionalAttributeSet.HasValue() && mOptionalAttributeSet.Value().IsSet(aAttribute);
}

Protocols::InteractionModel::Status Delegate::OnResetCondition()
{
    // call application specific pre reset logic,
    // anything other than Success will cause the command to fail, and not do any of the resets
    auto status = PreResetCondition();
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }
    // Handle the reset of the condition attribute, if supported
    if (mInstance->HasFeature(ResourceMonitoring::Feature::kCondition))
    {
        if (mInstance->GetDegradationDirection() == DegradationDirectionEnum::kDown)
        {
            mInstance->UpdateCondition(100);
        }
        else if (mInstance->GetDegradationDirection() == DegradationDirectionEnum::kUp)
        {
            mInstance->UpdateCondition(0);
        }
    }

    // handle the reset of the ChangeIndication attribute, mandatory
    mInstance->UpdateChangeIndication(ChangeIndicationEnum::kOk);

    // Handle the reset of the LastChangedTime attribute, if supported
    if (mInstance->HasOptionalAttribute(ResourceMonitoring::Attributes::LastChangedTime::Id))
    {
        System::Clock::Milliseconds64 currentUnixTimeMS;
        CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(currentUnixTimeMS);
        if (err == CHIP_NO_ERROR)
        {
            System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
            mInstance->UpdateLastChangedTime(DataModel::MakeNullable(currentUnixTime.count()));
        }
    }

    // call application specific post reset logic
    status = PostResetCondition();
    return status;
}

Protocols::InteractionModel::Status Delegate::PreResetCondition()
{
    return Protocols::InteractionModel::Status::Success;
}

Protocols::InteractionModel::Status Delegate::PostResetCondition()
{
    return Protocols::InteractionModel::Status::Success;
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
