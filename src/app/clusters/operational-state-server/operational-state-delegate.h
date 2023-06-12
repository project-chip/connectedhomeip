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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {
/*
 * GenericOperationalErrorState and GenericOperationalState are cluster-independent structs
 * that can be used with any alias of the Operational State cluster (unlike the cluster-specific structs
 * in the per-cluster Structs namespaces.
 */
constexpr size_t kErrorStateLabelMaxSize       = 64u;
constexpr size_t kErrorStateDetailsMaxSize     = 64u;
constexpr size_t kOperationalStateLabelMaxSize = 64u;

struct GenericOperationalErrorState
{
/*
 * Since OperationalErrorStateStruct is shared across different clusters, it can't use a
 * cluster-specific enum type for ErrorStateID.
 */
    uint8_t ErrorStateID;
    char ErrorStateLabel[kErrorStateLabelMaxSize];
    char ErrorStateDetails[kErrorStateDetailsMaxSize];
};

struct GenericOperationalState
{
    uint8_t OperationalStateID;
    char OperationalStateLabel[kOperationalStateLabelMaxSize];
};

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 */
class Delegate
{
public:
    /**
     * Handle Command Callback: Pause
     * Input the param for user to handle business logic.
     * Output the param to save the result after handle business logic.
     * @param[in,out] state make a copy of the operational state.
     * @param[in,out] error make a copy of the operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandlePauseState(GenericOperationalState & state, GenericOperationalErrorState & error) = 0;

    /**
     * Handle Command Callback: Resume
     * Input the param for user to handle business logic.
     * Output the param to save the result after handle business logic.
     * @param[in,out] state make a copy of the operational state.
     * @param[in,out] error make a copy of the operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleResumeState(GenericOperationalState & state, GenericOperationalErrorState & error) = 0;

    /**
     * Handle Command Callback: Start
     * Input the param for user to handle business logic.
     * Output the param to save the result after handle business logic.
     * @param[in,out] state make a copy of the operational state.
     * @param[in,out] error make a copy of the operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleStartState(GenericOperationalState & state, GenericOperationalErrorState & error) = 0;

    /**
     * Handle Command Callback: Stop
     * Input the param for user to handle business logic.
     * Output the param to save the result after handle business logic.
     * @param[in,out] state make a copy of the operational state.
     * @param[in,out] error make a copy of the operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    virtual void HandleStopState(GenericOperationalState & state, GenericOperationalErrorState & error) = 0;

    /**
     *  Init the delegate
     */
    virtual CHIP_ERROR Init() = 0;

    virtual ~Delegate() = default;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
