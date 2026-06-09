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

#include <app/clusters/commissioner-control-server/CommissionerControlCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

namespace chip::app::Clusters::CommissionerControl {

class CommissionerControlServer
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

    /**
     * @brief Destroys a Commissioner Control cluster instance. The Deinit() function needs to be called for this instance
     * to be unregistered and stopped from being called by the interaction model.
     */
    ~CommissionerControlServer();

    /**
     * @brief Initialise the Commissioner Control server instance.
     * This function must be called after defining an CommissionerControlServer class object.
     * @return Returns an error if the CommandHandler registration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Init();

    /**
     * @brief Uninitialise the Commissioner Control server instance.
     * @return Returns an error if the CommandHandler unregistration fails, else returns CHIP_NO_ERROR.
     */
    CHIP_ERROR Deinit();

    Protocols::InteractionModel::Status
    GetSupportedDeviceCategoriesValue(EndpointId endpoint,
                                      BitMask<SupportedDeviceCategoryBitmap> * supportedDeviceCategories) const;

    Protocols::InteractionModel::Status
    SetSupportedDeviceCategoriesValue(EndpointId endpoint, const BitMask<SupportedDeviceCategoryBitmap> supportedDeviceCategories);

    /**
     * @brief
     *   Called after the server return SUCCESS to a correctly formatted RequestCommissioningApproval command.
     */
    CHIP_ERROR
    GenerateCommissioningRequestResultEvent(EndpointId endpoint, const Events::CommissioningRequestResult::Type & result);

private:
    Delegate * mDelegate{};
    EndpointId mEndpointId{};

    // The Code Driven CommissionerControlCluster instance (lazy-initialized)
    chip::app::LazyRegisteredServerCluster<CommissionerControlCluster> mCluster;
};

} // namespace chip::app::Clusters::CommissionerControl
