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

#include <ImmutableReplacementProductListManager.h>
#include <StaticReplacementProductListManager.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;

CHIP_ERROR StaticReplacementProductListManager::Next(ReplacementProductStruct & item)
{
    if (mIndex < mReplacementProductListSize)
    {
        item = mReplacementProductsList[mIndex];
        mIndex++;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
}

CHIP_ERROR ImmutableReplacementProductListManager::Next(ReplacementProductStruct & item)
{
    if (mIndex >= kReplacementProductListMaxSize)
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }

    switch (mIndex)
    {
    case 0: {
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kUpc);
        item.SetProductIdentifierValue(CharSpan::fromCharString("111112222233"));
        break;
    case 1:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin8);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin8xxx"));
        break;
    case 2:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kEan);
        item.SetProductIdentifierValue(CharSpan::fromCharString("4444455555666"));
        break;
    case 3:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kGtin14);
        item.SetProductIdentifierValue(CharSpan::fromCharString("gtin14xxxxxxxx"));
        break;
    case 4:
        item.SetProductIdentifierType(ResourceMonitoring::ProductIdentifierTypeEnum::kOem);
        item.SetProductIdentifierValue(CharSpan::fromCharString("oem20xxxxxxxxxxxxxxx"));
        break;
    default:
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        break;
    }
    }
    mIndex++;
    return CHIP_NO_ERROR;
}
