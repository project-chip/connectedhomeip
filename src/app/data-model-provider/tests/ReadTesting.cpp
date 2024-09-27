/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/data-model-provider/tests/ReadTesting.h>

#include <app/MessageDef/ReportDataMessage.h>

namespace chip {
namespace app {
namespace Testing {
namespace {

CHIP_ERROR DecodeAttributeReportIBs(ByteSpan data, std::vector<DecodedAttributeData> & decoded_items)
{
    // Espected data format:
    //   CONTAINER (anonymous)
    //     0x01 => Array (i.e. report data ib)
    //       ReportIB*
    //
    // Generally this is VERY hard to process ...
    //
    TLV::TLVReader reportIBsReader;
    reportIBsReader.Init(data);

    ReturnErrorOnFailure(reportIBsReader.Next());
    if (reportIBsReader.GetType() != TLV::TLVType::kTLVType_Structure)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    TLV::TLVType outer1;
    reportIBsReader.EnterContainer(outer1);

    ReturnErrorOnFailure(reportIBsReader.Next());
    if (reportIBsReader.GetType() != TLV::TLVType::kTLVType_Array)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    TLV::TLVType outer2;
    reportIBsReader.EnterContainer(outer2);

    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == (err = reportIBsReader.Next()))
    {
        TLV::TLVReader attributeReportReader = reportIBsReader;
        AttributeReportIB::Parser attributeReportParser;
        ReturnErrorOnFailure(attributeReportParser.Init(attributeReportReader));

        AttributeDataIB::Parser dataParser;
        // NOTE: to also grab statuses, use GetAttributeStatus and check for CHIP_END_OF_TLV
        ReturnErrorOnFailure(attributeReportParser.GetAttributeData(&dataParser));

        DecodedAttributeData decoded;
        ReturnErrorOnFailure(decoded.DecodeFrom(dataParser));
        decoded_items.push_back(decoded);
    }

    if ((CHIP_END_OF_TLV != err) && (err != CHIP_NO_ERROR))
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(reportIBsReader.ExitContainer(outer2));
    ReturnErrorOnFailure(reportIBsReader.ExitContainer(outer1));

    err = reportIBsReader.Next();

    if (CHIP_ERROR_END_OF_TLV == err)
    {
        return CHIP_NO_ERROR;
    }
    if (CHIP_NO_ERROR == err)
    {
        // This is NOT ok ... we have multiple things in our buffer?
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

} // namespace

std::unique_ptr<AttributeValueEncoder> ReadOperation::StartEncoding(const EncodingParams & params)
{
    VerifyOrDie((mState == State::kEncoding) || (mState == State::kInitializing));
    mState = State::kEncoding;

    CHIP_ERROR err = mEncodedIBs.StartEncoding(mAttributeReportIBsBuilder);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Test, "FAILURE starting encoding %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    // mRequest.subjectDescriptor is known non-null because it is set in the constructor
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    return std::make_unique<AttributeValueEncoder>(mAttributeReportIBsBuilder, *mRequest.subjectDescriptor, mRequest.path,
                                                   params.GetDataVersion(), params.GetIsFabricFiltered(),
                                                   params.GetAttributeEncodeState());
}

CHIP_ERROR ReadOperation::FinishEncoding()
{
    VerifyOrDie(mState == State::kEncoding);
    mState = State::kFinished;
    return mEncodedIBs.FinishEncoding(mAttributeReportIBsBuilder);
}

CHIP_ERROR DecodedAttributeData::DecodeFrom(const AttributeDataIB::Parser & parser)
{
    ReturnErrorOnFailure(parser.GetDataVersion(&dataVersion));

    AttributePathIB::Parser pathParser;
    ReturnErrorOnFailure(parser.GetPath(&pathParser));
    ReturnErrorOnFailure(pathParser.GetConcreteAttributePath(attributePath, AttributePathIB::ValidateIdRanges::kNo));
    ReturnErrorOnFailure(parser.GetData(&dataReader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodedReportIBs::StartEncoding(app::AttributeReportIBs::Builder & builder)
{
    mEncodeWriter.Init(mTlvDataBuffer);
    ReturnErrorOnFailure(mEncodeWriter.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, mOuterStructureType));
    return builder.Init(&mEncodeWriter, to_underlying(ReportDataMessage::Tag::kAttributeReportIBs));
}

CHIP_ERROR EncodedReportIBs::FinishEncoding(app::AttributeReportIBs::Builder & builder)
{
    builder.EndOfContainer();
    ReturnErrorOnFailure(mEncodeWriter.EndContainer(mOuterStructureType));
    ReturnErrorOnFailure(mEncodeWriter.Finalize());

    mDecodeSpan = ByteSpan(mTlvDataBuffer, mEncodeWriter.GetLengthWritten());
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodedReportIBs::Decode(std::vector<DecodedAttributeData> & decoded_items) const
{
    return DecodeAttributeReportIBs(mDecodeSpan, decoded_items);
}

} // namespace Testing
} // namespace app
} // namespace chip
