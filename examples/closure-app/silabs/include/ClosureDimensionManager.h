/*
 *
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/closure-control-server/closure-control-server.h>

#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureDimension {

// This is an application level delegate to handle Closure Dimension commands according to the specific business logic.
class ClosureDimensionManager : public ClosureDimension::Delegate
{
public:
    void SetClosureDimensionInstance(ClosureDimension::Instance & instance);

    /*********************************************************************************
     *
     * Methods implementing the ClosureDimension::Delegate interface
     *
     *********************************************************************************/
    Protocols::InteractionModel::Status Stop() override;
    Protocols::InteractionModel::Status MoveTo(const Optional<TagPositionEnum> & tag, const Optional<TagLatchEnum> & latch,
                                               const Optional<Globals::ThreeLevelAutoEnum> & speed) override;
    Protocols::InteractionModel::Status Calibrate() override;

    // ------------------------------------------------------------------
    // Get attribute methods

    DataModel::Nullable<uint32_t> GetCountdownTime() override;

    /***************************************************************************
     *
     * ClosureDimensionDelegate specific methods
     *
     ***************************************************************************/
    CHIP_ERROR StartCurrentErrorListRead() override;
    CHIP_ERROR GetCurrentErrorListAtIndex(size_t Index, ClosureErrorEnum & closureError) override;
    CHIP_ERROR EndCurrentErrorListRead() override;

    void ClosureDimensionAttributeChangeHandler(EndpointId endpointId, AttributeId attributeId);

private:
    friend ClosureDimensionManager & ClosureCtrlMgr();

    /***************************************************************************
     *
     * ClosureDimensionDelegate specific variables
     *
     ***************************************************************************/

    // Need the following so can determine which features are supported
    ClosureDimension::Instance * mpClosureDimensionInstance = nullptr;
    bool IsManualLatch();
    bool IsDeviceReadytoMove();

    static ClosureDimensionManager sClosureCtrlMgr;
};

inline ClosureDimensionManager & ClosureCtrlMgr()
{
    return ClosureDimensionManager::sClosureCtrlMgr;
}
} // namespace ClosureDimension
} // namespace Clusters
} // namespace app
} // namespace chip
