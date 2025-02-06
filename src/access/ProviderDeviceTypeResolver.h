/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <access/AccessControl.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>

namespace chip {
namespace Access {

/// A device type resolver where the DataModel::Provider is fetched dynamically (may change over time)
class DynamicProviderDeviceTypeResolver : public chip::Access::AccessControl::DeviceTypeResolver
{
public:
    using ModelGetter = app::DataModel::Provider * (*) ();

    DynamicProviderDeviceTypeResolver(ModelGetter model) : mModelGetter(model) {}

    bool IsDeviceTypeOnEndpoint(chip::DeviceTypeId deviceType, chip::EndpointId endpoint) override
    {
        app::DataModel::ListBuilder<app::DataModel::DeviceTypeEntry> builder;
        (void) mModelGetter()->DeviceTypes(endpoint, builder);
        for (auto & type : builder.TakeBuffer())
        {
            if (type.deviceTypeId == deviceType)
            {
                return true;
            }
        }
        return false;
    }

private:
    ModelGetter mModelGetter;
};

} // namespace Access
} // namespace chip
