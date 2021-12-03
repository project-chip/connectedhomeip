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
    if (mPICSFilePath.HasValue())
    {
        PICS.SetValue(PICSBooleanReader::Read(mPICSFilePath.Value()));
    }

    NextTest();

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestCommand::WaitForCommissionee()
{
    return CurrentCommissioner().GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

void TestCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    ChipLogProgress(chipTool, " **** Test Setup: Device Connected\n");
    auto * command = static_cast<TestCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "Device connected, but cannot run the test, as the context is null"));
    command->mDevices[command->GetIdentity()] = device;

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

CHIP_ERROR TestCommand::Wait(chip::System::Clock::Timeout duration)
{
    return chip::DeviceLayer::SystemLayer().StartTimer(duration, OnWaitForMsFn, this);
}

CHIP_ERROR TestCommand::Log(const char * message)
{
    ChipLogDetail(chipTool, "%s", message);
    ReturnErrorOnFailure(ContinueOnChipMainThread());
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestCommand::UserPrompt(const char * message)
{
    ChipLogDetail(chipTool, "USER_PROMPT: %s", message);
    ReturnErrorOnFailure(ContinueOnChipMainThread());
    return CHIP_NO_ERROR;
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

bool TestCommand::CheckConstraintStartsWith(const char * itemName, const chip::Span<const char> current, const char * expected)
{
    std::string value(current.data(), current.size());
    if (value.rfind(expected, 0) != 0)
    {
        Exit(std::string(itemName) + " (\"" + value + "\") does not starts with: \"" + std::string(expected) + "\"");
        return false;
    }

    return true;
}

bool TestCommand::CheckConstraintEndsWith(const char * itemName, const chip::Span<const char> current, const char * expected)
{
    std::string value(current.data(), current.size());
    if (value.find(expected, value.size() - strlen(expected)) == std::string::npos)
    {
        Exit(std::string(itemName) + " (\"" + value + "\") does not ends with: \"" + std::string(expected) + "\"");
        return false;
    }

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

bool TestCommand::CheckValueAsString(const char * itemName, chip::ByteSpan current, chip::ByteSpan expected)
{
    if (!current.data_equal(expected))
    {
        Exit(std::string(itemName) + " value mismatch, expecting " +
             std::string(chip::Uint8::to_const_char(expected.data()), expected.size()));
        return false;
    }

    return true;
}

bool TestCommand::CheckValueAsString(const char * itemName, chip::CharSpan current, chip::CharSpan expected)
{
    if (!current.data_equal(expected))
    {
        Exit(std::string(itemName) + " value mismatch, expected '" + std::string(expected.data(), expected.size()) + "' but got '" +
             std::string(current.data(), current.size()) + "'");
        return false;
    }

    return true;
}

bool TestCommand::ShouldSkip(const char * expression)
{
    // If there is no PICS configuration file, considers that nothing should be skipped.
    if (!PICS.HasValue())
    {
        return false;
    }

    std::map<std::string, bool> pics(PICS.Value());
    bool shouldSkip = !PICSBooleanExpressionParser::Eval(expression, pics);
    if (shouldSkip)
    {
        ChipLogProgress(chipTool, " **** Skipping: %s == false\n", expression);
        ContinueOnChipMainThread();
    }
    return shouldSkip;
}
