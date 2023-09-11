/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Media Input plugin, the
 *server implementation of the Media Input cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "media-input-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaInput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace MediaInput
} // namespace Clusters
} // namespace app
} // namespace chip
