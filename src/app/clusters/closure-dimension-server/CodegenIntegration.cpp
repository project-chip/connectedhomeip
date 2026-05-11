/*
 *
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

#include "CodegenIntegration.h"
#include <app/clusters/closure-dimension-server/ClosureDimensionCluster.h>
#include <app/clusters/closure-dimension-server/ClosureDimensionClusterDelegate.h>
#include <clusters/ClosureDimension/Metadata.h>

#include <app/util/attribute-storage.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ClosureDimension;
using namespace chip::app::Clusters::ClosureDimension::Attributes;

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

Interface::Interface(EndpointId endpoint, ClosureDimensionClusterDelegate & delegate) : mEndpoint(endpoint), mDelegate(delegate) {}

CHIP_ERROR Interface::Init(ClusterConformance & conformance, ClusterInitParameters & initParams)
{
    mConformance = conformance;
    mInitParams  = initParams;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Interface::Init()
{
    ClosureDimensionCluster::Config config(mEndpoint, mDelegate);

    if (mConformance.HasFeature(Feature::kPositioning))
    {
        config.WithPositioning(mInitParams.resolution, mInitParams.stepValue);
    }
    if (mConformance.HasFeature(Feature::kMotionLatching))
    {
        config.WithMotionLatching(mInitParams.latchControlModes);
    }
    if (mConformance.HasFeature(Feature::kUnit))
    {
        config.WithUnit(mInitParams.unit, DataModel::Nullable<Structs::UnitRangeStruct::Type>());
    }
    if (mConformance.HasFeature(Feature::kLimitation))
    {
        config.WithLimitation(Structs::RangePercent100thsStruct::Type{});
    }
    if (mConformance.HasFeature(Feature::kSpeed))
    {
        config.WithSpeed();
    }
    if (mConformance.HasFeature(Feature::kTranslation))
    {
        config.WithTranslation(mInitParams.translationDirection);
    }
    if (mConformance.HasFeature(Feature::kRotation))
    {
        config.WithRotation(mInitParams.rotationAxis, mInitParams.overflow);
    }
    if (mConformance.HasFeature(Feature::kModulation))
    {
        config.WithModulation(mInitParams.modulationType);
    }

    mCluster.Create(config);
    return CodegenDataModelProvider::Instance().Registry().Register(mCluster.Registration());
}

CHIP_ERROR Interface::Shutdown()
{
    VerifyOrDie(mCluster.IsConstructed());
    return CodegenDataModelProvider::Instance().Registry().Unregister(&mCluster.Cluster());
}

CHIP_ERROR Interface::SetCurrentState(const DataModel::Nullable<GenericDimensionStateStruct> & currentState)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return mCluster.Cluster().SetCurrentState(currentState);
}
CHIP_ERROR Interface::SetTargetState(const DataModel::Nullable<GenericDimensionStateStruct> & targetState)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return mCluster.Cluster().SetTargetState(targetState);
}

CHIP_ERROR Interface::SetUnitRange(const DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return mCluster.Cluster().SetUnitRange(unitRange);
}
CHIP_ERROR Interface::SetLimitRange(const Structs::RangePercent100thsStruct::Type & limitRange)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    return mCluster.Cluster().SetLimitRange(limitRange);
}
CHIP_ERROR Interface::GetCurrentState(DataModel::Nullable<GenericDimensionStateStruct> & currentState)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    currentState = mCluster.Cluster().GetCurrentState();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetTargetState(DataModel::Nullable<GenericDimensionStateStruct> & targetState)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    targetState = mCluster.Cluster().GetTargetState();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetResolution(Percent100ths & resolution)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    resolution = mCluster.Cluster().GetResolution();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetStepValue(Percent100ths & stepValue)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    stepValue = mCluster.Cluster().GetStepValue();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetUnit(ClosureUnitEnum & unit)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    unit = mCluster.Cluster().GetUnit();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetUnitRange(DataModel::Nullable<Structs::UnitRangeStruct::Type> & unitRange)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    unitRange = mCluster.Cluster().GetUnitRange();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetLimitRange(Structs::RangePercent100thsStruct::Type & limitRange)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    limitRange = mCluster.Cluster().GetLimitRange();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetTranslationDirection(TranslationDirectionEnum & translationDirection)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    translationDirection = mCluster.Cluster().GetTranslationDirection();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetRotationAxis(RotationAxisEnum & rotationAxis)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    rotationAxis = mCluster.Cluster().GetRotationAxis();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetOverflow(OverflowEnum & overflow)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    overflow = mCluster.Cluster().GetOverflow();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetModulationType(ModulationTypeEnum & modulationType)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    modulationType = mCluster.Cluster().GetModulationType();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetLatchControlModes(BitFlags<LatchControlModesBitmap> & latchControlModes)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    latchControlModes = mCluster.Cluster().GetLatchControlModes();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetFeatureMap(BitFlags<Feature> & featureMap)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    featureMap = mCluster.Cluster().GetFeatureMap();
    return CHIP_NO_ERROR;
}
CHIP_ERROR Interface::GetClusterRevision(Attributes::ClusterRevision::TypeInfo::Type & clusterRevision)
{
    VerifyOrReturnError(mCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
    clusterRevision = kRevision;
    return CHIP_NO_ERROR;
}
ClusterConformance & Interface::GetConformance()
{
    return mConformance;
}
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterClosureDimensionClusterInitCallback(EndpointId endpointId) {}

void MatterClosureDimensionClusterShutdownCallback(EndpointId endpointId, MatterClusterShutdownType shutdownType) {}
// -----------------------------------------------------------------------------
// Plugin initialization

void MatterClosureDimensionPluginServerInitCallback() {}
