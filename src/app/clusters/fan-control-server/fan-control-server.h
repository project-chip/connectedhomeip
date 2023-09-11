/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "fan-control-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/util/af-types.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

void SetDefaultDelegate(EndpointId aEndpoint, Delegate * aDelegate);
Delegate * GetDelegate(EndpointId aEndpoint);

} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
