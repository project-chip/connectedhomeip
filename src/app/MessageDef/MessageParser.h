/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2016-2017 Nest Labs, Inc.
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
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    CHIP_ERROR CheckInteractionModelRevision(TLV::TLVReader & aReader) const;
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

    CHIP_ERROR GetInteractionModelRevision(InteractionModelRevision * const apInteractionModelRevision) const;
};
} // namespace app
} // namespace chip
