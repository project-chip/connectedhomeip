/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/af.h>

#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/enums.h>

using namespace chip;

void emberAfPumpConfigurationAndControlClusterServerInitCallback(EndpointId endpoint)
{
    emberAfDebugPrintln("Initialize PCC Server Cluster [EP:%d]", endpoint);
    // TODO
}

void emberAfPumpConfigurationAndControlClusterServerAttributeChangedCallback(EndpointId endpoint, AttributeId attributeId)
{
    emberAfDebugPrintln("PCC Server Cluster Attribute changed [EP:%d, ID:0x%x]", endpoint, attributeId);
    // TODO
}
