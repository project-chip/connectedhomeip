/*
 *   Copyright (c) 2025 Project CHIP Authors
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

/**
 * This file allocate/free memory using the chip platform abstractions
 * (Platform::MemoryCalloc and Platform::MemoryFree) for hosting a subset of the
 * data model internal types until they are consumed by the DataModel::Encode machinery:
 *   - chip::app:DataModel::List<T>
 *   - chip::ByteSpan
 *   - chip::CharSpan
 *
 * Memory allocation happens during the 'Setup' phase, while memory deallocation happens
 * during the 'Finalize' phase.
 *
 * The 'Finalize' phase during the destructor phase, and if needed, 'Finalize' will call
 * the 'Finalize' phase of its descendant.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <commands/common/HexConversion.h>
#include <json/json.h>
#include <lib/core/Optional.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeInt.h>

#include "JsonParser.h"

inline constexpr uint8_t kMaxLabelLength = UINT8_MAX;
inline constexpr char kNullString[]      = "null";

class ComplexArgumentParser
{
public:
    ComplexArgumentParser() {}

    template <typename T,
              typename std::enable_if_t<std::is_integral<T>::value && !std::is_signed<T>::value &&
                                            !std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, bool>::value,
                                        int> = 0>
    static CHIP_ERROR Setup(const char * label, T & request, Json::Value value)
    {
        if (value.isNumeric())
        {
            if (chip::CanCastTo<T>(value.asLargestUInt()))
            {
                request = static_cast<T>(value.asLargestUInt());
                return CHIP_NO_ERROR;
            }
        }
        else if (value.isString())
        {
            // Check for a hex number; JSON does not support those as numbers,
            // so they have to be done as strings.  And we might as well support
            // string-encoded unsigned numbers in general if we're doing that.
            bool isHexNotation = strncmp(value.asCString(), "0x", 2) == 0 || strncmp(value.asCString(), "0X", 2) == 0;

            std::stringstream str;
            isHexNotation ? str << std::hex << value.asCString() : str << value.asCString();
            uint64_t val;
            str >> val;
            if (!str.fail() && str.eof() && chip::CanCastTo<T>(val))
            {
                request = static_cast<T>(val);
                return CHIP_NO_ERROR;
            }
        }

        ChipLogError(NotSpecified, "Error while encoding %s as an unsigned integer.", label);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    template <typename T, std::enable_if_t<std::is_signed<T>::value, bool> = true>
    static CHIP_ERROR Setup(const char * label, T & request, Json::Value value)
    {
        if (!value.isNumeric() || !chip::CanCastTo<T>(value.asLargestInt()))
        {
            ChipLogError(NotSpecified, "Error while encoding %s as an unsigned integer.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = static_cast<T>(value.asLargestInt());
        return CHIP_NO_ERROR;
    }

    template <typename T, typename std::enable_if_t<std::is_enum<T>::value, int> = 0>
    static CHIP_ERROR Setup(const char * label, T & request, Json::Value value)
    {
        std::underlying_type_t<T> requestValue;
        ReturnErrorOnFailure(ComplexArgumentParser::Setup(label, requestValue, value));

        request = static_cast<T>(requestValue);
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR Setup(const char * label, chip::BitFlags<T> & request, Json::Value & value)
    {
        T requestValue;
        ReturnErrorOnFailure(ComplexArgumentParser::Setup(label, requestValue, value));

        request = chip::BitFlags<T>(requestValue);
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR Setup(const char * label, chip::BitMask<T> & request, Json::Value & value)
    {
        T requestValue;
        ReturnErrorOnFailure(ComplexArgumentParser::Setup(label, requestValue, value));

        request = chip::BitMask<T>(requestValue);
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR Setup(const char * label, chip::Optional<T> & request, Json::Value & value)
    {
        T requestValue;
        ReturnErrorOnFailure(ComplexArgumentParser::Setup(label, requestValue, value));

        request = chip::Optional<T>(requestValue);
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR Setup(const char * label, chip::app::DataModel::Nullable<T> & request, Json::Value & value)
    {
        if (value.isNull())
        {
            request.SetNull();
            return CHIP_NO_ERROR;
        }

        T requestValue;
        ReturnErrorOnFailure(ComplexArgumentParser::Setup(label, requestValue, value));

        request = chip::app::DataModel::Nullable<T>(requestValue);
        return CHIP_NO_ERROR;
    }

    template <typename T>
    static CHIP_ERROR Setup(const char * label, chip::app::DataModel::List<T> & request, Json::Value & value)
    {
        if (!value.isArray())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as an array.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        auto content = static_cast<typename std::remove_const<T>::type *>(chip::Platform::MemoryCalloc(value.size(), sizeof(T)));
        VerifyOrReturnError(content != nullptr, CHIP_ERROR_NO_MEMORY);

        Json::ArrayIndex size = value.size();
        for (Json::ArrayIndex i = 0; i < size; i++)
        {
            char labelWithIndex[kMaxLabelLength];
            // GCC 7.0.1 has introduced some new warnings for snprintf (-Werror=format-truncation) by default.
            // This is not particularly useful when using snprintf and especially in this context, so in order
            // to disable the warning the %s is constrained to be of max length: (254 - 11 - 2) where:
            //  - 254 is kMaxLabelLength - 1 (for null)
            //  - 11 is the maximum length of a %d (-2147483648, 2147483647)
            //  - 2 is the length for the "[" and "]" characters.
            snprintf(labelWithIndex, sizeof(labelWithIndex), "%.241s[%d]", label, i);
            ReturnErrorOnFailure(ComplexArgumentParser::Setup(labelWithIndex, content[i], value[i]));
        }

        request = chip::app::DataModel::List<T>(content, value.size());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, chip::ByteSpan & request, Json::Value & value)
    {
        if (!value.isString())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as an octet string: Not a string.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        auto str         = value.asString();
        auto size        = str.size();
        uint8_t * buffer = nullptr;

        if (IsStrString(str.c_str()))
        {
            // Skip the prefix
            str.erase(0, kStrStringPrefixLen);
            size = str.size();

            buffer = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(size, sizeof(uint8_t)));
            VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_NO_MEMORY);

            memcpy(buffer, str.c_str(), size);
        }
        else
        {
            if (IsHexString(str.c_str()))
            {
                // Skip the prefix
                str.erase(0, kHexStringPrefixLen);
                size = str.size();
            }

            CHIP_ERROR err = HexToBytes(
                chip::CharSpan(str.c_str(), size),
                [&buffer](size_t allocSize) {
                    buffer = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(allocSize, sizeof(uint8_t)));
                    return buffer;
                },
                &size);

            if (err != CHIP_NO_ERROR)
            {
                if (buffer != nullptr)
                {
                    chip::Platform::MemoryFree(buffer);
                }

                return err;
            }
        }

        request = chip::ByteSpan(buffer, size);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, chip::CharSpan & request, Json::Value & value)
    {
        if (!value.isString())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as a string: Not a string.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        size_t size = strlen(value.asCString());
        auto buffer = static_cast<char *>(chip::Platform::MemoryCalloc(size, sizeof(char)));
        VerifyOrReturnError(buffer != nullptr, CHIP_ERROR_NO_MEMORY);

        memcpy(buffer, value.asCString(), size);

        request = chip::CharSpan(buffer, size);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, float & request, Json::Value & value)
    {
        if (!value.isNumeric())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as a float: Not a number.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = static_cast<float>(value.asFloat());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, double & request, Json::Value & value)
    {
        if (!value.isNumeric())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as a double: Not a number.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = static_cast<double>(value.asDouble());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, bool & request, Json::Value & value)
    {
        if (!value.isBool())
        {
            ChipLogError(NotSpecified, "Error while encoding %s as a boolean: Not a boolean.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = value.asBool();
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR EnsureMemberExist(const char * label, const char * memberName, bool hasMember)
    {
        if (hasMember)
        {
            return CHIP_NO_ERROR;
        }

        ChipLogError(NotSpecified, "%s is required.  Should be provided as {\"%s\": value}", label, memberName);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    static CHIP_ERROR EnsureNoMembersRemaining(const char * label, const Json::Value & value)
    {
        auto remainingFields = value.getMemberNames();
        if (remainingFields.size() == 0)
        {
            return CHIP_NO_ERROR;
        }
#if CHIP_ERROR_LOGGING
        for (auto & field : remainingFields)
        {
            ChipLogError(NotSpecified, "Unexpected field name: '%s.%s'", label, field.c_str());
        }
#endif // CHIP_ERROR_LOGGING
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    template <typename T>
    static void Finalize(T & request)
    {
        // Nothing to do
    }

    template <typename T>
    static void Finalize(chip::Optional<T> & request)
    {
        VerifyOrReturn(request.HasValue());
        ComplexArgumentParser::Finalize(request.Value());
    }

    template <typename T>
    static void Finalize(chip::app::DataModel::Nullable<T> & request)
    {
        VerifyOrReturn(!request.IsNull());
        ComplexArgumentParser::Finalize(request.Value());
    }

    static void Finalize(chip::ByteSpan & request)
    {
        VerifyOrReturn(request.data() != nullptr);
        chip::Platform::MemoryFree(reinterpret_cast<void *>(const_cast<uint8_t *>(request.data())));
    }

    static void Finalize(chip::CharSpan & request)
    {
        VerifyOrReturn(request.data() != nullptr);
        chip::Platform::MemoryFree(reinterpret_cast<void *>(const_cast<char *>(request.data())));
    }

    template <typename T>
    static void Finalize(chip::app::DataModel::List<T> & request)
    {
        VerifyOrReturn(request.data() != nullptr);

        size_t size = request.size();
        auto data   = const_cast<typename std::remove_const<T>::type *>(request.data());
        for (size_t i = 0; i < size; i++)
        {
            Finalize(data[i]);
        }

        chip::Platform::MemoryFree(reinterpret_cast<void *>(data));
    }

#include <zap-generated/cluster/ComplexArgumentParser.h>
};

class ComplexArgument
{
public:
    virtual ~ComplexArgument() {}

    virtual CHIP_ERROR Parse(const char * label, const char * json) = 0;

    virtual void Reset() = 0;
};

template <typename T>
class TypedComplexArgument : public ComplexArgument
{
public:
    TypedComplexArgument() {}
    TypedComplexArgument(T * request) : mRequest(request) {}
    ~TypedComplexArgument()
    {
        if (mRequest != nullptr)
        {
            ComplexArgumentParser::Finalize(*mRequest);
        }
    }

    void SetArgument(T * request) { mRequest = request; };

    CHIP_ERROR Parse(const char * label, const char * json)
    {
        Json::Value value;
        if (strcmp(kNullString, json) == 0)
        {
            value = Json::nullValue;
        }
        else if (!JsonParser::ParseComplexArgument(label, json, value))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        return ComplexArgumentParser::Setup(label, *mRequest, value);
    }

    void Reset() { *mRequest = T(); }

private:
    T * mRequest;
};
