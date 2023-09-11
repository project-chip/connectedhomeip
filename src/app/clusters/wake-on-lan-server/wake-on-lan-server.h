/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Wake on LAN plugin, the
 *server implementation of the Wake on LAN cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "wake-on-lan-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace WakeOnLan {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace WakeOnLan
} // namespace Clusters
} // namespace app
} // namespace chip
