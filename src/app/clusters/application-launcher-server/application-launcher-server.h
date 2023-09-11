/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Application Launcher plugin, the
 *server implementation of the Application Launcher cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "application-launcher-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationLauncher {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace ApplicationLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
