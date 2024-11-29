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

class PairDeviceCommand : public Command, public admin::PairingDelegate
{
public:
    PairDeviceCommand(chip::NodeId nodeId, const char * payload);
    void OnCommissioningComplete(chip::NodeId deviceId, CHIP_ERROR err) override;
    CHIP_ERROR RunCommand() override;

private:
    chip::NodeId mNodeId;
    const char * mPayload;
};

} // namespace commands
