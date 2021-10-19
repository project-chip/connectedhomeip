/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "TestCommand.h"

CHIP_ERROR TestCommand::RunCommand()
{
    return mController.GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::Controller::Device * device)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connected\n");
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Device connected, but cannot run the test, as the context is null"));
    command->mDevice = device;
    command->NextTest();
}

void TestCommand::OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connection Failure [deviceId=%" PRIu64 ". Error %" CHIP_ERROR_FORMAT "\n]",
                    deviceId, error.Format());
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Test command context is null"));
    command->SetCommandExitStatus(error);
}

void TestCommand::OnWaitForMsFn(chip::System::Layer * systemLayer, void * context)
{
    auto * command = static_cast<TestCommand *>(context);
    command->NextTest();
}

CHIP_ERROR TestCommand::WaitForMs(uint32_t ms)
{
    return chip::DeviceLayer::SystemLayer().StartTimer(ms, OnWaitForMsFn, this);
}

void TestCommand::Exit(std::string message)
{
    ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
    SetCommandExitStatus(CHIP_ERROR_INTERNAL);
}

void TestCommand::ThrowFailureResponse()
{
    Exit("Expecting success response but got a failure response");
}

void TestCommand::ThrowSuccessResponse()
{
    Exit("Expecting failure response but got a success response");
}

bool TestCommand::CheckConstraintType(const char * itemName, const char * current, const char * expected)
{
    ChipLogError(chipTool, "Warning: %s type checking is not implemented yet. Expected type: '%s'", itemName, expected);
    return true;
}

bool TestCommand::CheckConstraintFormat(const char * itemName, const char * current, const char * expected)
{
    ChipLogError(chipTool, "Warning: %s format checking is not implemented yet. Expected format: '%s'", itemName, expected);
    return true;
}

bool TestCommand::CheckConstraintMinLength(const char * itemName, uint64_t current, uint64_t expected)
{
    if (current < expected)
    {
        Exit(std::string(itemName) + " length < minLength: " + std::to_string(current) + " < " + std::to_string(expected));
        return false;
    }

    return true;
}

bool TestCommand::CheckConstraintMaxLength(const char * itemName, uint64_t current, uint64_t expected)
{
    if (current > expected)
    {
        Exit(std::string(itemName) + " length > minLength: " + std::to_string(current) + " > " + std::to_string(expected));
        return false;
    }

    return true;
}

bool TestCommand::CheckValueAsList(const char * itemName, uint64_t current, uint64_t expected)
{
    if (current != expected)
    {
        Exit(std::string(itemName) + " count mismatch: " + std::to_string(current) + " != " + std::to_string(expected));
        return false;
    }

    return true;
}

bool TestCommand::CheckValueAsString(const char * itemName, const chip::ByteSpan current, const char * expected)
{
    const chip::ByteSpan expectedArgument = chip::ByteSpan(chip::Uint8::from_const_char(expected), strlen(expected));
    if (!current.data_equal(expectedArgument))
    {
        Exit(std::string(itemName) + " value mismatch, expecting " + std::string(expected));
        return false;
    }

    return true;
}
