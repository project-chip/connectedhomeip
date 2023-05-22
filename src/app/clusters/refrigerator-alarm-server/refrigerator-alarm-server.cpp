/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
 *
 */

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RefrigeratorAlarm;
using namespace chip::app::Clusters::RefrigeratorAlarm::Attributes;
using namespace chip::DeviceLayer;
using chip::Protocols::InteractionModel::Status;

static Status resetHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const BitMask<AlarmMap> & alarms,
                           const Optional<BitMask<AlarmMap>> & mask);

static Status resetHandler(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, const BitMask<AlarmMap> & alarms,
                           const Optional<BitMask<AlarmMap>> & mask)
{
    EndpointId endpoint = commandPath.mEndpointId;

    EmberAfStatus status;
    chip::BitMask<AlarmMap> state = 0;
    status                        = State::Get(endpoint, &state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

    state.Clear(alarms);
    status = State::Set(endpoint, state);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        return Status::Failure;
    }

    if (mask.HasValue())
    {
        status = Mask::Set(endpoint, mask.Value());
        if (status != EMBER_ZCL_STATUS_SUCCESS)
        {
            return Status::Failure;
        }
    }

    return Status::Success;
}
/**********************************************************
 * Callbacks Implementation
 *********************************************************/

bool emberAfRefrigeratorAlarmClusterResetCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                  const Commands::Reset::DecodableType & commandData)
{
    auto & alarms = commandData.alarms;
    auto & mask   = commandData.mask;

    Status status = resetHandler(commandObj, commandPath, alarms, mask);
    commandObj->AddStatus(commandPath, status);

    return true;
}

void emberAfRefrigeratorAlarmClusterServerInitCallback(chip::EndpointId endpoint) {}

void MatterRefrigeratorAlarmPluginServerInitCallback() {}
