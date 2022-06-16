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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/List.h>
#include <app/data-model/Nullable.h>
#include <json/json.h>
#include <lib/core/Optional.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/SafeInt.h>

#include "JsonParser.h"

constexpr uint8_t kMaxLabelLength = 100;

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

        ChipLogError(chipTool, "Error while encoding %s as an unsigned integer.", label);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    template <typename T, std::enable_if_t<std::is_signed<T>::value, bool> = true>
    static CHIP_ERROR Setup(const char * label, T & request, Json::Value value)
    {
        if (!value.isNumeric() || !chip::CanCastTo<T>(value.asLargestInt()))
        {
            ChipLogError(chipTool, "Error while encoding %s as an unsigned integer.", label);
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
            ChipLogError(chipTool, "Error while encoding %s as an array.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        auto content = static_cast<typename std::remove_const<T>::type *>(chip::Platform::MemoryCalloc(value.size(), sizeof(T)));

        Json::ArrayIndex size = value.size();
        for (Json::ArrayIndex i = 0; i < size; i++)
        {
            char labelWithIndex[kMaxLabelLength];
            snprintf(labelWithIndex, sizeof(labelWithIndex), "%s[%d]", label, i);
            ReturnErrorOnFailure(ComplexArgumentParser::Setup(labelWithIndex, content[i], value[i]));
        }

        request = chip::app::DataModel::List<T>(content, value.size());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, chip::ByteSpan & request, Json::Value & value)
    {
        if (!value.isString())
        {
            ChipLogError(chipTool, "Error while encoding %s as an octet string: Not a string.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (strlen(value.asCString()) % 2 != 0)
        {
            ChipLogError(chipTool, "Error while encoding %s as an octet string: Odd number of characters.", label);
            return CHIP_ERROR_INVALID_STRING_LENGTH;
        }

        size_t size       = strlen(value.asCString());
        auto buffer       = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(size / 2, sizeof(uint8_t)));
        size_t octetCount = chip::Encoding::HexToBytes(value.asCString(), size, buffer, size / 2);

        request = chip::ByteSpan(buffer, octetCount);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, chip::CharSpan & request, Json::Value & value)
    {
        if (!value.isString())
        {
            ChipLogError(chipTool, "Error while encoding %s as a string: Not a string.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        size_t size = strlen(value.asCString());
        auto buffer = static_cast<char *>(chip::Platform::MemoryCalloc(size, sizeof(char)));
        strncpy(buffer, value.asCString(), size);

        request = chip::CharSpan(buffer, size);
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, float & request, Json::Value & value)
    {
        if (!value.isNumeric())
        {
            ChipLogError(chipTool, "Error while encoding %s as a float: Not a number.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = static_cast<float>(value.asFloat());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, double & request, Json::Value & value)
    {
        if (!value.isNumeric())
        {
            ChipLogError(chipTool, "Error while encoding %s as a double: Not a number.", label);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        request = static_cast<double>(value.asDouble());
        return CHIP_NO_ERROR;
    }

    static CHIP_ERROR Setup(const char * label, bool & request, Json::Value & value)
    {
        if (!value.isBool())
        {
            ChipLogError(chipTool, "Error while encoding %s as a boolean: Not a boolean.", label);
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

        ChipLogError(chipTool, "%s is required.  Should be provided as {\"%s\": value}", label, memberName);
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
        if (!JsonParser::ParseComplexArgument(label, json, value))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        return ComplexArgumentParser::Setup(label, *mRequest, value);
    }

private:
    T * mRequest;
};
