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
 * This implementation allows for items to be added dynamically.
 */

class DynamicReplacementProductListManager : public ReplacementProductListManager
{
public:
    CHIP_ERROR Next(ReplacementProductStruct & item) override;

    /**
     * Creates a ReplacementProductStruct and adds it to the list. Can only add up to kReplacementProductListMaxSize
     * entries to a list (as defined by the spec).
     * 
     * @param aProductIdentifierType The identifier type of the product.
     * @param aProductIdentifierValue The value of the product.
     * @return CHIP_ERROR_NO_MEMORY if the list is full, a CHIP_ERROR if there was a problem creating the
     * ReplacementProductStruct and a CHIP_NO_ERROR otherwise.
    */
    CHIP_ERROR AddItemToList(ResourceMonitoring::ProductIdentifierTypeEnum aProductIdentifierType,
                             chip::CharSpan aProductIdentifierValue)
    {
        if (mReplacementProductListSize == kReplacementProductListMaxSize)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        ReplacementProductStruct * type = &mReplacementProductsList[mReplacementProductListSize];
        CHIP_ERROR err                  = type->SetProductIdentifierValue(aProductIdentifierValue);
        if (CHIP_NO_ERROR != err)
        {
            return err;
        };

        type->SetProductIdentifierType(aProductIdentifierType);
        mReplacementProductListSize++;

        return CHIP_NO_ERROR;
    }

private:
    ReplacementProductStruct mReplacementProductsList[ReplacementProductListManager::kReplacementProductListMaxSize];
    uint8_t mReplacementProductListSize;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
