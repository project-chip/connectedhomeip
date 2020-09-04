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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/****************************************************************************
 * @file
 * @brief APIs for the Barrier Control Server plugin.
 *******************************************************************************
 ******************************************************************************/

#ifndef BARRIER_CONTROL_SERVER_H
#define BARRIER_CONTROL_SERVER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// There are helper getter/setting APIs that are shared between the core
// implementation and the CLI code. They are private to the plugin.

// This will always either return the current BarrierPosition attribute value
// or assert.
uint8_t emAfPluginBarrierControlServerGetBarrierPosition(uint8_t endpoint);

// This will always either set the current BarrierPosition attribute value or
// assert.
void emAfPluginBarrierControlServerSetBarrierPosition(uint8_t endpoint, uint8_t barrierPosition);

// This will either return whether or not the PartialBarrier bit is set in the
// Capabilities attribute value, or it will assert.
bool emAfPluginBarrierControlServerIsPartialBarrierSupported(uint8_t endpoint);

// This will increment the OpenEvents, CloseEvents, CommandOpenEvents, and
// CommandCloseEvents attribute values depending on which combination of the
// open and command arguments are passed, or assert.
void emAfPluginBarrierControlServerIncrementEvents(uint8_t endpoint, bool open, bool command);

// This will read the SafetyStatus attribute and return the value, or assert.
uint16_t emAfPluginBarrierControlServerGetSafetyStatus(uint8_t endpoint);

// We use a minimum delay so that our barrier changes position in a realistic
// amount of time.
#define MIN_POSITION_CHANGE_DELAY_MS 30

#endif
#endif // BARRIER_CONTROL_SERVER_H
