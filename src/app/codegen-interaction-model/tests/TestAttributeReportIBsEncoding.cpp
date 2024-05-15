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
#include "TestAttributeReportIBsEncoding.h"

#include <app/MessageDef/AttributePathIB.h>
#include <app/MessageDef/AttributeReportIB.h>

using namespace chip::app;

namespace chip {
namespace Test {

CHIP_ERROR DecodedAttributeData::DecodeFrom(const chip::app::AttributeDataIB::Parser & parser)
{
    ReturnErrorOnFailure(parser.GetDataVersion(&dataVersion));

    AttributePathIB::Parser pathParser;
    ReturnErrorOnFailure(parser.GetPath(&pathParser));
    ReturnErrorOnFailure(pathParser.GetConcreteAttributePath(attributePath, AttributePathIB::ValidateIdRanges::kNo));
    ReturnErrorOnFailure(parser.GetData(&dataReader));

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeAttributeReportIBs(ByteSpan data, std::vector<DecodedAttributeData> & decoded_items)
{
    // Espected data format:
    //   CONTAINER (anonymous)
    //     0x01 => Array (i.e. report data ib)
    //       ReportIB*
    //
    // Overally this is VERY hard to process ...
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

} // namespace Test
} // namespace chip
