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
#pragma once

#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster.h>

namespace chip::app::Clusters::ValveConfigurationAndControl {

ValveConfigurationAndControlCluster * FindClusterOnEndpoint(EndpointId endpointId);

/**
 * The SetDelegate function relies on the ValveConfigurationAndControl being initialized in the 
 * given endpoint.
 */
void SetDefaultDelegate(EndpointId endpointId, Delegate * delegate);


CHIP_ERROR CloseValve(chip::EndpointId ep);
CHIP_ERROR UpdateCurrentLevel(chip::EndpointId ep, chip::Percent currentLevel);
CHIP_ERROR UpdateCurrentState(chip::EndpointId ep, ValveConfigurationAndControl::ValveStateEnum currentState);
CHIP_ERROR EmitValveFault(chip::EndpointId ep, chip::BitMask<ValveConfigurationAndControl::ValveFaultBitmap> fault);
void UpdateAutoCloseTime(uint64_t time);

} // namespace chip::app::Clusters::ValveConfigurationAndControl
