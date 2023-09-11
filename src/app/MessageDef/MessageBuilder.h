/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "StructBuilder.h"
#include <app/InteractionModelRevision.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
class MessageBuilder : public StructBuilder
{
public:
    CHIP_ERROR EncodeInteractionModelRevision();
};
} // namespace app
} // namespace chip
