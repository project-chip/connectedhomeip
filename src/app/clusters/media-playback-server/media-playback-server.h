/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Media Playback plugin, the
 *server implementation of the Media Playback cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "media-playback-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaPlayback {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace MediaPlayback
} // namespace Clusters
} // namespace app
} // namespace chip
