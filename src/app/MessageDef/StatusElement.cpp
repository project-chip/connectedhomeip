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
 *      This file defines StatusElement parser and builder in CHIP interaction model
 *
 */

#include "StatusElement.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

using namespace chip;
using namespace chip::TLV;

namespace chip {
namespace app {
CHIP_ERROR StatusElement::Parser::Init(const chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // make a copy of the reader here
    mReader.Init(aReader);
    VerifyOrExit(chip::TLV::kTLVType_Array == mReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

    err = mReader.EnterContainer(mOuterContainerType);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR StatusElement::Parser::DecodeStatusElement(Protocols::SecureChannel::GeneralStatusCode * apGeneralCode,
                                                      uint32_t * apProtocolId, uint16_t * apProtocolCode) const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::TLV::TLVReader lReader;

    uint16_t generalCode;
    lReader.Init(mReader);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(generalCode);
    SuccessOrExit(err);
    *apGeneralCode = static_cast<Protocols::SecureChannel::GeneralStatusCode>(generalCode);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apProtocolId);
    SuccessOrExit(err);

    err = lReader.Next();
    SuccessOrExit(err);
    VerifyOrExit(lReader.GetType() == chip::TLV::kTLVType_UnsignedInteger, err = CHIP_ERROR_WRONG_TLV_TYPE);
    err = lReader.Get(*apProtocolCode);
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
    return err;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusElement::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    uint16_t TagPresenceMask = 0;
    chip::TLV::TLVReader reader;

    PRETTY_PRINT("StatusElement =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        // This is an array; all elements are anonymous.
        VerifyOrExit(chip::TLV::AnonymousTag == reader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);

        if (!(TagPresenceMask & (1 << kCsTag_GeneralCode)))
        {
            TagPresenceMask |= (1 << kCsTag_GeneralCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t generalCode;
                err = reader.Get(generalCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\tGeneralCode = 0x%" PRIx16 ",", generalCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_ProtocolId)))
        {
            TagPresenceMask |= (1 << kCsTag_ProtocolId);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint32_t kCsTag_ProtocolId;
                err = reader.Get(kCsTag_ProtocolId);
                SuccessOrExit(err);

                PRETTY_PRINT("\tProtocolId = 0x%" PRIx32 ",", kCsTag_ProtocolId);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else if (!(TagPresenceMask & (1 << kCsTag_ProtocolCode)))
        {
            TagPresenceMask |= (1 << kCsTag_ProtocolCode);
            VerifyOrExit(chip::TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                uint16_t protocolCode;
                err = reader.Get(protocolCode);
                SuccessOrExit(err);

                PRETTY_PRINT("\tprotocolCode = 0x%" PRIx16 ",", protocolCode);
            }
#endif // CHIP_DETAIL_LOGGING
        }
        else
        {
            PRETTY_PRINT("\tExtra element in StatusElement");
        }
    }

    PRETTY_PRINT("},");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // check for required fields:
        const uint16_t RequiredFields = (1 << kCsTag_GeneralCode) | (1 << kCsTag_ProtocolId) | (1 << kCsTag_ProtocolCode);

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
    ChipLogFunctError(err);

    return err;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter)
{
    return ListBuilder::Init(apWriter);
}

CHIP_ERROR StatusElement::Builder::Init(chip::TLV::TLVWriter * const apWriter, const uint8_t aContextTagToUse)
{
    return ListBuilder::Init(apWriter, aContextTagToUse);
}

StatusElement::Builder & StatusElement::Builder::EncodeStatusElement(const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                                                                     const uint32_t aProtocolId, const uint16_t aProtocolCode)
{
    uint64_t tag = chip::TLV::AnonymousTag;

    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, static_cast<uint16_t>(aGeneralCode));
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aProtocolId);
    SuccessOrExit(mError);

    mError = mpWriter->Put(tag, aProtocolCode);
    SuccessOrExit(mError);

exit:
    ChipLogFunctError(mError);
    return *this;
}

StatusElement::Builder & StatusElement::Builder::EndOfStatusElement()
{
    EndOfContainer();
    return *this;
}

}; // namespace app
}; // namespace chip
