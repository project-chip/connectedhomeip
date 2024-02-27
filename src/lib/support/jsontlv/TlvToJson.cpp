/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "lib/support/CHIPMemString.h"
#include "lib/support/ScopedBuffer.h"
#include <json/json.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/jsontlv/ElementTypes.h>
#include <lib/support/jsontlv/TlvToJson.h>

namespace chip {

namespace {

// actual value of this does not actually matter, however we need
// a value to be able to read 32-bit implicit profile tags
//
// JSON format never has this and TLV payload contains "implicit profile"
// and this value is never stored.
constexpr uint32_t kTemporaryImplicitProfileId = 0xFF01;

/// RAII to switch the implicit profile id for a reader
class ImplicitProfileIdChange
{
public:
    ImplicitProfileIdChange(TLV::TLVReader & reader, uint32_t id) : mReader(reader), mOldImplicitProfileId(reader.ImplicitProfileId)
    {
        reader.ImplicitProfileId = id;
    }
    ~ImplicitProfileIdChange() { mReader.ImplicitProfileId = mOldImplicitProfileId; }

private:
    TLV::TLVReader & mReader;
    uint32_t mOldImplicitProfileId;
};

const char * GetJsonElementStrFromType(const ElementTypeContext & ctx)
{
    switch (ctx.tlvType)
    {
    case TLV::kTLVType_UnsignedInteger:
        return kElementTypeUInt;
    case TLV::kTLVType_SignedInteger:
        return kElementTypeInt;
    case TLV::kTLVType_Boolean:
        return kElementTypeBool;
    case TLV::kTLVType_FloatingPointNumber:
        return ctx.isDouble ? kElementTypeDouble : kElementTypeFloat;
    case TLV::kTLVType_ByteString:
        return kElementTypeBytes;
    case TLV::kTLVType_UTF8String:
        return kElementTypeString;
    case TLV::kTLVType_Null:
        return kElementTypeNull;
    case TLV::kTLVType_Structure:
        return kElementTypeStruct;
    case TLV::kTLVType_Array:
        return kElementTypeArray;
    default:
        return kElementTypeEmpty;
    }
};

/*
 * Encapsulates the element information required to construct a JSON element name string in a JSON object.
 *
 * The generated JSON element name string is constructed as:
 *     'TagNumber:ElementType-SubElementType'.
 */
struct JsonObjectElementContext
{
    JsonObjectElementContext(TLV::TLVReader & reader)
    {
        tag               = reader.GetTag();
        implicitProfileId = reader.ImplicitProfileId;
        type.tlvType      = reader.GetType();
        if (type.tlvType == TLV::kTLVType_FloatingPointNumber)
        {
            type.isDouble = reader.IsElementDouble();
        }
    }

    std::string GenerateJsonElementName() const
    {
        std::string str = "???";
        if (TLV::IsContextTag(tag))
        {
            // common case for context tags: raw value
            str = std::to_string(TLV::TagNumFromTag(tag));
        }
        else if (TLV::IsProfileTag(tag))
        {
            if (TLV::ProfileIdFromTag(tag) == implicitProfileId)
            {
                str = std::to_string(TLV::TagNumFromTag(tag));
            }
            else
            {
                uint32_t tagNumber = (static_cast<uint32_t>(TLV::VendorIdFromTag(tag)) << 16) | TLV::TagNumFromTag(tag);
                str                = std::to_string(tagNumber);
            }
        }
        str = str + ":" + GetJsonElementStrFromType(type);
        if (type.tlvType == TLV::kTLVType_Array)
        {
            str = str + "-" + GetJsonElementStrFromType(subType);
        }
        return str;
    }

    TLV::Tag tag;
    uint32_t implicitProfileId;
    ElementTypeContext type;
    ElementTypeContext subType;
};

/*
 * This templated function inserts a name/value pair into the Json object.
 * The value is templated to be of type T and accepts any of the following types:
 *
 *      bool, uint*_t, int*_t, char *, float, double, std::string, Json::Value
 *
 * This method uses the provided element context to generate Json name string.
 */
template <typename T>
void InsertJsonElement(Json::Value & json, const JsonObjectElementContext & ctx, T val)
{
    if (json.isArray())
    {
        json.append(val);
    }
    else
    {
        json[ctx.GenerateJsonElementName()] = val;
    }
}

static CHIP_ERROR TlvToJson(TLV::TLVReader & reader, Json::Value & jsonObj);

/*
 * Given a TLVReader positioned at TLV structure this function:
 *   - enters structure
 *   - converts all elements of a structure into JSON object representation
 *   - exits structure
 */
CHIP_ERROR TlvStructToJson(TLV::TLVReader & reader, Json::Value & jsonObj)
{
    CHIP_ERROR err;
    TLV::TLVType containerType;

    ReturnErrorOnFailure(reader.EnterContainer(containerType));

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        TLV::Tag tag = reader.GetTag();
        VerifyOrReturnError(TLV::IsContextTag(tag) || TLV::IsProfileTag(tag), CHIP_ERROR_INVALID_TLV_TAG);

        if (TLV::IsProfileTag(tag) && TLV::VendorIdFromTag(tag) == 0)
        {
            VerifyOrReturnError(TLV::TagNumFromTag(tag) > UINT8_MAX, CHIP_ERROR_INVALID_TLV_TAG);
        }

        // Recursively convert to JSON the item within the struct.
        ReturnErrorOnFailure(TlvToJson(reader, jsonObj));
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    return reader.ExitContainer(containerType);
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, Json::Value & jsonObj)
{
    JsonObjectElementContext context(reader);

    switch (reader.GetType())
    {
    case TLV::kTLVType_UnsignedInteger: {
        uint64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        if (CanCastTo<uint32_t>(v))
        {
            InsertJsonElement(jsonObj, context, v);
        }
        else
        {
            InsertJsonElement(jsonObj, context, std::to_string(v));
        }
        break;
    }

    case TLV::kTLVType_SignedInteger: {
        int64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        if (CanCastTo<int32_t>(v))
        {
            InsertJsonElement(jsonObj, context, v);
        }
        else
        {
            InsertJsonElement(jsonObj, context, std::to_string(v));
        }
        break;
    }

    case TLV::kTLVType_Boolean: {
        bool v;
        ReturnErrorOnFailure(reader.Get(v));
        InsertJsonElement(jsonObj, context, v);
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        double v;
        ReturnErrorOnFailure(reader.Get(v));
        if (v == std::numeric_limits<double>::infinity())
        {
            InsertJsonElement(jsonObj, context, kFloatingPointPositiveInfinity);
        }
        else if (v == -std::numeric_limits<double>::infinity())
        {
            InsertJsonElement(jsonObj, context, kFloatingPointNegativeInfinity);
        }
        else
        {
            InsertJsonElement(jsonObj, context, v);
        }
        break;
    }

    case TLV::kTLVType_ByteString: {
        ByteSpan span;
        ReturnErrorOnFailure(reader.Get(span));

        Platform::ScopedMemoryBuffer<char> byteString;
        byteString.Alloc(BASE64_ENCODED_LEN(span.size()) + 1);
        VerifyOrReturnError(byteString.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

        auto encodedLen              = Base64Encode(span.data(), static_cast<uint16_t>(span.size()), byteString.Get());
        byteString.Get()[encodedLen] = '\0';

        InsertJsonElement(jsonObj, context, byteString.Get());
        break;
    }

    case TLV::kTLVType_UTF8String: {
        CharSpan span;
        ReturnErrorOnFailure(reader.Get(span));

        std::string str(span.data(), span.size());
        InsertJsonElement(jsonObj, context, str);
        break;
    }

    case TLV::kTLVType_Null: {
        InsertJsonElement(jsonObj, context, Json::Value());
        break;
    }

    case TLV::kTLVType_Structure: {
        Json::Value jsonStruct(Json::objectValue);
        ReturnErrorOnFailure(TlvStructToJson(reader, jsonStruct));
        InsertJsonElement(jsonObj, context, jsonStruct);
        break;
    }

    case TLV::kTLVType_Array: {
        CHIP_ERROR err;
        Json::Value jsonArray(Json::arrayValue);
        ElementTypeContext prevSubType;
        ElementTypeContext nextSubType;
        TLV::TLVType containerType;

        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(reader.GetTag() == TLV::AnonymousTag(), CHIP_ERROR_INVALID_TLV_TAG);
            VerifyOrReturnError(reader.GetType() != TLV::kTLVType_Array, CHIP_ERROR_INVALID_TLV_ELEMENT);

            nextSubType.tlvType = reader.GetType();
            if (nextSubType.tlvType == TLV::kTLVType_FloatingPointNumber)
            {
                nextSubType.isDouble = reader.IsElementDouble();
            }

            if (jsonArray.empty())
            {
                prevSubType = nextSubType;
            }
            else
            {
                VerifyOrReturnError(prevSubType.tlvType == nextSubType.tlvType && prevSubType.isDouble == nextSubType.isDouble,
                                    CHIP_ERROR_INVALID_TLV_ELEMENT);
            }

            // Recursively convert to JSON the encompassing item within the array.
            ReturnErrorOnFailure(TlvToJson(reader, jsonArray));
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(containerType));

        context.subType = prevSubType;
        InsertJsonElement(jsonObj, context, jsonArray);
        break;
    }

    default:
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR TlvToJson(const ByteSpan & tlv, std::string & jsonString)
{
    TLV::TLVReader reader;
    reader.Init(tlv);
    reader.ImplicitProfileId = kTemporaryImplicitProfileId;

    ReturnErrorOnFailure(reader.Next());
    return TlvToJson(reader, jsonString);
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, std::string & jsonString)
{
    // The top level element must be a TLV Structure of Anonymous type.
    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == TLV::AnonymousTag(), CHIP_ERROR_INVALID_TLV_TAG);

    // During json conversion, a implicit profile ID is required
    ImplicitProfileIdChange implicitProfileIdChange(reader, kTemporaryImplicitProfileId);

    Json::Value jsonObject(Json::objectValue);
    ReturnErrorOnFailure(TlvStructToJson(reader, jsonObject));

    Json::StyledWriter writer;
    jsonString = writer.write(jsonObject);
    return CHIP_NO_ERROR;
}
} // namespace chip
