/*
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <app/clusters/basic-information/DeviceLayerBasicInformationPolicy.h>
#include <app/clusters/basic-information/PolicyBased.h>

namespace chip::app::Clusters {

// Default alias for out-of-the-box SDK usage, binding the cluster to the standard
// DeviceLayer policy (ConfigurationManager, DeviceInstanceInfoProvider, PlatformManager).
//
// Integrators that only need a subset of these capabilities, or that want to avoid
// pulling in the full DeviceLayer dependency, should instantiate BasicInformation::PolicyBased
// directly with a custom policy instead.
using BasicInformationCluster = BasicInformation::PolicyBased<DeviceLayerBasicInformationPolicy>;

} // namespace chip::app::Clusters
