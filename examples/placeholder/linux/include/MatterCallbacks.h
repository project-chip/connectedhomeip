/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "InteractiveServer.h"
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

void MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath,
                                       const chip::Access::SubjectDescriptor & subjectDescriptor)
{
    VerifyOrReturn(!InteractiveServer::GetInstance().Command(commandPath));

    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Command: " ChipLogFormatMEI,
                 commandPath.mEndpointId, ChipLogValueMEI(commandPath.mClusterId), ChipLogValueMEI(commandPath.mCommandId));

    test->CheckCommandPath(commandPath);
}

void MatterPostAttributeReadCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    VerifyOrReturn(!InteractiveServer::GetInstance().ReadAttribute(attributePath));

    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive READ attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    test->CheckAttributePath(attributePath);
}

void MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    VerifyOrReturn(!InteractiveServer::GetInstance().WriteAttribute(attributePath));

    auto test = GetTargetTest();
    VerifyOrReturn(test != nullptr && test->isRunning);

    ChipLogError(Zcl, "Receive WRITE attribute command: Endpoint: %u, Cluster: " ChipLogFormatMEI ", Attribute: " ChipLogFormatMEI,
                 attributePath.mEndpointId, ChipLogValueMEI(attributePath.mClusterId), ChipLogValueMEI(attributePath.mAttributeId));

    test->CheckAttributePath(attributePath);
}
