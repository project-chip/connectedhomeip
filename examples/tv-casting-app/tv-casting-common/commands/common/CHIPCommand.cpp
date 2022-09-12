/*
 *   Copyright (c) 2021-2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include <commands/common/CHIPCommand.h>

#include <core/CHIPBuildConfig.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

CHIP_ERROR CHIPCommand::Run()
{
    CHIP_ERROR err = StartWaiting(GetWaitDuration());
    return err;
}

void CHIPCommand::StartTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::InitTrace();

    if (mTraceFile.HasValue())
    {
        chip::trace::AddTraceStream(new chip::trace::TraceStreamFile(mTraceFile.Value()));
    }
    else if (mTraceLog.HasValue() && mTraceLog.Value())
    {
        chip::trace::AddTraceStream(new chip::trace::TraceStreamLog());
    }
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::StopTracing()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
}

void CHIPCommand::RunQueuedCommand(intptr_t commandArg)
{
    auto * command = reinterpret_cast<CHIPCommand *>(commandArg);
    CHIP_ERROR err = command->RunCommand();
    if (err != CHIP_NO_ERROR)
    {
        command->SetCommandExitStatus(err);
    }
}

static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    (reinterpret_cast<CHIPCommand *>(appState))->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}

CHIP_ERROR CHIPCommand::StartWaiting(chip::System::Clock::Timeout duration)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(RunQueuedCommand, reinterpret_cast<intptr_t>(this));
    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));
    return mCommandExitStatus;
}

void CHIPCommand::StopWaiting()
{
    Shutdown();
}

chip::Controller::DeviceCommissioner & CHIPCommand::CurrentCommissioner()
{
    CommissionerIdentity identity{ GetIdentity(), chip::kUndefinedNodeId };
    auto item = mCommissioners.find(identity);
    return *item->second;
}

constexpr chip::FabricId kIdentityOtherFabricId = 4;
std::map<CHIPCommand::CommissionerIdentity, std::unique_ptr<chip::Controller::DeviceCommissioner>> CHIPCommand::mCommissioners;

std::string CHIPCommand::GetIdentity()
{
    std::string name = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0)
    {
        chip::FabricId fabricId = strtoull(name.c_str(), nullptr, 0);
        if (fabricId >= kIdentityOtherFabricId)
        {
            // normalize name since it is used in persistent storage

            char s[24];
            sprintf(s, "%" PRIu64, fabricId);

            name = s;
        }
        else
        {
            ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]", name.c_str(),
                         kIdentityAlpha, kIdentityBeta, kIdentityGamma);
            chipDie();
        }
    }

    return name;
}
