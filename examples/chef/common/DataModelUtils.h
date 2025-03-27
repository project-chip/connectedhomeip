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

#include <app/data-model-provider/MetadataList.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chef {
namespace DataModelUtils {

// Common location to store all device type IDs
constexpr DeviceTypeId kCookSurfaceDeviceId                  = 0x0077;
constexpr DeviceTypeId kCooktopDeviceId                      = 0x0078;
constexpr DeviceTypeId kOvenDeviceId                         = 0x007B;
constexpr DeviceTypeId kRefrigeratorDeviceId                 = 0x0070;
constexpr DeviceTypeId kTemperatureControlledCabinetDeviceId = 0x0071;

// Datamodel Util APIs
bool EndpointHasDeviceType(EndpointId endpoint, DeviceTypeId deviceTypeId);
DataModel::ListBuilder<EndpointId> GetAllEndpointsHavingDeviceType(DeviceTypeId devieType);

} // namespace DataModelUtils
} // namespace chef
