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

#include "resource-monitoring-cluster-objects.h"

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

namespace Commands {
namespace ResetCondition {

CHIP_ERROR DecodableType::Decode(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;
    VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        // check that this is not malformed TLV
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}
} // namespace ResetCondition.
} // namespace Commands

namespace Attributes {
namespace ReplacementProductStruct {

CHIP_ERROR Type::Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kProductIdentifierType), productIdentifierType));
    ReturnErrorOnFailure(DataModel::Encode(writer, TLV::ContextTag(Fields::kProductIdentifierValue), productIdentifierValue));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

} // namespace ReplacementProductStruct
} // namespace Attributes

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
