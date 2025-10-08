/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <app/data-model/StructDecodeIterator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace detail {

CHIP_ERROR StructDecodeIterator::Next(uint8_t & context_tag)
{
    if (!mEntered)
    {
        VerifyOrReturnError(TLV::kTLVType_Structure == mReader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
        ReturnErrorOnFailure(mReader.EnterContainer(mOuter));
        mEntered = true;
    }

    while (true)
    {
        CHIP_ERROR err = mReader.Next();
        if (err != CHIP_NO_ERROR)
        {
            VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);
            break;
        }

        const TLV::Tag tag = mReader.GetTag();
        if (!TLV::IsContextTag(tag))
        {
            continue;
        }

        // we know context tags are 8-bit
        context_tag = static_cast<uint8_t>(TLV::TagNumFromTag(tag));
        return CHIP_NO_ERROR;
    }

    // we get here IFF error is CHIP_ERROR_END_OF_TLV. We exit the container but
    // forward the the `end of tlv` as a final marker
    ReturnErrorOnFailure(mReader.ExitContainer(mOuter));
    return CHIP_ERROR_END_OF_TLV;
}

} // namespace detail
} // namespace Clusters
} // namespace app
} // namespace chip
