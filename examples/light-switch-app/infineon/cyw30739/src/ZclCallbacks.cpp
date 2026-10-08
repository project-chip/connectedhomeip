/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

// Invoked only for attribute writes that go through the ember attribute table. Clusters migrated to
// the code-driven model handle writes themselves and never reach here, so this app has nothing to
// dispatch: every cluster it exposes is code-driven. The hook is kept as a starting point for apps
// that still have ember-backed clusters; add a `case <Cluster>::Id:` and read the written value from
// the `value` argument rather than reading the attribute back.
void MatterPostAttributeChangeCallback(const app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    printf("Unhandled cluster ID: 0x%04lx\n", attributePath.mClusterId);
}
