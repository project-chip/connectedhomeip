/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/operational-state-server/operational-state-server.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public Delegate
{

public:
    /**
     * Get the countdown time. This attribute is not used in this application.
     * @return The current countdown time.
     */
    DataModel::Nullable<uint32_t> GetCountdownTime() override { return {}; }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

    void HandlePauseStateCallback(GenericOperationalError & err) override;
    void HandleResumeStateCallback(GenericOperationalError & err) override;
    void HandleStartStateCallback(GenericOperationalError & err) override;
    void HandleStopStateCallback(GenericOperationalError & err) override;

    /**
     * @brief Calls the MatterPostAttributeChangeCallback for the modified attribute on the operationalState instance
     *
     * @param attributeId Id of the attribute that changed
     * @param type Type of the attribute
     * @param size Size of the attribute data
     * @param value Value of the attribute data
     */

    /**
     * @brief Calls the MatterPostAttributeChangeCallback for the modified attribute on the operationalState instance
     *
     * @param attributeId Id of the attribute that changed
     * @param type Type of the attribute
     * @param size Size of the attribute data
     * @param value Value of the attribute data
     */
    void PostAttributeChangeCallback(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value);

    void SetEndpointId(EndpointId endpointId);

private:
    const GenericOperationalState dishwasherOpStateList[4] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    EndpointId mEndpointId;

    DataModel::List<const GenericOperationalState> mOperationalStateList =
        Span<const GenericOperationalState>(dishwasherOpStateList);
    const Span<const CharSpan> mOperationalPhaseList;
};

OperationalState::Instance * GetInstance();
OperationalState::OperationalStateDelegate * GetDelegate();
void Shutdown();

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
