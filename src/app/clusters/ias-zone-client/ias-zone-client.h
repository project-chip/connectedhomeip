/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/util/af-types.h>
#include <app/util/basic-types.h>

typedef struct
{
    EmberEUI64 ieeeAddress;
    chip::NodeId nodeId;
    uint16_t zoneType;
    uint16_t zoneStatus;
    uint8_t zoneState;
    chip::EndpointId endpoint;
    uint8_t zoneId;
} IasZoneDevice;

extern IasZoneDevice emberAfIasZoneClientKnownServers[];

#define NO_INDEX 0xFF
#define UNKNOWN_ENDPOINT 0

#define UNKNOWN_ZONE_ID 0xFF

void emAfClearServers(void);

void emberAfPluginIasZoneClientZdoCallback(EmberNodeId emberNodeId, EmberApsFrame * apsFrame, uint8_t * message, uint16_t length);

void emberAfPluginIasZoneClientWriteAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

void emberAfPluginIasZoneClientReadAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);
