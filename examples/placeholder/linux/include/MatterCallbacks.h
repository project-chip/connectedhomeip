/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <zap-generated/test/Commands.h>

TestCommand * gTestCommand = nullptr;

void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ChipLogError(Zcl, "Commissioning complete");

        TestCommand * command = reinterpret_cast<TestCommand *>(arg);
        if (command == nullptr)
        {
            ChipLogError(Zcl, "No tests.");
            return;
        }

        gTestCommand = command;
        gTestCommand->NextTest();
        break;
    }
}

void MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath)
{
    VerifyOrReturn(gTestCommand != nullptr);

    ChipLogError(Zcl, "Receive command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Command: " ChipLogFormatMEI,
                 commandPath.mEndpointId, ChipLogValueMEI(commandPath.mClusterId), ChipLogValueMEI(commandPath.mCommandId));

    gTestCommand->CheckCommandPath(commandPath);
}

void MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    VerifyOrReturn(gTestCommand != nullptr);

    ChipLogError(Zcl, "Receive READ attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    gTestCommand->CheckAttributePath(attributePath);
}

void MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    VerifyOrReturn(gTestCommand != nullptr);

    ChipLogError(Zcl, "Receive WRITE attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    gTestCommand->CheckAttributePath(attributePath);
}
