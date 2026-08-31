/*
 *
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

#include <app/clusters/service-area-server/service-area-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <device/types/robotic-vacuum-cleaner/impl/RvcSimulationTopology.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace all_devices {
namespace rvc_simulation {

/**
 * Populates SupportedMaps and SupportedAreas with the default topology used by
 * all-devices-app robotic vacuum certification tests.
 */
template <typename ServiceAreaClusterLike>
void ApplyDefaultMapTopology(ServiceAreaClusterLike & cluster)
{
    using namespace Clusters;
    using namespace Clusters::ServiceArea;
    using DataModel::Nullable;
    using DataModel::NullNullable;

    cluster.ClearSupportedMaps();
    cluster.AddSupportedMap(Topology::kMapIdXX, "My Map XX"_span);
    cluster.AddSupportedMap(Topology::kMapIdYY, "My Map YY"_span);

    auto areaA = AreaStructureWrapper{}
                     .SetAreaId(Topology::kAreaIdA)
                     .SetMapId(Topology::kMapIdXX)
                     .SetLocationInfo("My Location A"_span, Nullable<int16_t>(4), Nullable<Globals::AreaTypeTag>());

    auto areaB = AreaStructureWrapper{}
                     .SetAreaId(Topology::kAreaIdB)
                     .SetMapId(Topology::kMapIdXX)
                     .SetLocationInfo("My Location B"_span, NullNullable, NullNullable);

    auto areaC = AreaStructureWrapper{}
                     .SetAreaId(Topology::kAreaIdC)
                     .SetMapId(Topology::kMapIdYY)
                     .SetLocationInfo(""_span, -1, Globals::AreaTypeTag::kPlayRoom)
                     .SetLandmarkInfo(Globals::LandmarkTag::kBackDoor, Globals::RelativePositionTag::kNextTo);

    auto areaD = AreaStructureWrapper{}
                     .SetAreaId(Topology::kAreaIdD)
                     .SetMapId(Topology::kMapIdYY)
                     .SetLocationInfo("My Location D"_span, NullNullable, NullNullable)
                     .SetLandmarkInfo(Globals::LandmarkTag::kCouch, Globals::RelativePositionTag::kNextTo);

    cluster.ClearSupportedAreas();
    cluster.AddSupportedArea(areaA);
    cluster.AddSupportedArea(areaB);
    cluster.AddSupportedArea(areaC);
    cluster.AddSupportedArea(areaD);
}

} // namespace rvc_simulation
} // namespace all_devices
} // namespace app
} // namespace chip
