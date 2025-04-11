/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "DeviceTypes.h"
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/CodeUtils.h>

using namespace chef;
using namespace chip;
using namespace chip::app;

bool DeviceTypes::EndpointHasDeviceType(EndpointId endpoint, DeviceTypeId deviceTypeId)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    CHIP_ERROR err = InteractionModelEngine::GetInstance()->GetDataModelProvider()->DeviceTypes(endpoint, deviceTypesList);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetDataModelProvider DeviceTypes returned error: %" CHIP_ERROR_FORMAT, err.Format());
        return false;
    }
    auto deviceTypes = deviceTypesList.TakeBuffer();
    for (const auto & type : deviceTypes)
    {
        if (type.deviceTypeId == deviceTypeId)
        {
            return true;
        }
    }
    return false;
}

ReadOnlyBuffer<EndpointId> DeviceTypes::GetAllEndpointsHavingDeviceType(DeviceTypeId deviceTypeId)
{
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsList;
    CHIP_ERROR err = InteractionModelEngine::GetInstance()->GetDataModelProvider()->Endpoints(endpointsList);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetDataModelProvider Endpoints returned error: %" CHIP_ERROR_FORMAT, err.Format());
        return {};
    }
    auto allEndpoints = endpointsList.TakeBuffer();

    ReadOnlyBufferBuilder<EndpointId> endpoints;
    err = endpoints.EnsureAppendCapacity(allEndpoints.size());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error ensuring append capacity for endpoints: %" CHIP_ERROR_FORMAT, err.Format());
    }

    for (const auto & ep : allEndpoints)
    {
        if (EndpointHasDeviceType(ep.id, deviceTypeId))
        {
            err = endpoints.Append(ep.id);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Error appending endpoint: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
    }
    return endpoints.TakeBuffer();
}
