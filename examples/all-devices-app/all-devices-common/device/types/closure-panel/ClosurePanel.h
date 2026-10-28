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

#pragma once

#include <app/clusters/closure-dimension-server/ClosureDimensionCluster.h>
#include <app/clusters/closure-dimension-server/ClosureDimensionClusterDelegate.h>
#include <device/api/SingleEndpoint.h>
#include <variant>

namespace chip::app {

class ClosurePanel : public SingleEndpoint
{
public:
    using SingleEndpoint::Register;

    struct TranslationParams
    {
        Clusters::ClosureDimension::TranslationDirectionEnum direction =
            Clusters::ClosureDimension::TranslationDirectionEnum::kUnknownEnumValue;
    };

    struct RotationParams
    {
        Clusters::ClosureDimension::RotationAxisEnum axis = Clusters::ClosureDimension::RotationAxisEnum::kUnknownEnumValue;
        Clusters::ClosureDimension::OverflowEnum overflow = Clusters::ClosureDimension::OverflowEnum::kUnknownEnumValue;
    };

    struct ModulationParams
    {
        Clusters::ClosureDimension::ModulationTypeEnum type = Clusters::ClosureDimension::ModulationTypeEnum::kUnknownEnumValue;
    };

    struct UnitParams
    {
        Clusters::ClosureDimension::ClosureUnitEnum unit = Clusters::ClosureDimension::ClosureUnitEnum::kUnknownEnumValue;
        DataModel::Nullable<Clusters::ClosureDimension::Structs::UnitRangeStruct::Type> range;
    };

    // Positioning (PS) requires exactly one of Translation / Rotation / Modulation, and
    // none of them is allowed without PS, so the motion choice lives inside the PS params.
    // Unit (UN) also requires PS, so it lives here too.
    struct PositioningParams
    {
        Percent100ths resolution;
        Percent100ths stepValue;
        std::variant<TranslationParams, RotationParams, ModulationParams> motion;
        std::optional<UnitParams> unit;
    };

    // At least one of positioning / motionLatching must be set.
    struct Config
    {
        bool withAccess = false;
        std::optional<BitFlags<Clusters::ClosureDimension::LatchControlModesBitmap>> motionLatching;
        std::optional<PositioningParams> positioning;
    };

    ClosurePanel(Clusters::ClosureDimension::ClosureDimensionClusterDelegate & dimensionDelegate, Config config);
    ~ClosurePanel() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::ClosureDimension::ClosureDimensionCluster & ClosureDimensionCluster()
    {
        VerifyOrDie(mClosureDimensionCluster.IsConstructed());
        return mClosureDimensionCluster.Cluster();
    }

private:
    const Config mConfig;
    Clusters::ClosureDimension::ClosureDimensionClusterDelegate & mDimensionDelegate;
    LazyRegisteredServerCluster<Clusters::ClosureDimension::ClosureDimensionCluster> mClosureDimensionCluster;
};

} // namespace chip::app
