/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "valve-configuration-and-control-matter-context.h"

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/reporting/reporting.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

CHIP_ERROR MatterContext::StoreDefaultOpenDuration(const DataModel::Nullable<uint32_t> & defaultOpenDuration)
{

    if (defaultOpenDuration.IsNull())
    {
        return mPersistentStorageDelegate.SyncDeleteKeyValue(
            DefaultStorageKeyAllocator::VCCDefaultOpenDuration(mEndpoint).KeyName());
    }
    uint32_t val = defaultOpenDuration.Value();
    return mPersistentStorageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::VCCDefaultOpenDuration(mEndpoint).KeyName(), &val,
                                                      sizeof(val));
}

CHIP_ERROR MatterContext::GetDefaultOpenDuration(uint32_t & returnVal)
{
    uint16_t size = static_cast<uint16_t>(sizeof(returnVal));
    return mPersistentStorageDelegate.SyncGetKeyValue(DefaultStorageKeyAllocator::VCCDefaultOpenDuration(mEndpoint).KeyName(),
                                                      &returnVal, size);
}

CHIP_ERROR MatterContext::StoreDefaultOpenLevel(const uint8_t defaultOpenLevel)
{

    return mPersistentStorageDelegate.SyncSetKeyValue(DefaultStorageKeyAllocator::VCCDefaultOpenLevel(mEndpoint).KeyName(),
                                                      &defaultOpenLevel, sizeof(defaultOpenLevel));
}

CHIP_ERROR MatterContext::GetDefaultOpenLevel(uint8_t & returnVal)
{
    uint16_t size = static_cast<uint16_t>(sizeof(returnVal));
    return mPersistentStorageDelegate.SyncGetKeyValue(DefaultStorageKeyAllocator::VCCDefaultOpenLevel(mEndpoint).KeyName(),
                                                      &returnVal, size);
}

void MatterContext::MarkDirty(const AttributeId attributeId)
{
    MatterReportingAttributeChangeCallback(mEndpoint, Id, attributeId);
}

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
