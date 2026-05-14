/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <data-model-providers/codegen/EmberAttributeDataBuffer.h>
#include <data-model-providers/codegen/EmberAttributeDecoder.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/AttributeReportBuilder.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/util/attribute-metadata.h>
#include <app/util/ember-io-storage.h>

namespace chip {
namespace app {

CHIP_ERROR DecodeAttributeToEmberBuffer(const AttributeDecoderParams & params, MutableByteSpan & outBuffer)
{
    // 1. Setup TLV Writer and Builder using outBuffer as scratch space
    TLV::TLVWriter writer;
    writer.Init(outBuffer.data(), outBuffer.size());

    TLV::TLVType outerStructureType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerStructureType));

    app::AttributeReportIBs::Builder builder;
    ReturnErrorOnFailure(builder.Init(&writer, to_underlying(app::ReportDataMessage::Tag::kAttributeReportIBs)));

    // 2. Create AttributeValueEncoder
    Access::SubjectDescriptor subjectDescriptor; // Default subject descriptor for internal use

    app::AttributeValueEncoder encoder(builder, subjectDescriptor, params.path, 0 /* data version */);

    // 3. Read from cluster
    app::DataModel::ReadAttributeRequest request(params.path, subjectDescriptor);
    auto status = params.cluster.ReadAttribute(request, encoder);
    VerifyOrReturnError(status.IsSuccess(), status.GetUnderlyingError());

    // 4. Finalize encoding
    builder.EndOfContainer();
    ReturnErrorOnFailure(writer.EndContainer(outerStructureType));
    ReturnErrorOnFailure(writer.Finalize());

    // 5. Extract data from TLV
    TLV::TLVReader reader;
    reader.Init(outBuffer.data(), writer.GetLengthWritten());

    // Enter structure
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    TLV::TLVType outer1;
    ReturnErrorOnFailure(reader.EnterContainer(outer1));

    // Enter Array
    ReturnErrorOnFailure(
        reader.Next(TLV::kTLVType_Array, TLV::ContextTag(to_underlying(app::ReportDataMessage::Tag::kAttributeReportIBs))));
    TLV::TLVType outer2;
    ReturnErrorOnFailure(reader.EnterContainer(outer2));

    // Next item should be AttributeReportIB
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

    app::AttributeReportIB::Parser attributeReportParser;
    ReturnErrorOnFailure(attributeReportParser.Init(reader));

    app::AttributeDataIB::Parser dataParser;
    ReturnErrorOnFailure(attributeReportParser.GetAttributeData(&dataParser));

    TLV::TLVReader attributeDataReader;
    ReturnErrorOnFailure(dataParser.GetData(&attributeDataReader));

    // 6. Use EmberAttributeDataBuffer to decode
    EmberAfAttributeMetadata dummyMeta = {
        .defaultValue  = EmberAfDefaultOrMinMaxAttributeValue(static_cast<uint8_t *>(nullptr)),
        .attributeId   = params.path.mAttributeId,
        .size          = params.emberSize,
        .attributeType = params.emberType,
        .mask          = static_cast<EmberAfAttributeMask>(params.isNullable ? MATTER_ATTRIBUTE_FLAG_NULLABLE : 0),
    };

    // We use outBuffer again as the target for decoding.
    // For Boolean, it is safe because the value is read before any large write happens (only 1 byte write).
    MutableByteSpan targetBuffer(outBuffer.data(), outBuffer.size());
    Ember::EmberAttributeDataBuffer emberBuffer(&dummyMeta, targetBuffer);

    EmberAfAttributeType baseType = Compatibility::Internal::AttributeBaseType(params.emberType);
    switch (baseType)
    {
    case ZCL_BOOLEAN_ATTRIBUTE_TYPE:
    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_INT16U_ATTRIBUTE_TYPE:
    case ZCL_INT24U_ATTRIBUTE_TYPE:
    case ZCL_INT32U_ATTRIBUTE_TYPE:
    case ZCL_INT40U_ATTRIBUTE_TYPE:
    case ZCL_INT48U_ATTRIBUTE_TYPE:
    case ZCL_INT56U_ATTRIBUTE_TYPE:
    case ZCL_INT64U_ATTRIBUTE_TYPE:
    case ZCL_INT8S_ATTRIBUTE_TYPE:
    case ZCL_INT16S_ATTRIBUTE_TYPE:
    case ZCL_INT24S_ATTRIBUTE_TYPE:
    case ZCL_INT32S_ATTRIBUTE_TYPE:
    case ZCL_INT40S_ATTRIBUTE_TYPE:
    case ZCL_INT48S_ATTRIBUTE_TYPE:
    case ZCL_INT56S_ATTRIBUTE_TYPE:
    case ZCL_INT64S_ATTRIBUTE_TYPE:
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: {
        CHIP_ERROR err = emberBuffer.Decode(attributeDataReader);
        if (err == CHIP_NO_ERROR)
        {
            // Update outBuffer size to reflect the actual data written
            //
            // targetBuffer size should have been updated already, since
            // Decode is guaranteed to do that.
            outBuffer.reduce_size(targetBuffer.size());
        }
        return err;
    }
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE: {
        // We use outBuffer for both intermediate TLV encoding and final Ember decoding.
        // For strings, the TLV data (`tlvData`) points inside `outBuffer` but further ahead
        // due to TLV framing overhead.
        // When we decode, we write the Ember length prefix at the beginning of `outBuffer`,
        // and then copy the string data. If the string is long, the destination (beginning of buffer)
        // and source (`tlvData`) will overlap.
        // We use `memmove` to handle this overlapping copy safely.
        bool isOctet =
            (params.emberType == ZCL_OCTET_STRING_ATTRIBUTE_TYPE || params.emberType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE);
        TLV::TLVType expectedType = isOctet ? TLV::kTLVType_ByteString : TLV::kTLVType_UTF8String;

        bool isLong =
            (params.emberType == ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE || params.emberType == ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE);
        uint32_t headerSize = isLong ? 2 : 1;

        if (attributeDataReader.GetType() == TLV::kTLVType_Null)
        {
            VerifyOrReturnError(params.isNullable, CHIP_ERROR_WRONG_TLV_TYPE);
            VerifyOrReturnError(outBuffer.size() >= headerSize, CHIP_ERROR_NO_MEMORY);

            // Write null length (0xFF or 0xFFFF)
            if (isLong)
            {
                outBuffer.data()[0] = 0xFF;
                outBuffer.data()[1] = 0xFF;
            }
            else
            {
                outBuffer.data()[0] = 0xFF;
            }

            outBuffer.reduce_size(headerSize);
            return CHIP_NO_ERROR;
        }

        VerifyOrReturnError(attributeDataReader.GetType() == expectedType, CHIP_ERROR_WRONG_TLV_TYPE);

        uint32_t stringLength = attributeDataReader.GetLength();
        const uint8_t * tlvData;
        ReturnErrorOnFailure(attributeDataReader.GetDataPtr(tlvData));

        uint32_t maxLength = isLong ? 65534 : 254;
        VerifyOrReturnError(stringLength <= maxLength, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(outBuffer.size() >= headerSize + stringLength, CHIP_ERROR_NO_MEMORY);

        // Write length (Pascal format)
        if (isLong)
        {
            outBuffer.data()[0] = static_cast<uint8_t>(stringLength & 0xFF);
            outBuffer.data()[1] = static_cast<uint8_t>((stringLength >> 8) & 0xFF);
        }
        else
        {
            outBuffer.data()[0] = static_cast<uint8_t>(stringLength);
        }

        // Ensure that writing the header doesn't overwrite the data we are about to move.
        // Since tlvData points into outBuffer, we verify the offset.
        VerifyOrReturnError(tlvData >= outBuffer.data() + headerSize, CHIP_ERROR_INTERNAL);

        // Move data safely even if overlapping
        //
        // NOTE: this is CRITICAL: outBuffer contains the TLV *and* is the
        //       output of the data. We use memmove and never touch the
        //       TLV again, so this should work ok.
        memmove(outBuffer.data() + headerSize, tlvData, stringLength);

        outBuffer.reduce_size(headerSize + stringLength);
        return CHIP_NO_ERROR;
    }
    default:
        // Fallthrough for unsupported types like Structs/Lists/NoData/Invalid.
        // Callers should not call this for such types.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
}

} // namespace app
} // namespace chip
