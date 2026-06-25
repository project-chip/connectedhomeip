/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#include "OvenCavityOperationalStateCluster.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {
namespace OvenCavityOperationalState {

OvenCavityOperationalStateCluster::OvenCavityOperationalStateCluster(
    EndpointId endpointId, OperationalState::OperationalStateCluster::Delegate * delegate,
    const OperationalState::OperationalStateCluster::Config & config) :
    OperationalState::OperationalStateCluster(endpointId, OvenCavityOperationalState::Id, OvenCavityOperationalState::kRevision,
                                              delegate, config)
{}

CHIP_ERROR OvenCavityOperationalStateCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                               ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        OvenCavityOperationalState::Commands::Stop::kMetadataEntry,
        OvenCavityOperationalState::Commands::Start::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

} // namespace OvenCavityOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
