/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/clusters/resource-monitoring-server/replacement-product-list-manager.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/util/basic-types.h>
#include <clusters/HepaFilterMonitoring/Enums.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class ResourceMonitoringCluster;

class Delegate
{
    friend class ResourceMonitoringCluster;

private:
    ResourceMonitoringCluster * mInstance = nullptr;
    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of an Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     */
    void SetInstance(ResourceMonitoringCluster * aInstance) { mInstance = aInstance; }

    friend class ResourceMonitoringCluster;

protected:
    ResourceMonitoringCluster * GetInstance() { return mInstance; }

public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    // The following methods should be overridden by the SDK user to implement the business logic of their application

    /**
     * This init method will be called during Resource Monitoring Server initialization after the instance information has been
     * validated and the instance has been registered. This method should be overridden by the SDK user to initialize the
     * application logic.
     *
     * @return CHIP_NO_ERROR    If the application was initialized successfully. All other values will cause the initialization to
     * fail.
     */
    virtual CHIP_ERROR Init() = 0;

    /**
     * This method may be overwritten by the SDK User, if the default behaviour is not desired.
     * Preferably, the SDK User should implement the PreResetCondition() and PostResetCondition() methods instead.
     *
     * The cluster implementation will handle all of the resets needed by the spec.
     * - Update the Condition attribute according to the DegradationDirection (if supported)
     * - Update the ChangeIndicator attribute to kOk
     * - Update the LastChangedTime attribute (if supported)
     *
     * The return value will depend on the PreResetCondition() and PostResetCondition() method, if one of them does not return
     * Success, this method will return the failure as well.
     * @return Status::Success      If the command was handled successfully.
     * @return All Other            PreResetCondition() or PostResetCondition() failed, these are application specific.
     */
    virtual Protocols::InteractionModel::Status OnResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something before the reset.
     * If there are some internal states of the devices or some specific methods that must be called, that are needed for the reset
     * and that can fail, they should be done here and not in PostResetCondition().
     *
     * @return Status::Success      All good, the reset may proceed.
     * @return All Other            The reset should not proceed. The reset command will fail.
     */
    virtual Protocols::InteractionModel::Status PreResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something after the reset.
     * If this fails, the attributes will already be updated, so the SDK User should not do something here
     * that can fail and that will affect the state of the device. Do the checks in the PreResetCondition() method instead.
     *
     * @return Status::Success      All good
     * @return All Other            Something went wrong. The attributes will already be updated. But the reset command will report
     *                              the failure.
     */
    virtual Protocols::InteractionModel::Status PostResetCondition();
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
