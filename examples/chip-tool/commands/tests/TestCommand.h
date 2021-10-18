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

#pragma once

#include "../common/CHIPCommand.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/UnitTestUtils.h>
#include <zap-generated/tests/CHIPClustersTest.h>

class TestCommand : public CHIPCommand
{
public:
    TestCommand(const char * commandName) :
        CHIPCommand(commandName), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("delayInMs", 0, UINT64_MAX, &mDelayInMs);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    uint16_t GetWaitDurationInSeconds() const override { return 30; }

    virtual void NextTest() = 0;

    /////////// GlobalCommands Interface /////////
    CHIP_ERROR WaitForMs(uint32_t ms);

protected:
    ChipDevice * mDevice;
    chip::NodeId mNodeId;

    static void OnDeviceConnectedFn(void * context, chip::Controller::Device * device);
    static void OnDeviceConnectionFailureFn(void * context, NodeId deviceId, CHIP_ERROR error);
    static void OnWaitForMsFn(chip::System::Layer * systemLayer, void * context);

    void Exit(std::string message);
    void ThrowFailureResponse();
    void ThrowSuccessResponse();

    bool CheckConstraintType(const char * itemName, const char * current, const char * expected);
    bool CheckConstraintFormat(const char * itemName, const char * current, const char * expected);
    bool CheckConstraintMinLength(const char * itemName, uint64_t current, uint64_t expected);
    bool CheckConstraintMaxLength(const char * itemName, uint64_t current, uint64_t expected);
    template <typename T>
    bool CheckConstraintMinValue(const char * itemName, T current, T expected)
    {
        if (current < expected)
        {
            Exit(std::string(itemName) + " value < minValue: " + std::to_string(current) + " < " + std::to_string(expected));
            return false;
        }

        return true;
    }
    template <typename T>
    bool CheckConstraintMaxValue(const char * itemName, T current, T expected)
    {
        if (current > expected)
        {
            Exit(std::string(itemName) + " value > maxValue: " + std::to_string(current) + " > " + std::to_string(expected));
            return false;
        }

        return true;
    }
    template <typename T>
    bool CheckConstraintNotValue(const char * itemName, T current, T expected)
    {
        if (current == expected)
        {
            Exit(std::string(itemName) + " value == notValue: " + std::to_string(current) + " == " + std::to_string(expected));
            return false;
        }

        return true;
    }
    template <typename T>
    bool CheckValue(const char * itemName, T current, T expected)
    {
        if (current != expected)
        {
            Exit(std::string(itemName) + " value mismatch: " + std::to_string(current) + " != " + std::to_string(expected));
            return false;
        }

        return true;
    }
    bool CheckValueAsList(const char * itemName, uint64_t current, uint64_t expected);
    bool CheckValueAsString(const char * itemName, chip::ByteSpan current, const char * expected);

    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    void Wait()
    {
        if (mDelayInMs)
        {
            chip::test_utils::SleepMillis(mDelayInMs);
        }
    };
    uint64_t mDelayInMs = 0;
};
