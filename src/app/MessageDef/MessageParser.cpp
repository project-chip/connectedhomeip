/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MessageParser.h"
#include "MessageDefHelper.h"
#include <app/InteractionModelRevision.h>

namespace chip {
namespace app {
CHIP_ERROR MessageParser::Init(TLV::TLVReader & aReader)
{
    ReturnErrorOnFailure(aReader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    return StructParser::Init(aReader);
}

CHIP_ERROR MessageParser::ExitContainer()
{
    // Ignore any other tags that might be here.
    ReturnErrorOnFailure(mReader.ExitContainer(mOuterContainerType));

    // There should be nothing outside the toplevel struct.
    VerifyOrReturnError(mReader.Next() == CHIP_END_OF_TLV, CHIP_ERROR_INVALID_TLV_TAG);
    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_IM_PRETTY_PRINT
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
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

CHIP_ERROR MessageParser::GetInteractionModelRevision(InteractionModelRevision * const apInteractionModelRevision) const
{
    return GetUnsignedInteger(kInteractionModelRevisionTag, apInteractionModelRevision);
}

} // namespace app
} // namespace chip
