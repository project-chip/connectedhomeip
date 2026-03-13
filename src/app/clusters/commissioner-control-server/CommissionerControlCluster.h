/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/CommandHandlerInterface.h>
#include <app/clusters/commissioner-control-server/Delegate.h>

namespace chip::app::Clusters {

class CommissionerControlServer : public CommandHandlerInterface
{
public:
    /**
     * @brief Creates a Commissioner Control cluster instance. The Init() function needs to be called for this instance
     * to be registered and called by the interaction model at the appropriate times.
     * @param delegate A pointer to the delegate to be used by this server.
     * Note: the caller must ensure that the delegate lives throughout the instance's lifetime.
     * @param endpointId The endpoint on which this cluster exists. This must match the zap configuration.
     */
    CommissionerControlServer(CommissionerControl::Delegate * delegate, EndpointId endpointId);

    ~CommissionerControlServer() override;

    /**
     * @brief Initialise the Commissioner Control server instance.
     * This function must be called after defining an CommissionerControlServer class object.
     * @return Returns an error if the CommandHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    Protocols::InteractionModel::Status GetSupportedDeviceCategoriesValue(
        EndpointId endpoint, BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const;

    Protocols::InteractionModel::Status
    SetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                      const BitMask<CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories);

    /**
     * @brief
     *   Called after the server return SUCCESS to a correctly formatted RequestCommissioningApproval command.
     */
    CHIP_ERROR
    GenerateCommissioningRequestResultEvent(EndpointId endpoint,
                                            const CommissionerControl::Events::CommissioningRequestResult::Type & result);

private:
    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    /**
     * @brief Handle Command: SetCookingParameters.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the input value is invalid, returns the Interaction Model status code of INVALID_COMMAND.
     * If the operational state is not in 'Stopped', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    void HandleRequestCommissioningApproval(HandlerContext & ctx,
                                            const CommissionerControl::Commands::RequestCommissioningApproval::DecodableType & req);

    /**
     * @brief Handle Command: AddMoreTime.
     * @param ctx Returns the Interaction Model status code which was user determined in the business logic.
     * If the cook time value is out of range, returns the Interaction Model status code of CONSTRAINT_ERROR.
     * If the operational state is in 'Error', returns the Interaction Model status code of INVALID_IN_STATE.
     */
    void HandleCommissionNode(HandlerContext & ctx, const CommissionerControl::Commands::CommissionNode::DecodableType & req);

    CommissionerControl::Delegate * mDelegate = nullptr;
};

} // namespace chip::app::Clusters
