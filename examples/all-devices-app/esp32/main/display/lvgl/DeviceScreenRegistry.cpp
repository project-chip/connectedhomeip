/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "DeviceScreenRegistry.h"

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace chip::app {

namespace {

// Called from the device factory hook, which runs on the CHIP thread, so the endpoint source
// may be queried here. The display task must not do this.
EndpointId ParentOf(DataModel::ProviderMetadataTree * endpointSource, EndpointId endpoint)
{
    VerifyOrReturnValue(endpointSource != nullptr, kInvalidEndpointId);
    VerifyOrReturnValue(endpoint != kInvalidEndpointId, kInvalidEndpointId);

    ReadOnlyBufferBuilder<DataModel::EndpointEntry> builder;
    VerifyOrReturnValue(endpointSource->Endpoints(builder) == CHIP_NO_ERROR, kInvalidEndpointId);

    for (const auto & entry : builder.TakeBuffer())
    {
        if (entry.id == endpoint)
        {
            return entry.parentId;
        }
    }
    return kInvalidEndpointId;
}

} // namespace

DeviceScreenRegistry & DeviceScreenRegistry::Instance()
{
    static DeviceScreenRegistry sInstance;
    return sInstance;
}

void DeviceScreenRegistry::Register(DeviceScreenEntry entry)
{
    entry.parentEndpointId = ParentOf(mEndpointSource, entry.endpointId);

    std::lock_guard<std::mutex> guard(mEntriesMutex);
    mEntries.push_back(std::move(entry));
}

} // namespace chip::app
