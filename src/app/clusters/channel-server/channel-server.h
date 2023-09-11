/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Channel plugin, the
 *server implementation of the Channel cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "channel-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
