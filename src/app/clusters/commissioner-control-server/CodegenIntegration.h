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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/commissioner-control-server/Delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

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
    CommissionerControlServer(Delegate * delegate, EndpointId endpointId);

    ~CommissionerControlServer() override;

    /**
     * @brief Initialise the Commissioner Control server instance.
     * This function must be called after defining an CommissionerControlServer class object.
     * @return Returns an error if the CommandHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    Protocols::InteractionModel::Status
    GetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                      BitMask<SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const;

    Protocols::InteractionModel::Status
    SetSupportedDeviceCategoriesValue(EndpointId endpoint, const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories);

    /**
     * @brief
     *   Called after the server return SUCCESS to a correctly formatted RequestCommissioningApproval command.
     */
    CHIP_ERROR GenerateCommissioningRequestResultEvent(EndpointId endpoint,
                                                       const Events::CommissioningRequestResult::Type & result);

private:
    /**
     * @brief Inherited from CommandHandlerInterface
     */
    void InvokeCommand(HandlerContext & ctx) override;

    /**
     * @brief Handle Command: RequestCommissioningApproval.
     * @param ctx The context for the command handling.
     * @param req The command request.
     * This command is sent by a client to request approval for a future CommissionNode call.
     * The server SHALL always return SUCCESS to a correctly formatted RequestCommissioningApproval
     * command, and then send a CommissioningRequestResult event once the result is ready.
     */
    void HandleRequestCommissioningApproval(HandlerContext & ctx,
                                            const Commands::RequestCommissioningApproval::DecodableType & req);

    /**
     * @brief Handle Command: CommissionNode.
     * @param ctx The context for the command handling.
     * @param req The command request.
     * This command is used to commission a node specified by a previously approved request.
     * The server SHALL return FAILURE if this command is not sent from the same
     * NodeId as the RequestCommissioningApproval or if the provided RequestId
     * does not match the value provided to RequestCommissioningApproval.
     */
    void HandleCommissionNode(HandlerContext & ctx, const Commands::CommissionNode::DecodableType & req);

    Delegate * mDelegate = nullptr;
};

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip
