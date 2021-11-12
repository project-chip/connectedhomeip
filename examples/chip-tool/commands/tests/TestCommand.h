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
#include <lib/support/TypeTraits.h>
#include <lib/support/UnitTestUtils.h>
#include <type_traits>
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
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

    virtual void NextTest() = 0;

    /////////// GlobalCommands Interface /////////
    CHIP_ERROR Wait(chip::System::Clock::Timeout ms);
    CHIP_ERROR WaitForMs(uint16_t ms) { return Wait(chip::System::Clock::Milliseconds32(ms)); }
    CHIP_ERROR Log(const char * message);

protected:
    ChipDevice * mDevice;
    chip::NodeId mNodeId;

    static void OnDeviceConnectedFn(void * context, chip::DeviceProxy * device);
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

    // Allow a different expected type from the actual value type, because if T
    // is short the literal we are using is not short-typed.
    template <typename T, typename U, typename std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    bool CheckValue(const char * itemName, T current, U expected)
    {
        if (current != expected)
        {
            Exit(std::string(itemName) + " value mismatch: expected " + std::to_string(expected) + " but got " +
                 std::to_string(current));
            return false;
        }

        return true;
    }

    template <typename T, typename U, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    bool CheckValue(const char * itemName, T current, U expected)
    {
        return CheckValue(itemName, chip::to_underlying(current), expected);
    }

    /**
     * Check that the next list item, which is at index "index", exists and
     * decodes properly.
     */
    template <typename ListType>
    bool CheckNextListItemDecodes(const char * listName, typename std::remove_reference_t<ListType>::Iterator & iter, size_t index)
    {
        bool hasValue = iter.Next();
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            Exit(std::string(listName) + " value mismatch: error '" + iter.GetStatus().AsString() + "'decoding item at index " +
                 std::to_string(index));
            return false;
        }

        if (hasValue)
        {
            return true;
        }

        Exit(std::string(listName) + " value mismatch: should have value at index " + std::to_string(index) +
             " but doesn't (actual value too short)");
        return false;
    }

    /**
     * Check that there are no more list items now that we have seen
     * "expectedCount" of them.
     */
    template <typename ListType>
    bool CheckNoMoreListItems(const char * listName, typename std::remove_reference_t<ListType>::Iterator & iter,
                              size_t expectedCount)
    {
        bool hasValue = iter.Next();
        if (iter.GetStatus() != CHIP_NO_ERROR)
        {
            Exit(std::string(listName) + " value mismatch: error '" + iter.GetStatus().AsString() +
                 "'decoding item after we have seen " + std::to_string(expectedCount) + " items");
            return false;
        }

        if (!hasValue)
        {
            return true;
        }

        Exit(std::string(listName) + " value mismatch: expected only " + std::to_string(expectedCount) +
             " items, but have more than that (actual value too long)");
        return false;
    }

    bool CheckValueAsString(const char * itemName, chip::ByteSpan current, chip::ByteSpan expected);

    bool CheckValueAsString(const char * itemName, chip::CharSpan current, chip::CharSpan expected);

    template <typename T>
    bool CheckValuePresent(const char * itemName, const chip::Optional<T> & value)
    {
        if (value.HasValue())
        {
            return true;
        }

        Exit(std::string(itemName) + " expected to have value but doesn't");
        return false;
    }

    template <typename T>
    bool CheckValueNull(const char * itemName, const chip::app::DataModel::Nullable<T> & value)
    {
        if (value.IsNull())
        {
            return true;
        }

        Exit(std::string(itemName) + " expected to be null but isn't");
        return false;
    }

    template <typename T>
    bool CheckValueNonNull(const char * itemName, const chip::app::DataModel::Nullable<T> & value)
    {
        if (!value.IsNull())
        {
            return true;
        }

        Exit(std::string(itemName) + " expected to not be null but is");
        return false;
    }

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    void Wait()
    {
        if (mDelayInMs)
        {
            chip::test_utils::SleepMillis(mDelayInMs);
        }
    };
    uint64_t mDelayInMs = 0;
};
