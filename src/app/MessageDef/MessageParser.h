/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "StructParser.h"
#include <app/AppBuildConfig.h>
#include <app/InteractionModelRevision.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
class MessageParser : public StructParser
{
public:
    CHIP_ERROR Init(TLV::TLVReader & aReader);
    CHIP_ERROR ExitContainer();
#if CHIP_CONFIG_IM_PRETTY_PRINT
    CHIP_ERROR CheckInteractionModelRevision(TLV::TLVReader & aReader) const;
#endif // CHIP_CONFIG_IM_PRETTY_PRINT

    CHIP_ERROR GetInteractionModelRevision(InteractionModelRevision * const apInteractionModelRevision) const;
};
} // namespace app
} // namespace chip
