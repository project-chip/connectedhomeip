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

#include "StructParser.h"

namespace chip {
namespace app {
CHIP_ERROR StructParser::Init(const TLV::TLVReader & aReader)
{
    mReader.Init(aReader);
    VerifyOrReturnError(TLV::kTLVType_Structure == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(mReader.EnterContainer(mOuterContainerType));
    return CheckSchemaOrdering();
}

CHIP_ERROR StructParser::CheckSchemaOrdering() const
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVReader reader;
    reader.Init(mReader);
    uint32_t preTagNum = 0;
    bool first         = true;
    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        if (!TLV::IsContextTag(reader.GetTag()))
        {
            // Just skip over non-context tags, for forward compat.
            continue;
        }
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        if (first || (preTagNum < tagNum))
        {
            preTagNum = tagNum;
        }
        else
        {
            return CHIP_ERROR_INVALID_TLV_TAG;
        }
        first = false;
    }
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return reader.ExitContainer(mOuterContainerType);
}
} // namespace app
} // namespace chip
