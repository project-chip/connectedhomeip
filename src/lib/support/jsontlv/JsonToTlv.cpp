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

#include <algorithm>
#include <json/json.h>
#include <lib/support/Base64.h>
#include <lib/support/SafeInt.h>
#include <lib/support/jsontlv/ElementTypes.h>
#include <lib/support/jsontlv/JsonToTlv.h>

namespace chip {

namespace {

// Not directly used: TLV encoding will not encode this number and
// will just encode "Implicit profile tag"
// This profile, but will be used for deciding what binary values to encode.
constexpr uint32_t kTemporaryImplicitProfileId = 0xFF01;

std::vector<std::string> SplitIntoFieldsBySeparator(const std::string & input, char separator)
{
    std::vector<std::string> substrings;
    std::stringstream ss(input);
    std::string substring;

    while (std::getline(ss, substring, separator))
    {
        substrings.push_back(std::move(substring));
    }

    return substrings;
}

CHIP_ERROR JsonTypeStrToTlvType(const char * elementType, ElementTypeContext & type)
{
    if (strcmp(elementType, kElementTypeInt) == 0)
    {
        type.tlvType = TLV::kTLVType_SignedInteger;
    }
    else if (strcmp(elementType, kElementTypeUInt) == 0)
    {
        type.tlvType = TLV::kTLVType_UnsignedInteger;
    }
    else if (strcmp(elementType, kElementTypeBool) == 0)
    {
        type.tlvType = TLV::kTLVType_Boolean;
    }
    else if (strcmp(elementType, kElementTypeFloat) == 0)
    {
        type.tlvType  = TLV::kTLVType_FloatingPointNumber;
        type.isDouble = false;
    }
    else if (strcmp(elementType, kElementTypeDouble) == 0)
    {
        type.tlvType  = TLV::kTLVType_FloatingPointNumber;
        type.isDouble = true;
    }
    else if (strcmp(elementType, kElementTypeBytes) == 0)
    {
        type.tlvType = TLV::kTLVType_ByteString;
    }
    else if (strcmp(elementType, kElementTypeString) == 0)
    {
        type.tlvType = TLV::kTLVType_UTF8String;
    }
    else if (strcmp(elementType, kElementTypeNull) == 0)
    {
        type.tlvType = TLV::kTLVType_Null;
    }
    else if (strcmp(elementType, kElementTypeStruct) == 0)
    {
        type.tlvType = TLV::kTLVType_Structure;
    }
    else if (strncmp(elementType, kElementTypeArray, strlen(kElementTypeArray)) == 0)
    {
        type.tlvType = TLV::kTLVType_Array;
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

bool IsUnsignedInteger(const std::string & s)
{
    size_t len = s.length();
    if (len == 0)
    {
        return false;
    }
    for (size_t i = 0; i < len; i++)
    {
        if (!isdigit(s[i]))
        {
            return false;
        }
    }
    return true;
}

bool IsSignedInteger(const std::string & s)
{
    if (s.length() == 0)
    {
        return false;
    }
    if (s[0] == '-')
    {
        return IsUnsignedInteger(s.substr(1));
    }
    return IsUnsignedInteger(s);
}

bool IsValidBase64String(const std::string & s)
{
    const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t len                    = s.length();

    // Check if the length is a multiple of 4
    if (len % 4 != 0)
    {
        return false;
    }

    size_t paddingLen = 0;
    if (s[len - 1] == '=')
    {
        paddingLen++;
        if (s[len - 2] == '=')
        {
            paddingLen++;
        }
    }

    // Check for invalid characters
    for (char c : s.substr(0, len - paddingLen))
    {
        if (base64Chars.find(c) == std::string::npos)
        {
            return false;
        }
    }

    return true;
}

struct ElementContext
{
    std::string jsonName;
    TLV::Tag tag = TLV::AnonymousTag();
    ElementTypeContext type;
    ElementTypeContext subType;
};

bool CompareByTag(const ElementContext & a, const ElementContext & b)
{
    // If tags are of the same type compare by tag number
    if (IsContextTag(a.tag) == IsContextTag(b.tag))
    {
        return TLV::TagNumFromTag(a.tag) < TLV::TagNumFromTag(b.tag);
    }
    // Otherwise, compare by tag type: context tags first followed by common profile tags
    return IsContextTag(a.tag);
}

CHIP_ERROR InternalConvertTlvTag(const uint64_t tagNumber, TLV::Tag & tag, const uint32_t profileId = kTemporaryImplicitProfileId)
{
    if (tagNumber <= UINT8_MAX)
    {
        tag = TLV::ContextTag(static_cast<uint8_t>(tagNumber));
    }
    else if (tagNumber <= UINT32_MAX)
    {
        tag = TLV::ProfileTag(profileId, static_cast<uint32_t>(tagNumber));
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseJsonName(const std::string name, ElementContext & elementCtx, uint32_t implicitProfileId)
{
    uint64_t tagNumber                  = 0;
    const char * elementType            = nullptr;
    std::vector<std::string> nameFields = SplitIntoFieldsBySeparator(name, ':');
    TLV::Tag tag                        = TLV::AnonymousTag();
    ElementTypeContext type;
    ElementTypeContext subType;

    if (nameFields.size() == 2)
    {
        VerifyOrReturnError(IsUnsignedInteger(nameFields[0]), CHIP_ERROR_INVALID_ARGUMENT);
        tagNumber   = std::strtoull(nameFields[0].c_str(), nullptr, 10);
        elementType = nameFields[1].c_str();
    }
    else if (nameFields.size() == 3)
    {
        VerifyOrReturnError(IsUnsignedInteger(nameFields[1]), CHIP_ERROR_INVALID_ARGUMENT);
        tagNumber   = std::strtoull(nameFields[1].c_str(), nullptr, 10);
        elementType = nameFields[2].c_str();
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(InternalConvertTlvTag(tagNumber, tag, implicitProfileId));
    ReturnErrorOnFailure(JsonTypeStrToTlvType(elementType, type));

    if (type.tlvType == TLV::kTLVType_Array)
    {
        std::vector<std::string> arrayFields = SplitIntoFieldsBySeparator(elementType, '-');
        VerifyOrReturnError(arrayFields.size() == 2, CHIP_ERROR_INVALID_ARGUMENT);

        if (strcmp(arrayFields[1].c_str(), kElementTypeEmpty) == 0)
        {
            subType.tlvType = TLV::kTLVType_NotSpecified;
        }
        else
        {
            ReturnErrorOnFailure(JsonTypeStrToTlvType(arrayFields[1].c_str(), subType));
        }
    }

    elementCtx.jsonName = name;
    elementCtx.tag      = tag;
    elementCtx.type     = type;
    elementCtx.subType  = subType;

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeTlvElement(const Json::Value & val, TLV::TLVWriter & writer, const ElementContext & elementCtx)
{
    TLV::Tag tag = elementCtx.tag;

    switch (elementCtx.type.tlvType)
    {
    case TLV::kTLVType_UnsignedInteger: {
        uint64_t v;
        if (val.isUInt64())
        {
            v = val.asUInt64();
        }
        else if (val.isString())
        {
            const std::string valAsString = val.asString();
            VerifyOrReturnError(IsUnsignedInteger(valAsString), CHIP_ERROR_INVALID_ARGUMENT);
            v = std::strtoull(valAsString.c_str(), nullptr, 10);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(writer.Put(tag, v));
        break;
    }

    case TLV::kTLVType_SignedInteger: {
        int64_t v;
        if (val.isInt64())
        {
            v = val.asInt64();
        }
        else if (val.isString())
        {
            const std::string valAsString = val.asString();
            VerifyOrReturnError(IsSignedInteger(valAsString), CHIP_ERROR_INVALID_ARGUMENT);
            v = std::strtoll(valAsString.c_str(), nullptr, 10);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(writer.Put(tag, v));
        break;
    }

    case TLV::kTLVType_Boolean: {
        VerifyOrReturnError(val.isBool(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.Put(tag, val.asBool()));
        break;
    }

    case TLV::kTLVType_FloatingPointNumber: {
        if (val.isNumeric())
        {
            if (elementCtx.type.isDouble)
            {
                ReturnErrorOnFailure(writer.Put(tag, val.asDouble()));
            }
            else
            {
                ReturnErrorOnFailure(writer.Put(tag, val.asFloat()));
            }
        }
        else if (val.isString())
        {
            const std::string valAsString = val.asString();
            bool isPositiveInfinity       = (valAsString == kFloatingPointPositiveInfinity);
            bool isNegativeInfinity       = (valAsString == kFloatingPointNegativeInfinity);
            VerifyOrReturnError(isPositiveInfinity || isNegativeInfinity, CHIP_ERROR_INVALID_ARGUMENT);
            if (elementCtx.type.isDouble)
            {
                if (isPositiveInfinity)
                {
                    ReturnErrorOnFailure(writer.Put(tag, std::numeric_limits<double>::infinity()));
                }
                else
                {
                    ReturnErrorOnFailure(writer.Put(tag, -std::numeric_limits<double>::infinity()));
                }
            }
            else
            {
                if (isPositiveInfinity)
                {
                    ReturnErrorOnFailure(writer.Put(tag, std::numeric_limits<float>::infinity()));
                }
                else
                {
                    ReturnErrorOnFailure(writer.Put(tag, -std::numeric_limits<float>::infinity()));
                }
            }
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        break;
    }

    case TLV::kTLVType_ByteString: {
        VerifyOrReturnError(val.isString(), CHIP_ERROR_INVALID_ARGUMENT);
        const std::string valAsString = val.asString();
        size_t encodedLen             = valAsString.length();
        VerifyOrReturnError(CanCastTo<uint16_t>(encodedLen), CHIP_ERROR_INVALID_ARGUMENT);

        VerifyOrReturnError(IsValidBase64String(valAsString), CHIP_ERROR_INVALID_ARGUMENT);

        Platform::ScopedMemoryBuffer<uint8_t> byteString;
        byteString.Alloc(BASE64_MAX_DECODED_LEN(static_cast<uint16_t>(encodedLen)));
        VerifyOrReturnError(byteString.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

        auto decodedLen = Base64Decode(valAsString.c_str(), static_cast<uint16_t>(encodedLen), byteString.Get());
        ReturnErrorOnFailure(writer.PutBytes(tag, byteString.Get(), decodedLen));
        break;
    }

    case TLV::kTLVType_UTF8String: {
        VerifyOrReturnError(val.isString(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.PutString(tag, val.asCString()));
        break;
    }

    case TLV::kTLVType_Null: {
        VerifyOrReturnError(val.isNull(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.PutNull(tag));
        break;
    }

    case TLV::kTLVType_Structure: {
        TLV::TLVType containerType;
        VerifyOrReturnError(val.isObject(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, containerType));

        std::vector<std::string> jsonNames = val.getMemberNames();
        std::vector<ElementContext> nestedElementsCtx;

        for (size_t i = 0; i < jsonNames.size(); i++)
        {
            ElementContext ctx;
            ReturnErrorOnFailure(ParseJsonName(jsonNames[i], ctx, writer.ImplicitProfileId));
            nestedElementsCtx.push_back(ctx);
        }

        // Sort Json object elements by Tag number (low to high).
        // Note that all sorted Context Tags will appear first followed by all sorted Common Tags.
        std::sort(nestedElementsCtx.begin(), nestedElementsCtx.end(), CompareByTag);

        for (auto & ctx : nestedElementsCtx)
        {
            ReturnErrorOnFailure(EncodeTlvElement(val[ctx.jsonName], writer, ctx));
        }

        ReturnErrorOnFailure(writer.EndContainer(containerType));
        break;
    }

    case TLV::kTLVType_Array: {
        TLV::TLVType containerType;
        VerifyOrReturnError(val.isArray(), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Array, containerType));

        if (elementCtx.subType.tlvType == TLV::kTLVType_NotSpecified)
        {
            VerifyOrReturnError(val.size() == 0, CHIP_ERROR_INVALID_ARGUMENT);
        }
        else
        {
            ElementContext nestedElementCtx;
            nestedElementCtx.tag  = TLV::AnonymousTag();
            nestedElementCtx.type = elementCtx.subType;
            for (Json::ArrayIndex i = 0; i < val.size(); i++)
            {
                ReturnErrorOnFailure(EncodeTlvElement(val[i], writer, nestedElementCtx));
            }
        }

        ReturnErrorOnFailure(writer.EndContainer(containerType));
        break;
    }

    default:
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
        break;
    }

    return CHIP_NO_ERROR;
}

} // namespace

CHIP_ERROR JsonToTlv(const std::string & jsonString, MutableByteSpan & tlv)
{
    TLV::TLVWriter writer;
    writer.Init(tlv);
    writer.ImplicitProfileId = kTemporaryImplicitProfileId;
    ReturnErrorOnFailure(JsonToTlv(jsonString, writer));
    ReturnErrorOnFailure(writer.Finalize());
    tlv.reduce_size(writer.GetLengthWritten());
    return CHIP_NO_ERROR;
}

CHIP_ERROR JsonToTlv(const std::string & jsonString, TLV::TLVWriter & writer)
{
    Json::Reader reader;
    Json::Value json;
    bool result = reader.parse(jsonString, json);
    VerifyOrReturnError(result, CHIP_ERROR_INTERNAL);

    ElementContext elementCtx;
    elementCtx.type = { TLV::kTLVType_Structure, false };
    return EncodeTlvElement(json, writer, elementCtx);
}

CHIP_ERROR ConvertTlvTag(const uint64_t tagNumber, TLV::Tag & tag)
{
    return InternalConvertTlvTag(tagNumber, tag);
}
} // namespace chip
