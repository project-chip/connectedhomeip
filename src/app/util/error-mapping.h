/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
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

inline Protocols::InteractionModel::Status ToInteractionModelStatus(EmberAfStatus code)
{
    return static_cast<Protocols::InteractionModel::Status>(code);
}

} // namespace app
} // namespace chip
