/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Low Power plugin, the
 *server implementation of the Low Power cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "low-power-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace LowPower {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace LowPower
} // namespace Clusters
} // namespace app
} // namespace chip
