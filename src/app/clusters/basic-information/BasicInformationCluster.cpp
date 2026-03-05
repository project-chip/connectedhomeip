/*
 *    Copyright (c) 2020-2026 Project CHIP Authors
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

#include <app/clusters/basic-information/BasicInformationCluster.h>

namespace chip {
namespace app {
namespace Clusters {

// Explicit instantiation of the BasicInformationCluster logic for the DeviceLayer policy.
// This ensures that the code is generated in this object file, preventing code bloat
// from header-only instantiation in multiple translation units.
template class BasicInformationClusterImpl<DeviceLayerBasicInformationPolicy>;

} // namespace Clusters
} // namespace app
} // namespace chip
