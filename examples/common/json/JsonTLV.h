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

#include <stddef.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/SafeInt.h>

#include <json/json.h>
#include <lib/core/Optional.h>

#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace chip {

namespace TLV {

/**
 *   @namespace chip::TLV::Json
 *
 *   @brief
 *     This namespace includes types and interfaces for json conversion of CHIP TLV.
 *
 */
namespace Json {

struct JsonContext
{
    Encoding::BufferWriter & mWriter;
    void * mContext;
};

extern CHIP_ERROR WriteJSON(const TLVReader & aReader, Encoding::BufferWriter & bWriter);

extern void ENFORCE_FORMAT(1, 2) TLVDumpWriter(const char * aFormat, ...);

} // namespace Json

} // namespace TLV

} // namespace chip

/**
 * Utility for converting a hex string to bytes, with the right error checking
 * and allocation size computation.
 *
 * Takes a functor to allocate the buffer to use for the hex bytes.  The functor
 * is expected to return uint8_t *.  The caller is responsible for cleaning up
 * this buffer as needed.
 *
 * On success, *octetCount is filled with the number of octets placed in the
 * buffer.  On failure, the value of *octetCount is undefined.
 */
template <typename F>
CHIP_ERROR ConvertHexToBytes(chip::CharSpan hex, F bufferAllocator, size_t * octetCount)
{
    *octetCount = 0;

    if (hex.size() % 2 != 0)
    {
        ChipLogError(chipTool, "Error while encoding '%.*s' as an octet string: Odd number of characters.",
                     static_cast<int>(hex.size()), hex.data());
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    const size_t bufferSize = hex.size() / 2;
    uint8_t * buffer        = bufferAllocator(bufferSize);
    if (buffer == nullptr && bufferSize != 0)
    {
        ChipLogError(chipTool, "Failed to allocate buffer of size: %llu", static_cast<unsigned long long>(bufferSize));
        return CHIP_ERROR_NO_MEMORY;
    }

    size_t byteCount = chip::Encoding::HexToBytes(hex.data(), hex.size(), buffer, bufferSize);
    if (byteCount == 0 && hex.size() != 0)
    {
        ChipLogError(chipTool, "Error while encoding '%.*s' as an octet string.", static_cast<int>(hex.size()), hex.data());
        return CHIP_ERROR_INTERNAL;
    }

    *octetCount = byteCount;
    return CHIP_NO_ERROR;
}

class JsonParser2
{
public:
    // Returns whether the parse succeeded.
    static bool ParseComplexArgument(const char * label, const char * json, Json::Value & value)
    {
        return Parse(label, json, /* strictRoot = */ true, value);
    }

    // Returns whether the parse succeeded.
    static bool ParseCustomArgument(const char * label, const char * json, Json::Value & value)
    {
        return Parse(label, json, /* strictRoot = */ false, value);
    }

private:
    static bool Parse(const char * label, const char * json, bool strictRoot, Json::Value & value)
    {
        Json::CharReaderBuilder readerBuilder;
        readerBuilder.settings_["strictRoot"]        = strictRoot;
        readerBuilder.settings_["allowSingleQuotes"] = true;
        readerBuilder.settings_["failIfExtra"]       = true;
        readerBuilder.settings_["rejectDupKeys"]     = true;

        auto reader = std::unique_ptr<Json::CharReader>(readerBuilder.newCharReader());
        std::string errors;
        if (reader->parse(json, json + strlen(json), &value, &errors))
        {
            return true;
        }

        // The CharReader API allows us to set failIfExtra, unlike Reader, but does
        // not allow us to get structured errors.  We get to try to manually undo
        // the work it did to create a string from the structured errors it had.
        ChipLogError(chipTool, "Error parsing JSON for %s:", label);

        // For each error "errors" has the following:
        //
        // 1) A line starting with "* " that has line/column info
        // 2) A line with the error message.
        // 3) An optional line with some extra info.
        //
        // We keep track of the last error column, in case the error message
        // reporting needs it.
        std::istringstream stream(errors);
        std::string error;
        chip::Optional<unsigned> errorColumn;
        while (getline(stream, error))
        {
            if (error.rfind("* ", 0) == 0)
            {
                // Flush out any pending error location.
                LogErrorLocation(errorColumn, json);

                // The format of this line is:
                //
                // * Line N, Column M
                //
                // Unfortunately it does not indicate end of error, so we can only
                // show its start.
                unsigned errorLine; // ignored in practice
                if (sscanf(error.c_str(), "* Line %u, Column %u", &errorLine, &errorColumn.Emplace()) != 2)
                {
                    ChipLogError(chipTool, "Unexpected location string: %s\n", error.c_str());
                    // We don't know how to make sense of this thing anymore.
                    break;
                }
                if (errorColumn.Value() == 0)
                {
                    ChipLogError(chipTool, "Expected error column to be at least 1");
                    // We don't know how to make sense of this thing anymore.
                    break;
                }
                // We are using our column numbers as offsets, so want them to be
                // 0-based.
                --errorColumn.Value();
            }
            else
            {
                ChipLogError(chipTool, "  %s", error.c_str());
                if (error == "  Missing ',' or '}' in object declaration" && errorColumn.HasValue() && errorColumn.Value() > 0 &&
                    json[errorColumn.Value() - 1] == '0' && (json[errorColumn.Value()] == 'x' || json[errorColumn.Value()] == 'X'))
                {
                    // Log the error location marker before showing the NOTE
                    // message.
                    LogErrorLocation(errorColumn, json);
                    ChipLogError(chipTool,
                                 "NOTE: JSON does not allow hex syntax beginning with 0x for numbers.  Try putting the hex number "
                                 "in quotes (like {\"name\": \"0x100\"}).");
                }
            }
        }

        // Write out the marker for our last error.
        LogErrorLocation(errorColumn, json);

        return false;
    }

private:
    static void LogErrorLocation(chip::Optional<unsigned> & errorColumn, const char * json)
    {
        if (!errorColumn.HasValue())
        {
            return;
        }

        const char * sourceText = json;
        unsigned error_start    = errorColumn.Value();
        // The whole JSON string might be too long to fit in our log
        // messages.  Just include 30 chars before the error.
        constexpr ptrdiff_t kMaxContext = 30;
        std::string errorMarker;
        if (error_start > kMaxContext)
        {
            sourceText += (error_start - kMaxContext);
            error_start = kMaxContext;
            ChipLogError(chipTool, "... %s", sourceText);
            // Add markers corresponding to the "... " above.
            errorMarker += "----";
        }
        else
        {
            ChipLogError(chipTool, "%s", sourceText);
        }
        for (unsigned i = 0; i < error_start; ++i)
        {
            errorMarker += "-";
        }
        errorMarker += "^";
        ChipLogError(chipTool, "%s", errorMarker.c_str());
        errorColumn.ClearValue();
    }
};

class JsonTLVParser
{
public:
    static CHIP_ERROR Put(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        if (value.isObject())
        {
            return JsonTLVParser::PutObject(writer, tag, value);
        }

        if (value.isArray())
        {
            return JsonTLVParser::PutArray(writer, tag, value);
        }

        if (value.isString())
        {
            return JsonTLVParser::PutCharString(writer, tag, value);
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
            ReturnErrorOnFailure(JsonTLVParser::Put(writer, chip::TLV::AnonymousTag(), value[i]));
        }

        return writer->EndContainer(outer);
    }

    static CHIP_ERROR PutObject(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        chip::TLV::TLVType outer;

        // need to check if its a tlvType or type
        if (value.isMember("tlvType") && value.isMember("value"))
        {
            constexpr const char kHexNumPrefix[] = "0x";
            constexpr size_t kHexNumPrefixLen    = ArraySize(kHexNumPrefix) - 1;

            Json::Value objTlvType = value.get("tlvType", "0x0c");
            if (strncmp(objTlvType.asCString(), kHexNumPrefix, kHexNumPrefixLen) != 0)
            {
                ChipLogError(chipTool, "bad tlvType=%s", objTlvType.asCString());
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            auto numberTlvType = std::stoull(objTlvType.asCString(), nullptr, 0);
            // ChipLogError(chipTool, "next tlvType=%s number=%llu", objTlvType.asCString(), numberTlvType);

            Json::Value objValue = value.get("value", "");

            chip::TLV::TLVElementType number = static_cast<chip::TLV::TLVElementType>(numberTlvType);
            if (number == chip::TLV::TLVElementType::Int8 || number == chip::TLV::TLVElementType::Int16 ||
                number == chip::TLV::TLVElementType::Int32 || number == chip::TLV::TLVElementType::Int64)
            {
                return chip::app::DataModel::Encode(*writer, tag, static_cast<int64_t>(objValue.asInt64()));
            }

            if (number == chip::TLV::TLVElementType::UInt8 || number == chip::TLV::TLVElementType::UInt16 ||
                number == chip::TLV::TLVElementType::UInt32 || number == chip::TLV::TLVElementType::UInt64)
            {
                return chip::app::DataModel::Encode(*writer, tag, static_cast<uint64_t>(objValue.asUInt64()));
            }

            if (number == chip::TLV::TLVElementType::FloatingPointNumber32 ||
                number == chip::TLV::TLVElementType::FloatingPointNumber64)
            {
                return chip::app::DataModel::Encode(*writer, tag, objValue.asFloat());
            }

            if (number == chip::TLV::TLVElementType::ByteString_1ByteLength ||
                number == chip::TLV::TLVElementType::ByteString_2ByteLength ||
                number == chip::TLV::TLVElementType::ByteString_4ByteLength ||
                number == chip::TLV::TLVElementType::ByteString_8ByteLength)
            {
                return PutOctetString(writer, tag, objValue);
            }

            // regular handling
            return JsonTLVParser::Put(writer, tag, objValue);
        }

        ReturnErrorOnFailure(writer->StartContainer(tag, chip::TLV::kTLVType_Structure, outer));

        for (auto const & id : value.getMemberNames())
        {
            auto index = std::stoul(id, nullptr, 0);
            VerifyOrReturnError(chip::CanCastTo<uint8_t>(index), CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(JsonTLVParser::Put(writer, chip::TLV::ContextTag(static_cast<uint8_t>(index)), value[id]));
        }

        return writer->EndContainer(outer);
    }

    static CHIP_ERROR PutOctetString(chip::TLV::TLVWriter * writer, chip::TLV::Tag tag, Json::Value & value)
    {
        const char * hexData = value.asCString();
        size_t hexDataLen    = strlen(hexData);
        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;

        size_t octetCount;
        ReturnErrorOnFailure(ConvertHexToBytes(
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
        size_t size    = strlen(value.asCString());
        CHIP_ERROR err = chip::app::DataModel::Encode(*writer, tag, chip::CharSpan(value.asCString(), size));
        return err;
    }
};

class JsonTLV
{
public:
    ~JsonTLV()
    {
        if (mData != nullptr)
        {
            chip::Platform::MemoryFree(mData);
        }
    }

    CHIP_ERROR Parse(const char * label, const char * json)
    {
        Json::Value value;
        if (!JsonParser2::ParseCustomArgument(label, json, value))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        mData = static_cast<uint8_t *>(chip::Platform::MemoryCalloc(sizeof(uint8_t), mDataMaxLen));
        VerifyOrReturnError(mData != nullptr, CHIP_ERROR_NO_MEMORY);

        chip::TLV::TLVWriter writer;
        writer.Init(mData, mDataMaxLen);

        ReturnErrorOnFailure(JsonTLVParser::Put(&writer, chip::TLV::AnonymousTag(), value));

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
