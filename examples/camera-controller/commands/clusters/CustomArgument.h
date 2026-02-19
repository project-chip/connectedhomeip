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

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <commands/common/HexConversion.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeInt.h>

#include <string>

#include "JsonParser.h"

namespace {
static constexpr char kPayloadHexPrefix[]         = "hex:";
static constexpr char kPayloadSignedPrefix[]      = "s:";
static constexpr char kPayloadUnsignedPrefix[]    = "u:";
static constexpr char kPayloadFloatPrefix[]       = "f:";
static constexpr char kPayloadDoublePrefix[]      = "d:";
static constexpr size_t kPayloadHexPrefixLen      = MATTER_ARRAY_SIZE(kPayloadHexPrefix) - 1;      // ignore null character
static constexpr size_t kPayloadSignedPrefixLen   = MATTER_ARRAY_SIZE(kPayloadSignedPrefix) - 1;   // ignore null character
static constexpr size_t kPayloadUnsignedPrefixLen = MATTER_ARRAY_SIZE(kPayloadUnsignedPrefix) - 1; // ignore null character
static constexpr size_t kPayloadFloatPrefixLen    = MATTER_ARRAY_SIZE(kPayloadFloatPrefix) - 1;    // ignore null character
static constexpr size_t kPayloadDoublePrefixLen   = MATTER_ARRAY_SIZE(kPayloadDoublePrefix) - 1;   // ignore null character
} // namespace

class CustomArgumentParser
{
public:
    static CHIP_ERROR Put(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        if (value.isObject())
        {
            return CustomArgumentParser::PutObject(writer, tag, value);
        }

        if (value.isArray())
        {
            return CustomArgumentParser::PutArray(writer, tag, value);
        }

        if (value.isString())
        {
            if (IsOctetString(value))
            {
                return CustomArgumentParser::PutOctetString(writer, tag, value);
            }
            if (IsUnsignedNumberPrefix(value))
            {
                return CustomArgumentParser::PutUnsignedFromString(writer, tag, value);
            }
            if (IsSignedNumberPrefix(value))
            {
                return CustomArgumentParser::PutSignedFromString(writer, tag, value);
            }
            if (IsFloatNumberPrefix(value))
            {
                return CustomArgumentParser::PutFloatFromString(writer, tag, value);
            }
            if (IsDoubleNumberPrefix(value))
            {
                return CustomArgumentParser::PutDoubleFromString(writer, tag, value);
            }

            return CustomArgumentParser::PutCharString(writer, tag, value);
        }

        if (value.isNull())
        {
            return chip::app::DataModel::Encode(*writer, tag, chip::app::DataModel::Nullable<uint8_t>());
        }

        if (value.isBool())
        {
            return chip::app::DataModel::Encode(*writer, tag, value.asBool());
        }

        if (value.isUInt())
        {
            return chip::app::DataModel::Encode(*writer, tag, value.asLargestUInt());
        }

        if (value.isInt())
        {
            return chip::app::DataModel::Encode(*writer, tag, value.asLargestInt());
        }

        if (value.isNumeric())
        {
            return chip::app::DataModel::Encode(*writer, tag, value.asDouble());
        }

        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

private:
    static CHIP_ERROR PutArray(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer->StartContainer(tag, chip::TLV::kTLVType_Array, outer));

        Json::ArrayIndex size = value.size();

        for (Json::ArrayIndex i = 0; i < size; i++)
        {
            ReturnErrorOnFailure(CustomArgumentParser::Put(writer, chip::TLV::AnonymousTag(), value[i]));
        }

        return writer->EndContainer(outer);
    }

    static CHIP_ERROR PutObject(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        chip::TLV::TLVType outer;
        ReturnErrorOnFailure(writer->StartContainer(tag, chip::TLV::kTLVType_Structure, outer));

        for (auto const & id : value.getMemberNames())
        {
            auto index = std::stoul(id, nullptr, 0);
            VerifyOrReturnError(chip::CanCastTo<uint8_t>(index), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(CustomArgumentParser::Put(writer, chip::TLV::ContextTag(static_cast<uint8_t>(index)), value[id]));
        }

        return writer->EndContainer(outer);
    }

    static CHIP_ERROR PutOctetString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        const char * hexData = value.asCString() + kPayloadHexPrefixLen;
        size_t hexDataLen    = strlen(hexData);
        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;

        size_t octetCount;
        ReturnErrorOnFailure(HexToBytes(
            chip::CharSpan(hexData, hexDataLen),
            [&buffer](size_t allocSize) {
                buffer.Calloc(allocSize);
                return buffer.Get();
            },
            &octetCount));

        return chip::app::DataModel::Encode(*writer, tag, chip::ByteSpan(buffer.Get(), octetCount));
    }

    static CHIP_ERROR PutCharString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        size_t size = strlen(value.asCString());
        return chip::app::DataModel::Encode(*writer, tag, chip::CharSpan(value.asCString(), size));
    }

    static CHIP_ERROR PutUnsignedFromString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        char numberAsString[21];
        chip::Platform::CopyString(numberAsString, value.asCString() + kPayloadUnsignedPrefixLen);

        auto number = std::stoull(numberAsString, nullptr, 0);
        return chip::app::DataModel::Encode(*writer, tag, static_cast<uint64_t>(number));
    }

    static CHIP_ERROR PutSignedFromString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        char numberAsString[21];
        chip::Platform::CopyString(numberAsString, value.asCString() + kPayloadSignedPrefixLen);

        auto number = std::stoll(numberAsString, nullptr, 0);
        return chip::app::DataModel::Encode(*writer, tag, static_cast<int64_t>(number));
    }

    static CHIP_ERROR PutFloatFromString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        char numberAsString[21];
        chip::Platform::CopyString(numberAsString, value.asCString() + kPayloadFloatPrefixLen);

        auto number = std::stof(numberAsString);
        return chip::app::DataModel::Encode(*writer, tag, number);
    }

    static CHIP_ERROR PutDoubleFromString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        char numberAsString[21];
        chip::Platform::CopyString(numberAsString, value.asCString() + kPayloadDoublePrefixLen);

        auto number = std::stod(numberAsString);
        return chip::app::DataModel::Encode(*writer, tag, number);
    }

    static bool IsOctetString(Json::Value & value)
    {
        return (strncmp(value.asCString(), kPayloadHexPrefix, kPayloadHexPrefixLen) == 0);
    }

    static bool IsUnsignedNumberPrefix(Json::Value & value)
    {
        return (strncmp(value.asCString(), kPayloadUnsignedPrefix, kPayloadUnsignedPrefixLen) == 0);
    }

    static bool IsSignedNumberPrefix(Json::Value & value)
    {
        return (strncmp(value.asCString(), kPayloadSignedPrefix, kPayloadSignedPrefixLen) == 0);
    }

    static bool IsFloatNumberPrefix(Json::Value & value)
    {
        return (strncmp(value.asCString(), kPayloadFloatPrefix, kPayloadFloatPrefixLen) == 0);
    }

    static bool IsDoubleNumberPrefix(Json::Value & value)
    {
        return (strncmp(value.asCString(), kPayloadDoublePrefix, kPayloadDoublePrefixLen) == 0);
    }
};

class CustomArgument
{
public:
    ~CustomArgument()
    {
        if (mData != nullptr)
        {
            chip::Platform::MemoryFree(mData);
        }
    }

    CHIP_ERROR Parse(const char * label, const char * json)
    {
        Json::Value value;
        static constexpr char kHexNumPrefix[] = "0x";
        constexpr size_t kHexNumPrefixLen     = MATTER_ARRAY_SIZE(kHexNumPrefix) - 1;
        if (strncmp(json, kPayloadHexPrefix, kPayloadHexPrefixLen) == 0 ||
            strncmp(json, kPayloadSignedPrefix, kPayloadSignedPrefixLen) == 0 ||
            strncmp(json, kPayloadUnsignedPrefix, kPayloadUnsignedPrefixLen) == 0 ||
            strncmp(json, kPayloadFloatPrefix, kPayloadFloatPrefixLen) == 0 ||
            strncmp(json, kPayloadDoublePrefix, kPayloadDoublePrefixLen) == 0)
        {
            value = Json::Value(json);
        }
        else if (strncmp(json, kHexNumPrefix, kHexNumPrefixLen) == 0)
        {
            // Assume that hex numbers are unsigned.  Prepend
            // kPayloadUnsignedPrefix and then let the rest of the logic handle
            // things.
            std::string str(kPayloadUnsignedPrefix);
            str += json;
            value = Json::Value(str);
        }
        else if (!JsonParser::ParseCustomArgument(label, json, value))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrReturnError(mData != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::TLV::TLVWriter writer;
        writer.Init(mData, mDataMaxLen);

        ReturnErrorOnFailure(CustomArgumentParser::Put(&writer, chip::TLV::AnonymousTag(), value));

        mDataLen = writer.GetLengthWritten();
        return writer.Finalize();
    }

    CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
    {
        chip::TLV::TLVReader reader;
        reader.Init(mData, mDataLen);
        ReturnErrorOnFailure(reader.Next());

        return writer.CopyElement(tag, reader);
    }

    // We trust our consumers to do the encoding of our data correctly, so don't
    // need to know whether we are being encoded for a write.
    static constexpr bool kIsFabricScoped = false;

private:
    uint8_t * mData                       = nullptr;
    uint32_t mDataLen                     = 0;
    static constexpr uint32_t mDataMaxLen = 4096;
};
