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

#include <app/clusters/temperature-control-server/TemperatureControlCluster.h>

namespace chip::app::Clusters::TemperatureControl {

TemperatureControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

CHIP_ERROR SetTemperatureSetpoint(EndpointId endpointId, int16_t temperatureSetpoint);

SupportedTemperatureLevelsIteratorDelegate * GetDelegate();
void SetDelegate(SupportedTemperatureLevelsIteratorDelegate * delegate);

} // namespace chip::app::Clusters::TemperatureControl
