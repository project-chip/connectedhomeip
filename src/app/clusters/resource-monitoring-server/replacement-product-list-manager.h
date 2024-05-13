/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>

#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

/**
 * Interface to help manage the replacement product list in the Resource Monitoring Cluster.
 */
class ReplacementProductListManager
{
public:
    // The max replacement product list size as defined in the specification
    static constexpr size_t kReplacementProductListMaxSize = 5u;

    ReplacementProductListManager() {}
    virtual ~ReplacementProductListManager() = default;

    void Reset() { mIndex = 0; }

    /**
     * Iterates through the entries in the ReplacementProductListManager. Each call to this function copies the next item into
     * the out param. Calls to this function will return CHIP_NO_ERROR if there are still valid elements in the list. The function
     * will return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the end of the list has been reached.
     *
     * @param[out] item An out parameter that has a copy of the item retrieved in the list.
     * @return CHIP_NO_ERROR if the pointer to the list element has moved to the next element and there are still valid remaining
     * entries in the list. Otherwise returns CHIP_ERROR_PROVIDER_LIST_EXHAUSTED if the list has hit the last element.
     */
    virtual CHIP_ERROR Next(ReplacementProductStruct & item) = 0;

protected:
    uint8_t mIndex;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
