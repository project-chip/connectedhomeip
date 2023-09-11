/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mode-base-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ModeBase {

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

} // namespace ModeBase
} // namespace Clusters
} // namespace app
} // namespace chip
