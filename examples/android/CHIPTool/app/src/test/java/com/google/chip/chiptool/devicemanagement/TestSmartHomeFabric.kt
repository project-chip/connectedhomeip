/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool.devicemanagement

/** Multi-device fabric used by Device Center unit tests. Not shipped in the app. */
internal val TEST_SMART_HOME_NODES: List<CommissionedNode> =
  listOf(
    // 1. Living Room Color Light
    CommissionedNode(
      nodeId = 0x1001L,
      nodeLabel = "Living Room Ceiling Light",
      roomName = "Living Room",
      productName = "Matter Smart Color Bulb",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.EXTENDED_COLOR_LIGHT,
            deviceTypeName = "Extended Color Light",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL,
                MatterClusterMetaRegistry.CLUSTER_COLOR_CONTROL,
              ),
          )
        ),
    ),
    // 2. Kitchen Dimmer Light
    CommissionedNode(
      nodeId = 0x1002L,
      nodeLabel = "Kitchen Counter Light",
      roomName = "Kitchen",
      productName = "Matter Dimmable Spotlight",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.DIMMABLE_LIGHT,
            deviceTypeName = "Dimmable Light",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_ON_OFF,
                MatterClusterMetaRegistry.CLUSTER_LEVEL_CONTROL,
              ),
          )
        ),
    ),
    // 3. Front Door Smart Lock
    CommissionedNode(
      nodeId = 0x2001L,
      nodeLabel = "Front Door Deadbolt",
      roomName = "Entryway",
      productName = "Matter Smart Deadbolt Lock",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.DOOR_LOCK,
            deviceTypeName = "Door Lock",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_DOOR_LOCK),
          )
        ),
    ),
    // 4. Living Room Window Shades
    CommissionedNode(
      nodeId = 0x2002L,
      nodeLabel = "Living Room Blinds",
      roomName = "Living Room",
      productName = "Motorized Roller Shades",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.WINDOW_COVERING,
            deviceTypeName = "Window Covering",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_WINDOW_COVERING),
          )
        ),
    ),
    // 5. Garage Door Barrier
    CommissionedNode(
      nodeId = 0x2003L,
      nodeLabel = "Garage Door",
      roomName = "Garage",
      productName = "Smart Garage Door Opener",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = 0x0103L,
            deviceTypeName = "Barrier Control",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_BARRIER_CONTROL),
          )
        ),
    ),
    // 6. Hallway Smart Thermostat
    CommissionedNode(
      nodeId = 0x3001L,
      nodeLabel = "Main Thermostat",
      roomName = "Hallway",
      productName = "Matter Smart Climate Controller",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.THERMOSTAT,
            deviceTypeName = "Thermostat",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_THERMOSTAT),
          )
        ),
    ),
    // 7. Bedroom Ceiling Fan
    CommissionedNode(
      nodeId = 0x3002L,
      nodeLabel = "Bedroom Ceiling Fan",
      roomName = "Bedroom",
      productName = "Smart Fan Controller",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.FAN,
            deviceTypeName = "Fan",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_FAN_CONTROL),
          )
        ),
    ),
    // 8. Living Room Smart TV
    CommissionedNode(
      nodeId = 0x4001L,
      nodeLabel = "Living Room TV",
      roomName = "Living Room",
      productName = "Matter Connected Smart TV",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.BASIC_VIDEO_PLAYER,
            deviceTypeName = "Basic Video Player",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_MEDIA_PLAYBACK,
                MatterClusterMetaRegistry.CLUSTER_KEYPAD_INPUT,
                MatterClusterMetaRegistry.CLUSTER_AUDIO_OUTPUT,
              ),
          )
        ),
    ),
    // 9. Robot Vacuum Cleaner
    CommissionedNode(
      nodeId = 0x5001L,
      nodeLabel = "RoboVac",
      roomName = "Downstairs",
      productName = "Matter Autonomous Robot Vacuum",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.ROBOTIC_VACUUM_CLEANER,
            deviceTypeName = "Robotic Vacuum Cleaner",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_RVC_RUN_MODE,
                MatterClusterMetaRegistry.CLUSTER_RVC_CLEAN_MODE,
                MatterClusterMetaRegistry.CLUSTER_OPERATIONAL_STATE,
              ),
          )
        ),
    ),
    // 10. Laundry Washing Machine
    CommissionedNode(
      nodeId = 0x5002L,
      nodeLabel = "Washing Machine",
      roomName = "Laundry Room",
      productName = "Smart Front-Load Washer",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = 0x0075L,
            deviceTypeName = "Smart Washer",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_OPERATIONAL_STATE,
                MatterClusterMetaRegistry.CLUSTER_MODE_SELECT,
              ),
          )
        ),
    ),
    // 11. Garage EV Charger
    CommissionedNode(
      nodeId = 0x6001L,
      nodeLabel = "EV Home Charger",
      roomName = "Garage",
      productName = "Level 2 Matter EVSE",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.EVSE,
            deviceTypeName = "Energy EVSE",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_ENERGY_EVSE,
                MatterClusterMetaRegistry.CLUSTER_DEVICE_ENERGY_MGMT,
              ),
          )
        ),
    ),
    // 12. Living Room Environmental Sensor
    CommissionedNode(
      nodeId = 0x7001L,
      nodeLabel = "Living Room Climate Sensor",
      roomName = "Living Room",
      productName = "Multi-Sensor (Temp, Humidity, Motion)",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.TEMPERATURE_SENSOR,
            deviceTypeName = "Temperature & Humidity Sensor",
            serverClusters =
              setOf(
                MatterClusterMetaRegistry.CLUSTER_TEMPERATURE_MEASUREMENT,
                MatterClusterMetaRegistry.CLUSTER_RELATIVE_HUMIDITY_MEASUREMENT,
                MatterClusterMetaRegistry.CLUSTER_OCCUPANCY_SENSING,
              ),
          )
        ),
    ),
    // 13. Front Window Contact Sensor
    CommissionedNode(
      nodeId = 0x7002L,
      nodeLabel = "Front Window Sensor",
      roomName = "Living Room",
      productName = "Door/Window Contact Sensor",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.CONTACT_SENSOR,
            deviceTypeName = "Contact Sensor",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_BOOLEAN_STATE),
          )
        ),
    ),
    // 14. Kitchen Smoke & CO Alarm
    CommissionedNode(
      nodeId = 0x7003L,
      nodeLabel = "Kitchen Smoke Detector",
      roomName = "Kitchen",
      productName = "Smart Smoke & CO Alarm",
      endpoints =
        listOf(
          CommissionedEndpoint(
            endpointId = 1,
            deviceTypeId = MatterDeviceTypes.SMOKE_CO_ALARM,
            deviceTypeName = "Smoke CO Alarm",
            serverClusters = setOf(MatterClusterMetaRegistry.CLUSTER_SMOKE_CO_ALARM),
          )
        ),
    ),
  )

/** Registers [TEST_SMART_HOME_NODES] in this registry, replacing any existing nodes. */
internal fun CommissionedNodeRegistry.loadTestSmartHomeFabric() {
  clear()
  TEST_SMART_HOME_NODES.forEach { registerNode(it) }
}
