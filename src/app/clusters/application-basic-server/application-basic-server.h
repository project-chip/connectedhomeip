/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "application-basic-delegate.h"
#include <app-common/zap-generated/cluster-objects.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ApplicationBasic {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate);
Delegate * GetDefaultDelegate(EndpointId endpoint);

} // namespace ApplicationBasic
} // namespace Clusters
} // namespace app
} // namespace chip
