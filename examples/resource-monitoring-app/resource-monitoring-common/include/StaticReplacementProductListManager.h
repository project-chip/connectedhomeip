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

#include <app/clusters/resource-monitoring-server/replacement-product-list-manager.h>
#include <app/util/af.h>
#include <app/util/config.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

/**
 * This implementation statically defines the options.
 */

class StaticReplacementProductListManager : public ReplacementProductListManager
{
public:
    uint8_t Size() override { return mReplacementProductListSize; };

    CHIP_ERROR Next(Attributes::ReplacementProductStruct::Type & item) override;

    ~StaticReplacementProductListManager() {}
    StaticReplacementProductListManager(Attributes::ReplacementProductStruct::Type * aReplacementProductsList,
                                        uint8_t aReplacementProductListSize)
    {
        mReplacementProductsList    = aReplacementProductsList;
        mReplacementProductListSize = aReplacementProductListSize;
    }

private:
    Attributes::ReplacementProductStruct::Type * mReplacementProductsList;
    uint8_t mReplacementProductListSize;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
