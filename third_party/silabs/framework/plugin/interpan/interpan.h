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
 * @brief Plugin for receiving InterPAN messages.
 * See @ref message for documentation.
 *******************************************************************************
   ******************************************************************************/

// The three types of inter-PAN messages.  The values are actually the
// corresponding APS frame controls.
//
// 0x03 is the special interPAN message type.  Unicast mode is 0x00,
// broadcast mode is 0x08, and multicast mode is 0x0C.
//

// MAC Frame Max size
// - Frame control   (2-bytes)
// - Sequence        (1-byte)
// - Dest PAN ID     (2-bytes)
// - Dest long       (8-bytes)
// - Source PAN ID   (2-bytes)
// - Source long     (8-bytes)
#define MAX_INTER_PAN_MAC_SIZE 23

// NWK stub frame has two control bytes.
#define STUB_NWK_SIZE 2
#define STUB_NWK_FRAME_CONTROL 0x000B

// Interpan APS Unicast
//  - Frame Control   (1-byte)
//  - Cluster ID      (2-bytes)
//  - Profile ID      (2-bytes)
#define INTERPAN_APS_UNICAST_SIZE 5

// Interpan APS Broadcast, same as unicast
#define INTERPAN_APS_BROADCAST_SIZE 5

// Interpan APS Multicast
//  - Frame Control   (1-byte)
//  - Group ID        (2-bytes)
//  - Cluster ID      (2-bytes)
//  - Profile ID      (2-bytes)
#define INTERPAN_APS_MULTICAST_SIZE  7

#define MAX_STUB_APS_SIZE (INTERPAN_APS_MULTICAST_SIZE)
#define MIN_STUB_APS_SIZE (INTERPAN_APS_UNICAST_SIZE)

#define INTERPAN_UNICAST_HEADER_SIZE \
  (MAX_INTER_PAN_MAC_SIZE + STUB_NWK_SIZE + INTERPAN_APS_UNICAST_SIZE)

#define INTERPAN_MULTICAST_HEADER_SIZE \
  (MAX_INTER_PAN_MAC_SIZE + STUB_NWK_SIZE + INTERPAN_APS_MULTICAST_SIZE)

#define MAX_INTER_PAN_HEADER_SIZE   (INTERPAN_MULTICAST_HEADER_SIZE)

#define INTERPAN_APS_FRAME_TYPE      0x03
#define INTERPAN_APS_FRAME_TYPE_MASK 0x03

// The only allowed APS FC value (without the delivery mode subfield)
#define INTERPAN_APS_FRAME_CONTROL_NO_DELIVERY_MODE (INTERPAN_APS_FRAME_TYPE)

#define INTERPAN_APS_FRAME_DELIVERY_MODE_MASK 0x0C
#define INTERPAN_APS_FRAME_SECURITY           0x20

// 5 byte AUX header + 4 byte MIC
#define INTERPAN_APS_ENCRYPTION_OVERHEAD      (5 + 4)

// Control byte (1), Index IPMF (1), Number IPMF (1), Len IPMF (1)
#define EMBER_APS_INTERPAN_FRAGMENTATION_OVERHEAD           4
// Control byte (1), Index IPMF (1), Response (1)
#define EMBER_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN       3
#define EMBER_APS_INTERPAN_FRAGMENT_MIN_LEN \
  EMBER_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN

#define INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_INDEX   0
#define INTERPAN_FRAGMENTATION_APS_INDEX_IPMF_INDEX     1
#define INTERPAN_FRAGMENTATION_APS_IPMF_RESPONSE_INDEX  2 // IPMF response only
#define INTERPAN_FRAGMENTATION_APS_LEN_IPMF_INDEX       3 // IPMF only
#define INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL            0x00
#define INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL   0x80

#define INTERPAN_IPMF_RESPONSE_SUCCESS    0x00
#define INTERPAN_IPMF_RESPONSE_FAILURE    0x01

#define EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_PAYLOAD_SIZE   1500

// NOTE: unlike regular frags, which rely on APS sequence numbers in the ACK to
// know when to send the next fragment, interpan frags do not have APS ACKs.
// The only way to distinguish is to judge based on the partner ID. For any
// partner, only 1 packet can be transmitted to or received from at a time
#define EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS 1
#define EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS 1
#define EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE \
  (EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_PAYLOAD_SIZE + INTERPAN_UNICAST_HEADER_SIZE)

#define EMBER_AF_PLUGIN_INTERPAN_FILTER_LIST        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_BROADCAST_SHORT \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_SHORT   \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_LONG    \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_BROADCAST_SHORT \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_SHORT   \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_LOCAL       \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_LONG    \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_BROADCAST_SHORT \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_SHORT   \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_NON_LOCAL \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_LONG    \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_BROADCAST_SHORT \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_SHORT   \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \
  (EMBER_MAC_FILTER_MATCH_ENABLED                   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_DEST_BROADCAST   \
   | EMBER_MAC_FILTER_MATCH_ON_PAN_SOURCE_LOCAL     \
   | EMBER_MAC_FILTER_MATCH_ON_DEST_UNICAST_LONG    \
   | EMBER_MAC_FILTER_MATCH_ON_SOURCE_LONG),        \

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)

#define UNUSED_TX_PACKET_ENTRY 0xFF
typedef struct {
  EmberOutgoingMessageType  messageType;
  EmberEUI64                destEui;
  uint8_t                     buffer[EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE]; // Whole message, including hdrs
  uint16_t                    bufLen;             // Whole message size
  uint8_t                     fragmentMaxLen;     // Max possible frag length
  uint8_t                     numFragments;       // Number of frags to be sent
  uint8_t                     fragmentNum;        // Running # of which frag is TX
  EmberEventControl*        eventControl;
} txFragmentedInterpanPacket;

typedef struct {
  EmberEUI64         sourceEui;
  uint8_t              buffer[EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE];
  uint16_t             bufLen;
  uint8_t              numFragments;
  uint8_t              lastFragmentNumReceived;
  EmberEventControl* eventControl;
} rxFragmentedInterpanPacket;

void interpanPluginSetFragmentMessageTimeout(uint16_t timeout); // seconds

#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

typedef enum {
  IPMF_SUCCESS = 0,
  IPMF_TX_TIMEOUT,        // IPMF response not received
  IPMF_RX_TIMEOUT,        // Next IPMF not received
  IPMF_TX_BAD_RESPONSE,   // Bad IPMF header data
  IPMF_RX_BAD_RESPONSE    // Bad IPMF header data
} EInterpanFragmentationStatus;

void interpanPluginInit();

void interpanPluginSetMacMatchFilterEnable(bool enable);

bool emAfPluginInterpanProcessMessage(uint8_t messageLength,
                                      uint8_t *messageContents);

EmberStatus emAfPluginInterpanSendRawMessage(uint8_t length, uint8_t* message);

EmberStatus emAfInterpanApsCryptMessage(bool encrypt,
                                        uint8_t* apsFrame,
                                        uint8_t* messageLength,
                                        uint8_t apsHeaderLength,
                                        EmberEUI64 remoteEui64);

void emAfPluginInterpanSetEnableState(bool enable);
