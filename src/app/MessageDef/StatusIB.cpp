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
 *      This file defines StatusIB parser and builder in CHIP interaction model
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
CHIP_ERROR StatusIB::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);
    VerifyOrExit(chip::TLV::kTLVType_Array == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    return err;
}

CHIP_ERROR StatusIB::Parser::DecodeStatusIB(Protocols::SecureChannel::GeneralStatusCode * apGeneralStatus,
                                            Protocols::InteractionModel::Status * apClusterStatus) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader lReader;

    uint16_t generalStatus;
    uint16_t clusterStatus;
    lReader.Init(mReader);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(generalStatus);
    SuccessOrExit(err);
    *apGeneralStatus = static_cast<Protocols::SecureChannel::GeneralStatusCode>(generalStatus);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(clusterStatus);
    SuccessOrExit(err);
    *apClusterStatus = static_cast<Protocols::InteractionModel::Status>(clusterStatus);

exit:
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("StatusIB =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // This is an array; all elements are anonymous.
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        if (!(TagPresenceMask & (1 << kCsTag_GeneralStatus)))
        {
            TagPresenceMask |= (1 << kCsTag_GeneralStatus);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t generalStatus;
                err = reader.Get(generalStatus);
                SuccessOrExit(err);

                PRETTY_PRINT("\tStatus = 0x%" PRIx16 ",", generalStatus);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_ClusterStatus)))
        {
            TagPresenceMask |= (1 << kCsTag_ClusterStatus);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t clusterStatus;
                err = reader.Get(clusterStatus);
                SuccessOrExit(err);

                PRETTY_PRINT("\tclusterStatus = 0x%" PRIx16 ",", clusterStatus);
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
        const uint16_t RequiredFields = (1 << kCsTag_GeneralStatus) | (1 << kCsTag_ClusterStatus);

        if ((TagPresenceMask & RequiredFields) == RequiredFields)
        {
            err = CHIP_NO_ERROR;
        }
        else
        {
            err = CHIP_ERROR_IM_MALFORMED_STATUS_CODE;
        }
    }
    SuccessOrExit(err);
    err = reader.ExitContainer(mOuterContainerType);

exit:

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusIB::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return ListBuilder::Init(apWriter);
}

CHIP_ERROR StatusIB::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return ListBuilder::Init(apWriter, aContextTagToUse);
}

StatusIB::Builder & StatusIB::Builder::EncodeStatusIB(const Protocols::SecureChannel::GeneralStatusCode aGeneralStatus,
                                                      const Protocols::InteractionModel::Status aClusterStatus)
{
    Tag tag = chip::TLV::AnonymousTag;

    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, static_cast<uint16_t>(aGeneralStatus));
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, static_cast<uint16_t>(aClusterStatus));
    SuccessOrExit(mError);

exit:
    return *this;
}

StatusIB::Builder & StatusIB::Builder::EndOfStatusIB()
{
    EndOfContainer();
    return *this;
}

}; // namespace app
}; // namespace chip
