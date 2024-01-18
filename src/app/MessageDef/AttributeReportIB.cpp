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
#if CHIP_CONFIG_IM_PRETTY_PRINT
CHIP_ERROR AttributeReportIB::Parser::PrettyPrint() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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
        case to_underlying(Tag::kAttributeStatus): {
            AttributeStatusIB::Parser attributeStatus;
            ReturnErrorOnFailure(attributeStatus.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(attributeStatus.PrettyPrint());
            PRETTY_PRINT_DECDEPTH();
        }
        break;
        case to_underlying(Tag::kAttributeData): {
            AttributeDataIB::Parser attributeData;
            ReturnErrorOnFailure(attributeData.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(attributeData.PrettyPrint());
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
        err = CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR AttributeReportIB::Parser::GetAttributeStatus(AttributeStatusIB::Parser * const apAttributeStatus) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kAttributeStatus), reader));
    return apAttributeStatus->Init(reader);
}

CHIP_ERROR AttributeReportIB::Parser::GetAttributeData(AttributeDataIB::Parser * const apAttributeData) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(Tag::kAttributeData), reader));
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

CHIP_ERROR AttributeReportIB::Builder::EndOfAttributeReportIB()
{
    EndOfContainer();
    return GetError();
}
} // namespace app
} // namespace chip
