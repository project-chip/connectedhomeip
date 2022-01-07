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
#include <app/tests/suites/pics/PICSBooleanExpressionParser.h>
#include <app/tests/suites/pics/PICSBooleanReader.h>
#include <lib/support/BitFlags.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/UnitTestUtils.h>
#include <type_traits>
#include <zap-generated/tests/CHIPClustersTest.h>

constexpr uint16_t kTimeoutInSeconds = 30;

class TestCommand : public CHIPCommand
{
public:
    TestCommand(const char * commandName) :
        CHIPCommand(commandName), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("delayInMs", 0, UINT64_MAX, &mDelayInMs);
        AddArgument("PICS", &mPICSFilePath);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(kTimeoutInSeconds); }

    virtual void NextTest() = 0;

    /////////// GlobalCommands Interface /////////
    CHIP_ERROR Wait(chip::System::Clock::Timeout ms);
    CHIP_ERROR WaitForMs(uint16_t ms) { return Wait(chip::System::Clock::Milliseconds32(ms)); }
    CHIP_ERROR WaitForCommissionee();
    CHIP_ERROR Log(const char * message);
    CHIP_ERROR UserPrompt(const char * message);

protected:
    std::map<std::string, ChipDevice *> mDevices;
    chip::NodeId mNodeId;

    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);
    static void OnWaitForMsFn(chip::System::Layer * systemLayer, void * context);

    CHIP_ERROR ContinueOnChipMainThread() { return WaitForMs(0); };

    void Exit(std::string message);
    void ThrowFailureResponse();
    void ThrowSuccessResponse();

    bool CheckConstraintType(const char * itemName, const char * current, const char * expected);
    bool CheckConstraintFormat(const char * itemName, const char * current, const char * expected);
    bool CheckConstraintMinLength(const char * itemName, uint64_t current, uint64_t expected);
    bool CheckConstraintMaxLength(const char * itemName, uint64_t current, uint64_t expected);
    bool CheckConstraintStartsWith(const char * itemName, const chip::Span<const char> current, const char * expected);
    bool CheckConstraintEndsWith(const char * itemName, const chip::Span<const char> current, const char * expected);
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
    template <typename T, typename U>
    bool CheckConstraintMinValue(const char * itemName, const chip::app::DataModel::Nullable<T> & current, U expected)
    {
        if (current.IsNull())
        {
            return true;
        }
        return CheckConstraintMinValue(itemName, current.Value(), static_cast<T>(expected));
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
    template <typename T, typename U>
    bool CheckConstraintMaxValue(const char * itemName, const chip::app::DataModel::Nullable<T> & current, U expected)
    {
        if (current.IsNull())
        {
            return true;
        }
        return CheckConstraintMaxValue(itemName, current.Value(), static_cast<T>(expected));
    }
    template <typename T, typename U>
    bool CheckConstraintNotValue(const char * itemName, T current, U expected)
    {
        if (current == expected)
        {
            Exit(std::string(itemName) + " got unexpected value: " + std::to_string(current));
            return false;
        }

        return true;
    }
    template <typename T, typename U>
    bool CheckConstraintNotValue(const char * itemName, const chip::app::DataModel::Nullable<T> & current, U expected)
    {
        if (current.IsNull())
        {
            return true;
        }
        return CheckConstraintNotValue(itemName, current.Value(), expected);
    }

    bool CheckConstraintNotValue(const char * itemName, chip::CharSpan current, chip::CharSpan expected)
    {
        if (current.data_equal(expected))
        {
            Exit(std::string(itemName) + " got unexpected value: " + std::string(current.data(), current.size()));
            return false;
        }

        return true;
    }

    bool CheckConstraintNotValue(const char * itemName, chip::ByteSpan current, chip::ByteSpan expected)
    {
        if (current.data_equal(expected))
        {
            Exit(std::string(itemName) + " got unexpected value of size: " + std::to_string(current.size()));
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

    template <typename T, typename U>
    bool CheckValue(const char * itemName, chip::BitFlags<T> current, U expected)
    {
        return CheckValue(itemName, current.Raw(), expected);
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

    bool ShouldSkip(const char * expression);

    void Wait()
    {
        if (mDelayInMs.HasValue())
        {
            chip::test_utils::SleepMillis(mDelayInMs.Value());
        }
    };
    chip::Optional<uint64_t> mDelayInMs;
    chip::Optional<char *> mPICSFilePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    chip::Optional<uint16_t> mTimeout;
    chip::Optional<std::map<std::string, bool>> PICS;
};
