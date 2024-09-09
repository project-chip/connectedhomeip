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

/// test-ember-api was created to consolidate and centralize stub functions that are related to ember and are used by the unit-tests

namespace chip {
namespace Test {
extern chip::EndpointId numEndpoints;
}
} // namespace chip

// Used by the code in TestPowerSourceCluster.cpp (and generally things using mock ember functions may need this).
uint16_t emberAfGetClusterServerEndpointIndex(chip::EndpointId endpoint, chip::ClusterId cluster,
                                              uint16_t fixedClusterServerEndpointCount);
