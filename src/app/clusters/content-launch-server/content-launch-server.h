/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "content-launch-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ContentLauncher {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);

} // namespace ContentLauncher
} // namespace Clusters
} // namespace app
} // namespace chip
