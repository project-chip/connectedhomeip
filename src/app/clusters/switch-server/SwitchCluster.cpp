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

// According to the spec, the minimum value is 2.
constexpr uint8_t kMinNumberOfPositions = 2;

SwitchCluster::SwitchCluster(EndpointId endpointId, const BitFlags<Feature> features,
                             const OptionalAttributeSet & optionalAttributeSet, const StartupConfiguration & config) :
    DefaultServerCluster({ endpointId, Switch::Id }),
    mFeatures(features), mOptionalAttributeSet(optionalAttributeSet), mConfig(config)
{
    mNumberOfPositions = kMinNumberOfPositions;
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
        return encoder.Encode(mConfig.multiPressMax);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR SwitchCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { mOptionalAttributeSet.IsSet(MultiPressMax::Id) && mFeatures.Has(Feature::kMomentarySwitchMultiPress),
          MultiPressMax::kMetadataEntry }
    };

    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR SwitchCluster::SetNumberOfPositions(uint8_t numberOfPositions)
{
    VerifyOrReturnError(numberOfPositions >= kMinNumberOfPositions, CHIP_ERROR_INVALID_ARGUMENT);

    SetAttributeValue(mNumberOfPositions, numberOfPositions, NumberOfPositions::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SwitchCluster::SetCurrentPosition(uint8_t currentPosition)
{
    // According to the spec, the valid range is zero to NumberOfPositions - 1.
    VerifyOrReturnError(currentPosition >= 0 && currentPosition <= mNumberOfPositions - 1, CHIP_ERROR_INVALID_ARGUMENT);

    SetAttributeValue(mCurrentPosition, currentPosition, CurrentPosition::Id);
    return CHIP_NO_ERROR;
}

void SwitchCluster::OnSwitchLatch(uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnSwitchLatch");

    VerifyOrReturn(mContext != nullptr);
    Events::SwitchLatched::Type event{ newPosition };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnInitialPress(uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnInitialPress");

    VerifyOrReturn(mContext != nullptr);
    Events::InitialPress::Type event{ newPosition };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnLongPress(uint8_t newPosition)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnLongPress");

    VerifyOrReturn(mContext != nullptr);
    Events::LongPress::Type event{ newPosition };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnShortRelease(uint8_t previousPosition)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnShortRelease");

    VerifyOrReturn(mContext != nullptr);
    Events::ShortRelease::Type event{ previousPosition };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnLongRelease(uint8_t previousPosition)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnLongRelease");

    VerifyOrReturn(mContext != nullptr);
    Events::LongRelease::Type event{ previousPosition };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnMultiPressOngoing(uint8_t newPosition, uint8_t count)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnMultiPressOngoing");

    VerifyOrReturn(mContext != nullptr);
    Events::MultiPressOngoing::Type event{ newPosition, count };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void SwitchCluster::OnMultiPressComplete(uint8_t previousPosition, uint8_t count)
{
    ChipLogProgress(Zcl, "SwitchCluster: OnMultiPressComplete");

    VerifyOrReturn(mContext != nullptr);
    Events::MultiPressComplete::Type event{ previousPosition, count };
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

} // namespace chip::app::Clusters
