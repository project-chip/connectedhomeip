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

#include "ClosurePanel.h"
#include <devices/Types.h>

namespace {

CHIP_ERROR ValidateClosurePanelTagList(chip::Span<const chip::app::EndpointComposition::SemanticTag> tags)
{
    size_t panelTagCount = 0;
    for (auto & tag : tags)
    {
        VerifyOrReturnError(tag.namespaceID != chip::app::CommonNamespace::kClosureId, CHIP_ERROR_INVALID_ARGUMENT);
        if (tag.namespaceID == chip::app::CommonNamespace::kClosurePanelId)
        {
            ++panelTagCount;
        }
    }
    VerifyOrReturnError(panelTagCount == 1, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip::app {
namespace {

// A panel starts out fully closed.
constexpr Percent100ths kFullyClosedPosition = 10000;

/// The state a closed, unlatched panel starts in. Members are only populated for the features the
/// panel actually has: SetCurrentState rejects a member whose feature is unsupported.
Clusters::ClosureDimension::GenericDimensionStateStruct DefaultCurrentState(BitFlags<Clusters::ClosureDimension::Feature> features)
{
    using Clusters::ClosureDimension::Feature;

    Optional<DataModel::Nullable<Percent100ths>> position = features.Has(Feature::kPositioning)
        ? MakeOptional(DataModel::MakeNullable<Percent100ths>(kFullyClosedPosition))
        : NullOptional;
    // Unlatched: a latched panel rejects any SetTarget that moves the position without also unlatching.
    Optional<DataModel::Nullable<bool>> latch =
        features.Has(Feature::kMotionLatching) ? MakeOptional(DataModel::MakeNullable(false)) : NullOptional;
    Optional<Clusters::Globals::ThreeLevelAutoEnum> speed =
        features.Has(Feature::kSpeed) ? MakeOptional(Clusters::Globals::ThreeLevelAutoEnum::kAuto) : NullOptional;

    return Clusters::ClosureDimension::GenericDimensionStateStruct(position, latch, speed);
}

} // namespace

ClosurePanel::ClosurePanel(Clusters::ClosureDimension::ClosureDimensionClusterDelegate & dimensionDelegate, Config config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kClosurePanel, 1)), mConfig(config),
    mDimensionDelegate(dimensionDelegate)
{}

CHIP_ERROR ClosurePanel::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    ReturnErrorOnFailure(ValidateClosurePanelTagList(composition.tagList));

    VerifyOrReturnError(mConfig.positioning.has_value() || mConfig.motionLatching.has_value(), CHIP_ERROR_INVALID_ARGUMENT);

    DeviceRegistrationTransaction transaction(*this, provider);
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    Clusters::ClosureDimension::ClosureDimensionCluster::Config dimensionConfig(endpoint, mDimensionDelegate);
    if (mConfig.withAccess)
    {
        dimensionConfig.WithAccess();
    }
    if (mConfig.motionLatching.has_value())
    {
        dimensionConfig.WithMotionLatching(mConfig.motionLatching.value());
    }
    if (mConfig.positioning.has_value())
    {
        const PositioningParams & positioning = mConfig.positioning.value();
        dimensionConfig.WithPositioning(positioning.resolution, positioning.stepValue);
        // The motion parameters become mandatory attributes of the cluster, so an unset one would
        // be served as kUnknownEnumValue, which is out of range for the attribute.
        if (auto * translation = std::get_if<TranslationParams>(&positioning.motion))
        {
            VerifyOrReturnError(translation->direction != Clusters::ClosureDimension::TranslationDirectionEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            dimensionConfig.WithTranslation(translation->direction);
        }
        else if (auto * rotation = std::get_if<RotationParams>(&positioning.motion))
        {
            VerifyOrReturnError(rotation->axis != Clusters::ClosureDimension::RotationAxisEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(rotation->overflow != Clusters::ClosureDimension::OverflowEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            dimensionConfig.WithRotation(rotation->axis, rotation->overflow);
        }
        else if (auto * modulation = std::get_if<ModulationParams>(&positioning.motion))
        {
            VerifyOrReturnError(modulation->type != Clusters::ClosureDimension::ModulationTypeEnum::kUnknownEnumValue,
                                CHIP_ERROR_INVALID_ARGUMENT);
            dimensionConfig.WithModulation(modulation->type);
        }
    }

    mClosureDimensionCluster.Create(dimensionConfig);

    // SetTarget and Step are rejected with InvalidInState while CurrentState is null, so a panel has
    // to start from a known state.
    Clusters::ClosureDimension::ClosureDimensionCluster & cluster = mClosureDimensionCluster.Cluster();
    ReturnErrorOnFailure(cluster.SetCurrentState(DataModel::MakeNullable(DefaultCurrentState(cluster.GetFeatureMap()))));

    ReturnErrorOnFailure(provider.AddCluster(mClosureDimensionCluster.Registration()));
    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ClosurePanel::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mClosureDimensionCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mClosureDimensionCluster.Cluster()));
        mClosureDimensionCluster.Destroy();
    }
}

} // namespace chip::app
