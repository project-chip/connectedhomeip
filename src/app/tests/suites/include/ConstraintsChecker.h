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

class ConstraintsChecker
{
public:
    ConstraintsChecker(){};
    virtual ~ConstraintsChecker(){};

protected:
    virtual void Exit(std::string message) = 0;

    bool CheckConstraintType(const char * itemName, const char * current, const char * expected)
    {
        ChipLogError(chipTool, "Warning: %s type checking is not implemented yet. Expected type: '%s'", itemName, expected);
        return true;
    }

    bool CheckConstraintFormat(const char * itemName, const char * current, const char * expected)
    {
        ChipLogError(chipTool, "Warning: %s format checking is not implemented yet. Expected format: '%s'", itemName, expected);
        return true;
    }

    bool CheckConstraintMinLength(const char * itemName, uint64_t current, uint64_t expected)
    {
        if (current < expected)
        {
            Exit(std::string(itemName) + " length < minLength: " + std::to_string(current) + " < " + std::to_string(expected));
            return false;
        }

        return true;
    }

    bool CheckConstraintMaxLength(const char * itemName, uint64_t current, uint64_t expected)
    {
        if (current > expected)
        {
            Exit(std::string(itemName) + " length > minLength: " + std::to_string(current) + " > " + std::to_string(expected));
            return false;
        }

        return true;
    }

    bool CheckConstraintStartsWith(const char * itemName, const chip::CharSpan current, const char * expected)
    {
        std::string value(current.data(), current.size());
        if (value.rfind(expected, 0) != 0)
        {
            Exit(std::string(itemName) + " (\"" + value + "\") does not starts with: \"" + std::string(expected) + "\"");
            return false;
        }

        return true;
    }

    bool CheckConstraintEndsWith(const char * itemName, const chip::CharSpan current, const char * expected)
    {
        std::string value(current.data(), current.size());
        if (value.find(expected, value.size() - strlen(expected)) == std::string::npos)
        {
            Exit(std::string(itemName) + " (\"" + value + "\") does not ends with: \"" + std::string(expected) + "\"");
            return false;
        }

        return true;
    }

    bool CheckConstraintIsUpperCase(const char * itemName, const chip::CharSpan current, bool expectUpperCase)
    {
        std::string value(current.data(), current.size());
        return CheckConstraintIsUpperCase(itemName, value.c_str(), expectUpperCase);
    }

    bool CheckConstraintIsUpperCase(const char * itemName, const char * current, bool expectUpperCase)
    {
        bool isUpperCase = true;
        for (size_t i = 0; i < strlen(current); i++)
        {
            if (!isdigit(current[i]) && !isupper(current[i]))
            {
                isUpperCase = false;
                break;
            }
        }

        if (expectUpperCase && !isUpperCase)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is not an upppercase string");
            return false;
        }

        if (!expectUpperCase && isUpperCase)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is an upppercase string");
            return false;
        }

        return true;
    }

    bool CheckConstraintIsLowerCase(const char * itemName, const chip::CharSpan current, bool expectLowerCase)
    {
        std::string value(current.data(), current.size());
        return CheckConstraintIsLowerCase(itemName, value.c_str(), expectLowerCase);
    }

    bool CheckConstraintIsLowerCase(const char * itemName, const char * current, bool expectLowerCase)
    {
        bool isLowerCase = true;
        for (size_t i = 0; i < strlen(current); i++)
        {
            if (isupper(current[i]))
            {
                isLowerCase = false;
                break;
            }
        }

        if (expectLowerCase && !isLowerCase)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is not a lowercase string");
            return false;
        }

        if (!expectLowerCase && isLowerCase)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is a lowercase string");
            return false;
        }

        return true;
    }

    bool CheckConstraintIsHexString(const char * itemName, const chip::CharSpan current, bool expectHexString)
    {
        std::string value(current.data(), current.size());
        return CheckConstraintIsHexString(itemName, value.c_str(), expectHexString);
    }

    bool CheckConstraintIsHexString(const char * itemName, const char * current, bool expectHexString)
    {
        bool isHexString = true;
        for (size_t i = 0; i < strlen(current); i++)
        {
            if (!isxdigit(current[i]))
            {
                isHexString = false;
                break;
            }
        }

        if (expectHexString && !isHexString)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is not a hexadecimal string");
            return false;
        }

        if (!expectHexString && isHexString)
        {
            Exit(std::string(itemName) + " (\"" + std::string(current) + "\") is a hexadecimal string");
            return false;
        }

        return true;
    }

    template <typename T, typename U, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    bool CheckConstraintMinValue(const char * itemName, T current, U expected)
    {
        if (current < expected)
        {
            Exit(std::string(itemName) + " value < minValue: " + std::to_string(current) + " < " + std::to_string(expected));
            return false;
        }

        return true;
    }

    template <typename T, typename U, std::enable_if_t<std::is_enum<T>::value, int> = 0>
    bool CheckConstraintMinValue(const char * itemName, T current, U expected)
    {
        return CheckConstraintMinValue(itemName, chip::to_underlying(current), expected);
    }

    template <typename T, typename U>
    bool CheckConstraintMinValue(const char * itemName, chip::BitFlags<T> current, U expected)
    {
        if (current.Raw() < expected)
        {
            Exit(std::string(itemName) + " value < minValue: " + std::to_string(current.Raw()) + " < " + std::to_string(expected));
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

    template <typename T, typename U, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    bool CheckConstraintMaxValue(const char * itemName, T current, U expected)
    {
        if (current > expected)
        {
            Exit(std::string(itemName) + " value > maxValue: " + std::to_string(current) + " > " + std::to_string(expected));
            return false;
        }

        return true;
    }

    template <typename T, typename U, std::enable_if_t<std::is_enum<T>::value, int> = 0>
    bool CheckConstraintMaxValue(const char * itemName, T current, U expected)
    {
        return CheckConstraintMaxValue(itemName, chip::to_underlying(current), expected);
    }

    template <typename T, typename U>
    bool CheckConstraintMaxValue(const char * itemName, chip::BitFlags<T> current, U expected)
    {
        if (current.Raw() > expected)
        {
            Exit(std::string(itemName) + " value > maxValue: " + std::to_string(current.Raw()) + " > " + std::to_string(expected));
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

    template <typename T, typename U, std::enable_if_t<!std::is_enum<T>::value, int> = 0>
    bool CheckConstraintNotValue(const char * itemName, T current, U expected)
    {
        if (current == expected)
        {
            Exit(std::string(itemName) + " got unexpected value: " + std::to_string(current));
            return false;
        }

        return true;
    }

    template <typename T, typename U, std::enable_if_t<std::is_enum<T>::value, int> = 0>
    bool CheckConstraintNotValue(const char * itemName, T current, U expected)
    {
        return CheckConstraintNotValue(itemName, chip::to_underlying(current), expected);
    }

    template <typename T>
    bool CheckConstraintNotValue(const char * itemName, chip::BitFlags<T> current, chip::BitFlags<T> expected)
    {
        if (current == expected)
        {
            Exit(std::string(itemName) + " got unexpected value: " + std::to_string(current.Raw()));
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

    bool CheckConstraintNotValue(const char * itemName, const chip::CharSpan current, const chip::CharSpan expected)
    {
        if (current.data_equal(expected))
        {
            Exit(std::string(itemName) + " got unexpected value: " + std::string(current.data(), current.size()));
            return false;
        }

        return true;
    }

    bool CheckConstraintNotValue(const char * itemName, const chip::ByteSpan current, const chip::ByteSpan expected)
    {
        if (current.data_equal(expected))
        {
            Exit(std::string(itemName) + " got unexpected value of size: " + std::to_string(current.size()));
            return false;
        }

        return true;
    }
};
