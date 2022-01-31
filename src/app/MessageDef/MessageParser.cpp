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

#include "MessageParser.h"
#include "MessageDefHelper.h"
#include <app/InteractionModelRevision.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR MessageParser::CheckInteractionModelRevision(TLV::TLVReader & aReader) const
{
#if CHIP_DETAIL_LOGGING
    {
        uint8_t interactionModelRevision = 0;
        ReturnErrorOnFailure(aReader.Get(interactionModelRevision));
        PRETTY_PRINT("\tInteractionModelRevision = %u", interactionModelRevision);
    }
#endif // CHIP_DETAIL_LOGGING
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR MessageParser::GetInteractionModelRevision(InteractionModelRevision * const apInteractionModelRevision) const
{
    return GetUnsignedInteger(kInteractionModelRevisionTag, apInteractionModelRevision);
}

} // namespace app
} // namespace chip
