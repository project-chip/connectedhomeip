/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <CommandRegistry.h>
#include <admin/PairingManager.h>

namespace commands {

class SyncDeviceCommand : public Command, public admin::CommissioningWindowDelegate, public admin::PairingDelegate
{
public:
    SyncDeviceCommand(chip::EndpointId remoteEndpointId);
    void OnCommissioningWindowOpened(chip::NodeId deviceId, CHIP_ERROR status, chip::SetupPayload payload) override;
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) override;
    CHIP_ERROR RunCommand() override;

private:
    chip::EndpointId mRemoteEndpointId = chip::kInvalidEndpointId;
    chip::NodeId mAssignedNodeId       = chip::kUndefinedNodeId;
};

} // namespace commands
