/*
 *    Copyright (c) 2021-2026 Project CHIP Authors
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

#include <app/clusters/switch-server/SwitchCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Switch/Enums.h>
#include <clusters/Switch/Events.h>
#include <clusters/Switch/Metadata.h>

namespace chip::app::Clusters {

using namespace Switch;
using namespace Switch::Attributes;

SwitchCluster::SwitchCluster(EndpointId endpointId, const BitFlags<Feature> features, const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, Switch::Id }), mFeatures(features), mNumberOfPositions(config.numberOfPositions),
    mMultiPressMax(config.multiPressMax)
{
    VerifyOrDie(mNumberOfPositions >= 2);
    VerifyOrDie(!mFeatures.Has(Feature::kMomentarySwitchMultiPress) || mMultiPressMax >= 2);
}

DataModel::ActionReturnStatus SwitchCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                           AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Switch::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case NumberOfPositions::Id:
        return encoder.Encode(mNumberOfPositions);
    case CurrentPosition::Id:
        return encoder.Encode(mCurrentPosition);
    case MultiPressMax::Id:
        return encoder.Encode(mMultiPressMax);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR SwitchCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mFeatures.Has(Feature::kMomentarySwitchMultiPress), MultiPressMax::kMetadataEntry }
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

inline bool SwitchCluster::PositionIsValid(uint8_t position) const
{
    // According to the spec, the valid range is zero to NumberOfPositions - 1.
    return (position <= mNumberOfPositions - 1);
}

CHIP_ERROR SwitchCluster::SetCurrentPosition(uint8_t currentPosition)
{
    VerifyOrReturnError(PositionIsValid(currentPosition), CHIP_ERROR_INVALID_ARGUMENT);

    SetAttributeValue(mCurrentPosition, currentPosition, CurrentPosition::Id);
    return CHIP_NO_ERROR;
}

std::optional<EventNumber> SwitchCluster::OnSwitchLatch(uint8_t newPosition)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kLatchingSwitch) && PositionIsValid(newPosition),
                        std::nullopt);

    Events::SwitchLatched::Type event{ newPosition };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnInitialPress(uint8_t newPosition)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitch) && PositionIsValid(newPosition),
                        std::nullopt);

    Events::InitialPress::Type event{ newPosition };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnLongPress(uint8_t newPosition)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitchLongPress) && PositionIsValid(newPosition),
                        std::nullopt);

    Events::LongPress::Type event{ newPosition };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnShortRelease(uint8_t previousPosition)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitchRelease) && PositionIsValid(previousPosition),
                        std::nullopt);

    Events::ShortRelease::Type event{ previousPosition };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnLongRelease(uint8_t previousPosition)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitchLongPress) &&
                            PositionIsValid(previousPosition),
                        std::nullopt);

    Events::LongRelease::Type event{ previousPosition };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnMultiPressOngoing(uint8_t newPosition, uint8_t count)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitchMultiPress) &&
                            !mFeatures.Has(Feature::kActionSwitch) && PositionIsValid(newPosition),
                        std::nullopt);

    Events::MultiPressOngoing::Type event{ newPosition, count };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

std::optional<EventNumber> SwitchCluster::OnMultiPressComplete(uint8_t previousPosition, uint8_t count)
{
    VerifyOrReturnValue(mContext != nullptr && mFeatures.Has(Feature::kMomentarySwitchMultiPress) &&
                            PositionIsValid(previousPosition),
                        std::nullopt);

    Events::MultiPressComplete::Type event{ previousPosition, count };
    return mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters
