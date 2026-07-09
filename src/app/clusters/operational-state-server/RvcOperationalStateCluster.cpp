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

#include "RvcOperationalStateCluster.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace app {
namespace Clusters {
namespace RvcOperationalState {

namespace {

// Builds the standard OperationalCommandResponse carrying `err` and adds it to `handler`.
std::optional<DataModel::ActionReturnStatus> AddCommandResponse(const ConcreteCommandPath & path, CommandHandler * handler,
                                                                const OperationalState::GenericOperationalError & err)
{
    OperationalState::Commands::OperationalCommandResponse::Type response;
    response.commandResponseState = err;
    handler->AddResponse(path, response);
    return std::nullopt;
}

} // namespace

RvcOperationalStateCluster::RvcOperationalStateCluster(EndpointId endpointId,
                                                       OperationalState::OperationalStateCluster::Delegate * delegate,
                                                       const OperationalState::OperationalStateCluster::Config & config) :
    OperationalState::OperationalStateCluster(endpointId, RvcOperationalState::Id, RvcOperationalState::kRevision, delegate, config)
{}

CHIP_ERROR
RvcOperationalStateCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kCommands[] = {
        RvcOperationalState::Commands::Pause::kMetadataEntry,
        RvcOperationalState::Commands::Resume::kMetadataEntry,
        RvcOperationalState::Commands::GoHome::kMetadataEntry,
    };
    return builder.ReferenceExisting(Span(kCommands));
}

bool RvcOperationalStateCluster::IsDerivedClusterStatePauseCompatible(uint8_t aState)
{
    return aState == to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger);
}

bool RvcOperationalStateCluster::IsDerivedClusterStateResumeCompatible(uint8_t aState)
{
    return (aState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
            aState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked));
}

std::optional<DataModel::ActionReturnStatus>
RvcOperationalStateCluster::HandleDerivedClusterCommand(const ConcreteCommandPath & path, chip::TLV::TLVReader & input,
                                                        CommandHandler * handler)
{
    switch (path.mCommandId)
    {
    case RvcOperationalState::Commands::GoHome::Id:
        return HandleGoHomeCommand(path, input, handler);
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

std::optional<DataModel::ActionReturnStatus> RvcOperationalStateCluster::HandleGoHomeCommand(const ConcreteCommandPath & path,
                                                                                             chip::TLV::TLVReader & input,
                                                                                             CommandHandler * handler)
{
    if (input.VerifyEndOfContainer() != CHIP_NO_ERROR)
    {
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    OperationalState::GenericOperationalError err(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    uint8_t opState = GetCurrentOperationalState();

    if (opState == to_underlying(RvcOperationalState::OperationalStateEnum::kCharging) ||
        opState == to_underlying(RvcOperationalState::OperationalStateEnum::kDocked))
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kCommandInvalidInState));
    }

    if (err.errorStateID == 0 && opState != to_underlying(RvcOperationalState::OperationalStateEnum::kSeekingCharger))
    {
        GetDelegate()->HandleGoHomeCommandCallback(err);
    }

    return AddCommandResponse(path, handler, err);
}

} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
