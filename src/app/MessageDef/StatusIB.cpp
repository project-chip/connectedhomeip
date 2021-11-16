/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/**
 *    @file
 *      This file defines Status Information Block in Interaction Model
 *
 */

#include "StatusIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR StatusIB::Parser::DecodeStatusIB(StatusIB & aStatusIB) const
{
    TLV::TLVReader reader;
    reader.Init(mReader);
    while (CHIP_NO_ERROR == reader.Next())
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case to_underlying(Tag::kStatus):
            ReturnErrorOnFailure(reader.Get(aStatusIB.mStatus));
            break;
        case to_underlying(Tag::kClusterStatus):
            ClusterStatus clusterStatus;
            ReturnErrorOnFailure(reader.Get(clusterStatus));
            aStatusIB.mClusterStatus.SetValue(clusterStatus);
            break;
        }
    }
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("StatusIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        if (!(TagPresenceMask & (1 << to_underlying(Tag::kStatus))))
        {
            TagPresenceMask |= (1 << to_underlying(Tag::kStatus));

#if CHIP_DETAIL_LOGGING
            {
                uint16_t status;
                ReturnErrorOnFailure(reader.Get(status));
                PRETTY_PRINT("\tstatus = 0x%" PRIx16 ",", status);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << to_underlying(Tag::kClusterStatus))))
        {
            TagPresenceMask |= (1 << to_underlying(Tag::kClusterStatus));

#if CHIP_DETAIL_LOGGING
            {
                ClusterStatus clusterStatus;
                ReturnErrorOnFailure(reader.Get(clusterStatus));
                PRETTY_PRINT("\tstatus = 0x%" PRIx8 ",", clusterStatus);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\tExtra element in StatusIB");
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const int RequiredFields = (1 << to_underlying(Tag::kStatus));

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_STATUS_CODE;
        }
    }
    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

StatusIB::Builder & StatusIB::Builder::EncodeStatusIB(const StatusIB & aStatusIB)
{
    mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kStatus)), aStatusIB.mStatus);
    SuccessOrExit(mError);

    if (aStatusIB.mClusterStatus.HasValue())
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kClusterStatus)), aStatusIB.mClusterStatus.Value());
        SuccessOrExit(mError);
    }

    EndOfContainer();
exit:
    return *this;
}

}; // namespace app
}; // namespace chip
