/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/****************************************************************************
 * @file
 * @brief Routines for the Audio Output plugin, the
 *server implementation of the Audio Output cluster.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "audio-output-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
