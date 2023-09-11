/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Target Navigator plugin, the
 *server implementation of the Target Navigator cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "target-navigator-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TargetNavigator {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace TargetNavigator
} // namespace Clusters
} // namespace app
} // namespace chip
