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

    // A panel expresses at most one motion aspect. std::monostate means none of the three is
    // configured, which is legal (e.g. a panel that only carries the Access feature).
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

    struct Config
    {
        bool withAccess = false;
        std::variant<std::monostate, TranslationParams, RotationParams, ModulationParams> motion;
    };

    ClosurePanel(Clusters::ClosureDimension::ClosureDimensionClusterDelegate & dimensionDelegate, Config config);
    ~ClosurePanel() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition) override;
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
