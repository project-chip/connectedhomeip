/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <app/clusters/commodity-metering-server/commodity-metering-server.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

#include <lib/support/CodeUtils.h>

CHIP_ERROR CommodityMeteringInit(chip::EndpointId endpointId);
CHIP_ERROR CommodityMeteringShutdown();

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityMetering {

/** @brief Helper function to return the singleton CommodityMeteringInstance instance
 *
 * This is needed to support TestEventTriggers which are called outside of any
 * class context. This allows the Delegate instance in which to invoke the test
 * events on.
 *
 * This function is typically found in main.cpp or wherever the singleton is created.
 */
Instance * GetCommodityMeteringInstance();

} // namespace CommodityMetering
} // namespace Clusters
} // namespace app
} // namespace chip
