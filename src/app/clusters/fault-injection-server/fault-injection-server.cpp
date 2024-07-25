/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "app/server/Server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandResponseHelper.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_WITH_NLFAULTINJECTION
#include <inet/InetFaultInjection.h>
#include <lib/support/CHIPFaultInjection.h>
#include <system/SystemFaultInjection.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::FaultInjection;
using chip::Protocols::InteractionModel::Status;

namespace {

#if CHIP_WITH_NLFAULTINJECTION
nl::FaultInjection::Manager * GetFaultInjectionManager(FaultType type)
{
    nl::FaultInjection::Manager * faultInjectionMgr = nullptr;

    switch (type)
    {
    case FaultType::kSystemFault:
        faultInjectionMgr = &chip::System::FaultInjection::GetManager();
        break;
    case FaultType::kInetFault:
        faultInjectionMgr = &chip::Inet::FaultInjection::GetManager();
        break;
    case FaultType::kChipFault:
        faultInjectionMgr = &chip::FaultInjection::GetManager();
        break;
    default:
        break;
    }

    return faultInjectionMgr;
}
#endif

} // anonymous namespace

bool emberAfFaultInjectionClusterFailAtFaultCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                     const Commands::FailAtFault::DecodableType & commandData)
{
    if (commandPath.mClusterId != Clusters::FaultInjection::Id)
    {
        // We shouldn't have been called at all.
        commandObj->AddStatus(commandPath, Status::UnsupportedCluster);
        return true;
    }

#if CHIP_WITH_NLFAULTINJECTION
    Status returnStatus                             = Status::Success;
    nl::FaultInjection::Manager * faultInjectionMgr = GetFaultInjectionManager(commandData.type);

    if (faultInjectionMgr != nullptr)
    {
        ChipLogProgress(Zcl, "FaultInjection: Configure a fault of type: %u and Id: %" PRIu32 " to be triggered deterministically",
                        static_cast<uint8_t>(commandData.type), commandData.id);
        int32_t err = faultInjectionMgr->FailAtFault(commandData.id, commandData.numCallsToSkip, commandData.numCallsToFail,
                                                     commandData.takeMutex);

        if (err != 0)
        {
            ChipLogError(Zcl, "FaultInjection: Pass invalid inputs to FailAtFault");
            returnStatus = Status::InvalidCommand;
        }
    }
    else
    {
        ChipLogError(Zcl, "FaultInjection: Failed to get Fault Injection manager");
        returnStatus = Status::Failure;
    }
#else
    Status returnStatus = Status::UnsupportedCommand;
#endif // CHIP_WITH_NLFAULTINJECTION

    commandObj->AddStatus(commandPath, returnStatus);
    return true;
}

bool emberAfFaultInjectionClusterFailRandomlyAtFaultCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                             const Commands::FailRandomlyAtFault::DecodableType & commandData)
{
    if (commandPath.mClusterId != Clusters::FaultInjection::Id)
    {
        // We shouldn't have been called at all.
        commandObj->AddStatus(commandPath, Status::UnsupportedCluster);
        return true;
    }

    if (commandData.percentage > 100)
    {
        commandObj->AddStatus(commandPath, Status::InvalidCommand);
        return true;
    }

#if CHIP_WITH_NLFAULTINJECTION
    Status returnStatus                             = Status::Success;
    nl::FaultInjection::Manager * faultInjectionMgr = GetFaultInjectionManager(commandData.type);

    if (faultInjectionMgr != nullptr)
    {
        ChipLogProgress(Zcl, "FaultInjection: Configure a fault of type: %u and Id: %" PRIu32 " to be triggered randomly",
                        static_cast<uint8_t>(commandData.type), commandData.id);
        int32_t err = faultInjectionMgr->FailRandomlyAtFault(commandData.id, commandData.percentage);

        if (err != 0)
        {
            ChipLogError(Zcl, "FaultInjection: Pass invalid inputs to FailAtFault");
            returnStatus = Status::InvalidCommand;
        }
    }
    else
    {
        ChipLogError(Zcl, "FaultInjection: Failed to get Fault Injection manager");
        returnStatus = Status::Failure;
    }
#else
    Status returnStatus = Status::UnsupportedCommand;
#endif // CHIP_WITH_NLFAULTINJECTION

    commandObj->AddStatus(commandPath, returnStatus);
    return true;
}

void MatterFaultInjectionPluginServerInitCallback() {}
