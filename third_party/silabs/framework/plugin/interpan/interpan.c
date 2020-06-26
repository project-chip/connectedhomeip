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
 * @brief Generic code related to the receipt and processing of interpan
 * messages.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/util/util.h"
#include "interpan.h"

//------------------------------------------------------------------------------
// Globals

// Global enable. Defaults to true. User can set to false if s/he wishes.
// Example: user will set to false for SE 1.2 Comms Hub (GBCS)
static bool gInterpanEnabled;

// Global timeout value in seconds
static uint16_t gMessageTimeout;

// MAC frame control
// Bits:
// | 0-2   |   3      |    4    |  5  |  6    |   7-9    | 10-11 |  12-13   | 14-15 |
// | Frame | Security |  Frame  | Ack | Intra | Reserved | Dest. | Reserved | Src   |
// | Type  | Enabled  | Pending | Req | PAN   |          | Addr. |          | Adrr. |
// |       |          |         |     |       |          | Mode  |          | Mode  |

// Frame Type
//   000       = Beacon
//   001       = Data
//   010       = Acknwoledgement
//   011       = MAC Command
//   100 - 111 = Reserved

// Addressing Mode
//   00 - PAN ID and address field are not present
//   01 - Reserved
//   10 - Address field contains a 16-bit short address
//   11 - Address field contains a 64-bit extended address

#define MAC_FRAME_TYPE_DATA           0x0001

#define MAC_SOURCE_ADDRESS_MODE_SHORT 0x8000
#define MAC_SOURCE_ADDRESS_MODE_LONG  0xC000

#define MAC_DEST_ADDRESS_MODE_SHORT   0x0800
#define MAC_DEST_ADDRESS_MODE_LONG    0x0C00

// The two possible incoming MAC frame controls.
// Using short source address is not allowed.
#define SHORT_DEST_FRAME_CONTROL (MAC_FRAME_TYPE_DATA           \
                                  | MAC_DEST_ADDRESS_MODE_SHORT \
                                  | MAC_SOURCE_ADDRESS_MODE_LONG)
#define LONG_DEST_FRAME_CONTROL  (MAC_FRAME_TYPE_DATA          \
                                  | MAC_DEST_ADDRESS_MODE_LONG \
                                  | MAC_SOURCE_ADDRESS_MODE_LONG)

#define MAC_ACK_REQUIRED              0x0020

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES)
  #define APS_ENCRYPTION_ALLOWED 1
#else
  #define APS_ENCRYPTION_ALLOWED 0
#endif

#define PRIMARY_ENDPOINT   0
#define ALL_ENDPOINTS      1
#define SPECIFIED_ENDPOINT 2

#if defined(EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO)
  #if EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO == PRIMARY_ENDPOINT
    #define ENDPOINT emberAfPrimaryEndpoint()

  #elif EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO == ALL_ENDPOINTS
    #define ENDPOINT EMBER_BROADCAST_ENDPOINT

  #elif EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO == SPECIFIED_ENDPOINT
    #define ENDPOINT EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO_SPECIFIED_ENDPOINT_VALUE

  #else
    #error Unknown value for interpan plugin option: EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO
  #endif
#else
  #error Invalid Interpan plugin Config: EMBER_AF_PLUGIN_INTERPAN_DELIVER_TO not defined.
#endif

//------------------------------------------------------------------------------
// Forward Declarations

static bool isMessageAllowed(EmberAfInterpanHeader *headerData,
                             uint8_t messageLength,
                             uint8_t *messageContents);

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)
static void printMessage(EmberAfInterpanHeader *headerData);
#else
  #define printMessage(x)
#endif

#if !defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES)
  #define handleApsSecurity(...) (EMBER_LIBRARY_NOT_PRESENT)
#endif

#if !defined EMBER_AF_PLUGIN_INTERPAN_CUSTOM_FILTER
  #define EMBER_AF_PLUGIN_INTERPAN_CUSTOM_FILTER
#endif

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
EmberStatus interpanFragmentationSendUnicast(EmberAfInterpanHeader* header, uint8_t* message, uint16_t messageLen);
bool isInterpanFragment(uint8_t* payload, uint8_t payloadLen);
EmberStatus interpanFragmentationSendIpmf(txFragmentedInterpanPacket* txPacket);
EmberStatus interpanFragmentationProcessIpmf(EmberAfInterpanHeader header, uint8_t *message, uint8_t messageLen);
void interpanFragmentationSendIpmfResponse(EmberAfInterpanHeader header, uint8_t fragNum, uint8_t responseStatus);
void interpanFragmentationProcessIpmfResponse(EmberAfInterpanHeader header, uint8_t* payload, uint8_t payloadLen);

static txFragmentedInterpanPacket txPackets[EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS];
static rxFragmentedInterpanPacket rxPackets[EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS];
static txFragmentedInterpanPacket* getFreeTxPacketEntry(EmberEUI64 destinationEui);
static rxFragmentedInterpanPacket* getFreeRxPacketEntry(EmberEUI64 sourceEui);
static txFragmentedInterpanPacket* txPacketLookUp(EmberEUI64 destEui);
static rxFragmentedInterpanPacket* rxPacketLookUp(EmberEUI64 sourceEui);
static void freeTxPacketEntry(txFragmentedInterpanPacket *txPacket, EInterpanFragmentationStatus status);
static void freeRxPacketEntry(rxFragmentedInterpanPacket *rxPacket, EInterpanFragmentationStatus status);
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

// These have to stay outside the ifdef interpan frag check because AppBuilder
// generates code that calls them, which cannot be ifdeffed
EmberEventControl emberAfPluginInterpanFragmentTransmitEventControl;
EmberEventControl emberAfPluginInterpanFragmentReceiveEventControl;
void emberAfPluginInterpanFragmentTransmitEventHandler(EmberEventControl *control);
void emberAfPluginInterpanFragmentReceiveEventHandler(EmberEventControl *control);
#define IPMF_MSG_TIMEOUT_MS       60 * 1000

//------------------------------------------------------------------------------

// Inter-PAN messages are a security risk because they are typically not
// encrypted and yet we must accept them.  Therefore, we are very strict about
// what we allow.  Listed below are ZCL messages that are allowed, including
// the profile, cluster, direction, and command id.  Any inter-PAN messages not
// matching one of these are dropped.
// If inter-PAN fragments (IPMF) are received, a different set of checks is
// performed.
static const EmberAfAllowedInterPanMessage messages[] = {
  // A custom filter can be added using this #define.
  // It should contain one or more 'EmberAfAllowedInterPanMessage' const structs
  EMBER_AF_PLUGIN_INTERPAN_CUSTOM_FILTER

#if defined (EMBER_AF_PLUGIN_INTERPAN_ALLOW_REQUIRED_SMART_ENERGY_MESSAGES)

#ifdef ZCL_USING_PRICE_CLUSTER_SERVER
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_GET_CURRENT_PRICE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_GET_SCHEDULED_PRICES_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
#endif

#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_REQUIRED_SMART_ENERGY_MESSAGES

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_SMART_ENERGY_RESPONSE_MESSAGES)

#ifdef ZCL_USING_PRICE_CLUSTER_CLIENT
  {
    SE_PROFILE_ID,
    ZCL_PRICE_CLUSTER_ID,
    ZCL_PUBLISH_PRICE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
#endif
#ifdef ZCL_USING_MESSAGING_CLUSTER_CLIENT
  {
    SE_PROFILE_ID,
    ZCL_MESSAGING_CLUSTER_ID,
    ZCL_DISPLAY_MESSAGE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
  {
    SE_PROFILE_ID,
    ZCL_MESSAGING_CLUSTER_ID,
    ZCL_CANCEL_MESSAGE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT,
  },
#endif

#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_SMART_ENERGY_RESPONSE_MESSAGES

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_KEY_ESTABLISHMENT)
  // Since client and server share the same command ID's,
  // we can get away with only listing the request messages.
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_INITIATE_KEY_ESTABLISHMENT_REQUEST_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_EPHEMERAL_DATA_RESPONSE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_CONFIRM_KEY_DATA_RESPONSE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_TERMINATE_KEY_ESTABLISHMENT_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_BOTH,
  },
  {
    SE_PROFILE_ID,
    ZCL_KEY_ESTABLISHMENT_CLUSTER_ID,
    ZCL_DEFAULT_RESPONSE_COMMAND_ID,
    EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER,
  },
#endif // defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_KEY_ESTABLISHMENT)

  { 0xFFFF }, // terminator
};

//#define ENCRYPT_DEBUG

#if !defined(ENCRYPT_DEBUG)
  #define printData(...)
#endif

// Private ZLL API's
bool emAfPluginZllCommissioningClientInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader *header,
                                                                        uint8_t msgLen,
                                                                        uint8_t *message);
bool emAfPluginZllCommissioningServerInterpanPreMessageReceivedCallback(const EmberAfInterpanHeader *header,
                                                                        uint8_t msgLen,
                                                                        uint8_t *message);

//------------------------------------------------------------------------------

void interpanPluginInit()
{
  gInterpanEnabled = true;
  gMessageTimeout  = IPMF_MSG_TIMEOUT_MS;

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  uint8_t i;
  // The following two loops need adjustment if more than one packet is desired
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txPackets[i].messageType  = UNUSED_TX_PACKET_ENTRY;
    txPackets[i].eventControl = &emberAfPluginInterpanFragmentTransmitEventControl;
  }
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    MEMSET(rxPackets[i].sourceEui, 0, EUI64_SIZE);
    rxPackets[i].eventControl = &emberAfPluginInterpanFragmentReceiveEventControl;
  }
#endif //EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION
}

// This function sets the global state of inter-PAN. If state is true, inter-PAN
// messages are sent and received, else, all inter-PANs are dropped.
void emAfPluginInterpanSetEnableState(bool enable)
{
  if (gInterpanEnabled != enable) {
    gInterpanEnabled = enable;
    interpanPluginSetMacMatchFilterEnable(enable);
  }
}

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
// This is a helper function for testing purposes only. It changes the inter-PAN
// fragment message timeout
void interpanPluginSetFragmentMessageTimeout(uint16_t timeout)  // seconds
{
  uint16_t timeoutMs = timeout * 1000;
  if (gMessageTimeout != timeoutMs) {
    gMessageTimeout = timeoutMs;
    emberAfAppPrintln("IPMF message timeout changed to %d seconds", timeout);
  }
}
#endif //EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

#if defined(ENCRYPT_DEBUG)

static void printData(const char * name, uint8_t* message, uint8_t length)
{
  uint8_t i;
  emberAfAppPrint("\n%p: ", name);
  for (i = 0; i < length; i++) {
    emberAfAppPrint("%X ", message[i]);
    emberAfAppFlush();
  }
  emberAfAppPrintln("");
  emberAfAppFlush();
}

#endif

static uint8_t* pushInt16u(uint8_t *finger, uint16_t value)
{
  *finger++ = LOW_BYTE(value);
  *finger++ = HIGH_BYTE(value);
  return finger;
}

static uint8_t* pushEui64(uint8_t *finger, uint8_t *value)
{
  MEMMOVE(finger, value, 8);
  return finger + 8;
}

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES)

static EmberStatus handleApsSecurity(bool encrypt,
                                     uint8_t* apsFrame,
                                     uint8_t apsHeaderLength,
                                     uint8_t* messageLength,
                                     uint8_t maxLengthForEncryption,
                                     EmberAfInterpanHeader* header)
{
  uint8_t keyIndex;
  EmberStatus status;

  if (!(header->options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS)) {
    // We won't encrypt/decrypt messages that don't have a long address.
    return EMBER_APS_ENCRYPTION_ERROR;
  }
  keyIndex = emberFindKeyTableEntry(header->longAddress,
                                    true);  // link key? (vs. master key)
  if (keyIndex == 0xFF) {
    return EMBER_APS_ENCRYPTION_ERROR;
  }

  if (encrypt
      && ((maxLengthForEncryption - *messageLength)
          < INTERPAN_APS_ENCRYPTION_OVERHEAD)) {
    return EMBER_APS_ENCRYPTION_ERROR;
  }

  apsFrame[0] |= INTERPAN_APS_FRAME_SECURITY;

  status = emAfInterpanApsCryptMessage(encrypt,
                                       apsFrame,
                                       messageLength,
                                       apsHeaderLength,
                                       header->longAddress);
  return status;
}
#endif

static EmberStatus makeInterPanMessage(EmberAfInterpanHeader *headerData,
                                       uint8_t *message,
                                       uint16_t maxLength,
                                       uint8_t *payload,
                                       uint16_t *payloadLength,
                                       uint16_t *returnLength)
{
  uint8_t *finger = message;
  bool haveLongAddress = (headerData->options
                          & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS);
  uint16_t macFrameControl = (haveLongAddress
                              ? LONG_DEST_FRAME_CONTROL
                              : SHORT_DEST_FRAME_CONTROL);

  EmberEUI64 eui;
  uint8_t* apsFrame;

  uint8_t headerSize = (EMBER_AF_INTER_PAN_UNICAST == headerData->messageType)
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;

  if (maxLength < (headerSize + *payloadLength)) {
    emberAfAppPrint("Error: Inter-pan message too big (%d + %d > %d)",
                    headerSize, *payloadLength,
                    maxLength);
    return EMBER_MESSAGE_TOO_LONG;
  }

  if (headerData->messageType == EMBER_AF_INTER_PAN_UNICAST) {
    macFrameControl |= MAC_ACK_REQUIRED;
  }

  emberAfGetEui64(eui);

  finger = pushInt16u(finger, macFrameControl);
  finger++;  // Skip Sequence number, stack sets the sequence number.

  finger = pushInt16u(finger, headerData->panId);
  if (headerData->options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    finger = pushEui64(finger, headerData->longAddress);
  } else {
    finger = pushInt16u(finger, headerData->shortAddress);
  }

  finger = pushInt16u(finger, emberAfGetPanId());
  finger = pushEui64(finger, eui);

  finger = pushInt16u(finger, STUB_NWK_FRAME_CONTROL);

  apsFrame = finger;
  *finger++ = (headerData->messageType
               | INTERPAN_APS_FRAME_TYPE);

  if (headerData->messageType == EMBER_AF_INTER_PAN_MULTICAST) {
    finger = pushInt16u(finger, headerData->groupId);
  }

  finger = pushInt16u(finger, headerData->clusterId);
  finger = pushInt16u(finger, headerData->profileId);

  uint8_t UNUSED apsHeaderLength = finger - apsFrame;

  MEMMOVE(finger, payload, *payloadLength);
  finger += *payloadLength;

  if (headerData->options & EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT) {
    EmberStatus status;
    if (!APS_ENCRYPTION_ALLOWED) {
      return EMBER_SECURITY_CONFIGURATION_INVALID;
    }
    uint8_t apsEncryptLength = finger - apsFrame;

    printData("Before Encryption", apsFrame, apsEncryptLength);

    status = handleApsSecurity(true,  // encrypt?
                               apsFrame,
                               apsHeaderLength,
                               &apsEncryptLength,
                               maxLength - (uint8_t)(apsFrame - message),
                               headerData);
    if (status != EMBER_SUCCESS) {
      return status;
    }

    printData("After Encryption ", apsFrame, apsEncryptLength);

    finger = apsFrame + apsEncryptLength;
  }

  *returnLength = finger - message;

  return EMBER_SUCCESS;
}

static uint8_t parseInterpanMessage(uint8_t *message,
                                    uint8_t *messageLength,
                                    EmberAfInterpanHeader *headerData)
{
  uint8_t *finger = message;
  uint16_t macFrameControl;
  uint8_t remainingLength;
  uint8_t apsFrameControl;
  uint8_t apsHeaderIndex;

  MEMSET(headerData, 0, sizeof(EmberAfInterpanHeader));

  // We rely on the stack to insure that the MAC frame is formatted
  // correctly and that the length is at least long enough
  // to contain that frame.

  macFrameControl = HIGH_LOW_TO_INT(finger[1], finger[0])
                    & ~(MAC_ACK_REQUIRED);

  if (macFrameControl == LONG_DEST_FRAME_CONTROL) {
    // control, sequence, dest PAN ID, long dest
    finger += 2 + 1 + 2 + 8;
  } else if (macFrameControl == SHORT_DEST_FRAME_CONTROL) {
    // control, sequence, dest PAN ID, short dest
    finger += 2 + 1 + 2 + 2;
  } else {
    return 0;
  }

  // Source PAN ID
  headerData->panId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

  // It is expected that the long Source Address is always present and
  // that the stack MAC filters insured that to be the case.
  headerData->options |= EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS;
  MEMMOVE(headerData->longAddress, finger, 8);
  finger += 8;

  // Now that we know the correct MAC length, verify the interpan
  // frame is the correct length.

  remainingLength = *messageLength - (uint8_t)(finger - message);

  if (remainingLength < (STUB_NWK_SIZE + MIN_STUB_APS_SIZE)) {
    return 0;
  }

  if (HIGH_LOW_TO_INT(finger[1], finger[0]) != STUB_NWK_FRAME_CONTROL) {
    return 0;
  }
  finger += 2;
  apsHeaderIndex = (finger - message);
  remainingLength -= 2;

  apsFrameControl = (*finger++);

  if ((apsFrameControl & ~(INTERPAN_APS_FRAME_DELIVERY_MODE_MASK)
       &~INTERPAN_APS_FRAME_SECURITY)
      != INTERPAN_APS_FRAME_CONTROL_NO_DELIVERY_MODE) {
    emberAfAppPrintln("%pBad APS frame control 0x%X",
                      "ERR: Inter-PAN ",
                      apsFrameControl);
    return 0;
  }
  headerData->messageType = (apsFrameControl
                             & INTERPAN_APS_FRAME_DELIVERY_MODE_MASK);

  switch (headerData->messageType) {
    case EMBER_AF_INTER_PAN_UNICAST:
    case EMBER_AF_INTER_PAN_BROADCAST:
      // Broadcast and unicast have the same size messages
      if (remainingLength < INTERPAN_APS_UNICAST_SIZE) {
        return 0;
      }
      break;
    case EMBER_AF_INTER_PAN_MULTICAST:
      if (remainingLength < INTERPAN_APS_MULTICAST_SIZE) {
        return 0;
      }
      headerData->groupId = HIGH_LOW_TO_INT(finger[1], finger[0]);
      finger += 2;
      break;
    default:
      emberAfAppPrintln("%pBad Delivery Mode 0x%X",
                        "ERR: Inter-PAN ",
                        headerData->messageType);
      return 0;
  }

  headerData->clusterId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;
  headerData->profileId = HIGH_LOW_TO_INT(finger[1], finger[0]);
  finger += 2;

  if (apsFrameControl & INTERPAN_APS_FRAME_SECURITY) {
    EmberStatus status;
    uint8_t apsEncryptLength = *messageLength - apsHeaderIndex;
    uint8_t UNUSED apsHeaderLength = (uint8_t)(finger - message) - apsHeaderIndex;
    headerData->options |= EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT;

    printData("Before Decryption",
              message + apsHeaderIndex,
              apsEncryptLength);

    status = handleApsSecurity(false,   // encrypt?
                               message + apsHeaderIndex,
                               apsHeaderLength,
                               &apsEncryptLength,
                               0,       // maxLengthForEncryption (ignored)
                               headerData);

    if (status != EMBER_SUCCESS) {
      emberAfAppPrintln("%pAPS decryption failed (0x%X).",
                        "ERR: Inter-PAN ",
                        status);
      return 0;
    }

    printData("After Decryption ",
              message + apsHeaderIndex,
              apsEncryptLength);

    *messageLength = apsHeaderIndex + apsEncryptLength;
  }

  return (finger - message);
}

bool emAfPluginInterpanProcessMessage(uint8_t messageLength,
                                      uint8_t *messageContents)
{
  EmberApsFrame apsFrame;
  EmberIncomingMessageType type;
  bool loopback;
  EmberAfInterpanHeader headerData;
  uint8_t payloadOffset, payloadLength;
  uint8_t* payload;

  emberAfDebugPrintln("emAfPluginInterpanProcessMessage");
  payloadOffset = parseInterpanMessage(messageContents,
                                       &messageLength,
                                       &headerData);
  if (payloadOffset == 0) {
    return false;
  }
  printMessage(&headerData);

  payload = messageContents + payloadOffset;
  payloadLength = messageLength - payloadOffset;

  if (emberAfPluginInterpanPreMessageReceivedCallback(&headerData,
                                                      payloadLength,
                                                      payload)
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT
      || emAfPluginZllCommissioningClientInterpanPreMessageReceivedCallback(&headerData,
                                                                            payloadLength,
                                                                            payload)
#endif
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER
      || emAfPluginZllCommissioningServerInterpanPreMessageReceivedCallback(&headerData,
                                                                            payloadLength,
                                                                            payload)
#endif
      ) {
    return true;
  }
#if defined(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_CLIENT) || defined(EMBER_AF_PLUGIN_ZLL_COMMISSIONING_SERVER)
  else if (headerData.profileId == EMBER_ZLL_PROFILE_ID
           && headerData.clusterId == ZCL_ZLL_COMMISSIONING_CLUSTER_ID) {
    // Drop any other ZLL commissioning interpans, if they get this far.
    emberAfDebugPrintln("Interpan - dropping unhandled interpans");
    return true;
  }
#endif

  if (!isMessageAllowed(&headerData, payloadLength, payload)) {
    return false;
  }

  if (headerData.options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    headerData.shortAddress = EMBER_NULL_NODE_ID;
    loopback = emberIsLocalEui64(headerData.longAddress);
  } else {
    loopback = (emberAfGetNodeId() == headerData.shortAddress);
  }

  apsFrame.profileId = headerData.profileId;
  apsFrame.clusterId = headerData.clusterId;
  apsFrame.sourceEndpoint = 1;   // arbitrary since not sent over-the-air
  apsFrame.destinationEndpoint = ENDPOINT;
  apsFrame.options = (EMBER_APS_OPTION_NONE
                      | ((headerData.options
                          & EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT)
                         ? EMBER_APS_OPTION_ENCRYPTION
                         : 0));
  apsFrame.groupId = headerData.groupId;
  apsFrame.sequence = 0x00; // unknown sequence number

  switch (headerData.messageType) {
    case EMBER_AF_INTER_PAN_UNICAST:
      type = EMBER_INCOMING_UNICAST;
      break;
    case EMBER_AF_INTER_PAN_BROADCAST:
      type = (loopback
              ? EMBER_INCOMING_BROADCAST_LOOPBACK
              : EMBER_INCOMING_BROADCAST);
      break;
    case EMBER_AF_INTER_PAN_MULTICAST:
      type = (loopback
              ? EMBER_INCOMING_MULTICAST_LOOPBACK
              : EMBER_INCOMING_MULTICAST);
      break;
    default:
      return false;
  }

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  // If interpan fragment, intercept
  if (SE_PROFILE_ID == headerData.profileId) {
    if (isInterpanFragment(payload, payloadLength)) {
      uint8_t fragNum = payload[INTERPAN_FRAGMENTATION_APS_INDEX_IPMF_INDEX];

      switch (payload[INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_INDEX]) {
        case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL:
        {
          EmberStatus status = interpanFragmentationProcessIpmf(headerData,
                                                                messageContents,
                                                                messageLength);
          interpanFragmentationSendIpmfResponse(headerData,
                                                fragNum,
                                                EMBER_SUCCESS == status
                                                ? INTERPAN_IPMF_RESPONSE_SUCCESS
                                                : INTERPAN_IPMF_RESPONSE_FAILURE);
        }
        break;
        case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL:
        {
          // This function will also send the next fragment if response is OK
          interpanFragmentationProcessIpmfResponse(headerData,
                                                   payload,
                                                   payloadLength);
        }
        break;
        default:
          break;
      }

      // All inter-PAN fragments are consumed
      return true;
    }
  }
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

  return emberAfProcessMessage(&apsFrame,
                               type,
                               payload,
                               payloadLength,
                               headerData.shortAddress,
                               &headerData);
}

static bool isMessageAllowed(EmberAfInterpanHeader *headerData,
                             uint8_t messageLength,
                             uint8_t *messageContents)
{
  uint8_t incomingMessageOptions = 0;
  uint8_t commandId;
  uint8_t i;

  if (messageLength < EMBER_AF_ZCL_OVERHEAD) {
    emberAfAppPrintln("%pmessage too short (%d < %d)!",
                      "ERR: Inter-PAN ",
                      messageLength,
                      EMBER_AF_ZCL_OVERHEAD);
    return false;
  }

  if (headerData->options & EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT) {
    return APS_ENCRYPTION_ALLOWED;
  }

  // Only the first bit is used for ZCL Frame type
  if (messageContents[0] & BIT(1)) {
    emberAfAppPrintln("%pUnsupported ZCL frame type.",
                      "ERR: Inter-PAN ");
    return false;
  }

  if (messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    incomingMessageOptions |= EMBER_AF_INTERPAN_MANUFACTURER_SPECIFIC;
  }
  if ((messageContents[0] & ZCL_CLUSTER_SPECIFIC_COMMAND)
      == ZCL_GLOBAL_COMMAND) {
    incomingMessageOptions |= EMBER_AF_INTERPAN_GLOBAL_COMMAND;
  }

  incomingMessageOptions |= ((messageContents[0] & ZCL_FRAME_CONTROL_DIRECTION_MASK)
                             ? EMBER_AF_INTERPAN_DIRECTION_SERVER_TO_CLIENT
                             : EMBER_AF_INTERPAN_DIRECTION_CLIENT_TO_SERVER);
  if (messageContents[0] & ZCL_MANUFACTURER_SPECIFIC_MASK) {
    if (messageLength < EMBER_AF_ZCL_MANUFACTURER_SPECIFIC_OVERHEAD) {
      emberAfAppPrintln("%pmessage too short!", "ERR: Inter-PAN ");
      return false;
    }
    commandId = messageContents[4];
  } else {
    commandId = messageContents[2];
  }

#if defined (EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  // If interpan fragmentation is enabled, we allow all messages of
  // SE profile ID, appropriate control byte value, and appropriate lengths
  if ((headerData->profileId == SE_PROFILE_ID)
      && isInterpanFragment(messageContents, messageLength)) {
    return true;
  }
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

  i = 0;
  while (messages[i].profileId != 0xFFFF) {
    if (messages[i].profileId == headerData->profileId
        && messages[i].clusterId == headerData->clusterId
        && messages[i].commandId == commandId
        // Here we only check if the direction is allowed
        && (messages[i].options & incomingMessageOptions)) {
      return true;
    }
    i++;
  }

  emberAfAppPrintln("%pprofile 0x%2x, cluster 0x%2x, command 0x%x not permitted",
                    "ERR: Inter-PAN ",
                    headerData->profileId,
                    headerData->clusterId,
                    commandId);
  return false;
}

#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)

static void printMessage(EmberAfInterpanHeader *headerData)
{
  emberAfAppPrint("RX inter-PAN message (");
  if (headerData->messageType == EMBER_AF_INTER_PAN_UNICAST) {
    emberAfAppPrint("uni");
  } else if (headerData->messageType == EMBER_AF_INTER_PAN_BROADCAST) {
    emberAfAppPrint("broad");
  } else if (headerData->messageType == EMBER_AF_INTER_PAN_MULTICAST) {
    emberAfAppPrint("multi");
  }
  emberAfAppPrintln("cast):");
  emberAfAppFlush();
  emberAfAppPrintln("  src pan id: 0x%2x", headerData->panId);
  if (headerData->options & EMBER_AF_INTERPAN_OPTION_MAC_HAS_LONG_ADDRESS) {
    emberAfAppPrint("  src long id: ");
    emberAfPrintBigEndianEui64(headerData->longAddress);
    emberAfAppPrintln("");
  } else {
    emberAfAppPrintln("  src short id: 0x%2x", headerData->shortAddress);
  }
  emberAfAppFlush();
  emberAfAppPrintln("  profile id: 0x%2x", headerData->profileId);
  emberAfAppPrint("  cluster id: 0x%2x ", headerData->clusterId);
  emberAfAppDebugExec(emberAfDecodeAndPrintClusterWithMfgCode(headerData->clusterId, emberAfGetMfgCodeFromCurrentCommand()));
  emberAfAppPrintln("");
  if (headerData->messageType == EMBER_AF_INTER_PAN_MULTICAST) {
    emberAfAppPrintln("  group id: 0x%2x", headerData->groupId);
  }
  emberAfAppFlush();
}

#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_APP)

EmberStatus emberAfInterpanSendMessageCallback(EmberAfInterpanHeader* header,
                                               uint16_t messageLength,
                                               uint8_t* messageBytes)
{
  EmberStatus status;
  uint8_t message[EMBER_AF_MAXIMUM_INTERPAN_LENGTH];
  uint16_t maxLen;
  uint8_t headerSize = EMBER_AF_INTER_PAN_UNICAST == header->messageType
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;

  if (!gInterpanEnabled) {
    return EMBER_INVALID_CALL;
  }

  if ((headerSize + messageLength) <= EMBER_AF_MAXIMUM_INTERPAN_LENGTH) {
    status = makeInterPanMessage(header,
                                 message,
                                 EMBER_AF_MAXIMUM_INTERPAN_LENGTH,
                                 messageBytes,
                                 &messageLength,
                                 &maxLen);
    if (status == EMBER_SUCCESS) {
      status = emAfPluginInterpanSendRawMessage((uint8_t)maxLen, message);
    }
  }
#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  else if ((headerSize + messageLength)
           <= EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE) {
    if (EMBER_AF_INTER_PAN_UNICAST != header->messageType) {
      return EMBER_INVALID_CALL;
    }

    status = interpanFragmentationSendUnicast(header,
                                              messageBytes,
                                              messageLength);
  }
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION
  else {
    return EMBER_MESSAGE_TOO_LONG;
  }

  if (status != EMBER_SUCCESS) {
    return status;
  }

  emberAfAppPrint("T%4x:Inter-PAN TX (%d) [",
                  emberAfGetCurrentTime(),
                  messageLength);
  emberAfAppPrintBuffer(messageBytes, messageLength, true);
  emberAfAppPrintln("], 0x%x", status);
  emberAfAppFlush();

  return status;
}

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)

// This function checks if packet is an inter-PAN fragment. It must do length
// checks as well since ZCL default responses come in with APS payload byte 1
// 0x00, which is also an inter-PAN IPMF control byte value.
bool isInterpanFragment(uint8_t* payload, uint8_t payloadLen)
{
  if (!payload || (payloadLen < EMBER_APS_INTERPAN_FRAGMENT_MIN_LEN)) {
    return false;
  }

  switch (payload[INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_INDEX]) {
    case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL:
    {
      if ((payloadLen >= EMBER_APS_INTERPAN_FRAGMENTATION_OVERHEAD)
          && (payload[INTERPAN_FRAGMENTATION_APS_LEN_IPMF_INDEX]
              == (payloadLen - EMBER_APS_INTERPAN_FRAGMENTATION_OVERHEAD))) {
        return true;
      }
    }
    break;
    case INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL:
    {
      if (EMBER_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN == payloadLen) {
        return true;
      }
    }
    break;
    default:
      break;
  }

  return false;
}

// Helper function to find a free buffer used to store a message sent via frags
static txFragmentedInterpanPacket* getFreeTxPacketEntry(EmberEUI64 destEui)
{
  uint8_t i;

  if (!destEui) {
    return NULL;
  }

  // Interpan fragmentation cannot handle sending mutliple fragmented
  // transmissions to the same partner due to the lack of APS ACKS with sequence
  // numbers. Ensure that we're not in the middle of sending to partner already
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedInterpanPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType != UNUSED_TX_PACKET_ENTRY) {
      if (!MEMCOMPARE(txPacket->destEui, destEui, EUI64_SIZE)) {
        return NULL;
      }
    }
  }

  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedInterpanPacket *txPacket = &(txPackets[i]);
    if (txPacket->messageType == UNUSED_TX_PACKET_ENTRY) {
      MEMCOPY(txPacket->destEui, destEui, EUI64_SIZE);
      return txPacket;
    }
  }
  return NULL;
}

// Helper function to free a buffer used to store a message sent via frags
static void freeTxPacketEntry(txFragmentedInterpanPacket *txPacket, EInterpanFragmentationStatus reason)
{
  if (!txPacket) {
    return;
  }
  txPacket->messageType = UNUSED_TX_PACKET_ENTRY;

  if (IPMF_SUCCESS != reason) {
    // User callback: something went wrong
    emberAfPluginInterpanFragmentTransmissionFailedCallback(reason,
                                                            txPacket->fragmentNum);
  }
}

// Helper function to find an existing buffer used to store a msg sent via frags
static txFragmentedInterpanPacket* txPacketLookUp(EmberEUI64 destEui)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedInterpanPacket *txPacket = &(txPackets[i]);
    if (!MEMCOMPARE(txPacket->destEui, destEui, EUI64_SIZE)) {
      return txPacket;
    }
  }
  return NULL;
}

// Helper function to find a free buffer used to store a message rcvd via frags
static rxFragmentedInterpanPacket* getFreeRxPacketEntry(EmberEUI64 sourceEui)
{
  uint8_t i;
  EmberEUI64 nullEui = { 0 };
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedInterpanPacket *rxPacket = &(rxPackets[i]);
    if (!MEMCOMPARE(rxPacket->sourceEui, nullEui, EUI64_SIZE)) {
      MEMCOPY(rxPacket->sourceEui, sourceEui, EUI64_SIZE);
      return rxPacket;
    }
  }
  return NULL;
}

// Helper function to free a buffer used to store a message received via frags
static void freeRxPacketEntry(rxFragmentedInterpanPacket *rxPacket, EInterpanFragmentationStatus reason)
{
  if (!rxPacket) {
    return;
  }
  MEMSET(rxPacket->sourceEui, 0, EUI64_SIZE);

  if (IPMF_SUCCESS != reason) {
    // User callback: something went wrong
    emberAfPluginInterpanFragmentTransmissionFailedCallback(reason,
                                                            rxPacket->lastFragmentNumReceived + 1);
  }
}

// Helper function to find an existing buffer used to store a msg rcvd via frags
static rxFragmentedInterpanPacket* rxPacketLookUp(EmberEUI64 sourceEui)
{
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedInterpanPacket *rxPacket = &(rxPackets[i]);
    if (!MEMCOMPARE(rxPacket->sourceEui, sourceEui, EUI64_SIZE)) {
      return rxPacket;
    }
  }
  return NULL;
}

// This function sends a whole message. It takes care of the fragmenting.
EmberStatus interpanFragmentationSendUnicast(EmberAfInterpanHeader* header,
                                             uint8_t* message,
                                             uint16_t messageLen)
{
  txFragmentedInterpanPacket *txPacket;
  EmberStatus status;
  uint8_t headerLen = INTERPAN_UNICAST_HEADER_SIZE;
  uint8_t maxFragLen, numFragments;

  if (!header || !message || (0 == messageLen)) {
    return EMBER_BAD_ARGUMENT;
  }

  if (messageLen > EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  // Claim a tx packet buffer. Only 1 tx packet per partner.
  txPacket = getFreeTxPacketEntry(header->longAddress);
  if (txPacket == NULL) {
    return EMBER_MAX_MESSAGE_LIMIT_REACHED;
  }

  // No interpan frag APS encryption allowed (payload assumed already encrypted)
  header->options &= ~EMBER_AF_INTERPAN_OPTION_APS_ENCRYPT;

  // Build a big packet
  status = makeInterPanMessage(header,
                               txPacket->buffer,
                               EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_BUFFER_SIZE,
                               message,
                               &messageLen,
                               &(txPacket->bufLen));
  if (EMBER_SUCCESS != status) {
    return status;
  }

  maxFragLen = EMBER_AF_MAXIMUM_INTERPAN_LENGTH
               - headerLen
               - EMBER_APS_INTERPAN_FRAGMENTATION_OVERHEAD;

  numFragments = messageLen / maxFragLen;
  if (messageLen % maxFragLen) {
    numFragments++;
  }
  if (numFragments > MAX_INT8U_VALUE) {
    return EMBER_MESSAGE_TOO_LONG;
  }

  // Fill in some info before we start the transmit process
  txPacket->messageType    = header->messageType;
  txPacket->fragmentMaxLen = maxFragLen;
  txPacket->numFragments   = numFragments;
  txPacket->fragmentNum    = 0;

  // Forward it to the frag-sending function
  status = interpanFragmentationSendIpmf(txPacket);

  return status;
}

// This function sends an individual fragment. It expects the caller to handle
// the setting up and handling of txPacket metadata.
EmberStatus interpanFragmentationSendIpmf(txFragmentedInterpanPacket *txPacket)
{
  if (!txPacket || !txPacket->bufLen) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberStatus status;
  uint8_t  message[EMBER_AF_MAXIMUM_INTERPAN_LENGTH];
  uint8_t* finger = message;
  uint8_t  headerLen = EMBER_AF_INTER_PAN_UNICAST == txPacket->messageType
                       ? INTERPAN_UNICAST_HEADER_SIZE
                       : INTERPAN_MULTICAST_HEADER_SIZE;
  uint16_t messageSentLen, messageLeftLen;
  uint8_t  messageToBeSentLen, packetLen;

  // Copy header info from pre-built packet
  MEMMOVE(finger, txPacket->buffer, headerLen);
  finger += headerLen;

  // Write the IPMF payload with a fragment of the whole message
  messageSentLen = txPacket->fragmentMaxLen * txPacket->fragmentNum;
  messageLeftLen = txPacket->bufLen - headerLen - messageSentLen;
  messageToBeSentLen =  messageLeftLen >= txPacket->fragmentMaxLen
                       ? txPacket->fragmentMaxLen : messageLeftLen;

  *finger++ = INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_VAL;
  *finger++ = txPacket->fragmentNum;
  *finger++ = txPacket->numFragments;
  *finger++ = messageToBeSentLen;
  MEMMOVE(finger,
          (txPacket->buffer + headerLen + messageSentLen),
          messageToBeSentLen);
  finger += messageToBeSentLen;

  packetLen = finger - message;

  emberAfAppPrintln("Sending interpan fragment %d of %d",
                    txPacket->fragmentNum, txPacket->numFragments - 1);

  status = emAfPluginInterpanSendRawMessage(packetLen, message);

  // Start a timer waiting for the IPMF response
  emberEventControlSetDelayMS(*(txPacket->eventControl), gMessageTimeout);

  return status;
}

// This function reads an IPMF message and stores necessary data. The caller
// will need to send an IPMF response based on the return from this function.
EmberStatus interpanFragmentationProcessIpmf(EmberAfInterpanHeader header,
                                             uint8_t *message, uint8_t messageLen)
{
  uint8_t* finger;
  uint8_t fragNum, numFrags, fragLen, headerLen = INTERPAN_UNICAST_HEADER_SIZE;
  rxFragmentedInterpanPacket *rxPacket;

  if (!message || !messageLen) {
    return EMBER_BAD_ARGUMENT;
  }

  finger = message + headerLen;
  finger++;   // Skip the control byte
  fragNum  = *finger++;
  numFrags = *finger++;
  fragLen  = *finger++;

  emberAfAppPrintln("Receiving inter-PAN fragment %d of %d", fragNum, numFrags - 1);

  // See if we already have an entry for this fragment transmission
  rxPacket = rxPacketLookUp(header.longAddress);
  if (!rxPacket && (0 != fragNum)) {
    emberAfAppPrintln("ERR: expected first IPMF index 0, got %d", fragNum);
    return EMBER_INDEX_OUT_OF_RANGE;
  } else if (rxPacket && (0 == fragNum)) {
    emberAfAppPrint("ERR: ");
    emberAfPrintLittleEndianEui64(header.longAddress);
    emberAfAppPrintln(" sending multiple IPMF index 0 fragments");
    freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  // If no entry, see if we have room to take a buffer
  if (NULL == rxPacket) {
    rxPacket = getFreeRxPacketEntry(header.longAddress);
    if (rxPacket) {
      // If we've allocated a new buffer, copy the header over and set data
      MEMMOVE(rxPacket->buffer, message, headerLen);
      rxPacket->bufLen                  = headerLen;
      rxPacket->numFragments            = numFrags;
      rxPacket->lastFragmentNumReceived = fragNum;
    }
  }

  if (NULL == rxPacket) {
    emberAfAppPrint("ERR: RX interpan fragment from ");
    emberAfPrintLittleEndianEui64(header.longAddress);
    emberAfAppPrintln(" but no available buffers");
    return EMBER_MAX_MESSAGE_LIMIT_REACHED;
  }

  // We received an IPMF, so turn off the timer for now
  emberEventControlSetInactive(*(rxPacket->eventControl));

  if (fragLen == 0) {
    emberAfAppPrintln("ERR: RX interpan fragment from ");
    emberAfPrintLittleEndianEui64(header.longAddress);
    emberAfAppPrintln(" with IPMF length 0");
    freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
    return EMBER_INDEX_OUT_OF_RANGE;
  }

  if (0 != fragNum) {
    if (rxPacket->lastFragmentNumReceived != (fragNum - 1)) {
      emberAfAppPrintln("ERR: RX IPMF fragment %d after %d, expecting %d",
                        fragNum, rxPacket->lastFragmentNumReceived,
                        rxPacket->lastFragmentNumReceived + 1);
      freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
      return EMBER_INDEX_OUT_OF_RANGE;
    }
    if (rxPacket->numFragments != numFrags) {
      emberAfAppPrintln("ERR: expecting total number of IPMFs %d, but received"
                        " %d in IPMF fragment %d",
                        rxPacket->numFragments, numFrags, fragNum);
      freeRxPacketEntry(rxPacket, IPMF_RX_BAD_RESPONSE);
      return EMBER_INDEX_OUT_OF_RANGE;
    }
    // Note that we allow sequential fragments to specify different lengths.
    // In theory, frags 0 to n-1 should be of the same maximum length, and the
    // last fragment n should be of shorter length. For now, we'll allow that
    // any of those middle packets may come in with a different length.
  }

  // Strip the payload and add it to the whole message buffer
  MEMMOVE(rxPacket->buffer + rxPacket->bufLen, finger, fragLen);
  rxPacket->bufLen += fragLen;
  rxPacket->lastFragmentNumReceived = fragNum;

  if (rxPacket->lastFragmentNumReceived == (rxPacket->numFragments - 1)) {
    // Full message received
    emberAfAppPrint("T%4x:Inter-PAN RX (%d B, %d fragments) [",
                    emberAfGetCurrentTime(),
                    rxPacket->bufLen - headerLen,
                    rxPacket->numFragments);
    emberAfAppPrintBuffer(rxPacket->buffer + headerLen,
                          rxPacket->bufLen - headerLen,
                          true);
    emberAfAppPrintln("], 0x%x", EMBER_SUCCESS);
    emberAfAppFlush();

    // User callback: all frags received, message reconstructed
    emberAfPluginInterpanMessageReceivedOverFragmentsCallback(
      &header,
      rxPacket->bufLen - headerLen,
      rxPacket->buffer + headerLen);

    freeRxPacketEntry(rxPacket, IPMF_SUCCESS);
  }

  return EMBER_SUCCESS;
}

// This function sends an IPMF response to the sender of the IPMF
void interpanFragmentationSendIpmfResponse(EmberAfInterpanHeader header,
                                           uint8_t fragNum, uint8_t response)
{
  EmberStatus status;
  uint8_t message[EMBER_AF_MAXIMUM_INTERPAN_LENGTH];
  uint8_t* finger;
  uint16_t messageLength, noPayloadLen = 0;
  uint8_t noPayload = 0;

  if ((fragNum > MAX_INT8U_VALUE)
      || ((INTERPAN_IPMF_RESPONSE_SUCCESS != response)
          && (INTERPAN_IPMF_RESPONSE_FAILURE != response))) {
    return;
  }

  // Construct a new IPMF message, but no payload
  status = makeInterPanMessage(&header,
                               message,
                               EMBER_AF_MAXIMUM_INTERPAN_LENGTH,
                               &noPayload,
                               &noPayloadLen,
                               &messageLength);
  if (EMBER_SUCCESS != status) {
    return;
  }

  // Move past the header and write the IPMF response payload
  finger = message + messageLength;
  *finger++ = INTERPAN_FRAGMENTATION_APS_CONTROL_BYTE_IPMF_RESPONSE_VAL;
  *finger++ = fragNum;
  *finger++ = response;

  messageLength += EMBER_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN;

  // Send the response
  emAfPluginInterpanSendRawMessage(messageLength, message);

  // Start a timer waiting for the next IPMF
  rxFragmentedInterpanPacket* rxPacket = rxPacketLookUp(header.longAddress);
  if (rxPacket) {
    emberEventControlSetDelayMS(*(rxPacket->eventControl), gMessageTimeout);
  }
}

// This function processes the IPMF response and either sends the next fragment
// or finishes the transmission
void interpanFragmentationProcessIpmfResponse(EmberAfInterpanHeader header,
                                              uint8_t* payload,
                                              uint8_t payloadLen)
{
  if (!payload
      || (EMBER_APS_INTERPAN_FRAGMENTATION_RESPONSE_LEN != payloadLen)) {
    return;
  }

  uint8_t fragmentNumAcked = payload[INTERPAN_FRAGMENTATION_APS_INDEX_IPMF_INDEX];
  uint8_t response = payload[INTERPAN_FRAGMENTATION_APS_IPMF_RESPONSE_INDEX];

  txFragmentedInterpanPacket* txPacket = txPacketLookUp(header.longAddress);
  if (!txPacket) {
    return;
  }

  // We received an IPMF, so turn off the timer for now
  emberEventControlSetInactive(*(txPacket->eventControl));

  if (fragmentNumAcked != txPacket->fragmentNum) {
    // Partner is not acking what we've sent it - error
    freeTxPacketEntry(txPacket, IPMF_TX_BAD_RESPONSE);
    return;
  }

  if (INTERPAN_IPMF_RESPONSE_SUCCESS != response) {
    // Partner acking bad response - error
    freeTxPacketEntry(txPacket, IPMF_TX_BAD_RESPONSE);
    return;
  }

  if (txPacket->fragmentNum == (txPacket->numFragments - 1)) {
    // Done!
    freeTxPacketEntry(txPacket, IPMF_SUCCESS);
    return;
  }

  txPacket->fragmentNum++;
  interpanFragmentationSendIpmf(txPacket);
}

#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION

// This function is called when an inter-PAN fragment transmission fails to
// receive a response within the allotted time
void emberAfPluginInterpanFragmentTransmitEventHandler(EmberEventControl *control)
{
  emberEventControlSetInactive(*control);

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_OUTGOING_PACKETS; i++) {
    txFragmentedInterpanPacket *txPacket = &(txPackets[i]);
    if (txPacket->eventControl == control) {
      emberAfAppPrintln("ERR: TX inter-PAN: fragment %d of %d has timed out "
                        "waiting for a fragment response",
                        txPacket->fragmentNum, txPacket->numFragments - 1);
      freeTxPacketEntry(txPacket, IPMF_TX_TIMEOUT);
    }
  }
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION
}

// This function is called when an inter-PAN fragment reception fails to receive
// the next fragment within the allotted time
void emberAfPluginInterpanFragmentReceiveEventHandler(EmberEventControl *control)
{
  emberEventControlSetInactive(*control);

#if defined(EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION)
  uint8_t i;
  for (i = 0; i < EMBER_AF_PLUGIN_INTERPAN_FRAGMENTATION_MAX_INCOMING_PACKETS; i++) {
    rxFragmentedInterpanPacket *rxPacket = &(rxPackets[i]);
    if (rxPacket->eventControl == control) {
      emberAfAppPrintln("ERR: RX inter-PAN: fragment %d of %d has timed out "
                        "waiting for the next fragment",
                        rxPacket->lastFragmentNumReceived,
                        rxPacket->numFragments - 1);
      freeRxPacketEntry(rxPacket, IPMF_RX_TIMEOUT);
    }
  }
#endif // EMBER_AF_PLUGIN_INTERPAN_ALLOW_FRAGMENTATION
}
