/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Keypad Input plugin, the
 *server implementation of the Keypad Input cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "keypad-input-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace KeypadInput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace KeypadInput
} // namespace Clusters
} // namespace app
} // namespace chip
