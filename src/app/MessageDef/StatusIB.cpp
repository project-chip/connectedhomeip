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

#include <app/AppConfig.h>
#include <lib/core/CHIPCore.h>

using namespace chip;
using namespace chip::TLV;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
CHIP_ERROR StatusIB::Parser::DecodeStatusIB(StatusIB & aStatusIB) const
{
    TLV::TLVReader reader;
    reader.Init(mReader);
    while (CHIP_NO_ERROR == reader.Next())
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
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
    int tagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("StatusIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        if (!(tagPresenceMask & (1 << to_underlying(Tag::kStatus))))
        {
            tagPresenceMask |= (1 << to_underlying(Tag::kStatus));

#if CHIP_DETAIL_LOGGING
            {
                uint8_t status;
                ReturnErrorOnFailure(reader.Get(status));
                PRETTY_PRINT("\tstatus = " ChipLogFormatIMStatus ",", ChipLogValueIMStatus(static_cast<Status>(status)));
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(tagPresenceMask & (1 << to_underlying(Tag::kClusterStatus))))
        {
            tagPresenceMask |= (1 << to_underlying(Tag::kClusterStatus));

#if CHIP_DETAIL_LOGGING
            {
                ClusterStatus clusterStatus;
                ReturnErrorOnFailure(reader.Get(clusterStatus));
                PRETTY_PRINT("\tcluster-status = 0x%x,", clusterStatus);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\tExtra element in StatusIB");
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT_BLANK_LINE();
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const int requiredFields = (1 << to_underlying(Tag::kStatus));
        err = (tagPresenceMask & requiredFields) == requiredFields ? CHIP_NO_ERROR : CHIP_ERROR_IM_MALFORMED_STATUS_IB;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
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

CHIP_ERROR StatusIB::ToChipError() const
{
    if (mStatus == Status::Success)
    {
        return CHIP_NO_ERROR;
    }

    if (mClusterStatus.HasValue())
    {
        return ChipError(ChipError::SdkPart::kIMClusterStatus, mClusterStatus.Value());
    }

    return ChipError(ChipError::SdkPart::kIMGlobalStatus, to_underlying(mStatus));
}

void StatusIB::InitFromChipError(CHIP_ERROR aError)
{
    if (aError.IsPart(ChipError::SdkPart::kIMClusterStatus))
    {
        mStatus        = Status::Failure;
        mClusterStatus = MakeOptional(aError.GetSdkCode());
        return;
    }

    mClusterStatus = NullOptional;
    if (aError == CHIP_NO_ERROR)
    {
        mStatus = Status::Success;
        return;
    }

    if (aError.IsPart(ChipError::SdkPart::kIMGlobalStatus))
    {
        mStatus = static_cast<Status>(aError.GetSdkCode());
        return;
    }

    mStatus = Status::Failure;
}

namespace {
bool FormatStatusIBError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    if (!err.IsIMStatus())
    {
        return false;
    }

    const char * desc = nullptr;
#if !CHIP_CONFIG_SHORT_ERROR_STR
    constexpr char generalFormat[] = "General error: " ChipLogFormatIMStatus;
    constexpr char clusterFormat[] = "Cluster-specific error: 0x%02x";

    // Formatting an 8-bit int will take at most 2 chars, and replace the '%02x'
    // so a buffer big enough to hold our format string will also hold our
    // formatted string, as long as we account for the possible string formats.
    constexpr size_t statusNameMaxLength =
#define CHIP_IM_STATUS_CODE(name, spec_name, value)                                                                                \
        max(sizeof(#spec_name),
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE
        static_cast<size_t>(0)
#define CHIP_IM_STATUS_CODE(name, spec_name, value)                                                                                \
        )
#include <protocols/interaction_model/StatusCodeList.h>
#undef CHIP_IM_STATUS_CODE
        ;
    constexpr size_t formattedSize = max(sizeof(generalFormat) + statusNameMaxLength, sizeof(clusterFormat));
    char formattedString[formattedSize];

    StatusIB status;
    status.InitFromChipError(err);
    if (status.mClusterStatus.HasValue())
    {
        snprintf(formattedString, formattedSize, clusterFormat, status.mClusterStatus.Value());
    }
    else
    {
        snprintf(formattedString, formattedSize, generalFormat, ChipLogValueIMStatus(status.mStatus));
    }
    desc = formattedString;
#endif // !CHIP_CONFIG_SHORT_ERROR_STR
    FormatError(buf, bufSize, "IM", err, desc);

    return true;
}
} // anonymous namespace

void StatusIB::RegisterErrorFormatter()
{
    static ErrorFormatter sStatusIBErrorFormatter = { FormatStatusIBError, nullptr };

    ::RegisterErrorFormatter(&sStatusIBErrorFormatter);
}

}; // namespace app
}; // namespace chip
