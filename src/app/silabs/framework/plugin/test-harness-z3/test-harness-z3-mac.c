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
/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 mac layer test harness functionality
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"

#include "test-harness-z3-core.h"

// -----------------------------------------------------------------------------
// Globals

#define NEGATIVE_BEHAVIOR_BAD_NWK_PROTOCOL_VERSION (BIT32(0))
#define NEGATIVE_BEHAVIOR_NO_ROUTER_CAPACITY       (BIT32(1))
#define NEGATIVE_BEHAVIOR_NO_END_DEVICE_CAPACITY   (BIT32(2))
#define NEGATIVE_BEHAVIOR_BAD_STACK_PROFILE        (BIT32(3))
#define NEGATIVE_BEHAVIOR_BAD_PROTOCOL_ID          (BIT32(4))
#define NEGATIVE_BEHAVIOR_RESERVED_BIT_16_ENABLED  (BIT32(5))
#define NEGATIVE_BEHAVIOR_RESERVED_BIT_17_ENABLED  (BIT32(6))
#define NEGATIVE_BEHAVIOR_2_BYTE_PAYLOAD           (BIT32(7))
#define NEGATIVE_BEHAVIOR_11_BYTE_PAYLOAD          (BIT32(8))
#define NEGATIVE_BEHAVIOR_PAYLOAD_TOO_LONG         (BIT32(9))
#define NEGATIVE_BEHAVIOR_SWITCH_OFF_RADIO         (BIT32(10))

// Beacon offsets from internal stack header
// These offsets are from the start of the beacon payload
#define BEACON_NWK_PROTOCOL_ID_OFFSET  0
#define BEACON_STACK_PROFILE_OFFSET    1
#define BEACON_DEVICE_CAPACITY_OFFSET  2
#define BEACON_EXTENDED_PAN_ID_OFFSET  3
#define BEACON_TX_OFFSET_OFFSET        11
#define BEACON_NWK_UPDATE_ID_OFFSET    14
#define BEACON_PAYLOAD_SIZE            15

// Delay before turning radio off after sending a beacon
#define BEACON_RADIO_OFF_DELAY_MS 1000

static uint32_t beaconConfigNegativeBehaviorMask = 0;
EmberEventControl emberAfPluginTestHarnessZ3BeaconSendEventControl;

// -----------------------------------------------------------------------------
// Beacon CLI Commands

static void radioControl(boolean state)
{
  // Based on emberAfPluginTestHarnessRadioOnOffCommand() in Test Harness plugin
  bool radioOff = !state;
  EmberStatus status;
  if (radioOff) {
    status = emberStartScan(EMBER_START_RADIO_OFF_SCAN,
                            0,   // channels (doesn't matter)
                            0);  // duration (doesn't matter)
  } else {
    status = emberStopScan();
  }
  emberAfCorePrintln("Radio %p status: 0x%X",
                     (radioOff ? "OFF" : "ON"),
                     status);
}

// plugin test-harness z3 beacon beacon-req
void emAfPluginTestHarnessZ3BeaconBeaconReqCommand(void)
{
  EmberNetworkParameters networkParameters;
  EmberStatus status;
#ifdef EZSP_HOST
  EmberNodeType nodeType;
  status = ezspGetNetworkParameters(&nodeType, &networkParameters);
#else
  status = emberGetNetworkParameters(&networkParameters);
#endif

  if (status == EMBER_SUCCESS) {
    status = emberStartScan(EMBER_ACTIVE_SCAN,
                            BIT32(networkParameters.radioChannel),
                            2); // scan duration, whatever
  } else {
    // We probably are not on a network, so try to use the network-steering
    // channels.
    extern uint32_t emAfPluginNetworkSteeringPrimaryChannelMask;
    status = emberStartScan(EMBER_ACTIVE_SCAN,
                            emAfPluginNetworkSteeringPrimaryChannelMask,
                            2); // scan duration, whatever
  }

  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Beacon request",
                     status);
}

// plugin test-harness z3 beacon beacons-config <options:4>
void emAfPluginTestHarnessZ3BeaconBeaconsConfigCommand(void)
{
  EmberStatus status = EMBER_INVALID_CALL;

#ifndef EZSP_HOST

  uint32_t options = emberUnsignedCommandArgument(0);

  beaconConfigNegativeBehaviorMask = 0;

  if (options) {
    if (options & BIT32(0)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_NWK_PROTOCOL_VERSION;
    }
    if (options & BIT32(1)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_NO_ROUTER_CAPACITY;
    }
    if (options & BIT32(2)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_NO_END_DEVICE_CAPACITY;
    }
    if (options & BIT32(3)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_STACK_PROFILE;
    }
    if (options & BIT32(4)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_BAD_PROTOCOL_ID;
    }
    if (options & BIT32(5)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_RESERVED_BIT_16_ENABLED;
    }
    if (options & BIT32(6)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_RESERVED_BIT_17_ENABLED;
    }
    if (options & BIT32(7)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_2_BYTE_PAYLOAD;
    }
    if (options & BIT32(8)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_11_BYTE_PAYLOAD;
    }
    if (options & BIT32(9)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_PAYLOAD_TOO_LONG;
    }
    if (options & BIT32(10)) {
      beaconConfigNegativeBehaviorMask |= NEGATIVE_BEHAVIOR_SWITCH_OFF_RADIO;
    }
  }
  status = EMBER_SUCCESS;
#endif /* EZSP_HOST */

  // All options are handled in the negative behaviour callbacks
  emberAfCorePrintln("%p: %p: 0x%X",
                     TEST_HARNESS_Z3_PRINT_NAME,
                     "Beacon",
                     status);
}

void emberAfPluginTestHarnessZ3BeaconSendEventHandler(void)
{
  emberEventControlSetInactive(emberAfPluginTestHarnessZ3BeaconSendEventControl);
  emberAfDebugPrintln("SendEventHandler - Switching radio off");
  radioControl(false); // switch radio off
}

// Allow the framework to modify the beacon, for negative conformance test cases.
// The function is called from our implementation of emberAfOutgoingPacketFilterCallback.
EmberPacketAction emAfPluginTestHarnessZ3ModifyBeaconPayload(uint8_t *beaconPayload, uint8_t *payloadLength)
{
  if (beaconConfigNegativeBehaviorMask) {
    emberAfDebugPrintln("Modifying beacon, mask = %4X", beaconConfigNegativeBehaviorMask);

    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_BAD_NWK_PROTOCOL_VERSION) {
      beaconPayload[BEACON_NWK_PROTOCOL_ID_OFFSET] = 0x08;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_NO_ROUTER_CAPACITY) {
      beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] &= ~0x04;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_NO_END_DEVICE_CAPACITY) {
      beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] &= ~0x80;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_BAD_STACK_PROFILE) {
      beaconPayload[BEACON_STACK_PROFILE_OFFSET] = (beaconPayload[BEACON_STACK_PROFILE_OFFSET] & 0xf0) | 0x03;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_BAD_PROTOCOL_ID) {
      beaconPayload[BEACON_STACK_PROFILE_OFFSET] = (beaconPayload[BEACON_STACK_PROFILE_OFFSET] & 0x0f) | 0x10;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_RESERVED_BIT_16_ENABLED) {
      beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] = (beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] & 0xfc) | 0x01;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_RESERVED_BIT_17_ENABLED) {
      beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] = (beaconPayload[BEACON_DEVICE_CAPACITY_OFFSET] & 0xfc) | 0x02;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_2_BYTE_PAYLOAD) {
      *payloadLength = 2;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_11_BYTE_PAYLOAD) {
      *payloadLength = 11;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_PAYLOAD_TOO_LONG) {
      assert(EXTRA_COMMAND_BYTES >= 5);
      MEMSET(beaconPayload + *payloadLength, 0x00, 5);
      *payloadLength += 5;
    }
    if (beaconConfigNegativeBehaviorMask & NEGATIVE_BEHAVIOR_SWITCH_OFF_RADIO) {
      emberAfDebugPrintln("Setting Send event active");
      emberEventControlSetDelayMS(emberAfPluginTestHarnessZ3BeaconSendEventControl,
                                  BEACON_RADIO_OFF_DELAY_MS);
    }
    return EMBER_MANGLE_PACKET;
  } else {
    return EMBER_ACCEPT_PACKET;
  }
}
