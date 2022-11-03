/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/af-enums.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {

inline EmberAfStatus ToEmberAfStatus(Protocols::InteractionModel::Status code)
{
    return static_cast<EmberAfStatus>(code);
}

Protocols::InteractionModel::Status ToInteractionModelStatus(EmberAfStatus code);

} // namespace app
} // namespace chip
