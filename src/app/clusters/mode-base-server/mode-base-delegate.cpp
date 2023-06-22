/*
*
*    Copyright (c) 2023 Project CHIP Authors
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

#include "mode-base-delegate.h"

using namespace std;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;

using ModeOptionStructType = detail::Structs::ModeOptionStruct::Type;

void Delegate::HandleChangeToMode(uint8_t NewMode, ModeBase::Commands::ChangeToModeResponse::Type &response)
{
    response.status = to_underlying(StatusCode::kSuccess);
}

uint8_t Delegate::NumberOfModes()
{
    return 0;
}

CHIP_ERROR Delegate::getModeLabelByIndex(uint8_t modeIndex, MutableCharSpan &label)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Delegate::getModeValueByIndex(uint8_t modeIndex, uint8_t &value)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR Delegate::getModeTagsByIndex(uint8_t modeIndex, List<SemanticTagStructType> &tags)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool Delegate::IsSupportedMode(uint8_t modeValue)
{
    for (uint8_t i = 0; i < NumberOfModes(); i++) {
        uint8_t value;
        auto err = getModeValueByIndex(i, value);
        if (err == CHIP_NO_ERROR) {
            if (value == modeValue) {
                return true;
            }
        } else {
            break;
        }
    }
    ChipLogDetail(Zcl, "Cannot find a mode with value %u", modeValue);
    return false;
}

namespace chip { namespace app { namespace Clusters { namespace ModeBase {
namespace Commands {
namespace ChangeToMode {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kNewMode), newMode));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;
    VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case to_underlying(Fields::kNewMode):
            ReturnErrorOnFailure(DataModel::Decode(reader, newMode));
            break;
        default:
            break;
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace ChangeToMode.
namespace ChangeToModeResponse {
CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kStatus), status));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kStatusText), statusText));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;
    VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            continue;
        }
        switch (TLV::TagNumFromTag(reader.GetTag()))
        {
        case to_underlying(Fields::kStatus):
            ReturnErrorOnFailure(DataModel::Decode(reader, status));
            break;
        case to_underlying(Fields::kStatusText):
            ReturnErrorOnFailure(DataModel::Decode(reader, statusText));
            break;
        default:
            break;
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace ChangeToModeResponse.
} // namespace Commands
} } } }