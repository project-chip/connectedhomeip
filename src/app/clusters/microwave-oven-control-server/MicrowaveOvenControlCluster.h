/*
 *    Copyright (c) 2023-2026 Project CHIP Authors
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

#include <app/InteractionModelEngine.h>
#include <app/clusters/microwave-oven-control-server/Delegate.h>
#include <app/clusters/mode-base-server/mode-base-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/MicrowaveOvenControl/Commands.h>
#include <clusters/MicrowaveOvenControl/Enums.h>

namespace chip::app::Clusters {

class MicrowaveOvenControlCluster : public DefaultServerCluster
{
public:
    // NOTE: this set is smaller than the full optional attributes supported by microwave oven control
    //       as other attributes are controlled by feature flags
    using OptionalAttributeSet = app::OptionalAttributeSet<MicrowaveOvenControl::Attributes::WattRating::Id>;

    struct Context
    {
        Clusters::OperationalState::Instance & opStateInstance;
        Clusters::ModeBase::Instance & microwaveOvenModeInstance;
        MicrowaveOvenControl::Delegate & delegate;
        InteractionModelEngine & interactionModelEngine;
        std::bitset<MicrowaveOvenControl::Commands::kAcceptedCommandsCount> acceptedCommands;
    };

    MicrowaveOvenControlCluster(EndpointId endpointId, BitMask<MicrowaveOvenControl::Feature> feature,
                                const OptionalAttributeSet & optionalAttributeSet, const Context context);

    // Server cluster implementation
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    uint8_t GetCountOfSupportedWattLevels() const;
    uint32_t GetCookTimeSec() const;
    void SetCookTimeSec(uint32_t cookTimeSec);

private:
    const BitMask<MicrowaveOvenControl::Feature> mFeature;
    const OptionalAttributeSet & mOptionalAttributeSet;
    MicrowaveOvenControl::Delegate & mDelegate;
    Clusters::OperationalState::Instance & mOpStateInstance;
    Clusters::ModeBase::Instance & mMicrowaveOvenModeInstance;
    InteractionModelEngine & mInteractionModelEngine;
    std::bitset<MicrowaveOvenControl::Commands::kAcceptedCommandsCount> mAcceptedCommands;

    uint32_t mCookTimeSec{};
    uint8_t mSupportedWattLevels{};

    /**
     * @brief Handle Command: SetCookingParameters.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * If the operational state is not in 'Stopped', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    std::optional<DataModel::ActionReturnStatus>
    HandleSetCookingParameters(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                               const MicrowaveOvenControl::Commands::SetCookingParameters::DecodableType & commandData);

    /**
     * @brief Handle Command: AddMoreTime.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the cook time value is out of range, returns the Interaction Model status code of CONSTRAINT_ERROR.
     * If the operational state is in 'Error', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    std::optional<DataModel::ActionReturnStatus>
    HandleAddMoreTime(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                      const MicrowaveOvenControl::Commands::AddMoreTime::DecodableType & commandData);
};

} // namespace chip::app::Clusters
