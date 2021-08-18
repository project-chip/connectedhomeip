/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "ReportingCommand.h"

#include "../common/Commands.h"
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

using namespace ::chip;

CHIP_ERROR ReportingCommand::Run()
{
    auto * ctx = GetExecContext();

    CHIP_ERROR err =
        ctx->commissioner->GetConnectedDevice(ctx->remoteId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(chipTool, "Failed in initiating connection to the device: %" PRIu64 ", error %s", ctx->remoteId,
                              ErrorStr(err)));

exit:
    return err;
}

void ReportingCommand::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    ReportingCommand * command = reinterpret_cast<ReportingCommand *>(context);
    VerifyOrReturn(command != nullptr,
                   ChipLogError(chipTool, "Device connected, but cannot send the command, as the context is null"));

    chip::Controller::BasicCluster cluster;
    cluster.Associate(device, command->mEndPointId);

    command->AddReportCallbacks(command->mEndPointId);

    CHIP_ERROR err = cluster.MfgSpecificPing(nullptr, nullptr);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Init failure! Ping failure: %s", ErrorStr(err));
        command->SetCommandExitStatus(err);
    }
}

void ReportingCommand::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR err)
{
    ChipLogError(chipTool, "Failed in connecting to the device %" PRIu64 ". Error %s", deviceId, ErrorStr(err));

    ReportingCommand * command = reinterpret_cast<ReportingCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "ReportingCommand context is null"));
    command->SetCommandExitStatus(err);
}
