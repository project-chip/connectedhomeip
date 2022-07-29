/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AttributeReportIB.h"
#include "AttributeDataIB.h"
#include "MessageDefHelper.h"
#include "StructBuilder.h"
#include "StructParser.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeReportIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributeReportIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kAttributeStatus):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kAttributeStatus))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kAttributeStatus));
            {
                AttributeStatusIB::Parser attributeStatus;
                ReturnErrorOnFailure(attributeStatus.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(attributeStatus.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kAttributeData):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kAttributeData))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kAttributeData));
            {
                AttributeDataIB::Parser attributeData;
                ReturnErrorOnFailure(attributeData.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(attributeData.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    if (CHIP_END_OF_TLV == err)
    {
        // check for at most field:
        const int CheckDataField   = 1 << to_underlying(Tag::kAttributeData);
        const int CheckStatusField = (1 << to_underlying(Tag::kAttributeStatus));

        if ((tagPresenceMask & CheckDataField) == CheckDataField && (tagPresenceMask & CheckStatusField) == CheckStatusField)
        {
            // kAttributeData and kAttributeStatus both exist
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_REPORT_IB;
        }
        else if ((tagPresenceMask & CheckDataField) != CheckDataField && (tagPresenceMask & CheckStatusField) != CheckStatusField)
        {
            // kPath and kErrorStatus not exist
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_REPORT_IB;
        }
        else
        {
            err = CHIP_NO_ERROR;
        }
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeReportIB::Parser::GetAttributeStatus(AttributeStatusIB::Parser * const apAttributeStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kAttributeStatus)), reader));
    return apAttributeStatus->Init(reader);
}

CHIP_ERROR AttributeReportIB::Parser::GetAttributeData(AttributeDataIB::Parser * const apAttributeData) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kAttributeData)), reader));
    return apAttributeData->Init(reader);
}

AttributeStatusIB::Builder & AttributeReportIB::Builder::CreateAttributeStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeStatus.Init(mpWriter, to_underlying(Tag::kAttributeStatus));
    }
    return mAttributeStatus;
}

AttributeDataIB::Builder & AttributeReportIB::Builder::CreateAttributeData()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeData.Init(mpWriter, to_underlying(Tag::kAttributeData));
    }
    return mAttributeData;
}

AttributeReportIB::Builder & AttributeReportIB::Builder::EndOfAttributeReportIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
