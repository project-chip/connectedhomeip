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
#include <app/EventLogging.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <operational-state-delegate-impl.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

/**
 * class to present Enquiry Table of Delegates
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
     * The delegate for the cluster instance given by mEndpointId and mClusterId.
     */
    Delegate * pItem;
};

/*
 * An example to present device's endpointId
 */
constexpr EndpointId kDemoEndpointId = 1;

/**
 * Operational State List
 * Note: User Define
 */
static const GenericOperationalState opStateList[] = {
    GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
    GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
};

/**
 * Phase List
 * Note: User Define
 */
static const GenericOperationalPhase opPhaseList[] = {
    /**
     * Phase List is null
     */
    GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
};

/**
 * Operational State Delegate
 * Note: User Define
 */
static OperationalStateDelegate opStateDelegate(to_underlying(OperationalStateEnum::kStopped),
                                                GenericOperationalError(to_underlying(ErrorStateEnum::kNoError)),
                                                Span<const GenericOperationalState>(opStateList),
                                                Span<const GenericOperationalPhase>(opPhaseList));

/**
 * Enquiry Table of Operational State Cluster and alias Cluter Delegate corresponding to endpointId and clusterId
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
            return kDelegatesEnquiryTable[i].pItem;
        }
    }
    return nullptr;
}

// @brief Instance getter for the delegate for the given operational state alias cluster on the given endpoint.
// The delegate API assumes there will be separate delegate objects for each cluster instance.
// (i.e. each separate operational state cluster derivation, on each separate endpoint)
// @note This API should always be called prior to using the delegate and the return pointer should never be cached.
// @return Default global delegate instance.
Delegate * GetOperationalStateDelegate(EndpointId endpointId, ClusterId clusterId)
{
    return getGenericDelegateTable(endpointId, clusterId);
}

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterOperationalStateServerInit()
{
    using namespace chip::app;
    static Clusters::OperationalState::OperationalStateServer operationalstateServer(Clusters::OperationalState::kDemoEndpointId,
                                                                                     Clusters::OperationalState::Id);
    operationalstateServer.Init();
}
