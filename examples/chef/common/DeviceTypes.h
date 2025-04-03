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
namespace DeviceTypes {

// Common location to store all device type IDs
// Official list is in the spec and a complete copy is in -
// https://github.com/project-chip/connectedhomeip/blob/master/src/app/zap-templates/zcl/data-model/chip/matter-devices.xml
// TODO: Device type IDs must be code generated from matter-devices.xml
constexpr chip::DeviceTypeId kCookSurfaceDeviceId                  = 0x0077;
constexpr chip::DeviceTypeId kCooktopDeviceId                      = 0x0078;
constexpr chip::DeviceTypeId kOvenDeviceId                         = 0x007B;
constexpr chip::DeviceTypeId kRefrigeratorDeviceId                 = 0x0070;
constexpr chip::DeviceTypeId kTemperatureControlledCabinetDeviceId = 0x0071;

// Devicetype APIs

/**
 * Returns true if the endpoint has the specified device type in its device types list.
 * Device types list for the given endpoint is fetched using DataModelProvider.
 */
bool EndpointHasDeviceType(chip::EndpointId endpoint, chip::DeviceTypeId deviceTypeId);

/**
 * Returns a list of all endpoints that have the specified device type in their respective device types list.
 * Endpoints list is fetched using DataModelProvider. Device type match is checked using EndpointHasDeviceType.
 */
chip::app::DataModel::ListBuilder<chip::EndpointId> GetAllEndpointsHavingDeviceType(chip::DeviceTypeId deviceTypeId);

} // namespace DeviceTypes
} // namespace chef
