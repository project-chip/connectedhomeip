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

#include "MessageBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
CHIP_ERROR MessageBuilder::EncodeInteractionModelRevision()
{
    return mpWriter->Put(TLV::ContextTag(Revision::kInteractionModelRevisionTag), Revision::kInteractionModelRevision);
}

CHIP_ERROR MessageBuilder::EncodeRequestFlags(const RequestFlagsOptions & options, TLV::Tag suppressResponseTag,
                                              TLV::Tag timedRequestTag)
{
    ReturnErrorOnFailure(mError);
    VerifyOrReturnError(!(options.isGroupcast && options.timedRequest), CHIP_ERROR_INVALID_ARGUMENT);

    const bool omitFalseFlags =
        options.peerRevision.has_value() && *options.peerRevision >= Revision::kFirstRevisionWithOptionalRequestFlags;
    if (options.suppressResponse || !omitFalseFlags)
    {
        ReturnErrorOnFailure(mpWriter->PutBoolean(suppressResponseTag, options.suppressResponse));
    }
    if (options.timedRequest || !omitFalseFlags)
    {
        ReturnErrorOnFailure(mpWriter->PutBoolean(timedRequestTag, options.timedRequest));
    }
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip
