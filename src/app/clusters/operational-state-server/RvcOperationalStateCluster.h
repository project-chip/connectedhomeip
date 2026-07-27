/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#pragma once

#include "OperationalStateCluster.h"

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {

class RvcOperationalStateCluster : public OperationalState::OperationalStateCluster
{
public:
    RvcOperationalStateCluster(EndpointId endpointId, OperationalState::OperationalStateCluster::Delegate * delegate,
                               const OperationalState::OperationalStateCluster::Config & config = {});

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

protected:
    bool IsDerivedClusterStatePauseCompatible(uint8_t aState) override;
    bool IsDerivedClusterStateResumeCompatible(uint8_t aState) override;

    std::optional<DataModel::ActionReturnStatus>
    HandleDerivedClusterCommand(const ConcreteCommandPath & path, chip::TLV::TLVReader & input, CommandHandler * handler) override;

private:
    std::optional<DataModel::ActionReturnStatus> HandleGoHomeCommand(const ConcreteCommandPath & path, chip::TLV::TLVReader & input,
                                                                     CommandHandler * handler);
};

} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
