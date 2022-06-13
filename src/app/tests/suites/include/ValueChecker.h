/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include <string>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

class ValueChecker
{
public:
    ValueChecker(){};
    virtual ~ValueChecker(){};

protected:
    virtual void Exit(std::string message, CHIP_ERROR err = CHIP_ERROR_INTERNAL) = 0;

    bool CheckDecodeValue(CHIP_ERROR error)
    {
        if (CHIP_NO_ERROR != error)
        {
            Exit(std::string("Can not decode data: ") + chip::ErrorStr(error));
            return false;
        }

        return true;
    }

    bool CheckValueAsString(const char * itemName, chip::ByteSpan current, chip::ByteSpan expected)
    {
        if (!current.data_equal(expected))
        {
            Exit(std::string(itemName) + " value mismatch, expecting " +
                 std::string(chip::Uint8::to_const_char(expected.data()), expected.size()));
            return false;
        }

        return true;
    }

    bool CheckValueAsString(const char * itemName, chip::CharSpan current, chip::CharSpan expected)
    {
        if (!current.data_equal(expected))
        {
            Exit(std::string(itemName) + " value mismatch, expected '" + std::string(expected.data(), expected.size()) +
                 "' but got '" + std::string(current.data(), current.size()) + "'");
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

    template <typename T, typename U>
    bool CheckValue(const char * itemName, chip::BitFlags<T> current, U expected)
    {
        return CheckValue(itemName, current.Raw(), expected);
    }

    template <typename T, typename U>
    bool CheckValue(const char * itemName, chip::BitMask<T> current, U expected)
    {
        return CheckValue(itemName, current.Raw(), expected);
    }

    // Allow an expected value that is a nullable wrapped around the actual
    // value (e.g. a SaveAs from reading a different attribute that has a value
    // space that includes null).  In that case we check that:
    // 1) The nullable is not in fact null.
    //
    // 2) The value in the nullable matches our test value.
    template <typename T>
    bool CheckValue(const char * itemName, T current, const chip::app::DataModel::Nullable<T> & expected)
    {
        auto nullableName = std::string(itemName);
        nullableName += " expected value";
        return CheckValueNonNull(nullableName.c_str(), expected) && CheckValue(itemName, current, expected.Value());
    }

    template <typename T>
    bool CheckValue(const char * itemName, chip::BitFlags<T> current, chip::BitFlags<T> expected)
    {
        return CheckValue(itemName, current.Raw(), expected.Raw());
    }

    template <typename T>
    bool CheckValue(const char * itemName, chip::BitMask<T> current, chip::BitMask<T> expected)
    {
        return CheckValue(itemName, current.Raw(), expected.Raw());
    }

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
};
