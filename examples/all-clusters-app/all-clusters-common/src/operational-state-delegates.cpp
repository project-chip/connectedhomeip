/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/util/config.h>
#include <operational-state-delegate-impl.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

/**
 * class to present Enquriy Table of Delegates
 */
struct DelegatesEnquiryTable
{
    /**
     * Endpoint Id
     */
    EndpointId mEndpointId;
    /**
     * Cluster Id
     */
    ClusterId mClusterId;
    /**
     * point of Array(Items)
     */
    Delegate * pItems;
    /**
     * ArraySize of Array(Items)
     */
    size_t numOfItems;
};

/**
 * Enquriy Table of Operational State List
 * Note: User Define
 */
static const GenericOperationalState opStateList[] = {
    GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
};

/**
 * Enquriy Table of Phase List
 * Note: User Define
 */
static const GenericOperationalPhase opPhaseList[] = {
    /**
     * Phase List is null
     */
    GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
};

/**
 * Enquriy Table of Operational State Delegate
 * Note: User Define
 */
static OperationalStateDelegate opStateDelegate(Clusters::OperationalState::kDemoEndpointId, Clusters::OperationalState::Id,
                                                GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
                                                GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)),
                                                Span<const GenericOperationalState>(opStateList),
                                                Span<const GenericOperationalPhase>(opPhaseList));

/**
 * Enquriy Table of Operational State Cluster and alias Cluter Delegate corresponding to endpointId and clusterId
 * Note: User Define
 */
constexpr DelegatesEnquiryTable kDelegatesEnquiryTable[] = {
    // EndpointId, ClusterId, Delegate
    { Clusters::OperationalState::kDemoEndpointId, Clusters::OperationalState::Id, &opStateDelegate },
};

/**
 * Get the pointer of target delegate for target endpoint and cluster
 * @param[in] aEndpiontId The endpointId
 * @param[in] aClusterID  The clusterId
 * @return the pointer of target delegate
 */
Delegate * getGenericDelegateTable(EndpointId aEndpointId, ClusterId aClusterId)
{
    for (size_t i = 0; i < ArraySize(kDelegatesEnquiryTable); ++i)
    {
        if (kDelegatesEnquiryTable[i].mEndpointId == aEndpointId && kDelegatesEnquiryTable[i].mClusterId == aClusterId)
        {
            return kDelegatesEnquiryTable[i].pItems;
        }
    }
    return nullptr;
}

Delegate * GetOperationalStateDelegate(EndpointId endpointId, ClusterId clusterId)
{
    return getGenericDelegateTable(endpointId, clusterId);
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterOperationalStatePluginServerInitCallback()
{
    using namespace chip::app;
    static Clusters::OperationalState::OperationalStateServer operationalstateServer(Clusters::OperationalState::kDemoEndpointId, Clusters::OperationalState::Id);
    operationalstateServer.Init();
}
