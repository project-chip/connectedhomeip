/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-License-Identifier: Apache-2.0
 */

#include "MessageBuilder.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

namespace chip {
namespace app {
CHIP_ERROR MessageBuilder::EncodeInteractionModelRevision()
{
    return mpWriter->Put(TLV::ContextTag(kInteractionModelRevisionTag),
                         static_cast<InteractionModelRevision>(CHIP_DEVICE_INTERACTION_MODEL_REVISION));
}
} // namespace app
} // namespace chip
