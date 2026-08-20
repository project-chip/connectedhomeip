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

// These implementations are here to allow compilation of imgui uis even
// when not all ember clusters are available.
//
// This is a WORKAROUND and should probably be fixed.
//
// See https://github.com/project-chip/connectedhomeip/issues/42984
//

#include <app/clusters/boolean-state-server/CodegenIntegration.h>
#include <app/clusters/occupancy-sensor-server/OccupancySensingCluster.h>

namespace {

struct LogNotAvailable
{
    LogNotAvailable(const char * what)
    {
        ChipLogError(AppServer, "WARNING: %s is NOT available/linked in. UI functionality on this will NOT work.", what);
    }
};

} // namespace

namespace chip::app::Clusters {

namespace OccupancySensing {

__attribute__((weak)) OccupancySensingCluster * FindClusterOnEndpoint(EndpointId endpointId)
{
    static LogNotAvailable logger("Occupancy Sensing Cluster");
    return nullptr;
}

} // namespace OccupancySensing

namespace BooleanState {

__attribute__((weak)) BooleanStateCluster * FindClusterOnEndpoint(EndpointId endpointId)
{

    static LogNotAvailable logger("Boolean State Cluster");
    return nullptr;
}

} // namespace BooleanState

} // namespace chip::app::Clusters
