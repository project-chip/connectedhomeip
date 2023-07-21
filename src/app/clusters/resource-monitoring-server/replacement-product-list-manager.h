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
#include <app/util/af-enums.h>
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
    ReplacementProductListManager() {}
    virtual ~ReplacementProductListManager() = default;

    void Reset() { mIndex = 0; }

    // Returns total size of Replacement Products List.
    virtual uint8_t Size() = 0;

    virtual CHIP_ERROR Next(Attributes::ReplacementProductStruct::Type & item) = 0;

protected:
    uint8_t mIndex;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
