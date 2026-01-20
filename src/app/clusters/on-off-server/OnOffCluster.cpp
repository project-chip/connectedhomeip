/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/on-off-server/OnOffCluster.h>

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/OnOff/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::OnOff;
using chip::Protocols::InteractionModel::Status;

namespace chip::app::Clusters {

namespace {

class OnOffValidator : public scenes::AttributeValuePairValidator
{
public:
    CHIP_ERROR Validate(const app::ConcreteClusterPath & clusterPath,
                        AttributeValuePairValidator::AttributeValuePairType & value) override
    {
        VerifyOrReturnError(clusterPath.mClusterId == Clusters::OnOff::Id, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(value.attributeID == Attributes::OnOff::Id, CHIP_ERROR_INVALID_ARGUMENT);
        return CHIP_NO_ERROR;
    }
};

OnOffValidator & GlobalOnOffValidator()
{
    static OnOffValidator sValidator;
    return sValidator;
}

} // namespace

OnOffCluster::OnOffCluster(EndpointId endpointId, const Context & context) :
    OnOffCluster(endpointId, context, { Feature::kDeadFrontBehavior, Feature::kOffOnly })
{}

OnOffCluster::OnOffCluster(EndpointId endpointId, const Context & context, BitMask<Feature> supportedFeatures) :
    DefaultServerCluster({ endpointId, Clusters::OnOff::Id }), DefaultSceneHandlerImpl(GlobalOnOffValidator()),
    mFeatureMap(context.featureMap), mOnOff(context.defaults.onOff), mTimerDelegate(context.timerDelegate), mSceneTimer(*this)
{
    VerifyOrDie(supportedFeatures.HasAll(context.featureMap));

    // Feature validity check: offonly does not support any of the other features.
    VerifyOrDie(!context.featureMap.Has(Feature::kOffOnly) || context.featureMap.HasOnly(Feature::kOffOnly));
}

OnOffCluster::~OnOffCluster()
{
    mSceneTimer.Cancel();
    mDelegates.Clear();
}

CHIP_ERROR OnOffCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    AttributePersistence attributePersistence(context.attributeStorage);
    attributePersistence.LoadNativeEndianValue(ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id),
                                               mOnOff, mOnOff);

    for (auto & delegate : mDelegates)
    {
        delegate.OnOffStartup(mOnOff);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (mFeatureMap.Has(Feature::kOffOnly))
    {
        static constexpr DataModel::AcceptedCommandEntry kOffOnlyCommands[] = {
            Commands::Off::kMetadataEntry,
        };
        return builder.ReferenceExisting(kOffOnlyCommands);
    }

    static constexpr DataModel::AcceptedCommandEntry kAllCommands[] = {
        Commands::Off::kMetadataEntry,
        Commands::On::kMetadataEntry,
        Commands::Toggle::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAllCommands);
}

CHIP_ERROR OnOffCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), {});
}

DataModel::ActionReturnStatus OnOffCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case Attributes::ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case Attributes::FeatureMap::Id:
        return encoder.Encode(mFeatureMap.Raw());
    case Attributes::OnOff::Id:
        return encoder.Encode(mOnOff);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR OnOffCluster::SetOnOff(bool on)
{
    mSceneTimer.Cancel();

    VerifyOrReturnError(mOnOff != on, CHIP_NO_ERROR);

    mOnOff = on;
    NotifyAttributeChanged(Attributes::OnOff::Id);

    // Persist
    LogErrorOnFailure(
        mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id),
                                              ByteSpan(reinterpret_cast<const uint8_t *>(&mOnOff), sizeof(mOnOff))));

    for (auto & delegate : mDelegates)
    {
        delegate.OnOnOffChanged(mOnOff);
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> OnOffCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                         TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    // Note: OffOnly feature validation is handled by the Interaction Model by checking the AcceptedCommands list.
    switch (request.path.mCommandId)
    {
    case Commands::Off::Id:
        return SetOnOff(false);
    case Commands::On::Id:
        return SetOnOff(true);
    case Commands::Toggle::Id:
        return SetOnOff(!mOnOff);
    default:
        return Status::UnsupportedCommand;
    }
}

bool OnOffCluster::SupportsCluster(EndpointId endpoint, ClusterId cluster)
{
    return (cluster == Clusters::OnOff::Id) && (endpoint == mPath.mEndpointId);
}

CHIP_ERROR OnOffCluster::SerializeSave(EndpointId endpoint, ClusterId cluster, MutableByteSpan & serializedBytes)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    if (!SupportsCluster(endpoint, cluster))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::OnOff::Id;
    pairs[0].valueUnsigned8.SetValue(mOnOff);

    app::DataModel::List<AttributeValuePair> attributeValueList(pairs);
    return EncodeAttributeValueList(attributeValueList, serializedBytes);
}

CHIP_ERROR OnOffCluster::ApplyScene(EndpointId endpoint, ClusterId cluster, const ByteSpan & serializedBytes,
                                    scenes::TransitionTimeMs timeMs)
{
    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> attributeValueList;

    if (!SupportsCluster(endpoint, cluster))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(DecodeAttributeValueList(serializedBytes, attributeValueList));

    auto pair_iterator = attributeValueList.begin();
    while (pair_iterator.Next())
    {
        auto & decodePair = pair_iterator.GetValue();

        // Match codegen strictness: verify attribute ID is strictly OnOff and value is present
        VerifyOrReturnError(decodePair.attributeID == Attributes::OnOff::Id, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(decodePair.valueUnsigned8.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);

        bool targetValue = static_cast<bool>(decodePair.valueUnsigned8.Value());

        if (timeMs > 0)
        {
            mSceneTimer.Cancel();
            mSceneTimer.Start(timeMs, targetValue);
        }
        else
        {
            ReturnErrorOnFailure(SetOnOff(targetValue));
        }
    }
    return pair_iterator.GetStatus();
}

void OnOffCluster::SceneTransitionTimer::Start(uint32_t timeMs, bool targetValue)
{
    mTargetValue = targetValue;
    LogErrorOnFailure(mCluster.mTimerDelegate.StartTimer(this, System::Clock::Milliseconds32(timeMs)));
}

void OnOffCluster::SceneTransitionTimer::Cancel()
{
    mCluster.mTimerDelegate.CancelTimer(this);
}

void OnOffCluster::SceneTransitionTimer::TimerFired()
{
    LogErrorOnFailure(mCluster.SetOnOff(mTargetValue));
}

} // namespace chip::app::Clusters
