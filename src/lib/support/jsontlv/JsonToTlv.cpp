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

#include <stdint.h>

#include <algorithm>
#include <charconv>
#include <sstream>
#include <string>
#include <vector>

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

// The profileId parameter is used when encoding a tag for a TLV element to specify the profile that the tag belongs to.
// If the vendor ID is zero but the tag ID does not fit within an 8-bit value, the function uses Implicit Profile Tag.
// Here, the kTemporaryImplicitProfileId serves as a default value for cases where no explicit profile ID is provided by
// the caller. This allows for the encoding of tags that are not vendor-specific or context-specific but are instead
// associated with a temporary implicit profile ID (0xFF01).
CHIP_ERROR InternalConvertTlvTag(uint32_t tagNumber, TLV::Tag & tag, const uint32_t profileId = kTemporaryImplicitProfileId)
{
    uint16_t vendor_id = static_cast<uint16_t>(tagNumber >> 16);
    uint16_t tag_id    = static_cast<uint16_t>(tagNumber & 0xFFFF);

    if (vendor_id != 0)
    {
        tag = TLV::ProfileTag(vendor_id, /*profileNum=*/0, tag_id);
    }
    else if (tag_id <= UINT8_MAX)
    {
        tag = TLV::ContextTag(static_cast<uint8_t>(tagNumber));
    }
    else
    {
        tag = TLV::ProfileTag(profileId, tagNumber);
    }
    return CHIP_NO_ERROR;
}

template <typename T>
CHIP_ERROR ParseNumericalField(const std::string & decimalString, T & outValue)
{
    const char * start_ptr       = decimalString.data();
    const char * end_ptr         = decimalString.data() + decimalString.size();
    auto [last_converted_ptr, _] = std::from_chars(start_ptr, end_ptr, outValue, 10);
    VerifyOrReturnError(last_converted_ptr == end_ptr, CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseJsonName(const std::string & name, ElementContext & elementCtx, uint32_t implicitProfileId)
{
    uint32_t tagNumber                  = 0;
    const char * elementType            = nullptr;
    std::vector<std::string> nameFields = SplitIntoFieldsBySeparator(name, ':');
    TLV::Tag tag                        = TLV::AnonymousTag();
    ElementTypeContext type;
    ElementTypeContext subType;

    if (nameFields.size() == 2)
    {
        ReturnErrorOnFailure(ParseNumericalField(nameFields[0], tagNumber));
        elementType = nameFields[1].c_str();
    }
    else if (nameFields.size() == 3)
    {
        ReturnErrorOnFailure(ParseNumericalField(nameFields[1], tagNumber));
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
        uint64_t v = 0;
        if (val.isUInt64())
        {
            v = val.asUInt64();
        }
        else if (val.isString())
        {
            ReturnErrorOnFailure(ParseNumericalField(val.asString(), v));
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        ReturnErrorOnFailure(writer.Put(tag, v));
        break;
    }

    case TLV::kTLVType_SignedInteger: {
        int64_t v = 0;
        if (val.isInt64())
        {
            v = val.asInt64();
        }
        else if (val.isString())
        {
            ReturnErrorOnFailure(ParseNumericalField(val.asString(), v));
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

        // Check if the length is a multiple of 4 as strict padding is required.
        VerifyOrReturnError(encodedLen % 4 == 0, CHIP_ERROR_INVALID_ARGUMENT);

        Platform::ScopedMemoryBuffer<uint8_t> byteString;
        byteString.Alloc(BASE64_MAX_DECODED_LEN(static_cast<uint16_t>(encodedLen)));
        VerifyOrReturnError(byteString.Get() != nullptr, CHIP_ERROR_NO_MEMORY);

        auto decodedLen = Base64Decode(valAsString.c_str(), static_cast<uint16_t>(encodedLen), byteString.Get());
        VerifyOrReturnError(decodedLen < UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorOnFailure(writer.PutBytes(tag, byteString.Get(), decodedLen));
        break;
    }

    case TLV::kTLVType_UTF8String: {
        VerifyOrReturnError(val.isString(), CHIP_ERROR_INVALID_ARGUMENT);
        const std::string valAsString = val.asString();
        ReturnErrorOnFailure(writer.PutString(tag, valAsString.data(), static_cast<uint32_t>(valAsString.size())));
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

    // Use kTemporaryImplicitProfileId as the default value for cases where no explicit implicit profile ID is provided by
    // the caller. This allows for the encoding of tags that are not vendor-specific or context-specific but are instead
    // associated with a temporary implicit profile ID (0xFF01).
    if (writer.ImplicitProfileId == TLV::kProfileIdNotSpecified)
    {
        writer.ImplicitProfileId = kTemporaryImplicitProfileId;
    }

    return EncodeTlvElement(json, writer, elementCtx);
}

CHIP_ERROR ConvertTlvTag(uint32_t tagNumber, TLV::Tag & tag)
{
    return InternalConvertTlvTag(tagNumber, tag);
}
} // namespace chip
