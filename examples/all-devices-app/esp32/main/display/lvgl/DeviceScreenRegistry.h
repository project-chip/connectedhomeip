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
#include <mutex>
#include <string>
#include <vector>

#include <app/data-model-provider/ProviderMetadataTree.h>
#include <lib/core/DataModelTypes.h>
#include <lvgl.h>

namespace chip::app {

struct DeviceScreenEntry
{
    using RenderFn = std::function<void(lv_obj_t * parent)>;

    std::string title;
    EndpointId endpointId;
    RenderFn renderFn;

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

    /// Runs `visitor` against the entry list with the registry locked. Devices register from
    /// the CHIP thread while the display task renders, so the list must not be read without
    /// this: appending an entry can reallocate it. The reference passed to the visitor must
    /// not outlive the call.
    template <typename Visitor>
    void WithEntries(Visitor && visitor) const
    {
        std::lock_guard<std::mutex> guard(mEntriesMutex);
        visitor(static_cast<const std::vector<DeviceScreenEntry> &>(mEntries));
    }

private:
    // Guards mEntries, and only mEntries: it is the one member written from the CHIP thread
    // (Register) while the display task reads it. mEndpointSource is set once during startup,
    // before the first registration, and is only ever read on the CHIP thread.
    //
    // Leaf lock: nothing is acquired while it is held, so it cannot deadlock against the
    // display or stack locks. Callers must not re-enter the registry from a visitor.
    mutable std::mutex mEntriesMutex;
    std::vector<DeviceScreenEntry> mEntries;
    DataModel::ProviderMetadataTree * mEndpointSource = nullptr;
};

} // namespace chip::app
