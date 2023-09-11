/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/af-types.h>
#include <app/util/basic-types.h>

/** @brief Occupancy Cluster Server Post Init
 *
 * Following resolution of the Occupancy state at startup for this endpoint,
 * perform any additional initialization needed; e.g., synchronize hardware
 * state.
 *
 * @param endpoint Endpoint that is being initialized  Ver.: always
 */
void emberAfPluginOccupancyClusterServerPostInitCallback(chip::EndpointId endpoint);
