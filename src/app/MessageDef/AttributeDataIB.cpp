/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "AttributeDataIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeDataIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributeDataIB =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
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
        case to_underlying(Tag::kDataVersion):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kDataVersion))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kDataVersion));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                chip::DataVersion version;
                ReturnErrorOnFailure(reader.Get(version));
                PRETTY_PRINT("\tDataVersion = 0x%" PRIx32 ",", version);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kPath):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kPath))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kPath));
            {
                AttributePathIB::Parser path;
                ReturnErrorOnFailure(path.Init(reader));

                PRETTY_PRINT_INCDEPTH();
                ReturnErrorOnFailure(path.CheckSchemaValidity());
                PRETTY_PRINT_DECDEPTH();
            }
            break;
        case to_underlying(Tag::kData):
            // check if this tag has appeared before
            VerifyOrReturnError(!(tagPresenceMask & (1 << to_underlying(Tag::kData))), CHIP_ERROR_INVALID_TLV_TAG);
            tagPresenceMask |= (1 << to_underlying(Tag::kData));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(CheckIMPayload(reader, 0, "Data"));
            PRETTY_PRINT_DECDEPTH();
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }
    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const int requiredFields = (1 << to_underlying(Tag::kPath)) | (1 << to_underlying(Tag::kData));

        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_IB;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributeDataIB::Parser::GetPath(AttributePathIB::Parser * const apPath) const
{
    TLV::TLVReader reader;
    ReturnErrorOnFailure(mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kPath)), reader));
    return apPath->Init(reader);
}

CHIP_ERROR AttributeDataIB::Parser::GetDataVersion(chip::DataVersion * const apVersion) const
{
    return GetUnsignedInteger(to_underlying(Tag::kDataVersion), apVersion);
}

CHIP_ERROR AttributeDataIB::Parser::GetData(TLV::TLVReader * const apReader) const
{
    return mReader.FindElementWithTag(TLV::ContextTag(to_underlying(Tag::kData)), *apReader);
}

AttributePathIB::Builder & AttributeDataIB::Builder::CreatePath()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mPath.Init(mpWriter, to_underlying(Tag::kPath));
    }
    return mPath;
}

AttributeDataIB::Builder & AttributeDataIB::Builder::DataVersion(const chip::DataVersion aDataVersion)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kDataVersion)), aDataVersion);
    }
    return *this;
}

AttributeDataIB::Builder & AttributeDataIB::Builder::EndOfAttributeDataIB()
{
    EndOfContainer();
    return *this;
}
} // namespace app
} // namespace chip
