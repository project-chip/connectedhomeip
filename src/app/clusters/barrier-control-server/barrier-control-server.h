/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <app/util/basic-types.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// There are helper getter/setting APIs that are shared between the core
// implementation and the CLI code. They are private to the plugin.

// This will always either return the current BarrierPosition attribute value
// or assert.
uint8_t emAfPluginBarrierControlServerGetBarrierPosition(chip::EndpointId endpoint);

// This will always either set the current BarrierPosition attribute value or
// assert.
void emAfPluginBarrierControlServerSetBarrierPosition(chip::EndpointId endpoint, uint8_t barrierPosition);

// This will either return whether or not the PartialBarrier bit is set in the
// Capabilities attribute value, or it will assert.
bool emAfPluginBarrierControlServerIsPartialBarrierSupported(chip::EndpointId endpoint);

// This will increment the OpenEvents, CloseEvents, CommandOpenEvents, and
// CommandCloseEvents attribute values depending on which combination of the
// open and command arguments are passed, or assert.
void emAfPluginBarrierControlServerIncrementEvents(chip::EndpointId endpoint, bool open, bool command);

// This will read the SafetyStatus attribute and return the value, or assert.
uint16_t emAfPluginBarrierControlServerGetSafetyStatus(chip::EndpointId endpoint);

// We use a minimum delay so that our barrier changes position in a realistic
// amount of time.
#define MIN_POSITION_CHANGE_DELAY_MS 30

#endif
