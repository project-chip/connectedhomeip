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

#pragma once

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"
#include <app/chip-zcl-zpro-codec.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <core/CHIPEncoding.h>

// Limits on endpoint values.
#define CHIP_ZCL_ENDPOINT_MIN 0x00
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

class ModelCommand : public Command
{
public:
    ModelCommand(const char * commandName) : Command(commandName) {}

    void AddArguments() { AddArgument("endpoint-id", CHIP_ZCL_ENDPOINT_MIN, CHIP_ZCL_ENDPOINT_MAX, &mEndPointId); }

    /////////// Command Interface /////////
    CHIP_ERROR Run(NodeId localId, NodeId remoteId) override;

    virtual CHIP_ERROR SendCommand(ChipDevice * device, uint8_t endPointId) = 0;

private:
    ChipDevice * mDevice;
    uint8_t mEndPointId;
};
