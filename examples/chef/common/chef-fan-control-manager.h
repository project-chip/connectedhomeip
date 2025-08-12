/*
 *
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

#include <lib/core/DataModelTypes.h>

#ifdef MATTER_DM_PLUGIN_FAN_CONTROL_SERVER
void HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value);

#ifdef MATTER_DM_PLUGIN_ON_OFF_SERVER
void HandleOnOffAttributeChangeForFan(EndpointId endpointId, bool value);
#endif // MATTER_DM_PLUGIN_ON_OFF_SERVER

#endif
