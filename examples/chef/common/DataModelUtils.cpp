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

#include <DataModelUtils.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <lib/support/CodeUtils.h>

using namespace chef;
using namespace chip;
using namespace chip::app;

bool DataModelUtils::EndpointHasDeviceType(EndpointId endpoint, DeviceTypeId deviceTypeId)
{
    DataModel::ListBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    InteractionModelEngine::GetInstance()->GetDataModelProvider()->DeviceTypes(endpoint, deviceTypesList);
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

DataModel::ListBuilder<EndpointId> DataModelUtils::GetAllEndpointsHavingDeviceType(DeviceTypeId devieType)
{
    DataModel::ListBuilder<DataModel::EndpointEntry> endpointsList;
    InteractionModelEngine::GetInstance()->GetDataModelProvider()->Endpoints(endpointsList);
    auto allEndpoints = endpointsList.TakeBuffer();

    DataModel::ListBuilder<EndpointId> endpoints;

    for (const auto & ep : allEndpoints)
    {
        if (EndpointHasDeviceType(ep.id, devieType))
        {
            endpoints.Append(ep.id);
        }
    }
    return endpoints;
}
