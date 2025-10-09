/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
#include <lib/core/DataModelTypes.h>
#include <lib/support/Base64.h>
#include <lib/support/jsontlv/TlvJson.h>

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
    enum KeyType
    {
        kRoot,
        kStructField,
        kArrayItem
    };

    KeyContext() = default;

    KeyContext(chip::FieldId fieldId)
    {
        keyType = kStructField;
        key     = fieldId;
    }

    KeyContext(chip::ListIndex listIndex)
    {
        keyType = kArrayItem;
        key     = listIndex;
    }

    KeyType keyType  = kRoot;
    unsigned int key = 0;
};
} // namespace

//
// For now, let's put a bound of the maximum length of a byte/char string to be the size of an IPv6
// MTU. While this is smaller than that of the limit defined in the data model specification,
// strings by virtue of not being chunked are intrinsically limited in size to the size of the encompassing packet.
//
static constexpr uint16_t kMaxStringLen = 1280;

constexpr char kBase64Header[]    = "base64:";
constexpr size_t kBase64HeaderLen = MATTER_ARRAY_SIZE(kBase64Header) - 1;

namespace chip {

/*
 * This templated function inserts a key/value pair into the Json value object.
 * The value is templated to be of type T and accepts any of the following primitive
 * types:
 *      bool, uint*_t, int*_t, char *, float, double.
 *
 * This method uses the provided key context to deduce the type of element being added.
 *
 */
template <typename T>
void InsertKeyValue(Json::Value & json, const KeyContext & keyContext, T val)
{
    //
    // This needs to accomodate either the string 'value', or a 32-bit integer.
    // The size of the largest 32-bit integer key represented as a string is 11 characters long.
    // Tack on 1 byte for the null character.
    //
    char keyBuf[12];

    if (keyContext.keyType == KeyContext::kRoot)
    {
        Platform::CopyString(keyBuf, sizeof(keyBuf), "value");
        json[keyBuf] = val;
    }
    else if (keyContext.keyType == KeyContext::kStructField)
    {
        snprintf(keyBuf, sizeof(keyBuf), "%u", keyContext.key);
        json[keyBuf] = val;
    }
    else
    {
        json[keyContext.key] = val;
    }
}

std::string JsonToString(Json::Value & json)
{
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    return writer.write(json);
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, KeyContext context, Json::Value & parent)
{
    switch (reader.GetType())
    {
    case TLV::kTLVType_UnsignedInteger: {
        uint64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        InsertKeyValue(parent, context, v);
        break;
    }

    case TLV::kTLVType_SignedInteger: {
        int64_t v;
        ReturnErrorOnFailure(reader.Get(v));
        InsertKeyValue(parent, context, v);
        break;
    }

    case TLV::kTLVType_Boolean: {
        bool v;
        ReturnErrorOnFailure(reader.Get(v));
        InsertKeyValue(parent, context, v);
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        double v;
        ReturnErrorOnFailure(reader.Get(v));
        InsertKeyValue(parent, context, v);
        break;
    }

    case TLV::kTLVType_ByteString: {
        ByteSpan span;

        ReturnErrorOnFailure(reader.Get(span));
        VerifyOrReturnError(span.size() < kMaxStringLen, CHIP_ERROR_INVALID_TLV_ELEMENT);

        Platform::ScopedMemoryBuffer<char> byteString;
        byteString.Alloc(kBase64HeaderLen + BASE64_ENCODED_LEN(span.size()) + 1);
        VerifyOrReturnError(byteString.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

        auto encodedLen = Base64Encode(span.data(), static_cast<uint16_t>(span.size()), byteString.Get() + kBase64HeaderLen);
        if (encodedLen)
        {
            memcpy(byteString.Get(), kBase64Header, kBase64HeaderLen);
            encodedLen = static_cast<uint16_t>(encodedLen + kBase64HeaderLen);
        }
        byteString.Get()[encodedLen] = '\0';

        InsertKeyValue(parent, context, byteString.Get());
        break;
    }

    case TLV::kTLVType_UTF8String: {
        CharSpan span;

        ReturnErrorOnFailure(reader.Get(span));
        VerifyOrReturnError(span.size() < kMaxStringLen, CHIP_ERROR_INVALID_TLV_ELEMENT);

        Platform::ScopedMemoryString charString(span.data(), span.size());
        InsertKeyValue(parent, context, charString.Get());
        break;
    }

    case TLV::kTLVType_Null: {
        InsertKeyValue(parent, context, Json::Value());
        break;
    }

    case TLV::kTLVType_Structure: {
        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        CHIP_ERROR err;
        Json::Value value;

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
            KeyContext context2(static_cast<chip::FieldId>(TLV::TagNumFromTag(reader.GetTag())));

            //
            // Recursively convert to JSON the encompassing item within the struct.
            //
            ReturnErrorOnFailure(TlvToJson(reader, context2, value));
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        InsertKeyValue(parent, context, value);
        break;
    }

    case TLV::kTLVType_Array: {
        TLV::TLVType containerType;
        ReturnErrorOnFailure(reader.EnterContainer(containerType));

        CHIP_ERROR err;
        Json::Value value = Json::Value(Json::arrayValue);
        size_t listIndex  = 0;

        while ((err = reader.Next()) == CHIP_NO_ERROR)
        {
            KeyContext context2(static_cast<chip::ListIndex>(listIndex++));

            //
            // Recursively convert to JSON the encompassing item within the array.
            //
            ReturnErrorOnFailure(TlvToJson(reader, context2, value));
        }

        VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
        ReturnErrorOnFailure(reader.ExitContainer(containerType));
        InsertKeyValue(parent, context, value);
        break;
    }

    default:
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TlvToJson(TLV::TLVReader & reader, Json::Value & root)
{
    KeyContext context;
    return TlvToJson(reader, context, root);
}

} // namespace chip
