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
#include <app/data-model/DecodableList.h>
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
    CHIP_ERROR Log(const char * message);

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

    template <typename T>
    bool CheckValueAsListHelper(const char * itemName, typename chip::app::DataModel::DecodableList<T>::Iterator iter)
    {
        if (iter.Next())
        {
            Exit(std::string(itemName) + " value mismatch: expected no more items but found " + std::to_string(iter.GetValue()));
            return false;
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            Exit(std::string(itemName) +
                 " value mismatch: expected no more items but got an error: " + iter.GetStatus().AsString());
            return false;
        }
        return true;
    }

    template <typename T, typename U, typename... ValueTypes>
    bool CheckValueAsListHelper(const char * itemName, typename chip::app::DataModel::DecodableList<T>::Iterator & iter,
                                const U & firstItem, ValueTypes &&... otherItems)
    {
        bool haveValue = iter.Next();
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            Exit(std::string(itemName) + " value mismatch: expected " + std::to_string(firstItem) +
                 " but got error: " + iter.GetStatus().AsString());
            return false;
        }
        if (!haveValue)
        {
            Exit(std::string(itemName) + " value mismatch: expected " + std::to_string(firstItem) +
                 " but found nothing or an error");
            return false;
        }
        if (iter.GetValue() != firstItem)
        {
            Exit(std::string(itemName) + " value mismatch: expected " + std::to_string(firstItem) + " but found " +
                 std::to_string(iter.GetValue()));
            return false;
        }
        return CheckValueAsListHelper<T>(itemName, iter, std::forward<ValueTypes>(otherItems)...);
    }

    template <typename T, typename... ValueTypes>
    bool CheckValueAsList(const char * itemName, chip::app::DataModel::DecodableList<T> list, ValueTypes &&... items)
    {
        auto iter = list.begin();
        return CheckValueAsListHelper<T>(itemName, iter, std::forward<ValueTypes>(items)...);
    }

    template <typename T>
    bool CheckValueAsListLength(const char * itemName, chip::app::DataModel::DecodableList<T> list, uint64_t expectedLength)
    {
        auto iter      = list.begin();
        uint64_t count = 0;
        while (iter.Next())
        {
            ++count;
        }
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            Exit(std::string(itemName) + " list length mismatch: expected " + std::to_string(expectedLength) + " but got an error");
            return false;
        }
        return CheckValueAsList(itemName, count, expectedLength);
    }

    bool CheckValueAsString(const char * itemName, chip::ByteSpan current, const char * expected);

    bool CheckValueAsString(const char * itemName, chip::CharSpan current, const char * expected);

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
