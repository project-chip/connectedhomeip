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
/***************************************************************************//**
 * @file
 * @brief ZigBee 3.0 zdo test harness functionality
 *******************************************************************************
   ******************************************************************************/

#define BEACON_REQUEST_COMMAND        (0x07)
#define BEACON_ASSOCIATION_PERMIT_BIT (0x80)
#define BEACON_HEADER_SIZE            (7)
#define BEACON_EXTENDED_PAN_ID_INDEX  (7)

// Zigbee part of MAC Payload, AKA the 'Beacon Payload'
#define BEACON_PAYLOAD_SIZE           (15)

// The size of a 802.15.4 MAC beacon payload in bytes.
#define MAC_BEACON_SIZE (4)

EmberPacketAction emAfPluginTestHarnessZ3ModifyBeaconPayload(uint8_t *beaconPayload,
                                                             uint8_t *payloadLength);
