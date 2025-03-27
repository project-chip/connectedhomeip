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

using namespace chip::app;

CHIP_ERROR DataModelUtils::isEndpointOfDeviceType(EndpointId endpoint, DeviceTypeId deviceTypeId, bool & isOfDeviceType)
{
    isOfDeviceType = false;
    DataModel::ListBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->DeviceTypes(endpoint, deviceTypesList));
    auto deviceTypes = deviceTypesList.TakeBuffer();
    for (const auto & type : deviceTypes)
    {
        if (type.deviceTypeId == deviceTypeId)
        {
            isOfDeviceType = true;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DataModelUtils::getAllEndpointsHavingDeviceType(DeviceTypeId devieType, DataModel::ListBuilder<EndpointId> & builder)
{

    DataModel::ListBuilder<DataModel::EndpointEntry> endpointsList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->Endpoints(endpointsList));
    auto allEndpoints = endpointsList.TakeBuffer();

    for (const auto & ep : allEndpoints)
    {
        bool isOfDeviceType;
        ReturnErrorOnFailure(isEndpointOfDeviceType(ep, devieType, isOfDeviceType));
        if (isOfDeviceType)
        {
            ReturnErrorOnFailure(builder.Append(ep));
        }
    }
    return CHIP_NO_ERROR;
}
