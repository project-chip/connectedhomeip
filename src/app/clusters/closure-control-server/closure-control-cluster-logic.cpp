/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {

/*
    ClusterStateAttributes Implementation
*/

CHIP_ERROR ClusterStateAttributes::SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterStateAttributes::SetMainState(const MainStateEnum & mainState)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterStateAttributes::SetOverallState(const DataModel::Nullable<GenericOverallState> & overallState)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterStateAttributes::SetTargetState(const DataModel::Nullable<GenericOverallTarget> & targetState)
{
    // TODO
    return CHIP_NO_ERROR;
}

CHIP_ERROR ClusterLogic::SetCountdownTime(const DataModel::Nullable<ElapsedS> & countdownTime)
{
    // TODO
    return CHIP_NO_ERROR;
}

/*
    ClusterLogic Implementation
*/

CHIP_ERROR ClusterLogic::Init(const ClusterConformance & conformance)
{
    VerifyOrReturnError(conformance.Valid(), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    mConformance = conformance;

    mIsInitialized = true;
    return CHIP_NO_ERROR;
}

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
