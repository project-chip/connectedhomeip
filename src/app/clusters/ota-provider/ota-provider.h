/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "ota-provider-delegate.h"

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(chip::EndpointId endpointId, OTAProviderDelegate * delegate);

}
} // namespace Clusters
} // namespace app
} // namespace chip
