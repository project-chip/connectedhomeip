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

/*
 * Encapsulates the different types of keys permissible.
 *
 * Root Key = Key with a name of 'value'. This is the top-most key in a given JSON object generated from TLV.
 * Struct Field = Key containing the 32-bit field ID of an item in a struct.
 * Array Item = Key containing the 16-bit list index of an item in a list.
 *
 * In the latter two modes, the actual field ID/list index is encapsulated within the 'key' member.
 *
 */
struct KeyContext
{
    enum ContainerType
    {
        kStruct,
        kArray
    };

    KeyContext() = default;

    KeyContext(chip::FieldId fieldId)
    {
        containerType = kStruct;
        tag           = fieldId;
    }

    KeyContext(chip::ListIndex listIndex)
    {
        containerType = kArray;
        tag           = listIndex;
    }

    void SetElementType(const char * elemType) { elementType = elemType; }

    void SetArrayElementType(const char * subType) { subElementType = subType; }

    void GenerateJsonKeyStr(char * buf) const
    {
        int len = 0;
        len     = sprintf(buf, "%u:", tag);
        len += sprintf(&buf[len], "%s", elementType);
        if (strcmp(elementType, kElementTypeArray) == 0)
        {
            sprintf(&buf[len], "-%s", subElementType);
        }
    }

    ContainerType containerType = kStruct;
    unsigned int tag            = 0;
    const char * elementType    = nullptr;
    const char * subElementType = nullptr;
};

/*
 * For now, let's put a bound of the maximum length of a byte/char string to be the size of an IPv6
 * MTU. While this is smaller than that of the limit defined in the data model specification,
 * strings by virtue of not being chunked are intrinsically limited in size to the size of the encompassing packet.
 */
static constexpr uint16_t kMaxStringLen = 1280;

/*
 * This templated function inserts a key/value pair into the Json value object.
 * The value is templated to be of type T and accepts any of the following primitive
 * types:
 *      bool, uint*_t, int*_t, char *, float, double, std::string, Json::Value
 *
 * This method uses the provided key context to deduce the type of element being added.
 *
 */
template <typename T>
void InsertKeyValue(Json::Value & json, const KeyContext & keyContext, T val)
{
    if (json.isArray())
    {
        json.append(val);
    }
    else
    {
        // This needs to accommodate the largest key value 'FieldId:ElementType-SubElementType'.
        // The largest 32-bit integer FieldId represented as a string is 11 characters long.
        // The largest ElementType and SubElementType strings are 6 characters long each.
        // Tack on 3 bytes for the ':' character, '-' character, and null terminator.
        char keyBuf[26];
        keyContext.GenerateJsonKeyStr(keyBuf);
        json[keyBuf] = val;
    }
}

static CHIP_ERROR TlvToJson(TLV::TLVReader & reader, KeyContext context, Json::Value & jsonObj);

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
        VerifyOrReturnError(TLV::IsContextTag(tag) ||
                                (TLV::IsProfileTag(tag) && (TLV::ProfileIdFromTag(tag) == TLV::kCommonProfileId)),
                            CHIP_ERROR_INVALID_TLV_TAG);
        KeyContext context(static_cast<chip::FieldId>(TLV::TagNumFromTag(tag)));

        // Recursively convert to JSON the item within the struct.
        ReturnErrorOnFailure(TlvToJson(reader, context, jsonObj));
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    return reader.ExitContainer(containerType);
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, KeyContext context, Json::Value & jsonObj)
{
    switch (reader.GetType())
    {
    case TLV::kTLVType_UnsignedInteger: {
        uint64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        context.SetElementType(kElementTypeUInt);
        if (CanCastTo<uint32_t>(v))
        {
            InsertKeyValue(jsonObj, context, v);
        }
        else
        {
            InsertKeyValue(jsonObj, context, std::to_string(v));
        }
        break;
    }

    case TLV::kTLVType_SignedInteger: {
        int64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        context.SetElementType(kElementTypeInt);
        if (CanCastTo<int32_t>(v))
        {
            InsertKeyValue(jsonObj, context, v);
        }
        else
        {
            InsertKeyValue(jsonObj, context, std::to_string(v));
        }
        break;
    }

    case TLV::kTLVType_Boolean: {
        bool v;
        ReturnErrorOnFailure(reader.Get(v));
        context.SetElementType(kElementTypeBool);
        InsertKeyValue(jsonObj, context, v);
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        double v;
        ReturnErrorOnFailure(reader.Get(v));
        VerifyOrReturnError(v != std::numeric_limits<double>::infinity() && v != -std::numeric_limits<double>::infinity(),
                            CHIP_ERROR_INVALID_TLV_ELEMENT);
        if (reader.IsElementDouble())
        {
            context.SetElementType(kElementTypeDouble);
        }
        else
        {
            context.SetElementType(kElementTypeFloat);
        }
        InsertKeyValue(jsonObj, context, v);
        break;
    }

    case TLV::kTLVType_ByteString: {
        ByteSpan span;
        ReturnErrorOnFailure(reader.Get(span));
        VerifyOrReturnError(span.size() < kMaxStringLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
        context.SetElementType(kElementTypeBytes);

        Platform::ScopedMemoryBuffer<char> byteString;
        byteString.Alloc(BASE64_ENCODED_LEN(span.size()) + 1);
        VerifyOrReturnError(byteString.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

        auto encodedLen              = Base64Encode(span.data(), static_cast<uint16_t>(span.size()), byteString.Get());
        byteString.Get()[encodedLen] = '\0';

        InsertKeyValue(jsonObj, context, byteString.Get());
        break;
    }

    case TLV::kTLVType_UTF8String: {
        CharSpan span;
        ReturnErrorOnFailure(reader.Get(span));
        VerifyOrReturnError(span.size() < kMaxStringLen, CHIP_ERROR_INVALID_TLV_ELEMENT);
        context.SetElementType(kElementTypeString);

        std::string str(span.data(), span.size());
        InsertKeyValue(jsonObj, context, str);
        break;
    }

    case TLV::kTLVType_Null: {
        context.SetElementType(kElementTypeNull);
        InsertKeyValue(jsonObj, context, Json::Value());
        break;
    }

    case TLV::kTLVType_Structure: {
        Json::Value jsonStruct(Json::objectValue);
        ReturnErrorOnFailure(TlvStructToJson(reader, jsonStruct));
        context.SetElementType(kElementTypeStruct);
        InsertKeyValue(jsonObj, context, jsonStruct);
        break;
    }

    case TLV::kTLVType_Array: {
        CHIP_ERROR err;
        Json::Value jsonArray(Json::arrayValue);
        const char * subType     = kElementTypeEmpty;
        const char * nextSubType = kElementTypeEmpty;
        size_t listIndex         = 0;
        TLV::TLVType containerType;

        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            switch (reader.GetType())
            {
            case TLV::kTLVType_UnsignedInteger: {
                nextSubType = kElementTypeUInt;
                break;
            }
            case TLV::kTLVType_SignedInteger: {
                nextSubType = kElementTypeInt;
                break;
            }
            case TLV::kTLVType_Boolean: {
                nextSubType = kElementTypeBool;
                break;
            }
            case TLV::kTLVType_FloatingPointNumber: {
                if (reader.IsElementDouble())
                {
                    nextSubType = kElementTypeDouble;
                }
                else
                {
                    nextSubType = kElementTypeFloat;
                }
                break;
            }
            case TLV::kTLVType_ByteString: {
                nextSubType = kElementTypeBytes;
                break;
            }
            case TLV::kTLVType_UTF8String: {
                nextSubType = kElementTypeString;
                break;
            }
            case TLV::kTLVType_Null: {
                nextSubType = kElementTypeNull;
                break;
            }
            case TLV::kTLVType_Structure: {
                nextSubType = kElementTypeStruct;
                break;
            }
            case TLV::kTLVType_Array:
            default:
                return CHIP_ERROR_INVALID_TLV_ELEMENT;
                break;
            }

            if (listIndex == 0)
            {
                subType = nextSubType;
            }
            else
            {
                VerifyOrReturnError(strcmp(subType, nextSubType) == 0, CHIP_ERROR_INVALID_TLV_ELEMENT);
            }

            KeyContext context2(static_cast<chip::ListIndex>(listIndex++));

            // Recursively convert to JSON the encompassing item within the array.
            ReturnErrorOnFailure(TlvToJson(reader, context2, jsonArray));
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        context.SetElementType(kElementTypeArray);
        context.SetArrayElementType(subType);
        InsertKeyValue(jsonObj, context, jsonArray);
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
    ReturnErrorOnFailure(reader.Next());
    return TlvToJson(reader, jsonString);
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, std::string & jsonString)
{
    // The top level element must be a TLV Structure of Anonymous type.
    VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == TLV::AnonymousTag(), CHIP_ERROR_INVALID_TLV_TAG);

    Json::Value jsonObject(Json::objectValue);
    ReturnErrorOnFailure(TlvStructToJson(reader, jsonObject));

    jsonString = jsonObject.toStyledString();
    return CHIP_NO_ERROR;
}

std::string StylizeJsonString(const std::string & jsonString)
{
    Json::Reader reader;
    Json::Value jsonObject;
    reader.parse(jsonString, jsonObject);
    return jsonObject.toStyledString();
}

} // namespace chip
