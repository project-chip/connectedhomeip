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

#pragma once

#include <functional>
#include <string>
#include <vector>

#include <app/data-model-provider/ProviderMetadataTree.h>
#include <lib/core/DataModelTypes.h>
#include <lvgl.h>

namespace chip::app {

struct DeviceScreenEntry
{
    std::string title;
    EndpointId endpointId;
    std::function<void(lv_obj_t * parent)> renderFn;

    // Parent in the endpoint tree, kInvalidEndpointId at the root. Filled in by
    // DeviceScreenRegistry::Register; callers leave it unset. Stored rather than a depth so
    // that it stays correct when endpoints are added or removed at runtime.
    EndpointId parentEndpointId = kInvalidEndpointId;
};

class DeviceScreenRegistry
{
public:
    static DeviceScreenRegistry & Instance();

    /// Where Register() reads endpoint parent relations from. The application injects the
    /// provider it registers its devices with, before the first device is created. Until then
    /// entries register without a parent.
    void SetEndpointSource(DataModel::ProviderMetadataTree * source) { mEndpointSource = source; }

    void Register(DeviceScreenEntry entry);
    const std::vector<DeviceScreenEntry> & Entries() const { return mEntries; }
    void Clear() { mEntries.clear(); }

private:
    std::vector<DeviceScreenEntry> mEntries;
    DataModel::ProviderMetadataTree * mEndpointSource = nullptr;
};

} // namespace chip::app
