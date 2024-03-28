/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Routines for the Low Power plugin, the
 *server implementation of the Low Power cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/clusters/low-power-server/low-power-delegate.h>
#include <app/clusters/low-power-server/low-power-server.h>

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceConfig.h>
#include <protocols/interaction_model/StatusCode.h>
#include <tracing/macros.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LowPower;

static constexpr size_t kLowPowerDelegateTableSize =
    MATTER_DM_LOW_POWER_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kLowPowerDelegateTableSize <= kEmberInvalidEndpointIndex, "LowPower Delegate table size error");

// -----------------------------------------------------------------------------
// Delegate Implementation

using chip::app::Clusters::LowPower::Delegate;

namespace {

Delegate * gDelegateTable[kLowPowerDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, chip::app::Clusters::LowPower::Id,
                                                       MATTER_DM_LOW_POWER_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kLowPowerDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "LowPower has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}
} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace LowPower {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, chip::app::Clusters::LowPower::Id,
                                                       MATTER_DM_LOW_POWER_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kLowPowerDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}

} // namespace LowPower
} // namespace Clusters
} // namespace app
} // namespace chip

bool emberAfLowPowerClusterSleepCallback(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath,
                                         const Commands::Sleep::DecodableType & commandData)
{
    MATTER_TRACE_SCOPE("Sleep", "LowPower");
    using Protocols::InteractionModel::Status;

    EndpointId endpoint = commandPath.mEndpointId;

    Delegate * delegate = GetDelegate(endpoint);
    Status status;
    if (isDelegateNull(delegate, endpoint))
    {
        ChipLogError(Zcl, "emberAfLowPowerClusterSleepCallback: no delegate");
        status = Status::Failure;
    }
    else
    {
        bool success = delegate->HandleSleep();
        status       = success ? Status::Success : Status::Failure;
    }
    command->AddStatus(commandPath, status);
    return true;
}

void MatterLowPowerPluginServerInitCallback() {}
