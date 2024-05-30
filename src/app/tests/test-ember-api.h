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
#include <app/util/basic-types.h>

/// test-ember-api was created to consolidate and centralize stub functions that are related to ember and are run for unit-tests
/// These were previously defined in the unit test-scripts themselves.

namespace chip {
namespace Test {
extern chip::EndpointId numEndpoints;
}
} // namespace chip

// was previously in TestPowerSourceCluster.cpp
uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                              uint16_t fixedClusterServerEndpointCount);
