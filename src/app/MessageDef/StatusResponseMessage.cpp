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

#include "StatusResponseMessage.h"
#include "MessageDefHelper.h"

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR StatusResponseMessage::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool statusTagPresence = false;
    TLV::TLVReader reader;
    PRETTY_PRINT("StatusResponseMessage =");
    PRETTY_PRINT("{");

    // make a copy of the reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case kCsTag_Status:
            VerifyOrReturnError(!statusTagPresence, CHIP_ERROR_INVALID_TLV_TAG);
            statusTagPresence = true;
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                uint16_t status;
                ReturnErrorOnFailure(reader.Get(status));
                PRETTY_PRINT("\tStatus = 0x%" PRIx16 ",", status);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            ReturnErrorOnFailure(CHIP_ERROR_INVALID_TLV_TAG);
        }
    }
    PRETTY_PRINT("}");
    PRETTY_PRINT("");

    if (CHIP_END_OF_TLV == err && statusTagPresence)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR StatusResponseMessage::Parser::GetStatus(Protocols::InteractionModel::Status & aStatus) const
{
    uint16_t status = 0;
    CHIP_ERROR err  = GetUnsignedInteger(kCsTag_Status, &status);
    aStatus         = static_cast<Protocols::InteractionModel::Status>(status);
    return err;
}

StatusResponseMessage::Builder & StatusResponseMessage::Builder::Status(const Protocols::InteractionModel::Status aStatus)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(kCsTag_Status), aStatus);
    }
    EndOfContainer();
    return *this;
}

} // namespace app
} // namespace chip
