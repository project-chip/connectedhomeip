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

#include "Options.h"

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

#include <zap-generated/test/Commands.h>

TestCommand * GetTargetTest()
{
    const char * command = LinuxDeviceOptions::GetInstance().command;
    if (command == nullptr)
    {
        return nullptr;
    }

    static auto test = GetTestCommand(command);
    if (test.get() == nullptr)
    {
        ChipLogError(chipTool, "Specified test command does not exist: %s", command);
        PrintTestCommands();
        return nullptr;
    }

    const char * PICSFilePath = LinuxDeviceOptions::GetInstance().PICS;
    if (PICSFilePath != nullptr)
    {
        test->PICS.SetValue(PICSBooleanReader::Read(PICSFilePath));
    }

    return test.get();
}

void MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath)
{
    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Command: " ChipLogFormatMEI,
                 commandPath.mEndpointId, ChipLogValueMEI(commandPath.mClusterId), ChipLogValueMEI(commandPath.mCommandId));

    test->CheckCommandPath(commandPath);
}

void MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive READ attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    test->CheckAttributePath(attributePath);
}

void MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive WRITE attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    test->CheckAttributePath(attributePath);
}
